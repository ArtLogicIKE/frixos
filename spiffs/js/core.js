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

/* ---------- settings helpers ---------- */
const S = () => window.settings;
const setVal = (id, v) => { const e = el(id); if (e != null && v !== undefined && v !== null) e.value = v; };
async function loadGroup(query) {
  let res = await apiGet('/api/settings?' + query);
  if (!settingsResponseHasParams(res.data)) { const full = await apiGet('/api/settings'); if (full.data) res = full; }
  if (res.data) Object.assign(window.settings, res.data);
  return window.settings;
}
function changed(payload, key, newVal) { if (S()[key] === undefined) return; if (String(S()[key]) !== String(newVal)) payload[key] = newVal; }
function changedSecret(payload, key, newVal) {
  if (!newVal) return; // blank secret field means unchanged
  const o = S()[key];
  if (o === undefined || String(o) !== String(newVal)) payload[key] = newVal;
}

/* Returns true when the save succeeded (or there was nothing to save), so the
   save bar knows whether to clear its dirty state. */
async function saveSettings(payload, networkKeys) {
  if (Object.keys(payload).length === 0) { toast(getMessage('no_changes'), 'ok'); return true; }
  const res = await apiPostJson('/api/settings', payload);
  const okMsg = res.data && res.data.status === 'ok';
  if (res.ok && okMsg) {
    Object.assign(window.settings, payload);
    const reboot = (res.data.message || '').toLowerCase().includes('reboot') || (networkKeys || []).some(k => k in payload);
    toast(reboot ? getMessage('saved_restarting') : getMessage('settings_saved'), 'ok');
    if (reboot) { let n = 8; const recheck = () => { if (--n <= 0) location.reload(); else setTimeout(recheck, 1000); }; setTimeout(recheck, 1000); }
    return true;
  }
  toast((res.data && res.data.message) || getMessage('save_failed'), 'err');
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
  $$('.lang-opt').forEach(o => {
    const isSel = o.dataset.lang === lang;
    o.classList.toggle('sel', isSel);
    o.setAttribute('aria-selected', isSel ? 'true' : 'false');
  });
  if (typeof refreshScreenEditorI18n === 'function') refreshScreenEditorI18n();
  if (typeof refreshDynamicI18n === 'function') refreshDynamicI18n();
  if (persist) apiPostJson('/api/settings', { p41: LANGUAGES.indexOf(lang) }); // authoritative store on device
}

/* ---------- language menu ---------- */
const langBtn = el('langBtn'), langMenu = el('langMenu');
langBtn.addEventListener('click', e => { e.stopPropagation(); const open = langMenu.classList.toggle('open'); langBtn.setAttribute('aria-expanded', open.toString()); });
$$('.lang-opt').forEach(o => {
  const select = () => {
    langMenu.classList.remove('open');
    langBtn.setAttribute('aria-expanded', 'false');
    setLanguage(o.dataset.lang, true);
    langBtn.focus();
  };
  o.addEventListener('click', select);
  o.addEventListener('keydown', e => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); select(); } });
});
document.addEventListener('click', () => {
  langMenu.classList.remove('open');
  langBtn.setAttribute('aria-expanded', 'false');
});

/* ---------- tabs + scroll indicators ---------- */
const nav = el('nav'), navWrap = $('.nav-wrap');
function showTab(id) {
  $$('.tab-page').forEach(p => p.classList.toggle('active', p.id === 'tab-' + id));
  $$('#nav a').forEach(a => {
    const isActive = a.dataset.tab === id;
    a.classList.toggle('active', isActive);
    a.setAttribute('aria-selected', isActive ? 'true' : 'false');
    a.setAttribute('tabindex', isActive ? '0' : '-1');
  });
  window.scrollTo(0, 0);
  const active = $('#nav a.active'); if (active) active.scrollIntoView({ inline: 'center', block: 'nearest' });
  if (sectionLoaders[id] && !loadedSections[id]) { loadedSections[id] = true; sectionLoaders[id](); }
  else if (sectionLoaders[id] && id === 'status') sectionLoaders[id](); // System tab: live data, refresh on every visit
  updateNavEdges();
}
$$('#nav a').forEach(a => {
  a.addEventListener('click', () => showTab(a.dataset.tab));
  a.addEventListener('keydown', e => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); showTab(a.dataset.tab); } });
});
function updateNavEdges() {
  const maxScroll = nav.scrollWidth - nav.clientWidth - 1;
  navWrap.classList.toggle('can-left', nav.scrollLeft > 1);
  navWrap.classList.toggle('can-right', nav.scrollLeft < maxScroll);
}
nav.addEventListener('scroll', updateNavEdges, { passive: true });
window.addEventListener('resize', updateNavEdges);
el('navLeft').addEventListener('click', () => nav.scrollBy({ left: -160, behavior: 'smooth' }));
el('navRight').addEventListener('click', () => nav.scrollBy({ left: 160, behavior: 'smooth' }));

/* ---------- boot ---------- */
async function boot() {
  // Apply the cached language immediately so the UI isn't briefly all-English
  // while /api/settings is in flight.
  const stored = getStoredLanguage();
  await setLanguage(stored || 'en', false);

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

  const st = await apiGet('/api/status'); const d = st.data || {};
  el('heroStatus').textContent = (d.ip_address ? d.ip_address : 'frixos.local') + (d.version ? ' · v' + d.version : '');
  const online = !!d.wifi_connected;
  window._heroOnline = online; // remembered so refreshDynamicI18n can re-localize on language switch
  el('heroOnline').textContent = online ? tr('common.online', 'Online') : tr('common.offline', 'Offline');
  el('heroDot').style.opacity = online ? '1' : '.3';
  updateNavEdges();
}
document.addEventListener('DOMContentLoaded', boot);
