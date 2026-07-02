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

function selectNetwork(node) {
  $$('#netlist .net').forEach(x => x.classList.remove('sel'));
  node.classList.add('sel');
  el('wifi_ssid').value = node.dataset.ssid;
  el('wifi_pass').focus();
}

function renderNets(nets) {
  if (!nets.length) { el('netlist').innerHTML = '<div class="net-empty">' + tr('settings.wifi.no_networks', 'No networks found.') + '</div>'; return; }
  nets.sort((a, b) => (b.signal_strength || 0) - (a.signal_strength || 0));
  el('netlist').innerHTML = nets.map(n => {
    const ssid = n.ssid || '';
    const secure = n.requires_password ? tr('settings.wifi.secure', 'Secure') : tr('settings.wifi.open', 'Open');
    const signal = tr('settings.wifi.signal', 'Signal strength') + ': ' + (n.signal_strength || 0) + '%';
    const label = `${ssid}, ${secure}, ${signal}`;
    return `<div class="net" role="button" tabindex="0" aria-label="${label.replace(/"/g, '&quot;')}" data-ssid="${ssid.replace(/"/g, '&quot;')}">${n.requires_password ? '<span class="lock" aria-hidden="true"></span>' : ''}<span class="nm" aria-hidden="true">${ssid}</span><span class="meta" aria-hidden="true">${bars(n.signal_strength)}</span></div>`;
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
