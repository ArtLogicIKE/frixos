/* ============================================================
   forms.js — Settings + Advanced tabs. Depends on core.js.
   ============================================================ */

/* ---------- generic UI ---------- */
$$('.pw .eye').forEach(b => b.addEventListener('click', () => {
  const inp = b.parentElement.querySelector('input');
  inp.type = inp.type === 'password' ? 'text' : 'password';
  b.style.opacity = inp.type === 'text' ? '1' : '';

  const t = translations[currentLanguage] || translations.en;
  const key = inp.type === 'password' ? 'common.show_password' : 'common.hide_password';
  const label = getNestedTranslation(t, key) || (inp.type === 'password' ? 'Show password' : 'Hide password');
  b.setAttribute('aria-label', label);
}));

function bindSwitch(id) { const g = el(id); if (g) g.addEventListener('click', () => g.classList.toggle('on')); return el(id); }
const swOn = id => el(id) && el(id).classList.contains('on');
const setSw = (id, on) => { const g = el(id); if (g) g.classList.toggle('on', !!on); };
['mirroring', 'fahrenheit', 'hour12', 'update_firmware'].forEach(bindSwitch);

const staticToggle = el('staticToggle'), staticPanel = el('staticPanel');
staticToggle.addEventListener('click', () => { staticToggle.classList.toggle('on'); staticPanel.hidden = !staticToggle.classList.contains('on'); });

const msg = el('message'), msgCounter = el('message-counter');
if (msg) msg.addEventListener('input', () => { msgCounter.textContent = msg.value.length + ' / 511'; });

/* ---------- SETTINGS ---------- */
async function loadSettings() {
  await loadGroup('group=settings&params=p03,p09');
  const s = S();
  setVal('hostname', s.p00); setVal('wifi_ssid', s.p34);
  if (s.p03 !== undefined) el('rotation').value = String(s.p03);
  setSw('mirroring', s.p09); setSw('fahrenheit', s.p36); setSw('hour12', s.p37); setSw('update_firmware', s.p39);
  setVal('static_ip', s.p60); setVal('static_gw', s.p61); setVal('static_nm', s.p62);
  if (s.p63 !== undefined) { const d = String(s.p63 || '').split(','); setVal('static_dns1', d[0] || ''); setVal('static_dns2', d[1] || ''); }
  const useStatic = !!(s.p60 && String(s.p60).trim());
  setSw('staticToggle', useStatic); staticPanel.hidden = !useStatic;
  if (s.p16 !== undefined && msg) { msg.value = s.p16 || ''; msgCounter.textContent = msg.value.length + ' / 511'; }
}
sectionLoaders.settings = loadSettings;

el('saveSettings').addEventListener('click', () => {
  const host = el('hostname');
  if (!host.value.trim()) { host.setAttribute('aria-invalid', 'true'); el('hostname-err').classList.add('show'); host.focus(); return; }
  const p = {};
  changed(p, 'p00', host.value.trim());
  changed(p, 'p34', el('wifi_ssid').value);
  changedSecret(p, 'p35', el('wifi_pass').value);
  changed(p, 'p03', parseInt(el('rotation').value, 10) || 0);
  changed(p, 'p09', swOn('mirroring') ? 1 : 0);
  changed(p, 'p36', swOn('fahrenheit') ? 1 : 0);
  changed(p, 'p37', swOn('hour12') ? 1 : 0);
  changed(p, 'p39', swOn('update_firmware') ? 1 : 0);
  if (msg) changed(p, 'p16', msg.value);
  const useStatic = swOn('staticToggle');
  changed(p, 'p60', useStatic ? el('static_ip').value.trim() : '');
  if (useStatic) {
    changed(p, 'p61', el('static_gw').value.trim());
    changed(p, 'p62', el('static_nm').value.trim());
    changed(p, 'p63', [el('static_dns1').value.trim(), el('static_dns2').value.trim()].filter(Boolean).join(','));
  }
  saveSettings(p, ['p00', 'p34', 'p35', 'p60']);
});
el('hostname').addEventListener('input', () => { if (el('hostname').value.trim()) { el('hostname').removeAttribute('aria-invalid'); el('hostname-err').classList.remove('show'); } });

/* ---------- ADVANCED ---------- */
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
  setVal('pwm_frequency', s.p42); setVal('max_power', s.p43);
  if (s.p55 !== undefined) el('dim_start').value = formatMinsToTimeString(s.p55);
  if (s.p56 !== undefined) el('dim_end').value = formatMinsToTimeString(s.p56);
  if (typeof applyDetectedHints === 'function') applyDetectedHints();
}
sectionLoaders.advanced = loadAdvanced;

el('saveAdvanced').addEventListener('click', () => {
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
  saveSettings(p, []); // location changes restart server-side
});
