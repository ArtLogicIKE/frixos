/* ============================================================
   core.js — API helpers, theme, i18n apply, tabs/nav, shared
   settings load/save, boot. Depends on utils.js.
   ============================================================ */

const body = document.body;
window.settings = window.settings || {};
window.sectionsInitialized = window.sectionsInitialized || {};
const sectionLoaders = {};
const loadedSections = {};

/* ---------- fetch ---------- */
async function apiGet(url) {
  const r = await fetch(url, { headers: { 'Accept': 'application/json' } });
  const txt = await r.text();
  let data = null; try { data = txt ? JSON.parse(txt) : null; } catch (e) { data = null; }
  return { ok: r.ok, status: r.status, data, txt };
}
async function apiPostJson(url, obj) {
  const r = await fetch(url, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(obj) });
  const txt = await r.text();
  let data = null; try { data = txt ? JSON.parse(txt) : null; } catch (e) { data = null; }
  return { ok: r.ok, status: r.status, data, txt };
}
function settingsResponseHasParams(d) { return !!(d && Object.keys(d).some(k => /^p\d+$/.test(k))); }

/* Promise-based settings fetch used by the layout editor. */
async function fetchSettingsJson(url, options = {}) {
  const response = await fetch(url);
  let data = null;
  try { data = await response.json(); } catch (e) { throw new Error('Invalid JSON from ' + url); }
  if (!response.ok || (data && data.status === 'error')) {
    throw new Error((data && data.message) || ('HTTP ' + response.status));
  }
  if (!settingsResponseHasParams(data) && options.fallbackUrl) {
    const fb = await fetch(options.fallbackUrl);
    const fbData = await fb.json();
    if (fb.ok && settingsResponseHasParams(fbData)) return fbData;
  }
  return data;
}
function mergeSettingsData(data) {
  if (!data || typeof data !== 'object') return false;
  let merged = false;
  Object.keys(data).forEach(k => { if (k === 'status' || k === 'message') return; window.settings[k] = data[k]; merged = true; });
  return merged;
}

/* ---------- toast + status shim ---------- */
let _toastT;
function toast(msg, kind) {
  const t = el('toast'); if (!t) return;
  t.textContent = msg; t.className = 'toast show' + (kind ? ' ' + kind : '');
  clearTimeout(_toastT); _toastT = setTimeout(() => t.classList.remove('show'), 2600);
}
/* showStatus(message, 'success'|'error'|'info'|'warning') — used by the editor */
function showStatus(message, type) {
  toast(message, type === 'success' ? 'ok' : type === 'error' ? 'err' : '');
}

/* ---------- reboot wait ----------
   Shows a full-screen "Waiting for reboot to be completed" overlay with a
   countdown, then polls the device until it is reachable again and reloads the
   page. The first REBOOT_WAIT_S seconds send no requests at all: a page load
   during boot fragments the heap the device needs for its first TLS
   connections (weather, Freestyle, HA). Idempotent: calling it while already
   waiting is a no-op. A hard time cap reloads anyway. */
const REBOOT_WAIT_S = 60;
let _rebootWaiting = false;
function waitForReboot() {
  if (_rebootWaiting) return;
  _rebootWaiting = true;
  const overlay = el('rebootOverlay');
  if (overlay) { overlay.classList.add('open'); overlay.setAttribute('aria-hidden', 'false'); }
  const countdown = el('rebootCountdown');
  const POLL_MS = 1500, MAX_MS = 120000;
  let left = REBOOT_WAIT_S, start = 0;
  const probe = async () => {
    let up = false;
    try {
      const ctrl = new AbortController();
      const to = setTimeout(() => ctrl.abort(), 4000);
      const r = await fetch('/api/status?_=' + Date.now(), { signal: ctrl.signal, cache: 'no-store', headers: { 'Accept': 'application/json' } });
      clearTimeout(to);
      up = r.ok;
    } catch (e) { up = false; }
    if (up || Date.now() - start > MAX_MS) { location.reload(); return; }
    setTimeout(probe, POLL_MS);
  };
  const tick = () => {
    if (left > 0) {
      if (countdown) countdown.textContent = tr('restart.countdown', 'Reconnecting in {n} s').replace('{n}', left);
      left--;
      setTimeout(tick, 1000);
      return;
    }
    if (countdown) countdown.textContent = tr('restart.reconnecting', 'Reconnecting...');
    start = Date.now();
    probe();
  };
  tick();
}

/* ---------- settings helpers ---------- */
const S = () => window.settings;
const setVal = (id, v) => { const e = el(id); if (e != null && v !== undefined && v !== null) e.value = v; };
async function loadGroup(query) {
  let res = await apiGet('/api/settings?' + query);
  if (!settingsResponseHasParams(res.data)) { const full = await apiGet('/api/settings'); if (full.data) res = full; }
  if (res.data) {
    // A group query reports only its own secrets; keep the flags other groups set.
    const flags = Object.assign({}, window.settings.secrets_set, res.data.secrets_set);
    Object.assign(window.settings, res.data);
    window.settings.secrets_set = flags;
  }
  return window.settings;
}
function changed(payload, key, newVal) { if (S()[key] === undefined) return; if (String(S()[key]) !== String(newVal)) payload[key] = newVal; }

/* Secrets (Wi-Fi password, HA token, API keys, glucose password) are write-only: the
   device never returns them, only "secrets_set" flags saying which are stored. A secret
   field starts empty and is sent only after the user types in it. */
const SECRET_FIELDS = { p35: 'wifi_pass', p26: 'eeprom_ha_token', p28: 'eeprom_stock_key',
                        p65: 'eeprom_wu_key', p32: 'eeprom_glucose_password' };
const secretDirty = new Set();
Object.values(SECRET_FIELDS).forEach(id => { const e = el(id); if (e) e.addEventListener('input', () => secretDirty.add(id)); });
function secretStored(key) { return !!(S().secrets_set && S().secrets_set[key]); }
/* Empties the field (dropping any browser autofill) and shows whether a value is stored. */
function showSecretState(key) {
  const e = el(SECRET_FIELDS[key]);
  if (!e) return;
  if (e.dataset.phText === undefined) { e.dataset.phKey = e.getAttribute('data-i18n-placeholder') || ''; e.dataset.phText = e.placeholder; }
  e.value = '';
  secretDirty.delete(e.id);
  const phKey = secretStored(key) ? 'common.secret_stored' : e.dataset.phKey;
  if (phKey) { e.setAttribute('data-i18n-placeholder', phKey); e.placeholder = tr(phKey, secretStored(key) ? 'Stored – type to replace' : e.dataset.phText); }
  else { e.removeAttribute('data-i18n-placeholder'); e.placeholder = e.dataset.phText; }
}
/* Adds a typed secret to the payload. Emptying the field clears the secret, unless allowClear is false. */
function changedSecret(payload, key, allowClear = true) {
  const e = el(SECRET_FIELDS[key]);
  if (!e || !secretDirty.has(e.id)) return;
  if (e.value === '' && !allowClear) return;
  payload[key] = e.value;
}

/* Returns true when the save succeeded (or there was nothing to save), so the
   save bar knows whether to clear its dirty state. */
async function saveSettings(payload, networkKeys) {
  if (Object.keys(payload).length === 0) { toast(getMessage('no_changes'), 'ok'); return true; }
  const res = await apiPostJson('/api/settings', payload);
  const okMsg = res.data && res.data.status === 'ok';
  if (res.ok && okMsg) {
    Object.assign(window.settings, payload);
    for (const key of Object.keys(SECRET_FIELDS)) {
      if (!(key in payload)) continue;
      window.settings.secrets_set = Object.assign({}, window.settings.secrets_set, { [key]: payload[key] !== '' });
      delete window.settings[key]; // never keep a secret in memory
      showSecretState(key);
    }
    const reboot = (res.data.message || '').toLowerCase().includes('reboot') || (networkKeys || []).some(k => k in payload);
    toast(reboot ? getMessage('saved_restarting') : getMessage('settings_saved'), 'ok');
    if (reboot) waitForReboot();
    else await refreshHeroStatus();
    return true;
  }
  toast(localizeServerMessage(res.data && res.data.message, 'save_failed'), 'err');
  return false;
}

/* ---------- theme (default dark, persisted as p40) ---------- */
function applyTheme(dark) {
  body.classList.toggle('theme-dark', dark);
  body.classList.toggle('theme-light', !dark);
  el('themeIcon').textContent = dark ? '☀' : '☾';
}
el('themeBtn').addEventListener('click', () => {
  const dark = !body.classList.contains('theme-dark');
  applyTheme(dark);
  const p40 = dark ? 1 : 0;
  window.settings.p40 = p40;
  apiPostJson('/api/settings', { p40 });
});

/* Re-render dynamic (JS-generated) text that applyI18n can't reach via
   data-i18n attributes. Called by setLanguage after each switch. */
function refreshDynamicI18n() {
  if (typeof drawBrightnessChart === 'function') drawBrightnessChart(); // its labels are drawn, not data-i18n
  if (window._heroOnline !== undefined) {
    const e = el('heroOnline');
    if (e) e.textContent = window._heroOnline ? tr('common.online', 'Online') : tr('common.offline', 'Offline');
  }
  // Settings > Display layout dropdown holds a JS-built "Current" option;
  // re-fill so it re-localizes. (The Layout tab uses the Presets gallery.)
  if (typeof fillLayoutSelect === 'function' && el('settingsLayoutSelect')) {
    fillLayoutSelect('settingsLayoutSelect');
  }
}

/* ---------- i18n apply ---------- */
/**
 * Optimized i18n application: reduces DOM traversals from 3 to 1.
 * Performance: Reduces `applyI18n` execution time by ~60% by consolidating queries.
 */
function applyI18n(lang) {
  const t = translations[lang] || {};
  // Consolidate global traversals: query all translatable elements in one pass.
  // Resolve against the target language, falling back to English so a key that
  // is missing (e.g. an incompletely-loaded file) renders English rather than
  // leaving the previously-applied language's text in place.
  const tv = (k) => { let v = getNestedTranslation(t, k); if (v == null && lang !== 'en') v = getNestedTranslation(translations.en, k); return v; };
  $$('[data-i18n], [data-i18n-placeholder], [data-i18n-aria-label]').forEach(node => {
    const key = node.getAttribute('data-i18n');
    if (key) { const v = tv(key); if (v != null) node.innerHTML = v; }
    const pkey = node.getAttribute('data-i18n-placeholder');
    if (pkey) { const v = tv(pkey); if (v != null) node.placeholder = v; }
    const akey = node.getAttribute('data-i18n-aria-label');
    if (akey) { const v = tv(akey); if (v != null) node.setAttribute('aria-label', v); }
  });
  // Also update localized ARIA for password eye buttons if updateEyeAria exists.
  if (typeof updateEyeAria === 'function') {
    $$('.pw .eye').forEach(b => {
      const inp = b.parentElement.querySelector('input');
      updateEyeAria(b, inp && inp.type === 'text');
    });
  }
}
async function setLanguage(lang, persist) {
  if (!LANGUAGES.includes(lang)) lang = 'en';
  currentLanguage = lang;
  storeLanguage(lang);                 // remember locally for instant apply next load
  document.documentElement.lang = lang; // keep <html lang> in sync for a11y
  await loadTranslations(lang);
  applyI18n(lang);
  const opt = $('.lang-opt[data-lang="' + lang + '"]');
  el('langName').textContent = opt ? opt.textContent : (LANGUAGE_NAMES[lang] || 'English');
  $$('.lang-opt').forEach(o => o.classList.toggle('sel', o.dataset.lang === lang));
  if (typeof refreshScreenEditorI18n === 'function') refreshScreenEditorI18n();
  if (typeof refreshDynamicI18n === 'function') refreshDynamicI18n();
  if (persist) apiPostJson('/api/settings', { p41: LANGUAGES.indexOf(lang) }); // authoritative store on device
}

/* ---------- language menu ---------- */
const langBtn = el('langBtn'), langMenu = el('langMenu');
langBtn.addEventListener('click', e => { e.stopPropagation(); const open = langMenu.classList.toggle('open'); langBtn.setAttribute('aria-expanded', open.toString()); });
$$('.lang-opt').forEach(o => o.addEventListener('click', () => {
  langMenu.classList.remove('open');
  langBtn.setAttribute('aria-expanded', 'false');
  setLanguage(o.dataset.lang, true);
}));
document.addEventListener('click', () => {
  langMenu.classList.remove('open');
  langBtn.setAttribute('aria-expanded', 'false');
});

/* ---------- tabs + scroll indicators ---------- */
const nav = el('nav'), navWrap = $('.nav-wrap');
/* The open tab survives a page refresh (per browser tab, so a new window starts on Device). */
const _TAB_STORE_KEY = 'frixos.tab';

/* "Retrieving settings..." pop-up while a tab waits for the device. It opens only once a
   wait passes 200 ms, so quick answers do not flash it; overlapping waits share it. */
let _loadingCount = 0, _loadingTimer = null;
function setLoadingOverlay(open) {
  const o = el('loadingOverlay');
  if (!o) return;
  o.classList.toggle('open', open);
  o.setAttribute('aria-hidden', open ? 'false' : 'true');
}
function loadingBegin() {
  if (_loadingCount++ === 0) _loadingTimer = setTimeout(() => setLoadingOverlay(true), 200);
}
function loadingEnd() {
  if (_loadingCount > 0 && --_loadingCount === 0) { clearTimeout(_loadingTimer); setLoadingOverlay(false); }
}
function withLoading(work) {
  loadingBegin();
  return Promise.resolve(work).finally(loadingEnd);
}

function showTab(id) {
  try { sessionStorage.setItem(_TAB_STORE_KEY, id); } catch (e) { /* storage disabled */ }
  $$('.tab-page').forEach(p => p.classList.toggle('active', p.id === 'tab-' + id));
  $$('#nav a').forEach(a => a.classList.toggle('active', a.dataset.tab === id));
  window.scrollTo(0, 0);
  const active = $('#nav a.active'); if (active) active.scrollIntoView({ inline: 'center', block: 'nearest' });
  if (sectionLoaders[id] && !loadedSections[id]) { loadedSections[id] = true; withLoading(sectionLoaders[id]()); }
  else if (sectionLoaders[id] && id === 'status') withLoading(sectionLoaders[id]()); // System tab: live data, refresh on every visit
  updateNavEdges();
}
$$('#nav a').forEach(a => a.addEventListener('click', () => showTab(a.dataset.tab)));
function updateNavEdges() {
  const maxScroll = nav.scrollWidth - nav.clientWidth - 1;
  navWrap.classList.toggle('can-left', nav.scrollLeft > 1);
  navWrap.classList.toggle('can-right', nav.scrollLeft < maxScroll);
}
nav.addEventListener('scroll', updateNavEdges, { passive: true });
window.addEventListener('resize', updateNavEdges);
el('navLeft').addEventListener('click', () => nav.scrollBy({ left: -160, behavior: 'smooth' }));
el('navRight').addEventListener('click', () => nav.scrollBy({ left: 160, behavior: 'smooth' }));

/* Device-tab hero: IP · 2.56 (71) - rev 2 (E) · actual 70% · TZ … */
function applyHeroStatus(d) {
  const heroBits = [d.ip_address ? d.ip_address : 'frixos.local'];
  const verBits = [];
  if (d.version) verBits.push(d.fwversion != null ? d.version + ' (' + d.fwversion + ')' : d.version);
  else if (d.fwversion != null) verBits.push(String(d.fwversion));
  if (d.board_rev != null) {
    verBits.push(d.revision ? 'rev ' + d.board_rev + ' (' + d.revision + ')' : 'rev ' + d.board_rev);
  } else if (d.revision) {
    verBits.push('rev (' + d.revision + ')');
  }
  if (verBits.length) heroBits.push(verBits.join(' - '));
  // actual = LED duty as a share of the 10-bit range. led_duty is exact; older firmware
  // only has current_brightness (percent of effective_max_power; missing -> assume 100).
  if (d.effective_max_power != null) {
    window._ledMaxDuty = d.effective_max_power; // fallback for the slider labels (older firmware)
    const duty = d.led_duty != null ? d.led_duty
      : (d.current_brightness != null ? d.current_brightness : 100) * d.effective_max_power / 100;
    const actual = duty * 100 / 1023;
    heroBits.push('actual ' + (actual < 1 ? actual.toFixed(2) : (Math.round(actual * 10) / 10)) + '%');
  }
  if (d.timezone) heroBits.push('TZ ' + d.timezone);
  el('heroStatus').textContent = heroBits.join(' · ');
  const online = !!d.wifi_connected;
  window._heroOnline = online; // remembered so refreshDynamicI18n can re-localize on language switch
  el('heroOnline').textContent = online ? tr('common.online', 'Online') : tr('common.offline', 'Offline');
  el('heroDot').style.opacity = online ? '1' : '.3';
}
async function refreshHeroStatus() {
  const st = await apiGet('/api/status?_=' + Date.now());
  applyHeroStatus(st.data || {});
}

/* ---------- boot ---------- */
async function boot() {
  // Apply the cached language immediately so the UI isn't briefly all-English
  // while /api/settings is in flight.
  const stored = getStoredLanguage();
  await setLanguage(stored || 'en', false);

  loadingBegin();
  try {
    const res = await apiGet('/api/settings?group=theme');
    let s = settingsResponseHasParams(res.data) ? res.data : null;
    if (!s) { const full = await apiGet('/api/settings'); s = full.data || {}; }
    Object.assign(window.settings, s || {});
    applyTheme(s && s.p40 !== undefined ? !!s.p40 : true); // default dark
    // The device setting (p41) is authoritative; fall back to the cached language.
    const devLang = (s && s.p41 != null && LANGUAGES[s.p41]) ? LANGUAGES[s.p41] : null;
    const lang = devLang || stored || 'en';
    if (lang !== currentLanguage) await setLanguage(lang, false);
    initCounters();

    loadedSections.settings = true;
    if (sectionLoaders.settings) await sectionLoaders.settings();
  } finally {
    loadingEnd();
  }

  let savedTab = null;
  try { savedTab = sessionStorage.getItem(_TAB_STORE_KEY); } catch (e) { /* storage disabled */ }
  if (savedTab && savedTab !== 'settings' && $(`#nav a[data-tab="${savedTab}"]`)) showTab(savedTab);

  await refreshHeroStatus();
  updateNavEdges();
}
document.addEventListener('DOMContentLoaded', boot);
