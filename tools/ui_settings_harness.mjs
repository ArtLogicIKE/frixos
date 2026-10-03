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
    child.parentElement = node;
    child.isConnected = true;
    node.children.push(child);
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
  URL,
  URLSearchParams,
  Intl,
  document,
  navigator: { language: 'en-US', clipboard: { async writeText() {} } },
  location: { reload() {} },
  fetch: fetchImpl,
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
    p53: 0, p54: '', p08: 0, p24: 0, p50: 0, p58: '', p59: ''
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
  window.settings.p58 = '';
  window.settings.p59 = '';
  screenGridDeviceState = 0;
  const leading = document.createElement('button');
  leading.id = 'screen_show_leading_zero';
  leading.className = 'gswitch on';
  document.body.appendChild(leading);
  const dots = document.createElement('button');
  dots.id = 'screen_dots_breathe';
  dots.className = 'gswitch on';
  document.body.appendChild(dots);
  const list = document.createElement('div');
  list.id = 'display-schedule-list';
  list.appendChild(buildSlotRow({ t: 2, d: 15 }));
  document.body.appendChild(list);
  const aux = document.createElement('div');
  aux.id = 'display-schedule-aux-list';
  aux.appendChild(buildSlotRow({ t: 0, d: 20 }));
  document.body.appendChild(aux);

  const sm = state.posts.length;
  await saveScreenTimeDisplaySettings();
  result.screen = (state.posts.slice(sm).find(p => String(p.url).includes('/api/settings')) || {}).body || null;
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
