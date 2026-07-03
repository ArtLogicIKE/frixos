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
function renderNets(nets) {
  if (!nets.length) { el('netlist').innerHTML = '<div class="net-empty">' + tr('settings.wifi.no_networks', 'No networks found.') + '</div>'; return; }
  nets.sort((a, b) => (b.signal_strength || 0) - (a.signal_strength || 0));
  el('netlist').innerHTML = nets.map(n => `<div class="net" data-ssid="${escHtml(n.ssid || '')}">${n.requires_password ? '<span class="lock"></span>' : ''}<span class="nm">${escHtml(n.ssid || '')}</span><span class="meta">${bars(n.signal_strength)}</span></div>`).join('');
  $$('#netlist .net').forEach(node => node.addEventListener('click', () => {
    $$('#netlist .net').forEach(x => x.classList.remove('sel')); node.classList.add('sel');
    el('wifi_ssid').value = node.dataset.ssid; el('wifi_pass').focus();
    if (window.saveBar) window.saveBar.markDirty(); // programmatic set: no input event fires
  }));
}
