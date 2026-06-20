/* ============================================================
   system.js — Status, Files, Update, Restart tabs. Depends on core.js.
   ============================================================ */

/* ---------- STATUS ---------- */
async function loadStatus() {
  const res = await apiGet('/api/status?logs=1');
  const d = res.data; if (!d) return;
  const txt = (id, v) => { const e = el(id); if (e) e.textContent = (v === undefined || v === null || v === '') ? '—' : v; };
  txt('lux', d.lux != null ? Number(d.lux).toFixed(1) : '—');
  txt('last_time_update', d.last_time_update ? ago(d.last_time_update) : '—');
  txt('timezone_val', d.timezone);
  txt('moon_status', MOON[d.moon_icon_index] || '—');
  txt('last_weather_update', d.last_weather_update ? ago(d.last_weather_update) : '—');
  txt('latitude', d.latitude); txt('longitude', d.longitude);
  txt('uptime', d.uptime != null ? formatUptime(d.uptime) : '—');
  txt('app', d.app); txt('version', d.version); txt('fwversion', d.fwversion);
  txt('poh', d.poh != null ? formatPOH(d.poh) : '—');
  txt('mac_address', (d.mac_address || '').replace(/(..)(?=.)/g, '$1:'));
  txt('ip_address', d.ip_address); txt('chip_revision', d.chip_revision);
  txt('flash_size', d.flash_size != null ? formatBytes(d.flash_size) : '—');
  txt('cpu_freq', d.cpu_freq != null ? (d.cpu_freq / 1e6) + ' MHz' : '—');
  txt('compile_time', d.compile_time);
  txt('free_heap', d.free_heap != null ? formatBytes(d.free_heap) : '—');
  txt('min_free_heap', d.min_free_heap != null ? formatBytes(d.min_free_heap) : '—');
  if (el('system_logs')) el('system_logs').value = Array.isArray(d.system_logs) ? d.system_logs.join('\n') : '';
  if (el('ha_status_textarea')) el('ha_status_textarea').value = Array.isArray(d.ha_tokens) ? d.ha_tokens.join('\n') : '';
  window.statusData = d;
}
sectionLoaders.status = loadStatus;
el('refreshStatus').addEventListener('click', () => { loadStatus(); toast('Status refreshed', 'ok'); });
el('supportCopy').addEventListener('click', async () => {
  try { await navigator.clipboard.writeText(JSON.stringify(window.statusData || {}, null, 2)); toast('Copied to clipboard', 'ok'); }
  catch (e) { toast('Copy failed', 'err'); }
});
el('supportEmail').addEventListener('click', () => {
  const b = encodeURIComponent('Frixos diagnostics:\n\n' + JSON.stringify(window.statusData || {}, null, 2));
  location.href = 'mailto:support@buyfrixos.com?subject=Frixos%20support&body=' + b;
});

/* ---------- FILES ---------- */
let FILES = [], sortKey = 'name', sortAsc = true;
async function loadFiles() {
  const res = await apiGet('/api/files');
  FILES = (res.data && res.data.files) || [];
  renderFiles();
}
sectionLoaders.files = loadFiles;
function renderFiles() {
  FILES.sort((a, b) => { const r = sortKey === 'size' ? a.size - b.size : a.name.localeCompare(b.name); return sortAsc ? r : -r; });
  el('filesBody').innerHTML = FILES.map(f => `<tr><td class="cbx"><input type="checkbox" class="fcb" data-name="${(f.name || '').replace(/"/g, '&quot;')}"></td><td><a href="/${encodeURIComponent(f.name)}" download>${f.name}</a></td><td class="size">${formatBytes(f.size)}</td></tr>`).join('');
  $$('.fcb').forEach(cb => cb.addEventListener('change', () => { cb.closest('tr').classList.toggle('sel', cb.checked); updFileBtns(); }));
  el('filesSummary').textContent = FILES.length + ' files · ' + formatBytes(FILES.reduce((s, f) => s + (f.size || 0), 0)) + ' total';
  el('selAll').checked = false; updFileBtns();
}
function selectedNames() { return $$('.fcb:checked').map(cb => cb.dataset.name); }
function updFileBtns() { const n = selectedNames().length; el('filesDelete').disabled = n === 0; el('filesRename').disabled = n !== 1; el('selAll').checked = n === FILES.length && n > 0; }
el('selAll').addEventListener('change', e => { $$('.fcb').forEach(cb => { cb.checked = e.target.checked; cb.closest('tr').classList.toggle('sel', e.target.checked); }); updFileBtns(); });
$$('th[data-sort] button').forEach(b => b.addEventListener('click', () => {
  const k = b.dataset.sort; if (k === sortKey) sortAsc = !sortAsc; else { sortKey = k; sortAsc = true; }
  $$('th[data-sort] button').forEach(x => { x.textContent = x.textContent.replace(/[ ↑↓]+$/, ''); if (x.dataset.sort === sortKey) x.textContent += sortAsc ? ' ↑' : ' ↓'; });
  renderFiles();
}));
el('filesRefresh').addEventListener('click', () => { loadFiles(); toast('Files refreshed', 'ok'); });
el('filesDelete').addEventListener('click', async () => {
  const names = selectedNames(); if (!names.length) return;
  const res = await apiPostJson('/api/files/delete', { files: names });
  if (res.data && res.data.status === 'ok') { toast('Deleted ' + (res.data.deleted || names.length) + ' file(s)', 'ok'); loadFiles(); }
  else toast('Delete failed', 'err');
});
el('filesRename').addEventListener('click', async () => {
  const names = selectedNames(); if (names.length !== 1) return;
  const newName = prompt('Rename "' + names[0] + '" to:', names[0]); if (!newName || newName === names[0]) return;
  const res = await apiPostJson('/api/files/rename', { oldName: names[0], newName });
  if (res.data && res.data.status === 'ok') { toast('File renamed', 'ok'); loadFiles(); }
  else toast((res.data && res.data.message) || 'Rename failed', 'err');
});

/* ---------- UPDATE ---------- */
const fwFile = el('fwFile'), uploadBtn = el('uploadBtn');
fwFile.addEventListener('change', () => { uploadBtn.disabled = !fwFile.files.length; });
uploadBtn.addEventListener('click', () => {
  const file = fwFile.files[0]; if (!file) return;
  const wrap = el('progressWrap'), fill = el('progressFill'), txt = el('progressTxt');
  wrap.style.display = 'block'; uploadBtn.disabled = true;
  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/api/ota', true);
  xhr.setRequestHeader('X-Filename', file.name);
  xhr.upload.addEventListener('progress', e => { if (e.lengthComputable) { const p = Math.round(e.loaded / e.total * 100); fill.style.width = p + '%'; txt.textContent = p + '%'; } });
  xhr.addEventListener('load', () => {
    let d = {}; try { d = JSON.parse(xhr.responseText); } catch (e) { }
    if (xhr.status >= 200 && xhr.status < 300 && d.status === 'ok') {
      if ((d.message || '').toLowerCase().includes('reboot')) { txt.textContent = 'Done — restarting…'; toast('Firmware uploaded — restarting', 'ok'); setTimeout(() => location.reload(), 8000); }
      else { txt.textContent = 'Done'; toast(d.message || 'Upload complete', 'ok'); }
    } else { toast((d.message) || 'Upload failed', 'err'); }
    uploadBtn.disabled = false;
  });
  xhr.addEventListener('error', () => { toast('Upload failed', 'err'); uploadBtn.disabled = false; });
  xhr.send(file);
});
el('reinstall').addEventListener('click', async () => {
  const res = await apiPostJson('/api/ota/reinstall', {});
  if (res.data && res.data.status === 'ok') toast('Reinstall started', 'ok');
  else toast((res.data && res.data.message) || 'Reinstall failed', 'err');
});

/* ---------- RESTART ---------- */
const modal = el('resetModal');
el('restartBtn').addEventListener('click', () => modal.classList.add('open'));
el('cancelReset').addEventListener('click', () => modal.classList.remove('open'));
modal.addEventListener('click', e => { if (e.target === modal) modal.classList.remove('open'); });
el('confirmReset').addEventListener('click', async () => {
  modal.classList.remove('open');
  await apiPostJson('/api/reset', {});
  toast('Device restarting…', 'ok');
  setTimeout(() => location.reload(), 8000);
});
