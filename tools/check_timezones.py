#!/usr/bin/env python3
"""Check spiffs/timezone.txt against the IANA time zone database.

Each table string is evaluated the way the firmware's libc does it (a model of
picolibc 1.8.10 tzset, localtime_r and __tzcalc_limits), then compared with IANA
data for every instant from now until --days ahead. Only UTC offsets are compared.

Fails (exit 1) when a line shows the wrong offset, an IANA Area/City name (current
or old alias such as Asia/Calcutta) has no line, or a line is malformed, duplicated
or names an unknown zone. Names without an area (US/Pacific, GB) are not required.
The default horizon gives a release cycle's notice of the next rule change.

    python -m pip install -U tzdata     # the IANA data; update it before each run
    python tools/check_timezones.py [--days 180] [--table spiffs/timezone.txt]
"""
import argparse
import calendar
import datetime as dt
import pathlib
import re
import sys
import zoneinfo

try:
    import importlib.resources as resources

    import tzdata
except ImportError:
    sys.exit("Needs the IANA data: python -m pip install -U tzdata")

UTC = dt.timezone.utc
DAY = 86400
DEFAULT_TABLE = pathlib.Path(__file__).resolve().parent.parent / "spiffs" / "timezone.txt"
AREAS = {"Africa", "America", "Antarctica", "Arctic", "Asia", "Atlantic", "Australia", "Europe", "Indian", "Pacific"}
MONTH_DAYS = [[31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
              [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]]


def parse_hms(text):
    """Model of sscanf "%hu%n:%hu%n:%hu%n": a negative hour wraps to 65535-style values."""
    match = re.match(r"([-+]?)(\d+)(?::(\d+))?(?::(\d+))?", text)
    if not match:
        return None
    sign = -1 if match[1] == "-" else 1
    hours = (sign * int(match[2])) & 0xFFFF
    return hours * 3600 + int(match[3] or 0) * 60 + int(match[4] or 0), match.end()


def parse_name(text):
    """Return the length of a time zone abbreviation, or None when libc rejects it."""
    if text.startswith("<"):
        match = re.match(r"<([-+0-9A-Za-z]{1,11})>", text)
        if not match or not 3 <= len(match[1]) <= 10:
            return None
        return match.end()
    match = re.match(r"[A-Za-z]{1,11}", text)
    if not match or not 3 <= len(match[0]) <= 10:
        return None
    return match.end()


def parse_offset(text):
    sign = 1
    if text[:1] in ("-", "+"):
        sign = -1 if text[0] == "-" else 1
        text = text[1:]
    parsed = parse_hms(text)
    if not parsed:
        return None, text
    return sign * parsed[0], text[parsed[1]:]


def parse_tz(tz):
    """Model of _tzset_unlocked. Returns None where libc silently falls back to UTC."""
    length = parse_name(tz)
    if length is None:
        return None
    std_offset, rest = parse_offset(tz[length:])
    if std_offset is None:
        return None
    if not rest.startswith("<") and not re.match(r"[A-Za-z]", rest):
        return {"std": std_offset}  # libc stops here: no summer time
    length = parse_name(rest)
    if length is None:
        return None
    dst_offset, rest = parse_offset(rest[length:])
    if dst_offset is None:
        dst_offset = std_offset - 3600

    rules = []
    for index in range(2):
        rest = rest.removeprefix(",")
        if rest.startswith("M"):
            match = re.match(r"M(\d+)\.(\d+)\.(\d+)", rest)
            if not match:
                return None
            month, week, weekday = (int(group) for group in match.groups())
            if not (1 <= month <= 12 and 1 <= week <= 5 and weekday <= 6):
                return None
            date = ("M", month, week, weekday)
            rest = rest[match.end():]
        else:
            kind = "J" if rest.startswith("J") else "D"
            rest = rest.removeprefix("J")
            match = re.match(r"\d+", rest)
            if match:
                date = (kind, int(match[0]))
                rest = rest[match.end():]
            else:
                date = ("M", 3, 2, 0) if index == 0 else ("M", 11, 1, 0)
        seconds = 7200
        if rest.startswith("/"):
            parsed = parse_hms(rest[1:])
            if not parsed:
                return None
            seconds = parsed[0]
            rest = rest[1 + parsed[1]:]
        rules.append((date, seconds))
    return {"std": std_offset, "dst": dst_offset, "rules": rules}


def rule_change(date, seconds, offset, year):
    """Model of __tzcalc_limits: the UTC instant a rule fires in `year`."""
    days = (dt.date(year, 1, 1) - dt.date(1970, 1, 1)).days
    leap = calendar.isleap(year)
    if date[0] == "J":
        days += date[1] + (1 if leap and date[1] >= 60 else 0) - 1
    elif date[0] == "D":
        days += date[1]
    else:
        _, month, week, weekday = date
        lengths = MONTH_DAYS[leap]
        days += sum(lengths[:month - 1])
        month_day = (week - 1) * 7 + (weekday - (4 + days) % 7) % 7
        while month_day >= lengths[month - 1]:
            month_day -= 7
        days += month_day
    return days * DAY + seconds + offset


def table_offset(parsed, instant):
    """Model of localtime_r: seconds east of UTC at `instant`."""
    if parsed is None:
        return 0
    if parsed.get("dst", parsed["std"]) == parsed["std"]:
        return -parsed["std"]
    year = dt.datetime.fromtimestamp(instant, UTC).year
    start = rule_change(*parsed["rules"][0], parsed["std"], year)
    end = rule_change(*parsed["rules"][1], parsed["dst"], year)
    summer = start <= instant < end if start < end else instant >= start or instant < end
    return -(parsed["dst"] if summer else parsed["std"])


def changes(offset_at, start, stop):
    """Instants where an offset changes, assuming at most one change per day."""
    found = []
    instant = start
    while instant < stop:
        following = min(instant + DAY, stop)
        if offset_at(instant) != offset_at(following):
            low, high = instant, following
            while high - low > 1:
                middle = (low + high) // 2
                if offset_at(middle) == offset_at(low):
                    low = middle
                else:
                    high = middle
            found.append(high)
        instant = following
    return found


def first_wrong(tz, iana, start, stop):
    """First instant `tz` disagrees with `iana`: (instant, table hours, IANA hours), or None."""
    parsed = parse_tz(tz)
    table_at = lambda instant: table_offset(parsed, instant)
    iana_at = lambda instant: int(dt.datetime.fromtimestamp(instant, iana).utcoffset().total_seconds())
    for instant in sorted({start, *changes(table_at, start, stop), *changes(iana_at, start, stop)}):
        if table_at(instant) != iana_at(instant):
            return instant, table_at(instant) / 3600, iana_at(instant) / 3600
    return None


def zone_data(key):
    return resources.files("tzdata").joinpath("zoneinfo", *key.split("/"))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--days", type=int, default=180, help="how far ahead to compare (default 180)")
    parser.add_argument("--table", type=pathlib.Path, default=DEFAULT_TABLE)
    args = parser.parse_args()

    start = int(dt.datetime.now(UTC).timestamp())
    stop = start + args.days * DAY
    problems = []
    seen = set()
    lines = args.table.read_text(encoding="ascii").splitlines()

    for number, line in enumerate(lines, 1):
        location, separator, tz = line.partition(";")
        key = location.replace(" ", "_")
        if not separator or not tz or ";" in tz:
            problems.append(f"line {number}: malformed: {line!r}")
            continue
        if key.lower() in seen:
            problems.append(f"line {number}: duplicate zone {location}")
        seen.add(key.lower())
        if not zone_data(key).is_file():
            problems.append(f"line {number}: {location} is not an IANA zone")
            continue
        if parse_tz(tz) is None:
            problems.append(f"line {number}: {location}: libc cannot parse {tz!r}, device would use UTC")
            continue

        data = zone_data(key).read_bytes()
        iana = zoneinfo.ZoneInfo.from_file(zone_data(key).open("rb"))
        wrong = first_wrong(tz, iana, start, stop)
        if wrong:
            instant, table_hours, iana_hours = wrong
            footer = data.rstrip(b"\n").rsplit(b"\n", 1)[-1].decode()
            hint = f"IANA footer: {footer}"
            if footer == tz or first_wrong(footer, iana, start, stop):
                hint += " (does not fit this period; needs a hand-written string)"
            when = dt.datetime.fromtimestamp(instant, UTC).strftime("%Y-%m-%d %H:%M UTC")
            problems.append(f"line {number}: {location}: {tz} gives UTC{table_hours:+g} but IANA says "
                            f"UTC{iana_hours:+g} at {when}. {hint}")

    for key in resources.files("tzdata").joinpath("zones").read_text(encoding="ascii").split():
        if key.split("/")[0] in AREAS and "/" in key and key.lower() not in seen:
            footer = zone_data(key).read_bytes().rstrip(b"\n").rsplit(b"\n", 1)[-1].decode()
            problems.append(f"missing zone: {key.replace('_', ' ')};{footer}")

    print(f"{args.table.name}: {len(lines)} lines, IANA {tzdata.IANA_VERSION}, next {args.days} days")
    for problem in problems:
        print(problem)
    print(f"{len(problems)} problem(s)" if problems else "OK")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
