/* ============================================================
   integrations.js — Integrations tab + display-schedule helpers
   (the schedule helpers are consumed by the layout editor).
   Depends on core.js.
   ============================================================ */

const INT_MAP = {
  eeprom_ha_url: 'p25', eeprom_ha_token: 'p26', eeprom_ha_refresh_mins: 'p27',
  eeprom_stock_key: 'p28', eeprom_stock_refresh_mins: 'p29',
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
}
sectionLoaders.integrations = loadIntegrations;

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

el('saveIntegrations').addEventListener('click', () => {
  const p = {};
  const secrets = ['eeprom_ha_token', 'eeprom_stock_key', 'eeprom_glucose_password'];
  for (const [id, key] of Object.entries(INT_MAP)) {
    const e = el(id); if (!e) continue;
    const v = (e.tagName === 'SELECT') ? (parseInt(e.value, 10) || 0)
      : (e.type === 'number') ? (parseInt(e.value, 10) || 0) : e.value;
    if (secrets.includes(id)) changedSecret(p, key, v); else changed(p, key, v);
  }
  saveSettings(p, []);
});

/* ---------- display schedule (Layout editor: time / time_aux slots) ---------- */
const SLOT_TYPES = [
  { value: 0, key: 'screen.display_schedule.type_time', label: 'Time' },
  { value: 1, key: 'screen.display_schedule.type_cgm', label: 'CGM (glucose)' },
  { value: 2, key: 'screen.display_schedule.type_temp', label: 'Outside Temperature' },
  { value: 3, key: 'screen.display_schedule.type_ha', label: 'Home Assistant entity' }
];

function buildSlotRow(slot) {
  const row = document.createElement('div');
  row.className = 'schedule-slot-row';

  const typeSelect = document.createElement('select');
  SLOT_TYPES.forEach(opt => {
    const o = document.createElement('option');
    o.value = opt.value; o.textContent = getScreenTranslation(opt.key, opt.label);
    if (slot.t === opt.value) o.selected = true;
    typeSelect.appendChild(o);
  });

  const durInput = document.createElement('input');
  durInput.type = 'number'; durInput.min = 1; durInput.max = 3600;
  durInput.value = slot.d || 30; durInput.title = getScreenTranslation('screen.display_schedule.duration_title', 'Duration (seconds)');

  const durLabel = document.createElement('span');
  durLabel.className = 'schedule-slot-sec'; durLabel.textContent = getScreenTranslation('screen.display_schedule.sec', 'sec');

  const entityInput = document.createElement('input');
  entityInput.type = 'text'; entityInput.className = 'schedule-slot-entity';
  entityInput.placeholder = getScreenTranslation('screen.display_schedule.entity_placeholder', 'entity_id (e.g. sensor.temp)'); entityInput.value = slot.e || '';
  entityInput.hidden = slot.t !== 3;

  const unitInput = document.createElement('input');
  unitInput.type = 'text'; unitInput.className = 'schedule-slot-unit';
  unitInput.placeholder = getScreenTranslation('screen.display_schedule.unit_placeholder', 'unit (e.g. °F)'); unitInput.maxLength = 7; unitInput.value = slot.l || '';
  unitInput.hidden = slot.t !== 3;

  const nameInput = document.createElement('input');
  nameInput.type = 'text'; nameInput.className = 'schedule-slot-label';
  nameInput.placeholder = getScreenTranslation('screen.display_schedule.label_placeholder', 'label (e.g. Outside Temp)');
  nameInput.maxLength = 25; nameInput.value = slot.n || '';

  typeSelect.addEventListener('change', () => {
    const isHA = parseInt(typeSelect.value) === 3;
    entityInput.hidden = !isHA; unitInput.hidden = !isHA;
  });

  const removeBtn = document.createElement('button');
  removeBtn.type = 'button'; removeBtn.textContent = '✕';
  removeBtn.addEventListener('click', () => row.remove());

  row.appendChild(typeSelect); row.appendChild(durInput); row.appendChild(durLabel);
  row.appendChild(entityInput); row.appendChild(unitInput); row.appendChild(nameInput); row.appendChild(removeBtn);
  return row;
}

function renderDisplaySchedule(listEl, settingsKey = 'p58') {
  const list = listEl || el('display-schedule-list');
  if (!list) return;
  list.innerHTML = '';
  let slots = [];
  const raw = window.settings && window.settings[settingsKey];
  if (typeof raw === 'string' && raw.trim().startsWith('[')) { try { slots = JSON.parse(raw); } catch (e) { slots = []; } }
  if (settingsKey === 'p58' && (!Array.isArray(slots) || slots.length === 0)) {
    if ((window.settings.p48 || 0) > 0) slots.push({ t: 0, d: window.settings.p48 });
    if ((window.settings.p49 || 0) > 0) slots.push({ t: 1, d: window.settings.p49 });
    if ((window.settings.p57 || 0) > 0) slots.push({ t: 2, d: window.settings.p57 });
    if (slots.length === 0) slots.push({ t: 0, d: 30 });
  }
  if (settingsKey === 'p59' && (!Array.isArray(slots) || slots.length === 0)) slots.push({ t: 0, d: 30 });
  slots.forEach(s => list.appendChild(buildSlotRow(s)));
}

function appendDisplayScheduleSection(container, options = {}) {
  const trans = translations[currentLanguage] || translations.en;
  const listId = options.listId || 'display-schedule-list';
  const titleKey = options.titleKey || 'screen.display_schedule.title';
  const descKey = options.descriptionKey || 'screen.display_schedule.description';
  const settingsKey = options.settingsKey || 'p58';

  const section = document.createElement('div');
  section.className = 'screen-display-schedule';

  const heading = document.createElement('h4');
  heading.textContent = getNestedTranslation(trans, titleKey) || 'Display Schedule';

  const desc = document.createElement('p');
  desc.className = 'screen-options-mode-note';
  desc.textContent = getNestedTranslation(trans, descKey)
    || 'Define what appears on the display and for how long. Slots cycle in order. A single Time slot means the clock always shows.';

  const list = document.createElement('div');
  list.id = listId; list.className = 'display-schedule-list';

  const addBtn = document.createElement('button');
  addBtn.type = 'button'; addBtn.className = 'screen-schedule-add';
  addBtn.textContent = getNestedTranslation(trans, 'screen.display_schedule.add_slot') || '+ Add Slot';
  addBtn.addEventListener('click', () => { list.appendChild(buildSlotRow({ t: 0, d: 30 })); });

  section.appendChild(heading); section.appendChild(desc); section.appendChild(list); section.appendChild(addBtn);
  container.appendChild(section);
  renderDisplaySchedule(list, settingsKey);
}

function serializeDisplaySchedule(listId = 'display-schedule-list') {
  const list = el(listId);
  if (!list) return '[]';
  const slots = [];
  list.querySelectorAll('.schedule-slot-row').forEach(row => {
    const typeEl = row.querySelector('select');
    const durEl = row.querySelectorAll('input[type="number"]')[0];
    const textInputs = row.querySelectorAll('input[type="text"]');
    const entEl = textInputs[0], unitEl = textInputs[1], nameEl = textInputs[2];
    const t = parseInt(typeEl ? typeEl.value : 0);
    const d = parseInt(durEl ? durEl.value : 30) || 1;
    const slot = { t, d };
    if (t === 3) {
      if (entEl && entEl.value.trim()) slot.e = entEl.value.trim();
      if (unitEl && unitEl.value.trim()) slot.l = unitEl.value.trim();
    }
    if (nameEl && nameEl.value.trim()) slot.n = nameEl.value.trim();
    if (t !== 3 || slot.e) slots.push(slot);
  });
  return JSON.stringify(slots);
}
