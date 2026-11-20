/* ============================================================
   forms.js — Device tab (connection, display, regional) field
   wiring + the payload collectors consumed by savebar.js.
   Depends on core.js.
   ============================================================ */

/* ---------- generic UI ---------- */
function updateEyeAria(btn, isVisible) {
  const t = translations[currentLanguage] || translations.en;
  const label = isVisible ? getNestedTranslation(t, 'common.hide_password') : getNestedTranslation(t, 'common.show_password');
  if (label) btn.setAttribute('aria-label', label);
}
$$('.pw .eye').forEach(b => b.addEventListener('click', () => {
  const inp = b.parentElement.querySelector('input');
  const isVisible = inp.type === 'password'; // about to become visible
  inp.type = isVisible ? 'text' : 'password';
  b.style.opacity = isVisible ? '1' : '';
  updateEyeAria(b, isVisible);
}));

/* Toggle switches. Toggling counts as an unsaved change unless the caller
   opts out (e.g. the auto-update switch, which saves itself immediately). */
function bindSwitch(id, opts) {
  const g = el(id);
  if (g) g.addEventListener('click', () => {
    const on = g.classList.toggle('on');
    g.setAttribute('aria-checked', on ? 'true' : 'false');
    if (!opts || !opts.quiet) { if (window.saveBar) window.saveBar.markDirty(); }
  });
  return el(id);
}
const swOn = id => el(id) && el(id).classList.contains('on');
const setSw = (id, on) => { const g = el(id); if (g) { g.classList.toggle('on', !!on); g.setAttribute('aria-checked', !!on ? 'true' : 'false'); } };
['mirroring', 'fahrenheit', 'hour12'].forEach(id => bindSwitch(id));

const staticToggle = el('staticToggle'), staticPanel = el('staticPanel');
staticToggle.addEventListener('click', () => {
  const on = staticToggle.classList.toggle('on');
  staticToggle.setAttribute('aria-checked', on ? 'true' : 'false');
  staticPanel.hidden = !on;
  if (window.saveBar) window.saveBar.markDirty();
});

/* Brightness sliders use a gamma 2.2 scale (p68), the same as pwm_duty_from_level() in
   f-pwm.c: every low step is one hardware step, so the light can go far below the old 1%.
   The label shows the actual light output as a share of this clock's maximum (board limit
   times max power), sent by GET /api/settings as led_max_duty. */
const LED_SLIDERS = ['brightness_LED0', 'brightness_LED1'];
const ledLevelTouched = new Set();
function ledMaxDuty() { return S().led_max_duty || window._ledMaxDuty || 1023; }
function ledDutyFromLevel(level, max) {
  if (level <= 0) return 0;
  level = Math.min(level, 100);
  return Math.min(max, Math.max(level, Math.round(max * Math.pow(level / 100, 2.2))));
}
/* Slider position for a stored p23 percent (linear), so the slider shows today's brightness. */
function ledLevelFromPercent(pct, max) {
  const duty = Math.floor(pct * max / 100);
  if (duty <= 0) return 0;
  let best = 1;
  for (let l = 1; l <= 100; l++) if (Math.abs(ledDutyFromLevel(l, max) - duty) < Math.abs(ledDutyFromLevel(best, max) - duty)) best = l;
  return best;
}
/* What profile i (0 day, 1 night) really outputs, in % of the maximum. Until its slider is
   moved, a profile without p68 still runs on the linear p23 percent ("linear": true); the
   slider only sits at the nearest gamma position. Otherwise the slider level is on the curve. */
function ledProfileState(i) {
  const id = LED_SLIDERS[i], max = ledMaxDuty(), s = S();
  const level = parseInt((el(id) || {}).value, 10) || 0;
  const p68 = Array.isArray(s.p68) ? s.p68[i] : null;
  if (!ledLevelTouched.has(id) && p68 == null && Array.isArray(s.p23))
    return { level, out: 100 * Math.floor(s.p23[i] * max / 100) / max, linear: true };
  return { level, out: 100 * ledDutyFromLevel(level, max) / max, linear: false };
}
function formatLedPercent(pct) {
  if (pct === 0) return '0%';
  return (pct < 1 ? pct.toFixed(2) : pct < 10 ? pct.toFixed(1) : Math.round(pct)) + '%';
}
function bindRangeVal(id) {
  const r = el(id); if (!r) return;
  const i = LED_SLIDERS.indexOf(id);
  const out = r.parentElement && r.parentElement.querySelector('.range-val');
  const upd = () => { if (out) out.textContent = formatLedPercent(ledProfileState(i).out); };
  r.addEventListener('input', () => { ledLevelTouched.add(id); upd(); drawBrightnessChart(); });
  r._updVal = upd; upd();
}
LED_SLIDERS.forEach(bindRangeVal);
function refreshRangeVals() {
  LED_SLIDERS.forEach(id => { const r = el(id); if (r && r._updVal) r._updVal(); });
  drawBrightnessChart();
}

/* Diagram: slider position (x) against light output (y). The gamma curve is the scale in use;
   the dashed line is the old linear scale. Day and night dots show the current settings; a
   hollow dot is a linear p23 setting, drawn at its real output until its slider is moved. */
let brightGeo = null, brightDrag = null; // last drawn scale and dot positions; dot being dragged
function drawBrightnessChart() {
  const svg = el('bright_svg'); if (!svg) return;
  const W = Math.max(240, Math.round(svg.getBoundingClientRect().width) || 300), H = 170;
  const L = 34, R = 10, T = 10, B = 20, pw = W - L - R, ph = H - T - B;
  svg.setAttribute('viewBox', `0 0 ${W} ${H}`);
  const max = ledMaxDuty();
  const X = l => L + l / 100 * pw, Y = p => T + ph - p / 100 * ph;
  let s = '';
  for (let g = 0; g <= 100; g += 25) {
    const y = Y(g).toFixed(1), x = X(g).toFixed(1);
    s += `<line class="grid" x1="${L}" x2="${W - R}" y1="${y}" y2="${y}"/><text x="${L - 4}" y="${+y + 3.5}" text-anchor="end">${g}%</text>`;
    s += `<line class="grid" x1="${x}" x2="${x}" y1="${T}" y2="${T + ph}"/><text x="${x}" y="${H - 6}" text-anchor="${g === 0 ? 'start' : g === 100 ? 'end' : 'middle'}">${g}</text>`;
  }
  s += `<line class="lin" x1="${X(0)}" y1="${Y(0)}" x2="${X(100)}" y2="${Y(100)}"/>`;
  let d = '';
  for (let l = 0; l <= 100; l++) d += (l ? 'L' : 'M') + X(l).toFixed(1) + ' ' + Y(100 * ledDutyFromLevel(l, max) / max).toFixed(1);
  s += `<path class="gam" d="${d}"/>`;
  brightGeo = { W, H, L, T, pw, ph, dots: [] };
  [['day', 0], ['night', 1]].forEach(([k, i]) => {
    const st = ledProfileState(i), x = X(st.level), y = Y(st.out);
    brightGeo.dots.push({ i, x, y });
    const label = tr('advanced.brightness.chart_' + k, k === 'day' ? 'Day' : 'Night') + ' ' + formatLedPercent(st.out);
    const right = x < L + pw * 0.62; // label beside the dot, on the side with room
    const ly = Math.min(T + ph - 2, Math.max(T + 9, y + (i === 0 ? -8 : 14)));
    s += `<circle class="mk ${k}${st.linear ? ' old' : ''}${brightDrag && brightDrag.i === i ? ' drag' : ''}" cx="${x.toFixed(1)}" cy="${y.toFixed(1)}" r="${brightDrag && brightDrag.i === i ? 6 : 4.5}"/>`;
    s += `<text class="mk-label" x="${(x + (right ? 8 : -8)).toFixed(1)}" y="${ly.toFixed(1)}" text-anchor="${right ? 'start' : 'end'}">${label}</text>`;
  });
  svg.innerHTML = s;
}
window.addEventListener('resize', () => drawBrightnessChart());

/* Drag a dot sideways to move its slider (the dot follows the curve); a click without moving
   focuses that slider for fine steps with the arrow keys. Only the dots react, so swiping
   over the diagram on a phone still scrolls the page. */
{
  const svg = el('bright_svg');
  if (svg) {
    // Pointer position in diagram units (the viewBox drawBrightnessChart set).
    const svgXY = e => {
      const r = svg.getBoundingClientRect(), g = brightGeo || { W: 300, H: 170 };
      return [(e.clientX - r.left) * g.W / r.width, (e.clientY - r.top) * g.H / r.height];
    };
    const dotAt = ([x, y]) => {
      if (!brightGeo) return null;
      const near = brightGeo.dots.map(d => [d, Math.hypot(d.x - x, d.y - y)]).filter(([, dist]) => dist <= 14)
        .sort((a, b) => a[1] - b[1]);
      return near.length ? near[0][0] : null;
    };
    const setLevel = (i, x) => {
      const g = brightGeo, level = Math.max(0, Math.min(100, Math.round((x - g.L) / g.pw * 100)));
      const r = el(LED_SLIDERS[i]);
      if (String(level) === r.value && ledLevelTouched.has(r.id)) return;
      r.value = String(level);
      r.dispatchEvent(new Event('input', { bubbles: true })); // label, diagram, save bar and p68 as for the slider
    };
    // Touch: stop the page from scrolling only when the finger starts on a dot.
    svg.addEventListener('touchstart', e => { if (dotAt(svgXY(e.touches[0]))) e.preventDefault(); }, { passive: false });
    svg.addEventListener('pointerdown', e => {
      const d = dotAt(svgXY(e)); if (!d) return;
      brightDrag = { i: d.i, x0: e.clientX, moved: false };
      try { svg.setPointerCapture(e.pointerId); } catch (err) { /* synthetic events */ }
      drawBrightnessChart();
      e.preventDefault();
    });
    svg.addEventListener('pointermove', e => {
      if (!brightDrag) { svg.style.cursor = dotAt(svgXY(e)) ? 'ew-resize' : ''; return; }
      if (!brightDrag.moved && Math.abs(e.clientX - brightDrag.x0) < 3) return; // a click, not a drag yet
      brightDrag.moved = true;
      setLevel(brightDrag.i, svgXY(e)[0]);
    });
    const end = () => {
      if (!brightDrag) return;
      const { i, moved } = brightDrag;
      brightDrag = null;
      drawBrightnessChart();
      const r = el(LED_SLIDERS[i]);
      if (moved) r.dispatchEvent(new Event('change', { bubbles: true }));
      else r.focus();
    };
    svg.addEventListener('pointerup', end);
    svg.addEventListener('pointercancel', end);
  }
}

/* The scrolling message is edited only in the Layout palette (per day/night
   profile); there is no Device-tab field for it. */

/* Only treat the WiFi password as changed when the user actually edits it.
   Without this, a browser/password-manager autofill leaves a value in the
   field that gets sent as p35 — overwriting the stored password and forcing a
   network-critical restart even when the user only touched an unrelated
   setting (e.g. display rotation). The field is cleared (autofill discarded)
   until the user types. */

/* ---------- DEVICE: connection / display / regional ---------- */
async function loadSettings() {
  await loadGroup('group=settings&params=p03,p09');
  showSecretState('p35'); // the device never returns the password; clears any autofill
  const s = S();
  setVal('hostname', s.p00); setVal('wifi_ssid', s.p34);
  if (s.p03 !== undefined) el('rotation').value = String(s.p03);
  setSw('mirroring', s.p09); setSw('fahrenheit', s.p36); setSw('hour12', s.p37); setSw('update_firmware', s.p39);
  const useStatic = !!(s.p60 && String(s.p60).trim());
  setSw('staticToggle', useStatic); staticPanel.hidden = !useStatic;
  if (useStatic) {
    setVal('static_ip', s.p60); setVal('static_gw', s.p61); setVal('static_nm', s.p62);
    if (s.p63 !== undefined) { const d = String(s.p63 || '').split(','); setVal('static_dns1', d[0] || ''); setVal('static_dns2', d[1] || ''); }
  } else {
    setVal('static_ip', ''); setVal('static_gw', ''); setVal('static_nm', ''); setVal('static_dns1', ''); setVal('static_dns2', '');
  }
  if (typeof populateSettingsLayoutSelect === 'function') await populateSettingsLayoutSelect();
}

const settingsLayoutLoadBtn = el('settingsLayoutLoadBtn');
if (settingsLayoutLoadBtn) settingsLayoutLoadBtn.addEventListener('click', () => applySettingsLayoutToDevice());

/* Same rule as the firmware (is_valid_hostname_char in f-settings.c): ASCII letters,
   digits and hyphens, no hyphen first or last. Returns the error's i18n path or null. */
function hostnameError(v) {
  if (!v) return 'settings.connection.hostname_required';
  if (/:\/\/|[\/.]/.test(v)) return 'settings.connection.hostname_url';
  if (!/^[A-Za-z0-9-]+$/.test(v) || v[0] === '-' || v[v.length - 1] === '-') return 'settings.connection.hostname_invalid';
  return null;
}
function showHostnameError(key) {
  const host = el('hostname'), err = el('hostname-err');
  if (key) {
    err.setAttribute('data-i18n', key);
    err.textContent = tr(key);
    host.setAttribute('aria-invalid', 'true'); err.classList.add('show');
  } else {
    host.removeAttribute('aria-invalid'); err.classList.remove('show');
  }
}
// Flag bad characters while typing; an empty field is only flagged on save.
el('hostname').addEventListener('input', () => {
  const v = el('hostname').value.trim();
  showHostnameError(v ? hostnameError(v) : null);
});

/* Connection-ish fields whose change triggers a device restart on save. */
const SETTINGS_NETWORK_KEYS = ['p00', 'p34', 'p35', 'p60', 'p61', 'p62', 'p63'];

/* Diff the Device-tab basics against stored settings. Returns null when
   validation fails (empty or invalid hostname), an empty object when nothing changed. */
function collectSettingsPayload() {
  const host = el('hostname');
  const hostErr = hostnameError(host.value.trim());
  if (hostErr) {
    showHostnameError(hostErr);
    showTab('settings'); host.focus();
    return null;
  }
  const p = {};
  changed(p, 'p00', host.value.trim());
  changed(p, 'p34', el('wifi_ssid').value);
  changedSecret(p, 'p35', false); // a blank field never clears the Wi-Fi password
  changed(p, 'p03', parseInt(el('rotation').value, 10) || 0);
  changed(p, 'p09', swOn('mirroring') ? 1 : 0);
  changed(p, 'p36', swOn('fahrenheit') ? 1 : 0);
  changed(p, 'p37', swOn('hour12') ? 1 : 0);
  const wasStatic = !!(S().p60 && String(S().p60).trim());
  const useStatic = swOn('staticToggle');
  if (useStatic) {
    changed(p, 'p60', el('static_ip').value.trim());
    changed(p, 'p61', el('static_gw').value.trim());
    changed(p, 'p62', el('static_nm').value.trim());
    changed(p, 'p63', [el('static_dns1').value.trim(), el('static_dns2').value.trim()].filter(Boolean).join(','));
  } else if (wasStatic) {
    changed(p, 'p60', '');
    changed(p, 'p61', '');
    changed(p, 'p62', '');
    changed(p, 'p63', '');
  }
  return p;
}

/* ---------- DEVICE: location / brightness (former Advanced tab) ---------- */
const dimMode = el('dim_mode');
function applyDim() { el('dimBright').hidden = dimMode.value !== '0'; el('dimTime').hidden = dimMode.value !== '2'; }
dimMode.addEventListener('change', applyDim);

/* ---------- light history chart (rolling 24 h, 10-minute averages) ---------- */
let luxHist = null, luxNow = null, luxHistLoaded = false;
let luxGeo = null, luxDrag = null; // last drawn scale; active drag {k: 'thr'|'hi'|'lo', top}
function luxText(key, fallback) {
  return getNestedTranslation(translations[currentLanguage], 'advanced.brightness.' + key)
    || getNestedTranslation(translations.en, 'advanced.brightness.' + key) || fallback;
}
async function loadLuxHistory() {
  const r = await apiGet('/api/status?luxhist=1&_=' + Date.now());
  const d = r.data || {};
  luxHist = d.luxhist || null;
  luxNow = d.lux != null ? Number(d.lux) : null;
  drawLuxChart();
}
function drawLuxChart() {
  const svg = el('lux_svg'); if (!svg) return;
  const W = Math.max(280, Math.round(svg.getBoundingClientRect().width) || 320), H = 140;
  const L = 34, R = 6, T = 8, B = 20, pw = W - L - R, ph = H - T - B;
  svg.setAttribute('viewBox', `0 0 ${W} ${H}`);
  const thr = parseFloat(el('lux_threshold').value) || 0, sens = parseFloat(el('lux_sensitivity').value) || 0;
  // x is local time of day (00:00 to 24:00); the rolling 24 h wraps at the current time,
  // so left of the "now" line is today and right of it is yesterday.
  const now = Date.now() / 1000, t0 = now - 86400;
  const mid = new Date(); mid.setHours(0, 0, 0, 0);
  const midnight = mid.getTime() / 1000;
  const tod = t => { const d = new Date(t * 1000); return (d.getHours() * 3600 + d.getMinutes() * 60 + d.getSeconds()) / 86400; };
  const pts = [];
  if (luxHist && Array.isArray(luxHist.v)) {
    const n = luxHist.v.length;
    luxHist.v.forEach((v, i) => {
      const t = Math.min(now, luxHist.end - (n - 1 - i) * luxHist.step + luxHist.step / 2); // current slot: not past "now"
      if (v != null && t >= t0) pts.push([t, v]); else pts.push(null);
    });
  }
  const vals = pts.filter(Boolean).map(p => p[1]);
  // Whole decades, so the scale only changes when a value crosses one and the band visibly moves.
  // Frozen while dragging so the band does not jump when it crosses a decade.
  const top = luxDrag ? luxDrag.top : Math.pow(10, Math.ceil(Math.log10(Math.max(10, thr + sens, ...vals) * 1.05)));
  const ly = v => T + ph - Math.log10(1 + Math.max(0, v)) / Math.log10(1 + top) * ph;
  const lx = t => L + tod(t) * pw;
  let s = '';
  for (let g = 1; g <= top; g *= 10) {
    const y = ly(g).toFixed(1);
    s += `<line class="grid" x1="${L}" x2="${W - R}" y1="${y}" y2="${y}"/><text x="${L - 4}" y="${+y + 3.5}" text-anchor="end">${g}</text>`;
  }
  s += `<line class="grid" x1="${L}" x2="${W - R}" y1="${T + ph}" y2="${T + ph}"/><text x="${L - 4}" y="${T + ph + 3.5}" text-anchor="end">0</text>`;
  for (let h = 0; h <= 24; h += 6) {
    const x = (L + h / 24 * pw).toFixed(1);
    s += `<line class="grid" x1="${x}" x2="${x}" y1="${T}" y2="${T + ph}"/><text x="${x}" y="${H - 6}" text-anchor="${h === 0 ? 'start' : h === 24 ? 'end' : 'middle'}">${String(h).padStart(2, '0')}:00</text>`;
  }
  const xNow = lx(now).toFixed(1);
  s += `<line class="now" x1="${xNow}" x2="${xNow}" y1="${T}" y2="${T + ph}"/>`;
  const yHi = ly(thr + sens), yLo = ly(Math.max(0, thr - sens));
  s += `<rect class="band" x="${L}" width="${pw}" y="${yHi.toFixed(1)}" height="${Math.max(0.5, yLo - yHi).toFixed(1)}"/>`;
  s += `<line class="edge" x1="${L}" x2="${W - R}" y1="${yHi.toFixed(1)}" y2="${yHi.toFixed(1)}"/><line class="edge" x1="${L}" x2="${W - R}" y1="${yLo.toFixed(1)}" y2="${yLo.toFixed(1)}"/>`;
  s += `<line class="thr" x1="${L}" x2="${W - R}" y1="${ly(thr).toFixed(1)}" y2="${ly(thr).toFixed(1)}"/>`;
  luxGeo = { H, T, ph, top, thr, sens, yThr: ly(thr), yHi, yLo };
  let seg = [], segOld = false;
  const flush = () => {
    const c = segOld ? ' old' : '';
    if (seg.length > 1) s += `<polyline class="line${c}" points="${seg.join(' ')}"/>`;
    else if (seg.length) { const [x, y] = seg[0].split(','); s += `<circle class="dot${c}" cx="${x}" cy="${y}" r="1.8"/>`; }
    seg = [];
  };
  pts.forEach(p => {
    if (!p) { flush(); return; }
    const old = p[0] < midnight;
    if (seg.length && old !== segOld) flush(); // break the line at midnight
    segOld = old;
    seg.push(lx(p[0]).toFixed(1) + ',' + ly(p[1]).toFixed(1));
  });
  flush();
  if (!vals.length) s += `<text class="empty" x="${L + pw / 2}" y="${T + ph / 2}" text-anchor="middle">${luxText('lux_history_empty', 'No readings yet')}</text>`;
  svg.innerHTML = s;
  el('lux_now').textContent = luxNow != null ? `${luxText('lux_now', 'Now')} ${luxNow.toFixed(1)}` : '';
}
['lux_threshold', 'lux_sensitivity'].forEach(id => ['input', 'change'].forEach(ev => el(id).addEventListener(ev, drawLuxChart)));
window.addEventListener('resize', () => { if (luxHistLoaded) drawLuxChart(); });

/* Drag the threshold line or a band edge to set threshold / sensitivity. */
{
  const svg = el('lux_svg');
  const svgY = e => { const r = svg.getBoundingClientRect(); return (e.clientY - r.top) * (luxGeo ? luxGeo.H : 140) / r.height; };
  const handleAt = y => {
    if (!luxGeo) return null;
    const c = [['thr', luxGeo.yThr], ['hi', luxGeo.yHi], ['lo', luxGeo.yLo]]
      .map(([k, hy]) => [k, Math.abs(y - hy)]).filter(([, d]) => d <= 10).sort((a, b) => a[1] - b[1]);
    return c.length ? c[0][0] : null;
  };
  const luxAt = y => {
    const g = luxGeo, f = (g.T + g.ph - y) / g.ph;
    return Math.max(0, Math.pow(10, f * Math.log10(1 + g.top)) - 1);
  };
  const round = v => v < 10 ? Math.round(v * 10) / 10 : Math.round(v);
  const setField = (id, v) => { const i = el(id); i.value = String(v); i.dispatchEvent(new Event('input', { bubbles: true })); };
  // Touch: stop the page from scrolling only when the finger starts on a handle.
  svg.addEventListener('touchstart', e => { if (handleAt(svgY(e.touches[0]))) e.preventDefault(); }, { passive: false });
  svg.addEventListener('pointerdown', e => {
    const k = handleAt(svgY(e)); if (!k) return;
    luxDrag = { k, top: luxGeo.top };
    try { svg.setPointerCapture(e.pointerId); } catch (err) { /* synthetic events */ }
    e.preventDefault();
  });
  svg.addEventListener('pointermove', e => {
    const y = svgY(e);
    if (!luxDrag) { svg.style.cursor = handleAt(y) ? 'ns-resize' : ''; return; }
    const v = luxAt(y), thr = luxGeo.thr;
    if (luxDrag.k === 'thr') setField('lux_threshold', Math.min(500, round(v)));
    else setField('lux_sensitivity', Math.min(50, round(Math.max(0, luxDrag.k === 'hi' ? v - thr : thr - v))));
  });
  const end = () => {
    if (!luxDrag) return;
    luxDrag = null;
    drawLuxChart();
    ['lux_threshold', 'lux_sensitivity'].forEach(id => el(id).dispatchEvent(new Event('change', { bubbles: true })));
  };
  svg.addEventListener('pointerup', end);
  svg.addEventListener('pointercancel', end);
}
{
  const box = el('dimBright').closest('details');
  if (box) box.addEventListener('toggle', () => { if (box.open && !luxHistLoaded) { luxHistLoaded = true; loadLuxHistory(); } });
}

async function loadAdvanced() {
  await loadGroup('group=advanced');
  const s = S();
  setVal('lat', s.p17); setVal('lon', s.p18); setVal('timezone', s.p19);
  if (s.tz_iana !== undefined) setVal('tz_iana', s.tz_iana);
  if (s.p46 !== undefined) el('wifi_start').value = formatMinsToTimeString(s.p46);
  if (s.p47 !== undefined) el('wifi_end').value = formatMinsToTimeString(s.p47);
  if (s.p22 !== undefined) { el('dim_mode').value = String(s.p22); applyDim(); }
  if (s.p21 !== undefined) setVal('lux_threshold', s.p21);
  if (s.p20 !== undefined) setVal('lux_sensitivity', s.p20);
  if (luxHistLoaded) drawLuxChart();
  // p68 holds the slider level per profile; where it is null the clock still uses the linear
  // p23 percent, shown at the matching slider position (display only, nothing is saved).
  ledLevelTouched.clear();
  LED_SLIDERS.forEach((id, i) => {
    const lvl = Array.isArray(s.p68) ? s.p68[i] : null;
    if (lvl != null) setVal(id, lvl);
    else if (Array.isArray(s.p23)) setVal(id, ledLevelFromPercent(s.p23[i], ledMaxDuty()));
  });
  refreshRangeVals();
  setVal('pwm_frequency', s.p42); setVal('max_power', s.p43);
  if (s.p55 !== undefined) el('dim_start').value = formatMinsToTimeString(s.p55);
  if (s.p56 !== undefined) el('dim_end').value = formatMinsToTimeString(s.p56);
  if (typeof applyDetectedHints === 'function') applyDetectedHints();
}

function collectAdvancedPayload() {
  const p = {};
  changed(p, 'p17', el('lat').value.trim());
  changed(p, 'p18', el('lon').value.trim());
  changed(p, 'p19', el('timezone').value.trim());
  if (S().tz_iana !== undefined && String(S().tz_iana) !== el('tz_iana').value.trim()) p.tz_iana = el('tz_iana').value.trim();
  else if (S().tz_iana === undefined && el('tz_iana').value.trim()) p.tz_iana = el('tz_iana').value.trim();
  changed(p, 'p46', parseTimeStringToMins(el('wifi_start').value));
  changed(p, 'p47', parseTimeStringToMins(el('wifi_end').value));
  changed(p, 'p22', parseInt(dimMode.value, 10) || 0);
  changed(p, 'p21', parseFloat(el('lux_threshold').value) || 0);
  changed(p, 'p20', parseFloat(el('lux_sensitivity').value) || 0);
  // Only a slider the user moved is saved (as p68); the other profile keeps its stored level,
  // or null where it still uses p23. The device keeps p23 as the rounded percent.
  if (ledLevelTouched.size) {
    p.p68 = LED_SLIDERS.map((id, i) => ledLevelTouched.has(id)
      ? (parseInt(el(id).value, 10) || 0)
      : (Array.isArray(S().p68) && S().p68[i] != null ? S().p68[i] : null));
  }
  changed(p, 'p42', parseInt(el('pwm_frequency').value, 10) || 0);
  changed(p, 'p43', parseInt(el('max_power').value, 10) || 0);
  changed(p, 'p55', parseTimeStringToMins(el('dim_start').value));
  changed(p, 'p56', parseTimeStringToMins(el('dim_end').value));
  return p;
}

/* The Device tab hosts both the former Settings and Advanced field groups. */
sectionLoaders.settings = async function () {
  await loadSettings();
  await loadAdvanced();
  if (typeof initFontsShortcut === 'function') initFontsShortcut();
};
