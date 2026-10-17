/**
 * Load the real SPIFFS UI scripts in a small DOM and exercise every control
 * that posts to /api/settings. Prints one JSON line: RESULT {...}
 *
 * No browser and no device. tools/test_ui_settings.py is the assertion side.
 */
import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const spiffs = path.join(root, 'spiffs');

const VOID_TAGS = new Set(['AREA', 'BASE', 'BR', 'COL', 'EMBED', 'HR', 'IMG', 'INPUT', 'LINK', 'META', 'SOURCE', 'TRACK', 'WBR']);

function attachClassList(node, initial) {
  const set = new Set(initial || []);
  node.classList = {
    add(...xs) { xs.forEach(x => { if (x) set.add(x); }); },
    remove(...xs) { xs.forEach(x => set.delete(x)); },
    toggle(name, force) {
      const on = force === undefined ? !set.has(name) : !!force;
      if (on) set.add(name); else set.delete(name);
      return on;
    },
    contains: (name) => set.has(name),
  };
  Object.defineProperty(node, 'className', {
    get() { return [...set].join(' '); },
    set(v) {
      set.clear();
      String(v || '').split(/\s+/).filter(Boolean).forEach(c => set.add(c));
    },
  });
}

function createNode(tag) {
  const node = {
    tagName: String(tag || 'DIV').toUpperCase(),
    id: '',
    children: [],
    parentElement: null,
    parentNode: null,
    isConnected: false,
    attributes: {},
    dataset: {},
    style: {},
    hidden: false,
    disabled: false,
    checked: false,
    selected: false,
    files: [],
    textContent: '',
    placeholder: '',
    _html: '',
    _type: '',
    _listeners: {},
    maxLength: -1,
  };
  attachClassList(node, []);
  if (node.tagName === 'CANVAS') {
    node.getContext = kind => kind === '2d' ? {
      font: '',
      measureText(text) {
        const size = Number((this.font.match(/([0-9.]+)px/) || [])[1]) || 16;
        return {
          width: [...String(text)].length * size * 0.5,
          actualBoundingBoxAscent: size * 0.7,
          actualBoundingBoxDescent: size * 0.2
        };
      }
    } : null;
  }
  let explicit = null;
  Object.defineProperty(node, 'value', {
    get() {
      if (node.tagName === 'SELECT') {
        const opts = node.children.filter(c => c.tagName === 'OPTION');
        const sel = opts.find(o => o.selected);
        if (sel) return sel.value;
        if (explicit != null) return explicit;
        return opts[0] ? opts[0].value : '';
      }
      return explicit == null ? '' : explicit;
    },
    set(v) {
      explicit = v == null ? '' : String(v);
      if (node.tagName !== 'SELECT') return;
      for (const o of node.children) {
        if (o.tagName !== 'OPTION') continue;
        o.selected = o.value === explicit;
      }
    },
  });
  Object.defineProperty(node, 'type', {
    get() { return node._type; },
    set(v) { node._type = String(v); node.attributes.type = node._type; },
  });
  Object.defineProperty(node, 'innerHTML', {
    get() { return node._html; },
    set(v) {
      node._html = String(v);
      if (node._html === '') {
        for (const c of node.children) c.isConnected = false;
        node.children.length = 0;
      }
    },
  });
  node.setAttribute = (k, v) => {
    const key = String(k).toLowerCase();
    node.attributes[key] = String(v);
    if (key.startsWith('data-')) {
      const dk = key.slice(5).replace(/-([a-z])/g, (_, c) => c.toUpperCase());
      node.dataset[dk] = String(v);
    }
  };
  node.getAttribute = (k) => {
    const key = String(k).toLowerCase();
    return Object.prototype.hasOwnProperty.call(node.attributes, key) ? node.attributes[key] : null;
  };
  node.removeAttribute = (k) => { delete node.attributes[String(k).toLowerCase()]; };
  node.addEventListener = (ev, fn) => { (node._listeners[ev] ||= []).push(fn); };
  node.removeEventListener = () => {};
  node.appendChild = (child) => {
    if (child.parentNode) {
      const oldIndex = child.parentNode.children.indexOf(child);
      if (oldIndex >= 0) child.parentNode.children.splice(oldIndex, 1);
    }
    child.parentElement = node;
    child.parentNode = node;
    child.isConnected = true;
    node.children.push(child);
    return child;
  };
  node.insertBefore = (child, reference) => {
    if (child.parentNode) {
      const oldIndex = child.parentNode.children.indexOf(child);
      if (oldIndex >= 0) child.parentNode.children.splice(oldIndex, 1);
    }
    const index = node.children.indexOf(reference);
    child.parentElement = node;
    child.parentNode = node;
    child.isConnected = true;
    node.children.splice(index < 0 ? node.children.length : index, 0, child);
    return child;
  };
  node.focus = () => {};
  node.click = () => {};
  node.scrollBy = () => {};
  node.scrollIntoView = () => {};
  node.scrollWidth = 0;
  node.clientWidth = 0;
  node.scrollLeft = 0;
  node.querySelector = (sel) => queryAll(node, sel)[0] || null;
  node.querySelectorAll = (sel) => queryAll(node, sel);
  node.closest = (sel) => {
    let n = node;
    while (n) {
      if (n.tagName && matchSimple(n, sel)) return n;
      n = n.parentElement;
    }
    return null;
  };
  return node;
}

function tokenizeSimple(sel) {
  return [...sel.matchAll(/#[\w-]+|\.[\w-]+|\[[^\]]+\]|[a-zA-Z][\w-]*/g)].map(m => m[0]);
}

function matchSimple(el, sel) {
  const tokens = tokenizeSimple(sel);
  if (!tokens.length) return false;
  for (const t of tokens) {
    if (t[0] === '#') {
      if (el.id !== t.slice(1)) return false;
    } else if (t[0] === '.') {
      if (!el.classList.contains(t.slice(1))) return false;
    } else if (t[0] === '[') {
      const body = t.slice(1, -1);
      const eq = body.match(/^([\w:-]+)\s*=\s*"?([^"]*)"?$/);
      if (eq) {
        if (String(el.getAttribute(eq[1]) ?? '') !== eq[2]) return false;
      } else if (el.getAttribute(body) == null) return false;
    } else if (el.tagName !== t.toUpperCase()) return false;
  }
  return true;
}

function walk(node, fn) {
  for (const c of node.children) {
    fn(c);
    walk(c, fn);
  }
}

function queryAll(root, selector) {
  const found = [];
  for (const group of selector.split(',').map(s => s.trim()).filter(Boolean)) {
    const parts = group.split(/\s+/);
    let nodes = [root];
    for (const part of parts) {
      const next = [];
      for (const n of nodes) {
        walk(n, child => { if (matchSimple(child, part)) next.push(child); });
      }
      nodes = next;
    }
    found.push(...nodes);
  }
  return [...new Set(found)];
}

function parseAttrs(s) {
  const attrs = {};
  const re = /([:@\w-]+)(?:\s*=\s*(?:"([^"]*)"|'([^']*)'|(\S+)))?/g;
  let m;
  while ((m = re.exec(s))) attrs[m[1].toLowerCase()] = m[2] ?? m[3] ?? m[4] ?? '';
  return attrs;
}

function parseHtml(html) {
  const doc = createNode('document');
  const stack = [doc];
  const re = /<!--[\s\S]*?-->|<\/([a-zA-Z0-9]+)>|<([a-zA-Z0-9]+)([^>]*?)(\/?)>/g;
  let m;
  while ((m = re.exec(html))) {
    if (m[0].startsWith('<!--')) continue;
    if (m[1]) {
      const tag = m[1].toUpperCase();
      while (stack.length > 1 && stack[stack.length - 1].tagName !== tag) stack.pop();
      if (stack.length > 1) stack.pop();
      continue;
    }
    const tag = m[2].toUpperCase();
    const attrs = parseAttrs(m[3] || '');
    const el = createNode(tag);
    if (attrs.id) el.id = attrs.id;
    if (attrs.class) el.className = attrs.class;
    if (attrs.type) el.type = attrs.type;
    if (attrs.maxlength != null && attrs.maxlength !== '') el.maxLength = parseInt(attrs.maxlength, 10);
    if (Object.prototype.hasOwnProperty.call(attrs, 'hidden')) el.hidden = true;
    if (Object.prototype.hasOwnProperty.call(attrs, 'disabled')) el.disabled = true;
    if (Object.prototype.hasOwnProperty.call(attrs, 'selected')) el.selected = true;
    for (const [k, v] of Object.entries(attrs)) el.setAttribute(k, v);
    if (attrs.value != null && tag !== 'SELECT') el.value = attrs.value;
    stack[stack.length - 1].appendChild(el);
    if (!m[4] && !VOID_TAGS.has(tag)) stack.push(el);
  }
  return doc;
}

const html = fs.readFileSync(path.join(spiffs, 'index.html'), 'utf8');
const document = parseHtml(html);
document.documentElement = document.children.find(c => c.tagName === 'HTML') || document;
document.body = walkBody(document);
document.getElementById = (id) => {
  let found = null;
  walk(document, n => { if (!found && n.id === id) found = n; });
  if (!found) {
    found = createNode('div');
    found.id = id;
    (document.body || document).appendChild(found);
  }
  return found;
};
document.querySelector = (sel) => queryAll(document, sel)[0] || null;
document.querySelectorAll = (sel) => queryAll(document, sel);
document.createElement = (tag) => createNode(tag);
document.addEventListener = (ev, fn) => { (document._listeners[ev] ||= []).push(fn); };

function walkBody(doc) {
  let body = null;
  walk(doc, n => { if (!body && n.tagName === 'BODY') body = n; });
  return body;
}

const state = { baseline: {}, posts: [] };

function jsonResponse(data, ok = true) {
  return {
    ok,
    status: ok ? 200 : 404,
    async text() { return JSON.stringify(data); },
    async json() { return data; },
    async arrayBuffer() { return new ArrayBuffer(0); },
  };
}

async function fetchImpl(url, opts) {
  const u = String(url);
  if (opts && String(opts.method || '').toUpperCase() === 'POST') {
    let body = opts.body;
    if (typeof body === 'string') {
      try { body = JSON.parse(body); } catch { /* binary or plain text */ }
    }
    state.posts.push({ url: u, body });
    return jsonResponse({ status: 'ok', message: 'Settings saved' });
  }
  if (u.includes('/api/settings')) return jsonResponse(structuredClone(state.baseline));
  if (u.includes('/api/files')) return jsonResponse({ files: [] });
  if (u.includes('/api/status')) return jsonResponse({ wifi_connected: true });
  return jsonResponse({}, false);
}

const sandbox = {
  console,
  setTimeout,
  clearTimeout,
  setInterval,
  clearInterval,
  queueMicrotask,
  structuredClone,
  AbortController,
  Image: class {},
  TextEncoder,
  TextDecoder,
  getComputedStyle: () => ({
    fontFamily: '', fontSize: '', fontWeight: '', lineHeight: '', letterSpacing: '',
    wordSpacing: '', padding: '', borderWidth: '', borderStyle: '', boxSizing: '', tabSize: ''
  }),
  URL,
  URLSearchParams,
  Intl,
  document,
  navigator: { language: 'en-US', clipboard: { async writeText() {} } },
  location: { reload() {} },
  fetch: fetchImpl,
  defaultLayoutData: fs.readFileSync(path.join(spiffs, 'layouts', 'Default.layout'), 'utf8'),
  weatherLayoutData: fs.readFileSync(path.join(spiffs, 'layouts', 'Weather.layout'), 'utf8'),
  state,
  localStorage: (() => {
    const m = new Map();
    return {
      getItem: (k) => (m.has(k) ? m.get(k) : null),
      setItem: (k, v) => m.set(k, String(v)),
      removeItem: (k) => m.delete(k),
    };
  })(),
};
sandbox.window = sandbox;
sandbox.globalThis = sandbox;
sandbox.self = sandbox;
sandbox.visualViewport = {
  offsetTop: 0, offsetLeft: 0, height: 800, width: 1280,
  addEventListener() {},
};
sandbox.innerHeight = 800;
sandbox.innerWidth = 1280;
sandbox.matchMedia = () => ({ matches: false, addEventListener() {}, removeEventListener() {} });
sandbox.requestAnimationFrame = (fn) => { fn(); return 1; };
sandbox.cancelAnimationFrame = () => {};
sandbox.scrollTo = () => {};
sandbox.addEventListener = () => {};
sandbox.removeEventListener = () => {};

vm.createContext(sandbox);

const scripts = [
  'js/utils.js', 'js/core.js', 'js/forms.js', 'js/wifi.js', 'js/integrations.js',
  'js/system.js', 'js/support.js', 'js/savebar.js', 'js/screen-editor.js',
];
let source = scripts.map(rel => fs.readFileSync(path.join(spiffs, rel), 'utf8')).join('\n;\n');

source += `
globalThis.__done = (async () => {
  waitForReboot = () => {};
  async function fire(node, evName) {
    if (!node) throw new Error('missing node for ' + evName);
    const ev = { target: node, currentTarget: node, key: '', preventDefault() {}, stopPropagation() {} };
    for (const fn of node._listeners[evName] || []) await fn(ev);
  }
  const result = {};

  state.baseline = {
    p00: 'frixos', p34: 'OldSSID', p03: 0, p09: 0, p36: 0, p37: 0, p39: 0,
    p40: 1, p41: 0, p60: '', p61: '', p62: '', p63: '',
    p17: '37.9838100', p18: '23.7275400', p19: 'GMT0', tz_iana: 'Etc/UTC',
    p46: 480, p47: 0, p22: 1, p21: 12, p20: 3, p23: [80, 25],
    p42: 133, p43: 1023, p55: 420, p56: 1335,
    p25: 'http://old.local', p26: 'old-token', p27: 1, p28: 'old-key', p29: 5,
    p64: 'OLDSTATION', p65: 'old-wu-key', p66: 15,
    p30: 0, p31: '', p32: '', p33: 5, p44: 0, p45: 30, p51: 175, p52: 70,
    p53: 0, p54: '', p08: 0, p24: 0, p50: 0
  };

  await sectionLoaders.settings();
  await sectionLoaders.integrations();
  loadedSections.integrations = true;

  result.reload = {
    dim_start: el('dim_start').value,
    dim_end: el('dim_end').value,
    wifi_start: el('wifi_start').value,
    wifi_end: el('wifi_end').value
  };

  el('hostname').value = 'frixos-ui';
  el('wifi_ssid').value = 'NewNet';
  el('wifi_pass').value = 'new-secret';
  el('rotation').value = '2';
  await fire(el('mirroring'), 'click');
  await fire(el('fahrenheit'), 'click');
  await fire(el('hour12'), 'click');
  await fire(el('staticToggle'), 'click');
  el('static_ip').value = '192.168.2.50';
  el('static_gw').value = '192.168.2.1';
  el('static_nm').value = '255.255.255.0';
  el('static_dns1').value = '8.8.8.8';
  el('static_dns2').value = '8.8.4.4';

  el('lat').value = '40.7128000';
  el('lon').value = '-74.006000';
  el('timezone').value = 'EST5EDT,M3.2.0,M11.1.0';
  el('tz_iana').value = 'America/New_York';
  el('wifi_start').value = '06:30';
  el('wifi_end').value = '22:00';
  el('dim_mode').value = '2';
  el('lux_threshold').value = '20.5';
  el('lux_sensitivity').value = '8.5';
  el('brightness_LED0').value = '60';
  el('brightness_LED1').value = '15';
  el('pwm_frequency').value = '250';
  el('max_power').value = '900';
  el('dim_start').value = '07:30';
  el('dim_end').value = '22:45';

  el('eeprom_ha_url').value = 'http://ha.local:8123/';
  el('eeprom_ha_token').value = 'new-token';
  el('eeprom_ha_refresh_mins').value = '2';
  el('eeprom_stock_key').value = 'new-key';
  el('eeprom_stock_refresh_mins').value = '10';
  el('eeprom_wu_station').value = 'KCASANFR123';
  el('eeprom_wu_key').value = 'new-wu-key';
  el('eeprom_wu_refresh_mins').value = '20';
  el('eeprom_dexcom_region').value = '1';
  el('eeprom_glucose_username').value = 'cgm-user';
  el('eeprom_glucose_password').value = 'glu-pass';
  el('eeprom_glucose_refresh').value = '3';
  el('eeprom_libre_region').value = '2';
  el('glucose_validity_duration').value = '45';
  el('eeprom_glucose_high').value = '180';
  el('eeprom_glucose_low').value = '65';
  el('eeprom_glucose_unit').value = '1';
  el('eeprom_ns_url').value = 'https://ns.example.com';

  const pre = collectSettingsPayload();
  result.p35BeforeDirty = !!(pre && Object.prototype.hasOwnProperty.call(pre, 'p35'));
  await fire(el('wifi_pass'), 'input');

  const mark = state.posts.length;
  await fire(el('saveAllBtn'), 'click');
  result.save = (state.posts.slice(mark).find(p => String(p.url).includes('/api/settings')) || {}).body || null;

  const imm = state.posts.length;
  await fire(el('themeBtn'), 'click');
  await fire(document.querySelector('.lang-opt[data-lang="de"]'), 'click');
  await fire(el('update_firmware'), 'click');
  await new Promise(r => setTimeout(r, 0));
  result.immediate = state.posts.slice(imm).filter(p => String(p.url).includes('/api/settings')).map(p => p.body);

  window.settings.p08 = 1;
  window.settings.p24 = 0;
  window.settings.p50 = 0;
  screenGridDeviceState = 0;
  const leading = document.createElement('button');
  leading.id = 'screen_show_leading_zero';
  leading.className = 'gswitch on';
  document.body.appendChild(leading);
  const dots = document.createElement('button');
  dots.id = 'screen_dots_breathe';
  dots.className = 'gswitch on';
  document.body.appendChild(dots);

  const sm = state.posts.length;
  await saveScreenTimeDisplaySettings();
  result.screen = (state.posts.slice(sm).find(p => String(p.url).includes('/api/settings')) || {}).body || null;

  window.screenLayout = prepareScreenLayout(cloneScreenDefaults());
  window.screenEditor.mode = 'day';
  const day = getProfileObj('day');
  const find = id => day.elements.find(e => e.id === id);
  const widgets = {};
  widgets.userThemeUploadPath = themeUploadPath('user1-font.jpg');
  widgets.themePreviewUploadPath = themeUploadPath('bold.jpg');
  widgets.layoutDisplayName = screenLayoutDisplayName('layouts/Default.layout');
  widgets.layoutFileUrl = screenFileUrl('layouts/Default.layout');
  widgets.nestedLayoutSaveName = normalizeDeviceLayoutName('layouts/Default.layout');
  widgets.layoutTraversalRejected = normalizeDeviceLayoutName('../Default.layout') === null;
  const originalScreenFiles = window.screenSpiffsFileList;
  window.screenSpiffsFileList = [
    { name: 'layouts/Default.layout' }, { name: 'Default.layout' }, { name: 'custom.layout' }
  ];
  widgets.layoutNames = getScreenLayoutFileNames();
  window.screenSpiffsFileList = originalScreenFiles;
  const defaultTemplate = prepareScreenLayout(JSON.parse(defaultLayoutData));
  const defaultClock = defaultTemplate.profiles.day.elements.find(e => e.id === 'tw_1');
  const weatherTemplate = prepareScreenLayout(JSON.parse(weatherLayoutData));
  const weatherTemperature = weatherTemplate.profiles.day.elements.find(e => e.id === 'tw_3');
  widgets.templateClock = { name: defaultClock.name, width: defaultClock.options.width, align: defaultClock.options.align };
  widgets.templateWeatherName = weatherTemperature.name;
  setScreenSelection(['weather']);
  renderScreenOptions();
  widgets.standardDeleteButton = !!el('screenOptions').querySelector('#screen_widget_delete');
  const weatherThemeSelect = el('screen_weather_theme');
  widgets.weatherThemeDefault = weatherThemeSelect.value;
  weatherThemeSelect.value = '4';
  renderScreenCanvas = () => {};
  await fire(weatherThemeSelect, 'change');
  widgets.weatherThemeSelected = day.weather_theme;
  setScreenSelection(['tw_1']);
  renderScreenOptions();
  widgets.instanceDeleteButton = !!el('screenOptions').querySelector('#screen_widget_delete');
  const tw1 = find('tw_1');
  const factoryClock = getScreenElementSize(findElementDef('tw_1'), tw1);
  const factoryAmpm = getScreenElementSize(findElementDef('tw_2'), find('tw_2'));
  const factoryWidgets = {
    clock: { name: tw1.name, width: tw1.options.width, align: tw1.options.align, height: factoryClock.h },
    ampm: { name: find('tw_2').name, width: find('tw_2').options.width, height: factoryAmpm.h },
    scroll: { name: find('tw_3').name, enabled: find('tw_3').enabled, scroll: find('tw_3').options.scroll, width: find('tw_3').options.width }
  };
  Object.assign(tw1, { enabled: 1, x: 20, y: 20 });
  Object.assign(tw1.options, { family: 0, size: 16 });
  tw1.options.slots = [{ text: '', duration: 10 }];
  window.settings.p37 = 0;
  Object.assign(find('tw_2'), { enabled: 0, instanced: 1, name: 'Off one' });
  // The factory scroll widget owns tw_3; free it so the add/delete checks below use a free slot.
  Object.assign(find('tw_3'), { enabled: 0, instanced: 0, name: '' });
  find('tw_3').options.scroll = false;

  widgets.factory = factoryWidgets;
  const autoProbe = { ...tw1, options: { ...tw1.options, width: 0, slots: [{ text: 'abc', duration: 10 }] } };
  widgets.autoWidth = getScreenElementSize(findElementDef('tw_1'), autoProbe).w;
  const emptyProbe = { ...tw1, options: { ...tw1.options, slots: [{ text: '', duration: 10 }] } };
  const emptyTokenProbe = { ...tw1, options: { ...tw1.options, slots: [{ text: '[ampm12]', duration: 10 }] } };
  widgets.emptyHeight = getScreenElementSize(findElementDef('tw_1'), emptyProbe).h;
  widgets.emptyTokenHeight = getScreenElementSize(findElementDef('tw_1'), emptyTokenProbe).h;
  widgets.emptyTokenSample = twSampleText(emptyTokenProbe.options, 0);
  widgets.emptyLineHeight = twLineHeight(emptyProbe.options) + 4;
  widgets.trashZone = screenPointerOverTrash(562, 522, { left: 100, top: 50, width: 514, height: 514 });
  widgets.paletteInitial = getScreenPaletteDefs().map(d => d.id);

  // Every canvas repaint normalizes the options again, as the real one does.
  renderScreenCanvas = () => ensureScreenTwOptions(tw1);
  setScreenSelection(['tw_1']);
  renderScreenOptions();
  const area = el('editor-textarea-tw_1');
  const optionSections = el('screenOptions').querySelectorAll('.screen-tw-section');
  const sectionTitleFor = node => node.closest('.screen-tw-section')
    ?.querySelector('.screen-tw-section-title')?.textContent || '';
  widgets.optionSectionTitles = optionSections.map(section =>
    section.querySelector('.screen-tw-section-title')?.textContent || '');
  widgets.enabledSection = sectionTitleFor(el('screen_element_enabled'));
  widgets.scrollSection = sectionTitleFor(el('tw_scroll'));
  widgets.textSection = sectionTitleFor(area);
  widgets.fontSectionHasPicker = !!optionSections[1]?.querySelector('.tw-font-picker');
  const fontGroupLabels = el('screenOptions').querySelectorAll('.tw-font-group-label');
  widgets.fontGroupLabels = fontGroupLabels.map(label => label.textContent);
  widgets.fontGroupLabelTags = fontGroupLabels.map(label => label.tagName);
  widgets.contentsLabelTag = el('screenOptions').querySelectorAll('.tw-slots-label')[0]?.tagName || '';
  widgets.widthSection = optionSections.find(section =>
    section.querySelectorAll('.form-group').some(row =>
      (row.children[0]?.textContent || '').startsWith('Width'))
  )?.querySelector('.screen-tw-section-title')?.textContent || '';
  for (const ch of '[hour24][colon][min]') {
    area.value += ch;
    await fire(area, 'input');
  }
  const nixie = el('screenOptions').querySelectorAll('.screen-font-sample-box')
    .find(button => button.children[1] && button.children[1].textContent === 'Nixie');
  if (!nixie) throw new Error('missing Nixie font option');
  widgets.activeFontBeforeChange = el('screenOptions').querySelectorAll('.screen-font-sample-box')
    .find(button => button.classList.contains('selected'))?.children[1].textContent || '';
  widgets.fontCards = el('screenOptions').querySelectorAll('.screen-font-sample-box')
    .map(button => button.children[1].textContent);
  const sizeSelect = el('screenOptions').querySelectorAll('select')[1];
  const centerBeforeSizeChange = tw1.x + getScreenElementSize(findElementDef('tw_1'), tw1).w / 2;
  sizeSelect.value = '24';
  await fire(sizeSelect, 'change');
  widgets.centerShiftAfterSizeChange = Math.abs(centerBeforeSizeChange -
    (tw1.x + getScreenElementSize(findElementDef('tw_1'), tw1).w / 2));
  await fire(nixie, 'click');
  widgets.activeFontAfterChange = el('screenOptions').querySelectorAll('.screen-font-sample-box')
    .find(button => button.classList.contains('selected'))?.children[1].textContent || '';
  widgets.textAfterFontChange = el('editor-textarea-tw_1').value;
  widgets.textStoredAfterFontChange = tw1.options.slots[0].text;
  widgets.familyAfterFontChange = tw1.options.family;

  const defaultColorMode = el('tw_color_mode_default');
  const customColorMode = el('tw_color_mode_custom');
  widgets.defaultColorMode = defaultColorMode.checked ? 'default' : 'custom';
  customColorMode.checked = true;
  await fire(customColorMode, 'change');
  const colorInput = el('screenOptions').querySelectorAll('input[type=color]')[0];
  colorInput.value = '#ff4020';
  await fire(colorInput, 'input');
  widgets.customColorEnabled = tw1.options.color_enabled;
  defaultColorMode.checked = true;
  customColorMode.checked = false;
  await fire(defaultColorMode, 'change');
  widgets.colorModeAfterNone = tw1.options.color_enabled;

  renderScreenCanvas = () => {};
  addScreenWidgetInstance('text');
  const tw3 = find('tw_3');
  widgets.added = { selected: window.screenEditor.selectedId, enabled: tw3.enabled, instanced: tw3.instanced, name: tw3.name, x: tw3.x, y: tw3.y };
  const nameInput = el('screen_widget_name');
  nameInput.value = 'Température élevée';
  await fire(nameInput, 'input');
  widgets.renamed = tw3.name;
  widgets.paletteLabel = getScreenPaletteItemLabel(findElementDef('tw_3'), day);
  await fire(el('screen_widget_delete'), 'click');
  widgets.paletteAfterDelete = getScreenPaletteDefs().map(d => d.id);
  widgets.deletedInstanced = find('tw_3').instanced;
  addScreenWidgetInstance('text');
  const dragDelete = find('tw_3');
  window.screenEditor.dragging = {
    id: 'tw_3', elem: dragDelete, fromPalette: false, moved: true, deleteTarget: true,
    members: [{ id: 'tw_3' }]
  };
  onScreenPointerUp({});
  widgets.deletedByDrop = find('tw_3').instanced;
  const weatherWidget = find('weather');
  window.screenEditor.dragging = {
    id: 'weather', elem: weatherWidget, fromPalette: false, moved: true, deleteTarget: true,
    members: [{ id: 'weather' }]
  };
  onScreenPointerUp({});
  widgets.disabledByTrash = weatherWidget.enabled;

  window.settings.tz_iana = 'Etc/UTC';
  const noon = new Date(Date.UTC(2026, 9, 16, 15, 7));
  window.settings.p37 = 0;
  widgets.clock24 = twExpandText('[hour24][colon][min]', true, noon);
  window.settings.p37 = 1;
  window.settings.p24 = 1;
  widgets.clock12 = twExpandText('[clock][ampm12]', true, noon);
  window.settings.p24 = 0;
  widgets.clock12NoZero = twExpandText('[clock]', true, noon);
  widgets.stripped = twExpandText('[temp]', false, noon);
  widgets.unknown = twExpandText('[bogus]', true, noon);

  const wire = encodeScreenLayoutBinary(window.screenLayout);
  tw1.options.color_enabled = true;
  tw1.options.color = '#ff4020';
  const colorWire = encodeScreenLayoutBinary(window.screenLayout);
  const back = decodeScreenLayoutBinary(colorWire);
  const backTw2 = back.profiles.day.elements.find(e => e.id === 'tw_2');
  const backTw1 = back.profiles.day.elements.find(e => e.id === 'tw_1');
  widgets.colorOverrideWire = { enabled: backTw1.options.color_enabled, color: backTw1.options.color };
  widgets.wire = { size: colorWire.byteLength, expected: SCREEN_BIN_FULL_WIRE_SIZE, weatherThemeDay: back.profiles.day.weather_theme, weatherThemeNight: back.profiles.night.weather_theme, tw2: { enabled: backTw2.enabled, instanced: backTw2.instanced, name: backTw2.name } };
  const v3 = colorWire.slice();
  v3[SCREEN_BIN_WIRE_SIZE + 4] = SCREEN_BIN_EXT_VERSION_V3;
  const v3Back = decodeScreenLayoutBinary(v3);
  widgets.v3Decoded = v3Back.profiles.day.elements.some(e => e.id === 'tw_1');
  widgets.v3ColorOverride = v3Back.profiles.day.elements.find(e => e.id === 'tw_1').options.color_enabled;
  const v2 = wire.slice(0, SCREEN_BIN_FULL_WIRE_SIZE_V2);
  v2[SCREEN_BIN_WIRE_SIZE + 4] = SCREEN_BIN_EXT_VERSION_V2;
  widgets.v2Decoded = decodeScreenLayoutBinary(v2).profiles.day.elements.some(e => e.id === 'tw_1');

  result.widgets = widgets;
  return result;
})();
`;

const script = new vm.Script(source, { filename: 'spiffs-ui.js' });
try {
  script.runInContext(sandbox);
} catch (err) {
  console.error(err && err.stack ? err.stack : err);
  process.exit(1);
}

try {
  const result = await sandbox.__done;
  console.log('RESULT ' + JSON.stringify(result));
  process.exit(0);
} catch (err) {
  console.error(err && err.stack ? err.stack : err);
  process.exit(1);
}
