/* ============================================================
   support.js — Advanced tab location helpers: city search,
   geolocate, timezone lookup, "Detected:" hints. Depends on core.js.
   ============================================================ */

function isValidPosix(tz) { return tz && !/\s/.test(tz); }

/* Localized location hint helper: resolves advanced.location.<key> and fills the
   {tz}/{city}/{iana}/{where} placeholders. */
function locStr(key, fallback, vars) {
  let s = tr('advanced.location.' + key, fallback);
  if (vars) Object.keys(vars).forEach(k => { s = s.replace('{' + k + '}', vars[k]); });
  return s;
}

let _tzMap = null;
async function loadTzMap() {
  if (_tzMap) return _tzMap; _tzMap = new Map();
  try {
    const r = await fetch('/timezone.txt');
    if (r.ok) (await r.text()).split('\n').forEach(line => { const t = line.trim(); const i = t.indexOf(';'); if (i > 0) _tzMap.set(t.slice(0, i).replace(/\s/g, '_'), t.slice(i + 1).trim()); });
  } catch (e) { }
  return _tzMap;
}
async function posixFromIana(iana) {
  if (!iana) return null; const key = iana.replace(/\s/g, '_');
  const m = await loadTzMap(); if (m.has(key)) return m.get(key);
  try { const r = await fetch('/api/timezone?location=' + encodeURIComponent(iana)); if (r.ok) { const d = await r.json(); if (d.posix && isValidPosix(d.posix)) return d.posix; } } catch (e) { }
  return null;
}
async function tzFromCoords(lat, lon) {
  try { const r = await fetch('/api/timezone?lat=' + encodeURIComponent(lat) + '&lon=' + encodeURIComponent(lon)); if (r.ok) { const d = await r.json(); if (d.posix && isValidPosix(d.posix)) return { iana: d.iana || '', posix: d.posix }; } } catch (e) { }
  return null;
}
function setCoords(lat, lon) { el('lat').value = lat; el('lon').value = lon; }
/* Overwrite the timezone fields for a freshly chosen location. When the device
   could not resolve a zone (tz == null) we clear the now-stale strings rather
   than leaving the previous city's timezone in place. */
function applyTz(tz) {
  el('timezone').value = tz ? tz.posix : '';
  el('tz_iana').value = (tz && tz.iana) ? tz.iana : '';
}

async function applyDetectedHints() {
  let iana = ''; try { iana = Intl.DateTimeFormat().resolvedOptions().timeZone || ''; } catch (e) { }
  if (iana && el('tz_iana') && !el('tz_iana').value) el('tz_iana').placeholder = locStr('detected', 'Detected: {tz}', { tz: iana });
  if (iana && el('timezone') && !el('timezone').value) { const px = await posixFromIana(iana); if (px && !el('timezone').value) el('timezone').placeholder = locStr('detected', 'Detected: {tz}', { tz: px }); }
  const st = await apiGet('/api/status'); const d = st.data || {};
  if (d.latitude && el('lat') && !el('lat').value) el('lat').placeholder = locStr('detected', 'Detected: {tz}', { tz: d.latitude });
  if (d.longitude && el('lon') && !el('lon').value) el('lon').placeholder = locStr('detected', 'Detected: {tz}', { tz: d.longitude });
}

async function searchCity(q) {
  const lang = navigator.language || 'en';
  const r = await fetch('https://nominatim.openstreetmap.org/search?q=' + encodeURIComponent(q) + '&format=json&limit=1&accept-language=' + lang, { headers: { 'Accept-Language': lang } });
  const data = await r.json();
  if (!data || !data.length) return null;
  return { lat: parseFloat(data[0].lat).toFixed(7), lon: parseFloat(data[0].lon).toFixed(7), name: (data[0].display_name || '').split(',')[0].trim() };
}

const cityHint = el('city-hint');
el('citySearch').addEventListener('click', doCitySearch);
el('city_search').addEventListener('keydown', e => { if (e.key === 'Enter') { e.preventDefault(); doCitySearch(); } });
async function doCitySearch() {
  const q = el('city_search').value.trim(); if (!q) return;
  cityHint.textContent = locStr('city_searching', 'Searching…'); el('citySearch').disabled = true;
  try {
    const res = await searchCity(q);
    if (!res) { cityHint.textContent = locStr('city_not_found', 'City not found.'); return; }
    setCoords(res.lat, res.lon);
    const tz = await tzFromCoords(res.lat, res.lon);
    applyTz(tz);
    cityHint.textContent = tz
      ? (tz.iana ? locStr('city_found_tz', 'Found: {city} ({iana})', { city: res.name, iana: tz.iana })
                 : locStr('city_found', 'Found: {city}', { city: res.name }))
      : locStr('found_no_tz', 'Found: {city} — timezone lookup failed, set it manually.', { city: res.name });
  } catch (e) { cityHint.textContent = locStr('city_search_error', 'Search failed. Check your connection.'); }
  finally { el('citySearch').disabled = false; }
}
el('geolocate').addEventListener('click', async () => {
  cityHint.textContent = locStr('geolocate_searching', 'Looking up your approximate location…'); el('geolocate').disabled = true;
  try {
    const r = await apiGet('/api/locate'); const d = r.data || {};
    if (!r.ok || !d.lat || !d.lon) { cityHint.textContent = locStr('geolocate_failed', 'Could not determine location from IP.'); return; }
    setCoords(d.lat, d.lon);
    // The device resolves a POSIX zone from the IP's IANA zone; fall back to a
    // coordinate lookup if it could not.
    let tz = (d.posix && isValidPosix(d.posix)) ? { iana: d.iana || '', posix: d.posix } : null;
    if (!tz) tz = await tzFromCoords(d.lat, d.lon);
    applyTz(tz);
    const where = d.city ? d.city : locStr('ip_location', 'IP location');
    cityHint.textContent = tz
      ? (tz.iana ? locStr('located_tz', 'Located: {where} ({iana})', { where: where, iana: tz.iana })
                 : locStr('located', 'Located: {where}', { where: where }))
      : locStr('located_no_tz', 'Located: {where} — timezone lookup failed, set it manually.', { where: where });
  } catch (e) { cityHint.textContent = locStr('geolocate_error', 'Failed to look up location.'); }
  finally { el('geolocate').disabled = false; }
});
