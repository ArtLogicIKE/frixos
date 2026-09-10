/* ============================================================
   savebar.js — unified sticky save bar with dirty tracking.
   Watches the Device + Integrations tabs and shows a floating
   "unsaved changes" bar; Save merges the payload collectors
   from forms.js / integrations.js into a single POST. The
   Layout editor keeps its own Apply flow and is not tracked.
   Depends on core.js (saveSettings), forms.js, integrations.js.
   ============================================================ */

(() => {
  const bar = el('saveBar');
  if (!bar) return;
  let dirty = false;
  const MOBILE = '(max-width: 560px)';

  /* Keep the bar inside the visual viewport so iOS zoom / the on-screen
     keyboard cannot push Save off-screen. */
  function pinBar() {
    if (!dirty || bar.hidden) return;
    const vv = window.visualViewport;
    if (!vv) return;
    const inset = Math.max(0, window.innerHeight - vv.offsetTop - vv.height);
    if (window.matchMedia(MOBILE).matches) {
      bar.style.left = vv.offsetLeft + 'px';
      bar.style.width = vv.width + 'px';
      bar.style.right = 'auto';
      bar.style.bottom = inset + 'px';
      bar.style.transform = 'none';
    } else {
      bar.style.left = (vv.offsetLeft + vv.width / 2) + 'px';
      bar.style.width = '';
      bar.style.right = 'auto';
      bar.style.bottom = (inset + 18) + 'px';
      bar.style.transform = 'translateX(-50%)';
    }
  }
  function clearPin() {
    bar.style.left = '';
    bar.style.width = '';
    bar.style.right = '';
    bar.style.bottom = '';
    bar.style.transform = '';
  }

  function markDirty() {
    if (dirty) return;
    dirty = true;
    bar.hidden = false;
    document.body.classList.add('has-savebar');
    requestAnimationFrame(pinBar);
  }
  function clearDirty() {
    dirty = false;
    bar.hidden = true;
    document.body.classList.remove('has-savebar');
    clearPin();
  }
  window.saveBar = { markDirty, clearDirty, isDirty: () => dirty };

  const vv = window.visualViewport;
  if (vv) {
    vv.addEventListener('resize', pinBar);
    vv.addEventListener('scroll', pinBar);
  }
  window.addEventListener('resize', pinBar);

  /* Any edit to a form control inside the watched tabs counts as dirty.
     Programmatic value changes don't fire these events; the code paths that
     set values on the user's behalf (WiFi network pick, city search) call
     saveBar.markDirty() themselves. */
  ['tab-settings', 'tab-integrations'].forEach(tabId => {
    const sec = el(tabId);
    if (!sec) return;
    ['input', 'change'].forEach(ev => sec.addEventListener(ev, e => {
      const t = e.target;
      if (!t || !t.tagName) return;
      const tag = t.tagName;
      if (tag !== 'INPUT' && tag !== 'SELECT' && tag !== 'TEXTAREA') return;
      if (t.id === 'city_search') return;   // helper field, not a setting
      if (t.type === 'file') return;
      markDirty();
    }));
  });

  el('saveAllBtn').addEventListener('click', async () => {
    const sp = collectSettingsPayload();
    if (sp === null) return; // hostname validation failed; user was focused there
    const ap = collectAdvancedPayload();
    // Only include tabs whose values have actually been loaded into the DOM —
    // otherwise HTML defaults would be diffed against stored settings.
    const ip = loadedSections.integrations ? collectIntegrationsPayload() : {};
    const payload = Object.assign({}, sp, ap, ip);
    const btn = el('saveAllBtn');
    toggleLoading(btn, true);
    const ok = await saveSettings(payload, SETTINGS_NETWORK_KEYS);
    toggleLoading(btn, false);
    if (ok) {
      clearDirty();
      if (typeof updateIntegrationDots === 'function') updateIntegrationDots();
    }
  });

  el('discardBtn').addEventListener('click', async () => {
    const btn = el('discardBtn');
    toggleLoading(btn, true);
    try {
      await loadSettings();
      await loadAdvanced();
      if (loadedSections.integrations) await loadIntegrations();
    } finally {
      toggleLoading(btn, false);
    }
    clearDirty();
    toast(getMessage('changes_discarded'), 'ok');
  });
})();
