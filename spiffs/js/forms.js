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

/* Brightness sliders show their value live. */
function bindRangeVal(id) {
  const r = el(id); if (!r) return;
  const out = r.parentElement && r.parentElement.querySelector('.range-val');
  const upd = () => { if (out) out.textContent = r.value + '%'; };
  r.addEventListener('input', upd);
  r._updVal = upd; upd();
}
['brightness_LED0', 'brightness_LED1'].forEach(bindRangeVal);
function refreshRangeVals() { ['brightness_LED0', 'brightness_LED1'].forEach(id => { const r = el(id); if (r && r._updVal) r._updVal(); }); }

/* The scrolling message is edited only in the Layout palette (per day/night
   profile); there is no Device-tab field for it. */

/* Only treat the WiFi password as changed when the user actually edits it.
   Without this, a browser/password-manager autofill leaves a value in the
   field that gets sent as p35 — overwriting the stored password and forcing a
   network-critical restart even when the user only touched an unrelated
   setting (e.g. display rotation). The field is cleared (autofill discarded)
   until the user types. */
let wifiPassDirty = false;
const wifiPass = el('wifi_pass');
if (wifiPass) wifiPass.addEventListener('input', () => { wifiPassDirty = true; });

/* ---------- DEVICE: connection / display / regional ---------- */
async function loadSettings() {
  await loadGroup('group=settings&params=p03,p09');
  delete window.settings.p35; // never keep WiFi password in memory
  if (wifiPass) { wifiPass.value = ''; wifiPassDirty = false; } // discard any browser autofill
  const s = S();
  setVal('hostname', s.p00); setVal('wifi_ssid', s.p34);
  updateCharCounter(el('hostname'), el('hostname-counter'));
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

el('hostname').addEventListener('input', () => { if (el('hostname').value.trim()) { el('hostname').removeAttribute('aria-invalid'); el('hostname-err').classList.remove('show'); } });

/* Connection-ish fields whose change triggers a device restart on save. */
const SETTINGS_NETWORK_KEYS = ['p00', 'p34', 'p35', 'p60', 'p61', 'p62', 'p63'];

/* Diff the Device-tab basics against stored settings. Returns null when
   validation fails (invalid hostname), an empty object when nothing changed. */
function collectSettingsPayload() {
  const host = el('hostname');
  if (!host.value.trim()) {
    host.setAttribute('aria-invalid', 'true'); el('hostname-err').classList.add('show');
    showTab('settings'); host.focus();
    return null;
  }
  const p = {};
  changed(p, 'p00', host.value.trim());
  changed(p, 'p34', el('wifi_ssid').value);
  if (wifiPassDirty) changedSecret(p, 'p35', el('wifi_pass').value);
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
  if (Array.isArray(s.p23)) { setVal('brightness_LED0', s.p23[0]); setVal('brightness_LED1', s.p23[1]); }
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
  const ledNew = [parseInt(el('brightness_LED0').value, 10) || 0, parseInt(el('brightness_LED1').value, 10) || 0];
  if (S().p23 === undefined || JSON.stringify(S().p23) !== JSON.stringify(ledNew)) p.p23 = ledNew;
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
  if (typeof initNightFontShortcut === 'function') initNightFontShortcut();
};
