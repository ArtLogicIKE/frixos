// Generates spiffs/weather.layout: a layout for weather fans.
//   Top:  clock + weather icon + moon + AM/PM (+ wifi-off when offline)
//   Grid: 8 static cells (2 columns x 4 rows) showing the live weather params
//   Msg:  scrolling line carrying the remaining params (pressure, 3-day, sun)
// Day and Night are identical except Night uses the 'light' digit font.
// Built by running the screen editor's own code (same harness as
// gen_default_layout.js) so format + wire encoding stay authoritative.
// Run: node tools/gen_weather_layout.js
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

// Element top-left positions in 128x128 panel space.
sandbox.POS = {
  time:    [4, 2],   // clock spans ~x4-82
  ampm:    [86, 8],  // tucked between clock and weather icon
  weather: [96, 2],  // weather icon, top-right
  moon:    [100, 24],
  wifi_off:[96, 2],  // shares the weather slot (mutually exclusive: shown only offline)
  // Weather grid: 2 columns (x2 / x66), 4 rows.
  text_1: [2, 44],  text_2: [66, 44],
  text_3: [2, 56],  text_4: [66, 56],
  text_5: [2, 68],  text_6: [66, 68],
  text_7: [2, 80],  text_8: [66, 80],
  message: [0, 94],
};
sandbox.TEXTS = {
  text_1: 'Now [temp]',  text_2: 'Hum [hum]',
  text_3: 'Hi [high]',   text_4: 'Lo [low]',
  text_5: 'Wind [wind]', text_6: 'Gust [gust]',
  text_7: 'UV [uv]',     text_8: 'Rain [precip]',
};
sandbox.SCROLL =
  '[greeting] [day] [date] [mon], Press [pressure], 3-day [3high]/[3low], Sun [rise]-[set]';

const json = vm.runInContext(
  `window.screenLayout = cloneScreenDefaults();
   const exp = prepareScreenLayoutForExport();
   const ENABLE = ['time','ampm','weather','moon','wifi_off','message',
                   'text_1','text_2','text_3','text_4','text_5','text_6','text_7','text_8'];
   const DISABLE = ['glucose_level','glucose_trend','graph','digit_label','time_aux','digit_label_aux'];
   // Day and Night share the exact same layout; only the font differs (below).
   ['day','night'].forEach(mode => {
     const prof = exp.profiles[mode];
     const byId = Object.fromEntries(prof.elements.map(e => [e.id, e]));
     for (const id in POS) if (byId[id]) { byId[id].x = POS[id][0]; byId[id].y = POS[id][1]; }
     ENABLE.forEach(id => { if (byId[id]) byId[id].enabled = 1; });
     DISABLE.forEach(id => { if (byId[id]) byId[id].enabled = 0; });
     for (const id in TEXTS) {
       prof.static_texts[id] = TEXTS[id];
       if (byId[id] && byId[id].options) { byId[id].options.width = 60; byId[id].options.font = 0; }
     }
     prof.scroll_text = SCROLL;
   });
   // Identical look, day = bold, night = light.
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

const out = path.join(root, 'spiffs/weather.layout');
fs.writeFileSync(out, json + '\n');
console.log(`Wrote ${out} (${json.length} chars, encodes to ${bytes} bytes OK)`);
