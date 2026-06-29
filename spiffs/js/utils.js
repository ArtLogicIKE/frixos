/* ============================================================
   utils.js — shared helpers, formatters, i18n primitives
   Classic (non-module) script: top-level declarations are shared
   across all js/*.js files. Load order: utils → core → feature
   modules → screen-editor.
   ============================================================ */

const LANGUAGES = ['en', 'de', 'fr', 'it', 'pt', 'sv', 'da', 'pl', 'es'];
const LANGUAGE_NAMES = {
  en: 'English', de: 'Deutsch', fr: 'Français', it: 'Italiano', pt: 'Português',
  sv: 'Svenska', da: 'Dansk', pl: 'Polski', es: 'Español'
};

/* ---------- DOM ---------- */
const _domCache = new Map();
const el = (id) => {
  if (!id) return null;
  let e = _domCache.get(id);
  if (!e || !e.isConnected) { e = document.getElementById(id); if (e) _domCache.set(id, e); }
  return e;
};
const $ = (s, r = document) => r.querySelector(s);
const $$ = (s, r = document) => [...r.querySelectorAll(s)];

/* ---------- formatting ---------- */
const pad2 = n => String(n).padStart(2, '0');
const formatMinsToTimeString = m => { m = parseInt(m, 10) || 0; return pad2(Math.floor(m / 60) % 24) + ':' + pad2(m % 60); };
const parseTimeStringToMins = s => { const m = /^(\d{1,2}):(\d{2})$/.exec(s || ''); return m ? (parseInt(m[1], 10) * 60 + parseInt(m[2], 10)) : 0; };
function formatBytes(b, dec = 1) {
  b = +b || 0; if (b < 1024) return b + ' B';
  const k = 1024, u = ['KB', 'MB', 'GB'];
  const idx = Math.max(0, Math.floor(Math.log(b) / Math.log(k)) - 1);
  return (b / Math.pow(k, idx + 1)).toFixed(dec) + ' ' + u[Math.min(idx, u.length - 1)];
}
function formatUptime(s) { s = +s || 0; const d = Math.floor(s / 86400), h = Math.floor(s % 86400 / 3600), m = Math.floor(s % 3600 / 60), sec = s % 60; return (d ? d + 'd ' : '') + pad2(h) + ':' + pad2(m) + ':' + pad2(sec); }
function formatPOH(h) { h = +h || 0; const y = Math.floor(h / 8760), mo = Math.floor(h % 8760 / 730), d = Math.floor(h % 730 / 24), hr = h % 24; return [y && y + 'y', mo && mo + 'mo', d && d + 'd', hr + 'h'].filter(Boolean).join(' '); }
const MOON = ['New moon', 'Waxing crescent', 'First quarter', 'Waxing gibbous', 'Full moon', 'Waning gibbous', 'Last quarter', 'Waning crescent'];
function ago(ts) { if (!ts) return '—'; const diff = Math.max(0, Math.floor(Date.now() / 1000 - ts)); if (diff < 60) return diff + 's ago'; if (diff < 3600) return Math.floor(diff / 60) + ' min ago'; if (diff < 86400) return Math.floor(diff / 3600) + 'h ago'; return Math.floor(diff / 86400) + 'd ago'; }

/* ---------- button loading state ---------- */
function toggleLoading(btn, isLoading) {
  if (!btn) return;
  if (isLoading) { btn.classList.add('is-loading'); btn.disabled = true; btn.setAttribute('aria-busy', 'true'); }
  else { btn.classList.remove('is-loading'); btn.disabled = false; btn.removeAttribute('aria-busy'); }
}

/* ---------- i18n ----------
   The English base also serves as the fallback for getMessage() and for
   the layout editor's getNestedTranslation(...) || 'English' lookups.
   Language JSON files (/i18n/language_<lang>.json) are merged on demand. */
let currentLanguage = 'en';
const translations = {
  en: {
    common: { insert: 'Insert', show_password: 'Show password', hide_password: 'Hide password' },
    screen: { day: 'Day', night: 'Night', options: 'Options', enabled: 'Enabled' },
    messages: {
      error_loading_settings: 'Error loading settings',
      saving_settings: 'Saving settings…',
      settings_saved: 'Settings saved',
      error_saving_settings: 'Error saving settings: ',
      error_saving_unknown: 'Error saving settings. Check console for details.',
      layout_saved_to_file: 'Layout saved to file',
      layout_read_from_file: 'Layout loaded from file. Click Apply to apply to the device.',
      layout_read_invalid: 'Invalid layout file',
      layout_system_loaded: 'System layout loaded. Click Apply to apply to the device.',
      layout_system_select_file: 'Select a layout file first',
      layout_applied: 'Layout applied to the device',
      layout_apply_failed: 'Failed to apply layout: ',
      sending_reset: 'Sending reset command…',
      device_restarting: 'Device is restarting. This page will refresh in ',
      device_restarting_seconds: ' seconds.',
      failed_restart: 'Failed to restart device: ',
      error_reset_connection: 'Error sending reset command: Connection lost'
    }
  }
};

const _translationsLoaded = { en: true };
const _translationPromises = {};
function deepMergeTranslations(target, source) {
  if (!source || typeof source !== 'object') return target;
  Object.keys(source).forEach(k => {
    const v = source[k];
    if (v && typeof v === 'object' && !Array.isArray(v)) {
      if (!target[k] || typeof target[k] !== 'object') target[k] = {};
      deepMergeTranslations(target[k], v);
    } else { target[k] = v; }
  });
  return target;
}
/**
 * Loads translation JSON for a given language.
 * Performance: Uses a fetch promise cache to prevent redundant concurrent network
 * requests for the same language (O(1) network efficiency for concurrent calls).
 */
async function loadTranslations(lang) {
  if (_translationsLoaded[lang]) return;
  if (_translationPromises[lang]) return _translationPromises[lang];
  _translationPromises[lang] = (async () => {
    try {
      const r = await fetch('/i18n/language_' + lang + '.json');
      if (r.ok) {
        const data = await r.json();
        if (lang === 'en') deepMergeTranslations(translations.en, data);
        else translations[lang] = deepMergeTranslations(JSON.parse(JSON.stringify(translations.en)), data);
      }
    } catch (e) { /* keep english */ }
    _translationsLoaded[lang] = true;
    delete _translationPromises[lang];
  })();
  return _translationPromises[lang];
}

const _pathCache = new Map();
function getNestedTranslation(obj, path) {
  let parts = _pathCache.get(path);
  if (!parts) { parts = path.split('.'); _pathCache.set(path, parts); }
  let cur = obj;
  for (let i = 0; i < parts.length; i++) { cur = cur && cur[parts[i]]; if (cur == null) break; }
  return cur;
}
function getMessage(key, ...args) {
  let m = getNestedTranslation(translations[currentLanguage], 'messages.' + key);
  if (!m) m = getNestedTranslation(translations.en, 'messages.' + key) || key;
  return args.length ? m + args.join('') : m;
}

/* CGM mutual exclusivity (Dexcom / Libre / Nightscout) */
function updateCgmExclusivity() {
  const dexcom = el('eeprom_dexcom_region'), libre = el('eeprom_libre_region'), nsUrl = el('eeprom_ns_url');
  if (!dexcom || !libre || !nsUrl) return;
  const d = dexcom.value !== '0', l = libre.value !== '0', n = nsUrl.value.trim() !== '';
  libre.disabled = d || n; nsUrl.disabled = d || l; dexcom.disabled = l || n;
}

/* ---------- character counters ---------- */
function updateCharCounter(input, counter) {
  if (!input || !counter) return;
  const len = input.value.length;
  const max = input.maxLength;
  if (max < 0) return;
  counter.textContent = len + ' / ' + max;
  counter.classList.toggle('warn', len >= max * 0.9 && len < max);
  counter.classList.toggle('max', len >= max);
}
function initCounters() {
  $$('[maxlength]').forEach(input => {
    const counter = el(input.id + '-counter');
    if (counter) {
      input.setAttribute('aria-describedby', (input.getAttribute('aria-describedby') || '') + ' ' + counter.id);
      counter.setAttribute('aria-live', 'polite');
      input.addEventListener('input', () => updateCharCounter(input, counter));
      updateCharCounter(input, counter);
    }
  });
}
