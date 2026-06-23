// Generates spiffs/homeassistant.layout: a dashboard for Home Assistant users.
//   Top:  clock + AM/PM (+ wifi-off when offline)
//   Grid: 9 HA value cells in a 3x3 grid, grouped into 3 colour-coded rows:
//           Climate (blue)  |  Energy (green)  |  System (amber)
//   Msg:  scrolling greeting / date line
// The 9th cell reuses the digit_label element (same text rendering + bg colour as
// the static cells; with no display schedule it just shows its configured text).
// Tokens use commonly-named HA entities -- edit them to match your setup.
// Day/Night are identical except Night uses the 'light' font.
// Run: node tools/gen_ha_layout.js
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const root = path.resolve(__dirname, '..');
const src = fs.readFileSync(path.join(root, 'spiffs/js/screen-editor.js'), 'utf8');

const noop = () => {};
const sandbox = {
  window: {},
  document: new Proxy({}, { get: () => noop }),
  navigator: { language: 'en' },
  console,
  translations: { en: {} },
  currentLanguage: 'en',
  sectionLoaders: {},
  TextEncoder,
  TextDecoder,
};
sandbox.window.settings = undefined;
vm.createContext(sandbox);
vm.runInContext(src, sandbox, { filename: 'screen-editor.js' });

sandbox.POS = { time: [4, 2], ampm: [86, 8], wifi_off: [96, 2], message: [0, 90] };

// id -> [token text, x, y, width, bgColor]. Columns touch (0-43 / 43-86 / 86-128)
// so each 3-cell row reads as one solid colour band. digit_label is the 9th cell.
const BLUE = '#163a5a', GREEN = '#1a4a1a', AMBER = '#5a3a14';
sandbox.CELLS = {
  // Climate (blue)
  text_1: ['In [HA:sensor.living_room_temperature]', 0, 46, 43, BLUE],
  text_2: ['Hum [HA:sensor.living_room_humidity]', 43, 46, 43, BLUE],
  text_3: ['Out [HA:sensor.outdoor_temperature]', 86, 46, 42, BLUE],
  // Energy (green)
  text_4: ['Pwr [HA:sensor.power_consumption]', 0, 60, 43, GREEN],
  text_5: ['Sol [HA:sensor.solar_power]', 43, 60, 43, GREEN],
  text_6: ['kWh [HA:sensor.energy_today]', 86, 60, 42, GREEN],
  // System (amber)
  text_7: ['CPU [HA:sensor.processor_use]', 0, 74, 43, AMBER],
  text_8: ['Bat [HA:sensor.battery_level]', 43, 74, 43, AMBER],
  digit_label: ['RAM [HA:sensor.memory_use_percent]', 86, 74, 42, AMBER],
};

const json = vm.runInContext(
  `window.screenLayout = cloneScreenDefaults();
   const exp = prepareScreenLayoutForExport();
   const ENABLE = ['time','ampm','wifi_off','message',
                   'text_1','text_2','text_3','text_4','text_5','text_6','text_7','text_8','digit_label'];
   const DISABLE = ['glucose_level','glucose_trend','weather','moon','graph','time_aux','digit_label_aux'];
   ['day','night'].forEach(mode => {
     const prof = exp.profiles[mode];
     const byId = Object.fromEntries(prof.elements.map(e => [e.id, e]));
     for (const id in POS) if (byId[id]) { byId[id].x = POS[id][0]; byId[id].y = POS[id][1]; }
     ENABLE.forEach(id => { if (byId[id]) byId[id].enabled = 1; });
     DISABLE.forEach(id => { if (byId[id]) byId[id].enabled = 0; });
     for (const id in CELLS) {
       const [text, x, y, w, bg] = CELLS[id];
       const e = byId[id];
       if (!e) continue;
       e.x = x; e.y = y;
       if (!e.options) e.options = {};
       e.options.width = w;
       e.options.font = 0;
       e.options.align = 0;
       e.options.color = '#ffffff';
       e.options.bg_color = bg;
       prof.static_texts[id] = text; // static_texts.digit_label maps to digit_label_text
     }
     prof.scroll_text = '[greeting] [day], [date] [mon]';
   });
   exp.day_font = 'bold';   exp.day_aux_font = 'bold';
   exp.night_font = 'light'; exp.night_aux_font = 'light';
   JSON.stringify(exp, null, 2);`,
  sandbox
);

const bytes = vm.runInContext(`encodeScreenLayoutBinary(window.screenLayout).byteLength;`, sandbox);
const expected = vm.runInContext(`SCREEN_BIN_WIRE_SIZE;`, sandbox);
if (bytes !== expected) {
  console.error(`FAIL: encodes to ${bytes} bytes, expected ${expected}`);
  process.exit(1);
}

const out = path.join(root, 'spiffs/homeassistant.layout');
fs.writeFileSync(out, json + '\n');
console.log(`Wrote ${out} (${json.length} chars, encodes to ${bytes} bytes OK)`);
