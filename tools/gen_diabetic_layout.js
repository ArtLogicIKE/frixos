// Generates spiffs/diabetic.layout: a layout tuned for CGM users.
//   Top:    clock + weather / moon / glucose readout / wifi / trend
//   Middle: scrolling message
//   Bottom: glucose chart (graph bound to [CGM:glucose])
// Built by running the screen editor's own code (same harness as
// gen_default_layout.js) so the format and wire encoding stay authoritative.
// Run: node tools/gen_diabetic_layout.js
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

// Three-band placement in 128x128 panel space (x, y of the element's top-left).
sandbox.POS = {
  // Top: status row (y2-25) then the clock (y25-61).
  weather:       [54, 2],
  glucose_level: [27, 5],
  wifi_off:      [22, 5],
  moon:          [87, 7],
  glucose_trend: [42, 8],
  time:          [22, 25],
  ampm:          [101, 32],
  // Middle: scrolling message centred in the clock->graph gap.
  message:       [0, 68],
  // Bottom: glucose chart, 80x36 centred horizontally, 6px bottom margin.
  graph:         [24, 86],
};

const json = vm.runInContext(
  `window.screenLayout = cloneScreenDefaults();
   const exp = prepareScreenLayoutForExport();
   ['day','night'].forEach(mode => {
     const els = exp.profiles[mode].elements;
     const byId = Object.fromEntries(els.map(e => [e.id, e]));
     for (const id in POS) {
       if (byId[id]) { byId[id].x = POS[id][0]; byId[id].y = POS[id][1]; }
     }
     // Visible: clock, status, message, graph.
     ['weather','glucose_level','wifi_off','moon','glucose_trend','time','ampm','message','graph']
       .forEach(id => { if (byId[id]) byId[id].enabled = 1; });
     // Configure the bottom glucose chart.
     const g = byId.graph;
     ensureScreenGraphOptions(g);
     Object.assign(g.options, {
       token: '[CGM:glucose]',
       interval_min: 5,
       points: 60,
       gwidth: 80,
       gheight: 36,
       autoscale: true,
       show_axis: true,
       show_value: true,
       backfill: true,
       band_on: false,
       boolean: false,
       thick: false,
     });
   });
   JSON.stringify(exp, null, 2);`,
  sandbox
);

const bytes = vm.runInContext(`encodeScreenLayoutBinary(window.screenLayout).byteLength;`, sandbox);
const expected = vm.runInContext(`SCREEN_BIN_WIRE_SIZE;`, sandbox);
if (bytes !== expected) {
  console.error(`FAIL: encodes to ${bytes} bytes, expected ${expected}`);
  process.exit(1);
}

const out = path.join(root, 'spiffs/diabetic.layout');
fs.writeFileSync(out, json + '\n');
console.log(`Wrote ${out} (${json.length} chars, encodes to ${bytes} bytes OK)`);
