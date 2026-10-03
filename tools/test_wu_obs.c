/* Host check for the Weather Underground observation parser and display strings.
 * Compile this file together with cJSON. Does not touch met.no [temp]. */
#include <stdio.h>
#include <string.h>
#include <time.h>

#define WU_HOST_TEST 1
#include "../main/f-wu.c"

static int failures = 0;

static void expect_str(const char *name, const char *got, const char *want)
{
    if (strcmp(got, want) != 0)
    {
        fprintf(stderr, "FAIL %s: got \"%s\" want \"%s\"\n", name, got, want);
        failures++;
    }
}

static void expect_true(const char *name, int cond)
{
    if (!cond)
    {
        fprintf(stderr, "FAIL %s\n", name);
        failures++;
    }
}

static const char *CANNED =
    "{"
    "\"observations\":[{"
    "\"stationID\":\"KCASANFR123\","
    "\"epoch\":1700000000,"
    "\"humidity\":45,"
    "\"winddir\":315,"
    "\"uv\":6.2,"
    "\"metric\":{"
    "\"temp\":22.4,"
    "\"dewpt\":10.1,"
    "\"windSpeed\":18.0,"
    "\"windGust\":28.8,"
    "\"pressure\":1013.2,"
    "\"precipTotal\":1.5"
    "}}]}";

static const char *EXPIRED =
    "{\"errors\":[{\"error\":{\"message\":\"Data Expired\"}}]}";

static const char *NO_GUST =
    "{"
    "\"observations\":[{"
    "\"epoch\":1700000000,"
    "\"humidity\":40,"
    "\"winddir\":0,"
    "\"metric\":{\"temp\":10,\"windSpeed\":0,\"windGust\":null,\"precipTotal\":0}"
    "}]}";

int main(void)
{
    wu_obs_t parsed;
    memset(&parsed, 0, sizeof(parsed));
    parsed.temp_c = 1234; /* sentinel: a failed parse must leave this alone */
    expect_true("expired leaves obs unchanged", !wu_parse_current(EXPIRED, &parsed));
    expect_true("expired sentinel", parsed.temp_c == 1234 && !parsed.valid);

    expect_true("parse canned", wu_parse_current(CANNED, &parsed));
    wu_obs = parsed;
    expect_true("epoch", wu_obs.obs_epoch == (time_t)1700000000);
    expect_true("wind m/s", wu_obs.wind_mps > 4.99 && wu_obs.wind_mps < 5.01);
    expect_true("gust m/s", wu_obs.gust_mps > 7.99 && wu_obs.gust_mps < 8.01);

    char buf[32];
    const char *tokens[] = {
        "[wu:temp]", "[wu:hum]", "[wu:dew]", "[wu:wind]",
        "[wu:gust]", "[wu:pressure]", "[wu:rain]", "[wu:uv]"};
    const char *metric[] = {
        "22°C", "45%", "10°C", "5.0 m/s NW",
        "8.0 m/s NW", "1013 hPa", "1.5 mm", "6.2"};
    const char *imperial[] = {
        "72°F", "45%", "50°F", "11.2 mph NW",
        "17.9 mph NW", "29.92 inHg", "0.06 in.", "6.2"};

    for (int i = 0; i < 8; i++)
    {
        buf[0] = '\0';
        expect_true(tokens[i], wu_format_token(tokens[i], buf, sizeof(buf), false));
        expect_str(tokens[i], buf, metric[i]);
        buf[0] = '\0';
        expect_true(tokens[i], wu_format_token(tokens[i], buf, sizeof(buf), true));
        expect_str(tokens[i], buf, imperial[i]);
    }

    /* Graph numbers follow met.no: display units for temperature, SI for wind/rain/pressure. */
    expect_true("graph wind is m/s", wu_obs.wind_mps > 4.99 && wu_obs.wind_mps < 5.01);
    expect_true("graph rain is mm", wu_obs.rain_mm == 1.5);
    expect_true("graph pressure is hPa", wu_obs.pressure_hpa == 1013.2);

    expect_true("fresh at obs time", wu_obs_is_fresh(&wu_obs, (time_t)1700000000));
    expect_true("stale after 31 min", !wu_obs_is_fresh(&wu_obs, (time_t)1700000000 + 31 * 60));
    char age[8];
    wu_format_age(&wu_obs, (time_t)1700000000 + 4 * 60, age, sizeof(age));
    expect_str("age", age, "4m");

    expect_true("parse no gust", wu_parse_current(NO_GUST, &parsed));
    wu_obs = parsed;
    expect_true("gust null", !wu_obs.has_gust && wu_obs.has_wind && wu_obs.has_temp);
    buf[0] = '-';
    buf[1] = '\0';
    expect_true("gust token stays unset", !wu_format_token("[wu:gust]", buf, sizeof(buf), false));
    expect_str("gust placeholder", buf, "-");
    buf[0] = '\0';
    expect_true("zero wind still formats", wu_format_token("[wu:wind]", buf, sizeof(buf), false));
    expect_str("calm", buf, "0.0 m/s N");

    if (failures)
    {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS wu observation parse and format\n");
    return 0;
}
