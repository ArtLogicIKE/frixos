/* ============================================================
   support.js — Advanced tab location helpers: city search,
   geolocate, timezone lookup, "Detected:" hints. Depends on core.js.
   ============================================================ */

function isValidPosix(tz) { return tz && !/\s/.test(tz); }

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

async function applyDetectedHints() {
  let iana = ''; try { iana = Intl.DateTimeFormat().resolvedOptions().timeZone || ''; } catch (e) { }
  if (iana && el('tz_iana') && !el('tz_iana').value) el('tz_iana').placeholder = 'Detected: ' + iana;
  if (iana && el('timezone') && !el('timezone').value) { const px = await posixFromIana(iana); if (px && !el('timezone').value) el('timezone').placeholder = 'Detected: ' + px; }
  const st = await apiGet('/api/status'); const d = st.data || {};
  if (d.latitude && el('lat') && !el('lat').value) el('lat').placeholder = 'Detected: ' + d.latitude;
  if (d.longitude && el('lon') && !el('lon').value) el('lon').placeholder = 'Detected: ' + d.longitude;
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
  cityHint.textContent = 'Searching…'; el('citySearch').disabled = true;
  try {
    const res = await searchCity(q);
    if (!res) { cityHint.textContent = 'City not found.'; return; }
    setCoords(res.lat, res.lon);
    const tz = await tzFromCoords(res.lat, res.lon);
    if (tz) { el('timezone').value = tz.posix; if (tz.iana) el('tz_iana').value = tz.iana; }
    cityHint.textContent = 'Found: ' + res.name + (tz && tz.iana ? ' (' + tz.iana + ')' : '');
  } catch (e) { cityHint.textContent = 'Search failed. Check your connection.'; }
  finally { el('citySearch').disabled = false; }
}
el('geolocate').addEventListener('click', async () => {
  cityHint.textContent = 'Reading location from device…'; el('geolocate').disabled = true;
  try {
    const st = await apiGet('/api/status'); const d = st.data || {};
    if (!d.latitude || !d.longitude) { cityHint.textContent = 'No coordinates available on device yet.'; return; }
    setCoords(d.latitude, d.longitude);
    const tz = await tzFromCoords(d.latitude, d.longitude);
    if (tz) { el('timezone').value = tz.posix; if (tz.iana) el('tz_iana').value = tz.iana; cityHint.textContent = 'Located' + (tz.iana ? ': ' + tz.iana : ''); }
    else cityHint.textContent = 'Located.';
  } catch (e) { cityHint.textContent = 'Failed to read location.'; }
  finally { el('geolocate').disabled = false; }
});
