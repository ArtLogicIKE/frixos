/* ============================================================
   wifi.js — WiFi scan on the Settings tab. Depends on core.js.
   ============================================================ */

let _scanPoll;
el('scanBtn').addEventListener('click', async () => {
  el('netlist').innerHTML = '<div class="spinner"></div><div class="net-empty">' + tr('settings.wifi.scanning', 'Scanning for networks…') + '</div>';
  clearInterval(_scanPoll);
  const start = await apiGet('/api/wifi/scan');
  if (!(start.data && start.data.status === 'ok')) { el('netlist').innerHTML = '<div class="net-empty">' + tr('settings.wifi.scan_failed', 'Scan failed.') + '</div>'; return; }
  let tries = 0;
  _scanPoll = setInterval(async () => {
    tries++;
    const st = await apiGet('/api/wifi/status');
    const d = st.data || {};
    if ((!d.scanning && d.scan_done) || tries > 20) { clearInterval(_scanPoll); renderNets(d.networks || []); }
  }, 1000);
});

const bars = pct => { const lv = Math.max(1, Math.min(4, Math.ceil((pct || 0) / 25))); return '<span class="sig s' + lv + '"><i></i><i></i><i></i><i></i></span>'; };
/* SSIDs are attacker-controlled (anyone can broadcast one) — escape before
   they touch innerHTML. */
const escHtml = s => String(s).replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));

function selectNetwork(node) {
  $$('#netlist .net').forEach(x => x.classList.remove('sel'));
  node.classList.add('sel');
  el('wifi_ssid').value = node.dataset.ssid;
  el('wifi_pass').focus();
  if (window.saveBar) window.saveBar.markDirty(); // programmatic set: no input event fires
}

function renderNets(nets) {
  if (!nets.length) { el('netlist').innerHTML = '<div class="net-empty">' + tr('settings.wifi.no_networks', 'No networks found.') + '</div>'; return; }
  nets.sort((a, b) => (b.signal_strength || 0) - (a.signal_strength || 0));

  const signalLabel = tr('settings.wifi.signal', 'Signal strength');
  const secureLabel = tr('settings.wifi.secure', 'Secure');
  const openLabel = tr('settings.wifi.open', 'Open');

  el('netlist').innerHTML = nets.map(n => {
    const ssid = escHtml(n.ssid || '');
    const pct = n.signal_strength || 0;
    const isSecure = !!n.requires_password;
    const ariaLabel = `${ssid}, ${isSecure ? secureLabel : openLabel}, ${signalLabel} ${pct}%`;

    return `<div class="net" role="button" tabindex="0" data-ssid="${ssid}" aria-label="${ariaLabel}">` +
           `${isSecure ? '<span class="lock"></span>' : ''}` +
           `<span class="nm">${ssid}</span>` +
           `<span class="meta">${bars(pct)}</span></div>`;
  }).join('');

  $$('#netlist .net').forEach(node => {
    node.addEventListener('click', () => selectNetwork(node));
    node.addEventListener('keydown', e => {
      if (e.key === 'Enter' || e.key === ' ') {
        e.preventDefault();
        selectNetwork(node);
      }
    });
  });
}
