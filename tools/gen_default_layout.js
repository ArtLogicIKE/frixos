// Generates spiffs/default.layout by running the screen editor's OWN default
// layout code (cloneScreenDefaults -> prepareScreenLayoutForExport), so the
// preset is byte-identical to what the editor's old "Restore default" produced
// and what saveScreenLayoutToFile() would download. Run: node tools/gen_default_layout.js
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const root = path.resolve(__dirname, '..');
const src = fs.readFileSync(path.join(root, 'spiffs/js/screen-editor.js'), 'utf8');

// Minimal browser stubs. The default/export/encode paths only touch `window`
// (settings absent -> they fall back to the same 'bold'/0 defaults baked into
// SCREEN_DEFAULT_LAYOUT). `document` is a no-op proxy in case anything pokes it.
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

const json = vm.runInContext(
  `window.screenLayout = cloneScreenDefaults();
   const exp = prepareScreenLayoutForExport();
   // makeDefaultScreenProfile() omits the graph, so ensureScreenProfileShape()
   // re-adds it with the generic enabled=1 default. The firmware factory default
   // (and the real default look) has the graph OFF, so force it off in both
   // profiles. It stays off on reload because the element is now present with an
   // explicit enabled=0.
   ['day','night'].forEach(m => {
     const g = exp.profiles[m].elements.find(e => e.id === 'graph');
     if (g) g.enabled = 0;
   });
   JSON.stringify(exp, null, 2);`,
  sandbox
);

// Validate: it must encode to the exact device wire size.
const bytes = vm.runInContext(
  `encodeScreenLayoutBinary(window.screenLayout).byteLength;`,
  sandbox
);
const expected = vm.runInContext(`SCREEN_BIN_WIRE_SIZE;`, sandbox);
if (bytes !== expected) {
  console.error(`FAIL: encodes to ${bytes} bytes, expected ${expected}`);
  process.exit(1);
}

const out = path.join(root, 'spiffs/default.layout');
fs.writeFileSync(out, json + '\n');
console.log(`Wrote ${out} (${json.length} chars, encodes to ${bytes} bytes OK)`);
