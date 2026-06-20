/* ============================================================
   wifi.js — WiFi scan on the Settings tab. Depends on core.js.
   ============================================================ */

let _scanPoll;
el('scanBtn').addEventListener('click', async () => {
  el('netlist').innerHTML = '<div class="spinner"></div><div class="net-empty">Scanning for networks…</div>';
  clearInterval(_scanPoll);
  const start = await apiGet('/api/wifi/scan');
  if (!(start.data && start.data.status === 'ok')) { el('netlist').innerHTML = '<div class="net-empty">Scan failed.</div>'; return; }
  let tries = 0;
  _scanPoll = setInterval(async () => {
    tries++;
    const st = await apiGet('/api/wifi/status');
    const d = st.data || {};
    if ((!d.scanning && d.scan_done) || tries > 20) { clearInterval(_scanPoll); renderNets(d.networks || []); }
  }, 1000);
});

const bars = pct => { const lv = Math.max(1, Math.min(4, Math.ceil((pct || 0) / 25))); return '<span class="sig s' + lv + '"><i></i><i></i><i></i><i></i></span>'; };
function renderNets(nets) {
  if (!nets.length) { el('netlist').innerHTML = '<div class="net-empty">No networks found.</div>'; return; }
  nets.sort((a, b) => (b.signal_strength || 0) - (a.signal_strength || 0));
  el('netlist').innerHTML = nets.map(n => `<div class="net" data-ssid="${(n.ssid || '').replace(/"/g, '&quot;')}">${n.requires_password ? '<span class="lock"></span>' : ''}<span class="nm">${n.ssid || ''}</span><span class="meta">${bars(n.signal_strength)}</span></div>`).join('');
  $$('#netlist .net').forEach(node => node.addEventListener('click', () => {
    $$('#netlist .net').forEach(x => x.classList.remove('sel')); node.classList.add('sel');
    el('wifi_ssid').value = node.dataset.ssid; el('wifi_pass').focus();
  }));
}
