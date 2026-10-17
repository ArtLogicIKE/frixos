// Generates spiffs/layouts/weather.layout: a layout for weather fans.
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

// Element top-left positions in 128x128 panel space. tw_1 is the clock, tw_2 the AM/PM mark,
// tw_3..tw_8 the six cells of the weather grid.
sandbox.POS = {
  tw_1:    [4, 2],   // clock spans ~x4-82
  tw_2:    [86, 8],  // tucked between clock and weather icon
  weather: [96, 2],  // weather icon, top-right
  moon:    [100, 24],
  wifi_off:[96, 2],  // shares the weather slot (mutually exclusive: shown only offline)
  // Weather grid: 2 columns (x2 / x66), 3 rows.
  tw_3: [2, 44],  tw_4: [66, 44],
  tw_5: [2, 58],  tw_6: [66, 58],
  tw_7: [2, 72],  tw_8: [66, 72],
  message: [0, 94],
};
sandbox.TEXTS = {
  tw_3: 'Now [temp]',  tw_4: 'Hum [hum]',
  tw_5: 'Hi [high]',   tw_6: 'Lo [low]',
  tw_7: 'Wind [wind]', tw_8: 'Gust [gust]',
};
sandbox.SCROLL =
  '[greeting] [day] [date] [mon], UV [uv], Rain [precip], Press [pressure], 3-day [3high]/[3low], Sun [rise]-[set]';

const json = vm.runInContext(
  `window.screenLayout = cloneScreenDefaults();
   const exp = prepareScreenLayoutForExport();
   const ENABLE = ['tw_1','tw_2','weather','moon','wifi_off','message',
                   'tw_3','tw_4','tw_5','tw_6','tw_7','tw_8'];
   const DISABLE = ['glucose_level','glucose_trend','graph'];
   // Day and Night share the exact same layout; only the font differs (below).
   ['day','night'].forEach(mode => {
     const prof = exp.profiles[mode];
     const byId = Object.fromEntries(prof.elements.map(e => [e.id, e]));
     for (const id in POS) if (byId[id]) { byId[id].x = POS[id][0]; byId[id].y = POS[id][1]; }
     ENABLE.forEach(id => { if (byId[id]) byId[id].enabled = 1; });
     DISABLE.forEach(id => { if (byId[id]) byId[id].enabled = 0; });
     for (const id in TEXTS) {
       const o = byId[id].options;
       o.slots = [{ text: TEXTS[id], duration: 10 }];
       o.width = 60; o.size = 8; o.family = 0; o.show_units = true; o.tabular = false;
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
const expected = vm.runInContext(`SCREEN_BIN_FULL_WIRE_SIZE;`, sandbox);
if (bytes !== expected) {
  console.error(`FAIL: encodes to ${bytes} bytes, expected ${expected}`);
  process.exit(1);
}

const out = path.join(root, 'spiffs/layouts/weather.layout');
fs.writeFileSync(out, json + '\n');
console.log(`Wrote ${out} (${json.length} chars, encodes to ${bytes} bytes OK)`);
