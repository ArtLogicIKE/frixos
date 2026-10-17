// Generates spiffs/layouts/homeassistant.layout: a dashboard for Home Assistant users.
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

sandbox.POS = { tw_1: [4, 2], tw_2: [86, 8], wifi_off: [96, 2], message: [0, 90] };

// id -> [token text, x, y, width, bgColor]. The clock and the AM/PM mark use tw_1 and tw_2,
// which leaves six text widgets. Columns touch (0-43 / 43-86 / 86-128) so each 3-cell row reads
// as one solid colour band.
const BLUE = '#163a5a', GREEN = '#1a4a1a';
sandbox.CELLS = {
  // Climate (blue)
  tw_3: ['In [HA:sensor.living_room_temperature]', 0, 46, 43, BLUE],
  tw_4: ['Hum [HA:sensor.living_room_humidity]', 43, 46, 43, BLUE],
  tw_5: ['Out [HA:sensor.outdoor_temperature]', 86, 46, 42, BLUE],
  // Energy (green)
  tw_6: ['Pwr [HA:sensor.power_consumption]', 0, 60, 43, GREEN],
  tw_7: ['Sol [HA:sensor.solar_power]', 43, 60, 43, GREEN],
  tw_8: ['kWh [HA:sensor.energy_today]', 86, 60, 42, GREEN],
};

const json = vm.runInContext(
  `window.screenLayout = cloneScreenDefaults();
   const exp = prepareScreenLayoutForExport();
   const ENABLE = ['tw_1','tw_2','wifi_off','message',
                   'tw_3','tw_4','tw_5','tw_6','tw_7','tw_8'];
   const DISABLE = ['glucose_level','glucose_trend','weather','moon','graph'];
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
       Object.assign(e.options, {
         width: w, size: 8, family: 0, align: 0, show_units: true, tabular: false,
         color: '#ffffff', bg_color: bg, slots: [{ text, duration: 10 }]
       });
     }
     prof.scroll_text = '[greeting] [day], [date] [mon]';
   });
   exp.day_font = 'bold';   exp.day_aux_font = 'bold';
   exp.night_font = 'light'; exp.night_aux_font = 'light';
   JSON.stringify(exp, null, 2);`,
  sandbox
);

const bytes = vm.runInContext(`encodeScreenLayoutBinary(window.screenLayout).byteLength;`, sandbox);
const expected = vm.runInContext(`SCREEN_BIN_FULL_WIRE_SIZE;`, sandbox);
if (bytes !== expected) {
  console.error(`FAIL: encodes to ${bytes} bytes, expected ${expected}`);
  process.exit(1);
}

const out = path.join(root, 'spiffs/layouts/homeassistant.layout');
fs.writeFileSync(out, json + '\n');
console.log(`Wrote ${out} (${json.length} chars, encodes to ${bytes} bytes OK)`);
