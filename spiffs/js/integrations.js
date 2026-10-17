/* ============================================================
   integrations.js — Integrations tab
   Depends on core.js.
   ============================================================ */

const INT_MAP = {
  eeprom_ha_url: 'p25', eeprom_ha_token: 'p26', eeprom_ha_refresh_mins: 'p27',
  eeprom_stock_key: 'p28', eeprom_stock_refresh_mins: 'p29',
  eeprom_wu_station: 'p64', eeprom_wu_key: 'p65', eeprom_wu_refresh_mins: 'p66',
  eeprom_dexcom_region: 'p30', eeprom_glucose_username: 'p31', eeprom_glucose_password: 'p32',
  eeprom_glucose_refresh: 'p33', eeprom_libre_region: 'p44', glucose_validity_duration: 'p45',
  eeprom_glucose_high: 'p51', eeprom_glucose_low: 'p52', eeprom_glucose_unit: 'p53', eeprom_ns_url: 'p54'
};

async function loadIntegrations() {
  await loadGroup('group=integrations');
  const s = S();
  for (const [id, p] of Object.entries(INT_MAP)) {
    if (s[p] !== undefined) {
      setVal(id, s[p]);
      const c = el(id + '-counter');
      if (c) updateCharCounter(el(id), c);
    }
  }
  initCgmDeviceType();
  updateIntegrationDots();
}
sectionLoaders.integrations = loadIntegrations;

/* ---------- integration status dots ----------
   Green: configured and the firmware reports active tokens / recent data.
   Amber: configured but no data reported yet. Gray: not configured.
   Source: /api/status ha_tokens lines (stable English firmware strings) +
   the stored settings for the configured check. */
function updateIntegrationDots() {
  const lines = (window.statusData && Array.isArray(window.statusData.ha_tokens))
    ? window.statusData.ha_tokens.join('\n') : '';
  const s = S();
  const setDot = (dotId, stateId, configured, activeRe) => {
    const dot = el(dotId); if (!dot) return;
    let cls = 'dot off', txt = tr('integrations.state_off', 'Not configured');
    if (configured) {
      const active = activeRe.test(lines);
      cls = active ? 'dot ok' : 'dot warn';
      txt = active ? tr('integrations.state_ok', 'Active') : tr('integrations.state_wait', 'Configured — no data yet');
    }
    dot.className = cls;
    dot.title = txt;
    const state = el(stateId);
    if (state) state.textContent = txt;
  };
  setDot('dot-ha', 'state-ha', !!(s.p25 && String(s.p25).trim()), /Home Assistant tokens:\s*[1-9]/);
  setDot('dot-stock', 'state-stock', !!(s.p28 && String(s.p28).trim()), /Stock \(finnhub\.io\) tokens:\s*[1-9]/);
  setDot('dot-wu', 'state-wu', !!(s.p64 && String(s.p64).trim() && s.p65 && String(s.p65).trim()), /Weather Underground: \S+, obs /);
  setDot('dot-cgm', 'state-cgm', deriveCgmType() !== 'none', /(Dexcom|FreeStyle Libre|Nightscout) active:/);
}

/* ---------- CGM device type ----------
   "Device Type" is a UI-only selector; there is no dedicated stored field.
   The active device is derived from whichever underlying field is set
   (dexcom region / libre region / nightscout URL) and the three are mutually
   exclusive, so switching device type clears the fields owned by the others. */
function deriveCgmType() {
  const dx = el('eeprom_dexcom_region'), lb = el('eeprom_libre_region'), ns = el('eeprom_ns_url');
  if (dx && dx.value !== '0') return 'dexcom';
  if (lb && lb.value !== '0') return 'libre';
  if (ns && ns.value.trim() !== '') return 'nightscout';
  return 'none';
}

function applyCgmVisibility(type) {
  document.querySelectorAll('#tab-integrations .cgm-field').forEach(f => {
    f.hidden = !(f.dataset.cgm || '').split(/\s+/).includes(type);
  });
}

function resetCgmFieldsFor(type) {
  // Reset only the fields owned by the other two options (username/password are
  // shared by dexcom + libre, so they survive a switch between those two).
  const dx = el('eeprom_dexcom_region'), lb = el('eeprom_libre_region'), ns = el('eeprom_ns_url'),
        user = el('eeprom_glucose_username'), pass = el('eeprom_glucose_password');
  const clearSel = e => { if (e) e.value = '0'; };
  const clearTxt = e => { if (!e) return; e.value = ''; const c = el(e.id + '-counter'); if (c) updateCharCounter(e, c); };
  if (type === 'dexcom') { clearSel(lb); clearTxt(ns); }
  else if (type === 'libre') { clearSel(dx); clearTxt(ns); }
  else if (type === 'nightscout') { clearSel(dx); clearSel(lb); clearTxt(user); clearTxt(pass); }
  else { clearSel(dx); clearSel(lb); clearTxt(ns); clearTxt(user); clearTxt(pass); }
}

function initCgmDeviceType() {
  const sel = el('cgm_device_type');
  if (!sel) return;
  sel.value = deriveCgmType();
  applyCgmVisibility(sel.value);
}

(() => {
  const sel = el('cgm_device_type');
  if (sel) sel.addEventListener('change', () => {
    resetCgmFieldsFor(sel.value);
    applyCgmVisibility(sel.value);
  });
})();

/* Diff the Integrations fields against stored settings (for savebar.js). */
function collectIntegrationsPayload() {
  const p = {};
  const secrets = ['eeprom_ha_token', 'eeprom_stock_key', 'eeprom_wu_key', 'eeprom_glucose_password'];
  for (const [id, key] of Object.entries(INT_MAP)) {
    const e = el(id); if (!e) continue;
    const v = (e.tagName === 'SELECT') ? (parseInt(e.value, 10) || 0)
      : (e.type === 'number') ? (parseInt(e.value, 10) || 0) : e.value;
    if (secrets.includes(id)) changedSecret(p, key, v); else changed(p, key, v);
  }
  return p;
}
