/* Frixos Device UI — interactions (mock firmware behaviour) */
(function () {
  'use strict';
  const $ = (s, r = document) => r.querySelector(s);
  const $$ = (s, r = document) => [...r.querySelectorAll(s)];
  const LS = 'frixos.ui';
  const store = JSON.parse(localStorage.getItem(LS) || '{}');
  const persist = () => localStorage.setItem(LS, JSON.stringify(store));

  /* ---------- THEME ---------- */
  const body = document.body, themeIcon = $('#themeIcon');
  function applyTheme(t) {
    body.classList.toggle('theme-dark', t === 'dark');
    body.classList.toggle('theme-light', t !== 'dark');
    themeIcon.textContent = t === 'dark' ? '☀' : '☾';
  }
  applyTheme(store.theme || 'light');
  $('#themeBtn').addEventListener('click', () => {
    store.theme = body.classList.contains('theme-dark') ? 'light' : 'dark';
    applyTheme(store.theme); persist();
  });

  /* ---------- TABS ---------- */
  function showTab(id) {
    $$('.tab-page').forEach(p => p.classList.toggle('active', p.id === 'tab-' + id));
    $$('#nav a').forEach(a => a.classList.toggle('active', a.dataset.tab === id));
    store.tab = id; persist();
    window.scrollTo(0, 0);
  }
  $$('#nav a').forEach(a => a.addEventListener('click', () => showTab(a.dataset.tab)));
  if (store.tab && $('#tab-' + store.tab)) showTab(store.tab);

  /* ---------- LANGUAGE ---------- */
  const langBtn = $('#langBtn'), langMenu = $('#langMenu');
  langBtn.addEventListener('click', e => {
    e.stopPropagation();
    const open = langMenu.classList.toggle('open');
    langBtn.setAttribute('aria-expanded', open);
  });
  $$('.lang-opt').forEach(o => o.addEventListener('click', () => {
    $$('.lang-opt').forEach(x => x.classList.remove('sel'));
    o.classList.add('sel'); $('#langName').textContent = o.textContent;
    langMenu.classList.remove('open'); toast('Language set to ' + o.textContent);
  }));
  document.addEventListener('click', () => langMenu.classList.remove('open'));

  /* ---------- TOGGLES / PASSWORD / SAVE ---------- */
  $$('.gswitch').forEach(g => { if (g.id !== 'staticToggle') g.addEventListener('click', () => g.classList.toggle('on')); });
  $$('.pw .eye').forEach(b => b.addEventListener('click', () => {
    const inp = b.parentElement.querySelector('input');
    inp.type = inp.type === 'password' ? 'text' : 'password';
    b.style.opacity = inp.type === 'text' ? '1' : '';
  }));
  let toastT;
  function toast(msg, kind) {
    const t = $('#toast'); t.textContent = msg; t.className = 'toast show' + (kind ? ' ' + kind : '');
    clearTimeout(toastT); toastT = setTimeout(() => t.classList.remove('show'), 2400);
  }
  $$('[data-save]').forEach(b => b.addEventListener('click', () => {
    const host = $('#hostname');
    if (b.closest('#tab-settings') && host && !host.value.trim()) {
      host.setAttribute('aria-invalid', 'true'); $('#hostname-err').classList.add('show'); host.focus(); return;
    }
    toast(b.dataset.save, 'ok');
  }));
  const host = $('#hostname');
  if (host) host.addEventListener('input', () => {
    if (host.value.trim()) { host.removeAttribute('aria-invalid'); $('#hostname-err').classList.remove('show'); }
  });

  /* ---------- STATIC IP ---------- */
  const staticToggle = $('#staticToggle'), staticPanel = $('#staticPanel');
  staticToggle.addEventListener('click', () => {
    staticToggle.classList.toggle('on');
    staticPanel.hidden = !staticToggle.classList.contains('on');
  });

  /* ---------- DIM MODE ---------- */
  const dimMode = $('#dimMode');
  function applyDim() {
    $('#dimBright').hidden = dimMode.value !== 'bright';
    $('#dimTime').hidden = dimMode.value !== 'time';
  }
  dimMode.addEventListener('change', applyDim); applyDim();

  /* ---------- MISC BUTTONS ---------- */
  const simpleBtns = {
    citySearch: 'Searching cities…', geolocate: 'Locating…', loadSystem: 'System layout loaded',
    restoreDefault: 'Default layout restored', saveFile: 'Saved to file', loadFile: 'Loaded from file',
    reinstall: 'Reinstalling current firmware…', refreshStatus: 'Status refreshed'
  };
  Object.entries(simpleBtns).forEach(([id, msg]) => { const b = $('#' + id); if (b) b.addEventListener('click', () => toast(msg, 'ok')); });

  /* ---------- WIFI SCAN ---------- */
  const NETS = [
    { n: 'HomeNet_5G', s: 4, lock: true, conn: true }, { n: 'TP-Link_A4F2', s: 3, lock: true },
    { n: 'NETGEAR-Office', s: 2, lock: true }, { n: 'Frixos_Setup', s: 3, lock: false }, { n: 'Cafe_Guest', s: 1, lock: false }
  ];
  const bars = s => `<span class="sig s${s}"><i></i><i></i><i></i><i></i></span>`;
  function renderNets() {
    $('#netlist').innerHTML = NETS.map(x => `
      <div class="net${x.conn ? ' sel' : ''}">${x.lock ? '<span class="lock"></span>' : ''}
        <span class="nm">${x.n}</span>${x.conn ? '<span class="conn">Connected</span>' : ''}
        <span class="meta">${bars(x.s)}${x.conn ? '<span class="check">✓</span>' : ''}</span></div>`).join('');
    $$('#netlist .net').forEach((el, i) => el.addEventListener('click', () => {
      $$('#netlist .net').forEach(n => n.classList.remove('sel')); el.classList.add('sel');
      const ssid = $('#wifi_ssid'); if (ssid) ssid.value = NETS[i].n;
    }));
  }
  $('#scanBtn').addEventListener('click', () => {
    $('#netlist').innerHTML = '<div class="spinner"></div><div class="net-empty">Scanning for networks…</div>';
    setTimeout(renderNets, 1100);
  });

  /* ---------- FILES ---------- */
  let FILES = [
    { name: 'config.json', size: 2048 }, { name: 'day.layout', size: 1180 }, { name: 'night.layout', size: 1180 },
    { name: 'logo.jpg', size: 1142 }, { name: 'user-font.jpg', size: 8800 }, { name: 'timezone.txt', size: 320 }
  ];
  let sortKey = 'name', sortAsc = true;
  const fmtSize = b => b < 1024 ? b + ' B' : (b / 1024).toFixed(1) + ' KB';
  function renderFiles() {
    FILES.sort((a, b) => { const r = sortKey === 'size' ? a.size - b.size : a.name.localeCompare(b.name); return sortAsc ? r : -r; });
    $('#filesBody').innerHTML = FILES.map((f, i) => `<tr data-i="${i}"><td class="cbx"><input type="checkbox" class="fcb"></td><td>${f.name}</td><td class="size">${fmtSize(f.size)}</td></tr>`).join('');
    $$('.fcb').forEach(cb => cb.addEventListener('change', () => { cb.closest('tr').classList.toggle('sel', cb.checked); updFileBtns(); }));
    $('#filesSummary').textContent = FILES.length + ' files · ' + fmtSize(FILES.reduce((s, f) => s + f.size, 0)) + ' total';
  }
  function updFileBtns() {
    const n = $$('.fcb:checked').length;
    $('#filesDelete').disabled = n === 0; $('#filesRename').disabled = n !== 1;
    $('#selAll').checked = n === FILES.length && n > 0;
  }
  $('#selAll').addEventListener('change', e => {
    $$('.fcb').forEach(cb => { cb.checked = e.target.checked; cb.closest('tr').classList.toggle('sel', e.target.checked); }); updFileBtns();
  });
  $$('th[data-sort] button').forEach(b => b.addEventListener('click', () => {
    const k = b.dataset.sort; if (k === sortKey) sortAsc = !sortAsc; else { sortKey = k; sortAsc = true; }
    $$('th[data-sort] button').forEach(x => { x.textContent = x.textContent.replace(/[ ↑↓]+$/, ''); if (x.dataset.sort === sortKey) x.textContent += sortAsc ? ' ↑' : ' ↓'; });
    renderFiles();
  }));
  $('#filesRefresh').addEventListener('click', () => { renderFiles(); toast('Files refreshed', 'ok'); });
  $('#filesDelete').addEventListener('click', () => {
    const keep = $$('#filesBody tr').filter(tr => !tr.querySelector('.fcb').checked).map(tr => +tr.dataset.i);
    FILES = keep.map(i => FILES[i]); renderFiles(); updFileBtns(); toast('Selected files deleted', 'ok');
  });
  $('#filesRename').addEventListener('click', () => toast('Rename — pick a new name'));
  renderFiles();

  /* ---------- UPDATE ---------- */
  const fwFile = $('#fwFile'), uploadBtn = $('#uploadBtn');
  fwFile.addEventListener('change', () => { uploadBtn.disabled = !fwFile.files.length; });
  uploadBtn.addEventListener('click', () => {
    const wrap = $('#progressWrap'), fill = $('#progressFill'), txt = $('#progressTxt');
    wrap.style.display = 'block'; uploadBtn.disabled = true; let p = 0;
    const iv = setInterval(() => {
      p = Math.min(100, p + Math.random() * 14); fill.style.width = p + '%'; txt.textContent = Math.round(p) + '%';
      if (p >= 100) { clearInterval(iv); txt.textContent = 'Done — restarting…'; toast('Upload complete', 'ok'); uploadBtn.disabled = false; }
    }, 260);
  });

  /* ---------- RESTART MODAL ---------- */
  const modal = $('#resetModal');
  $('#restartBtn').addEventListener('click', () => modal.classList.add('open'));
  $('#cancelReset').addEventListener('click', () => modal.classList.remove('open'));
  $('#confirmReset').addEventListener('click', () => { modal.classList.remove('open'); toast('Device restarting…', 'ok'); });
  modal.addEventListener('click', e => { if (e.target === modal) modal.classList.remove('open'); });

  /* ===================== SCREEN / LAYOUT EDITOR ===================== */
  const SCALE = 3, SIZE = 128;
  const DEF = {
    glucose_level: { label: 'Glucose Level', w: 14, h: 20, img: 'default-glucose.jpg' },
    glucose_trend: { label: 'Glucose Trend', w: 12, h: 14, img: 'default-trend.jpg' },
    wifi_off: { label: 'WiFi off', w: 20, h: 20, img: 'default-wifi-off.jpg' },
    weather: { label: 'Weather', w: 32, h: 22, img: 'default-weather.jpg' },
    moon: { label: 'Moon', w: 14, h: 14, img: 'default-moon.jpg' },
    time: { label: 'Digit Display', w: 80, h: 36, img: 'bold.jpg' },
    digit_label: { label: 'Digit Label', w: 80, h: 8, text: true },
    time_aux: { label: 'Digit Display (Aux)', w: 80, h: 36, img: 'bold.jpg' },
    digit_label_aux: { label: 'Digit Label (Aux)', w: 80, h: 8, text: true },
    ampm: { label: 'AM/PM', w: 10, h: 20, img: 'default-ampm.jpg' },
    message: { label: 'Scrolling Message', w: 128, h: 8, text: true },
    text_1: { label: 'Text 1', w: 80, h: 8, text: true }, text_2: { label: 'Text 2', w: 80, h: 8, text: true },
    text_3: { label: 'Text 3', w: 80, h: 8, text: true }, text_4: { label: 'Text 4', w: 80, h: 8, text: true },
    text_5: { label: 'Text 5', w: 80, h: 8, text: true }, text_6: { label: 'Text 6', w: 80, h: 8, text: true },
    text_7: { label: 'Text 7', w: 80, h: 8, text: true }, text_8: { label: 'Text 8', w: 80, h: 8, text: true }
  };
  const TEXTDEF = { message: 'Customize your frixos projection clock', text_1: 'UV [uv]', text_2: '[pressure]', text_3: 'Wind [wind]', text_4: 'Gust [gust]', text_5: 'Rain [precip]' };
  const PALETTE = ['time', 'digit_label', 'ampm', 'message', 'time_aux', 'digit_label_aux', 'weather', 'moon', 'wifi_off', 'glucose_level', 'glucose_trend', 'text'];
  const TEXT_SLOTS = ['text_1', 'text_2', 'text_3', 'text_4', 'text_5', 'text_6', 'text_7', 'text_8'];
  const FONTS = [['0', '8pt'], ['1', '10pt'], ['2', '12pt'], ['3', '14pt'], ['4', '16pt']];
  const ALIGN = [['0', 'Left'], ['1', 'Center'], ['2', 'Right']];

  function makeDefaultProfile() {
    return [
      { id: 'glucose_level', x: 27, y: 27, z: 4 }, { id: 'glucose_trend', x: 42, y: 30, z: 4 },
      { id: 'wifi_off', x: 22, y: 27, z: 4 }, { id: 'weather', x: 54, y: 24, z: 3 }, { id: 'moon', x: 87, y: 29, z: 3 },
      { id: 'time', x: 22, y: 47, z: 1 }, { id: 'ampm', x: 101, y: 54, z: 2 },
      { id: 'message', x: 0, y: 86, z: 0, opt: { font: 0, color: '#ffffff', align: 0, text: TEXTDEF.message } }
    ].map(e => ({ x: 0, y: 0, z: 0, opt: {}, ...e }));
  }
  const editor = store.layout || { mode: 'day', grid: false, day: makeDefaultProfile(), night: makeDefaultProfile() };
  let selId = null;
  const profile = () => editor[editor.mode];
  const saveLayout = () => { store.layout = editor; persist(); };

  function elText(e) { return (e.opt && e.opt.text != null) ? e.opt.text : (TEXTDEF[e.id] || ''); }

  function renderCanvas() {
    const c = $('#canvas'); if (!c) return;
    c.classList.toggle('grid-on', editor.grid);
    c.querySelectorAll('.el').forEach(n => n.remove());
    profile().slice().sort((a, b) => (a.z || 0) - (b.z || 0)).forEach(e => {
      const d = DEF[e.id]; if (!d) return;
      const node = document.createElement('div');
      node.className = 'el' + (e.id === selId ? ' sel' : '');
      node.dataset.id = e.id;
      node.style.cssText = `left:${e.x * SCALE}px;top:${e.y * SCALE}px;width:${d.w * SCALE}px;height:${d.h * SCALE}px;z-index:${(e.z || 0) + 1}`;
      if (d.text) {
        const t = document.createElement('div'); t.className = 'eltext';
        t.style.fontSize = Math.round(d.h * SCALE * 0.82) + 'px';
        t.style.color = (e.opt && e.opt.color) || '#ffffff';
        const al = e.opt && e.opt.align; t.style.justifyContent = al === 1 ? 'center' : al === 2 ? 'flex-end' : 'flex-start';
        t.textContent = elText(e) || '(empty)';
        node.appendChild(t);
      } else {
        const img = document.createElement('img'); img.src = 'img/' + d.img; img.alt = ''; node.appendChild(img);
      }
      c.appendChild(node);
      attachDrag(node, e);
    });
  }

  function status(txt) { const s = $('#screenStatus'); if (s) s.textContent = txt; }
  function statusFor(e) {
    if (!e) { status('Selected: <None> @ (—,—)'); return; }
    const d = DEF[e.id];
    status(`Selected: ${d.label} @ (${e.x},${e.y}) size (${d.w},${d.h})`);
  }

  function select(id) { selId = id; renderCanvas(); renderOptions(); statusFor(profile().find(e => e.id === id)); }

  function attachDrag(node, e) {
    let sx, sy, ox, oy, moved;
    node.addEventListener('pointerdown', ev => {
      ev.preventDefault(); select(e.id);
      sx = ev.clientX; sy = ev.clientY; ox = e.x; oy = e.y; moved = false;
      node.setPointerCapture(ev.pointerId); node.classList.add('dragging');
    });
    node.addEventListener('pointermove', ev => {
      if (sx === undefined || !node.classList.contains('dragging')) return;
      const dx = Math.round((ev.clientX - sx) / SCALE), dy = Math.round((ev.clientY - sy) / SCALE);
      if (Math.abs(dx) + Math.abs(dy) > 0) moved = true;
      const d = DEF[e.id];
      e.x = Math.max(0, Math.min(SIZE - d.w, ox + dx));
      e.y = Math.max(0, Math.min(SIZE - d.h, oy + dy));
      node.style.left = e.x * SCALE + 'px'; node.style.top = e.y * SCALE + 'px';
      statusFor(e); syncOptPos(e);
    });
    node.addEventListener('pointerup', ev => {
      node.classList.remove('dragging'); sx = undefined;
      if (moved) saveLayout();
    });
  }

  function syncOptPos(e) {
    const px = $('#optX'), py = $('#optY'); if (px) px.value = e.x; if (py) py.value = e.y;
  }

  function renderPalette() {
    const host = $('#paletteItems'); if (!host) return;
    const present = id => profile().some(e => e.id === id);
    const items = PALETTE.flatMap(id => {
      if (id === 'text') {
        const free = TEXT_SLOTS.find(s => !present(s));
        return free ? [{ id: free, label: 'New Text Line', add: true }] : [];
      }
      return [{ id, label: DEF[id].label, add: !present(id) }];
    });
    host.innerHTML = items.map(it => {
      const d = DEF[it.id];
      const thumb = d.text ? `<span class="thumb txt">T</span>` : `<span class="thumb"><img src="img/${d.img}" alt=""></span>`;
      return `<div class="pchip" data-id="${it.id}" draggable="true">${thumb}<span class="plabel">${it.label}</span><span class="padd">${it.add ? '+' : '✎'}</span></div>`;
    }).join('');
    $$('#paletteItems .pchip').forEach(chip => {
      chip.addEventListener('click', () => addOrSelect(chip.dataset.id));
      chip.addEventListener('dragstart', ev => ev.dataTransfer.setData('text/id', chip.dataset.id));
    });
  }

  function addOrSelect(id, x, y) {
    let e = profile().find(el => el.id === id);
    if (!e) {
      const d = DEF[id];
      e = { id, x: x != null ? Math.max(0, Math.min(SIZE - d.w, x)) : Math.round((SIZE - d.w) / 2),
        y: y != null ? Math.max(0, Math.min(SIZE - d.h, y)) : Math.round((SIZE - d.h) / 2), z: 2,
        opt: d.text ? { font: 0, color: '#ffffff', align: 0, text: TEXTDEF[id] || '' } : {} };
      profile().push(e); saveLayout(); renderPalette();
    }
    select(id);
  }

  function renderOptions() {
    const host = $('#screenOptions'); if (!host) return;
    const e = profile().find(el => el.id === selId);
    if (!e) { host.innerHTML = '<p class="muted-p" style="margin:0">Select an element to edit its options.</p>'; return; }
    const d = DEF[e.id];
    const layerOpts = [['0', '1 (back)'], ['1', '2'], ['2', '3'], ['3', '4'], ['4', '5 (front)']];
    let h = `<div class="opt-title"><span>${d.label}</span><button class="opt-del" id="optDel">Remove</button></div><div class="opt-grid">`;
    h += `<div class="field"><label>X</label><input type="number" id="optX" min="0" max="${SIZE - d.w}" value="${e.x}"></div>`;
    h += `<div class="field"><label>Y</label><input type="number" id="optY" min="0" max="${SIZE - d.h}" value="${e.y}"></div>`;
    h += `<div class="field"><label>Layer</label><select id="optZ">${layerOpts.map(o => `<option value="${o[0]}"${(e.z || 0) == o[0] ? ' selected' : ''}>${o[1]}</option>`).join('')}</select></div>`;
    if (d.text) {
      h += `<div class="field"><label>Font</label><select id="optFont">${FONTS.map(o => `<option value="${o[0]}"${(e.opt.font || 0) == o[0] ? ' selected' : ''}>${o[1]}</option>`).join('')}</select></div>`;
      h += `<div class="field"><label>Align</label><select id="optAlign">${ALIGN.map(o => `<option value="${o[0]}"${(e.opt.align || 0) == o[0] ? ' selected' : ''}>${o[1]}</option>`).join('')}</select></div>`;
      h += `<div class="field"><label>Color</label><div class="color-row"><input type="color" id="optColor" value="${e.opt.color || '#ffffff'}"><span class="swatch-val">${(e.opt.color || '#ffffff').toUpperCase()}</span></div></div>`;
      h += `</div><div class="field" style="margin-top:12px"><label>Text</label><input type="text" id="optText" value="${(elText(e)).replace(/"/g, '&quot;')}"></div>`;
    } else { h += `</div>`; }
    host.innerHTML = h;

    $('#optX').addEventListener('input', ev => { e.x = +ev.target.value || 0; renderCanvas(); statusFor(e); saveLayout(); });
    $('#optY').addEventListener('input', ev => { e.y = +ev.target.value || 0; renderCanvas(); statusFor(e); saveLayout(); });
    $('#optZ').addEventListener('change', ev => { e.z = +ev.target.value; renderCanvas(); saveLayout(); });
    $('#optDel').addEventListener('click', () => {
      editor[editor.mode] = profile().filter(el => el.id !== e.id); selId = null;
      renderCanvas(); renderPalette(); renderOptions(); statusFor(null); saveLayout();
    });
    if (d.text) {
      $('#optFont').addEventListener('change', ev => { e.opt.font = +ev.target.value; saveLayout(); });
      $('#optAlign').addEventListener('change', ev => { e.opt.align = +ev.target.value; renderCanvas(); saveLayout(); });
      $('#optColor').addEventListener('input', ev => { e.opt.color = ev.target.value; ev.target.nextElementSibling.textContent = ev.target.value.toUpperCase(); renderCanvas(); saveLayout(); });
      $('#optText').addEventListener('input', ev => { e.opt.text = ev.target.value; renderCanvas(); saveLayout(); });
    }
  }

  // canvas drop from palette
  const canvas = $('#canvas');
  if (canvas) {
    canvas.addEventListener('dragover', ev => ev.preventDefault());
    canvas.addEventListener('drop', ev => {
      ev.preventDefault();
      const id = ev.dataTransfer.getData('text/id'); if (!id) return;
      const rect = canvas.getBoundingClientRect();
      const d = DEF[id];
      addOrSelect(id, Math.round((ev.clientX - rect.left) / SCALE - d.w / 2), Math.round((ev.clientY - rect.top) / SCALE - d.h / 2));
    });
    canvas.addEventListener('pointerdown', ev => { if (ev.target === canvas) { selId = null; renderCanvas(); renderOptions(); statusFor(null); } });
  }

  // mode toggle
  function setMode(m) {
    editor.mode = m; selId = null;
    $('#modeDay').classList.toggle('active', m === 'day');
    $('#modeNight').classList.toggle('active', m === 'night');
    renderCanvas(); renderPalette(); renderOptions(); statusFor(null); saveLayout();
  }
  $('#modeDay').addEventListener('click', () => setMode('day'));
  $('#modeNight').addEventListener('click', () => setMode('night'));
  $('#copyNight').addEventListener('click', () => { editor.night = JSON.parse(JSON.stringify(editor.day)); saveLayout(); if (editor.mode === 'night') { renderCanvas(); renderPalette(); } toast('Day layout copied to Night', 'ok'); });
  $('#saveLayout').addEventListener('click', () => { saveLayout(); toast('Layout saved', 'ok'); });
  $('#restoreDefault').addEventListener('click', () => { editor.day = makeDefaultProfile(); editor.night = makeDefaultProfile(); selId = null; renderCanvas(); renderPalette(); renderOptions(); statusFor(null); saveLayout(); });

  // init editor
  $('#modeDay').classList.toggle('active', editor.mode === 'day');
  $('#modeNight').classList.toggle('active', editor.mode === 'night');
  renderCanvas(); renderPalette(); statusFor(null);
})();
