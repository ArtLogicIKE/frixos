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
function changedSecret(payload, key, newVal) { const o = S()[key]; if (o === undefined) { if (newVal) payload[key] = newVal; return; } if (String(o) !== String(newVal)) payload[key] = newVal; }

async function saveSettings(payload, networkKeys) {
  if (Object.keys(payload).length === 0) { toast('No changes to save', 'ok'); return; }
  const res = await apiPostJson('/api/settings', payload);
  const okMsg = res.data && res.data.status === 'ok';
  if (res.ok && okMsg) {
    Object.assign(window.settings, payload);
    const reboot = (res.data.message || '').toLowerCase().includes('reboot') || (networkKeys || []).some(k => k in payload);
    toast(reboot ? 'Saved — device restarting…' : 'Settings saved', 'ok');
    if (reboot) { let n = 8; const recheck = () => { if (--n <= 0) location.reload(); else setTimeout(recheck, 1000); }; setTimeout(recheck, 1000); }
  } else {
    toast((res.data && res.data.message) || 'Save failed', 'err');
  }
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
  apiPostJson('/api/settings', { p40: dark ? 1 : 0 });
});

/* ---------- i18n apply ---------- */
function applyI18n(lang) {
  const t = translations[lang] || {};
  $$('[data-i18n]').forEach(node => { const v = getNestedTranslation(t, node.getAttribute('data-i18n')); if (v != null) node.innerHTML = v; });
  $$('[data-i18n-placeholder]').forEach(node => { const v = getNestedTranslation(t, node.getAttribute('data-i18n-placeholder')); if (v != null) node.placeholder = v; });
  $$('[data-i18n-aria-label]').forEach(node => {
    const v = getNestedTranslation(t, node.getAttribute('data-i18n-aria-label'));
    if (v != null) node.setAttribute('aria-label', v);
  });
  $$('[data-i18n-title]').forEach(node => { const v = getNestedTranslation(t, node.getAttribute('data-i18n-title')); if (v != null) node.setAttribute('title', v); });
}
async function setLanguage(lang, persist) {
  currentLanguage = lang;
  await loadTranslations(lang);
  applyI18n(lang);
  const opt = $('.lang-opt[data-lang="' + lang + '"]');
  el('langName').textContent = opt ? opt.textContent : (LANGUAGE_NAMES[lang] || 'English');
  $$('.lang-opt').forEach(o => o.classList.toggle('sel', o.dataset.lang === lang));
  if (typeof refreshScreenEditorI18n === 'function') refreshScreenEditorI18n();
  if (persist) apiPostJson('/api/settings', { p41: LANGUAGES.indexOf(lang) });
}

/* ---------- language menu ---------- */
const langBtn = el('langBtn'), langMenu = el('langMenu');
langBtn.addEventListener('click', e => { e.stopPropagation(); const open = langMenu.classList.toggle('open'); langBtn.setAttribute('aria-expanded', open); });
$$('.lang-opt').forEach(o => o.addEventListener('click', () => { langMenu.classList.remove('open'); setLanguage(o.dataset.lang, true); }));
document.addEventListener('click', () => langMenu.classList.remove('open'));

/* ---------- tabs + scroll indicators ---------- */
const nav = el('nav'), navWrap = $('.nav-wrap');
function showTab(id) {
  $$('.tab-page').forEach(p => p.classList.toggle('active', p.id === 'tab-' + id));
  $$('#nav a').forEach(a => a.classList.toggle('active', a.dataset.tab === id));
  window.scrollTo(0, 0);
  const active = $('#nav a.active'); if (active) active.scrollIntoView({ inline: 'center', block: 'nearest' });
  if (sectionLoaders[id] && !loadedSections[id]) { loadedSections[id] = true; sectionLoaders[id](); }
  else if (sectionLoaders[id] && (id === 'status' || id === 'files')) sectionLoaders[id]();
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

/* ---------- boot ---------- */
async function boot() {
  const res = await apiGet('/api/settings?group=theme');
  let s = settingsResponseHasParams(res.data) ? res.data : null;
  if (!s) { const full = await apiGet('/api/settings'); s = full.data || {}; }
  Object.assign(window.settings, s || {});
  applyTheme(s && s.p40 !== undefined ? !!s.p40 : true); // default dark
  const lang = (s && s.p41 != null && LANGUAGES[s.p41]) ? LANGUAGES[s.p41] : 'en';
  await setLanguage(lang, false);

  loadedSections.settings = true;
  if (sectionLoaders.settings) await sectionLoaders.settings();

  const st = await apiGet('/api/status'); const d = st.data || {};
  el('heroStatus').textContent = (d.ip_address ? d.ip_address : 'frixos.local') + (d.version ? ' · v' + d.version : '');
  const online = !!d.wifi_connected;
  el('heroOnline').textContent = online ? 'Online' : 'Offline';
  el('heroDot').style.opacity = online ? '1' : '.3';
  updateNavEdges();
}
document.addEventListener('DOMContentLoaded', boot);
