const fs = require('fs');
const path = require('path');
const vm = require('vm');

const root = path.resolve(__dirname, '..');
const src = fs.readFileSync(path.join(root, 'spiffs/js/screen-editor.js'), 'utf8');
const layoutPath = path.join(root, 'spiffs/layouts/Default.layout');
const outputPath = path.join(root, 'spiffs/layouts/Default.layout.bin');
const noop = () => {};
const sandbox = {
  window: { settings: undefined },
  document: new Proxy({}, { get: () => noop }),
  navigator: { language: 'en' },
  console,
  translations: { en: {} },
  currentLanguage: 'en',
  sectionLoaders: {},
  TextEncoder,
  TextDecoder,
};

vm.createContext(sandbox);
vm.runInContext(src, sandbox, { filename: 'screen-editor.js' });
sandbox.window.screenLayout = JSON.parse(fs.readFileSync(layoutPath, 'utf8'));
const bytes = vm.runInContext(`
  window.screenLayout = prepareScreenLayout(window.screenLayout);
  encodeScreenLayoutBinary(window.screenLayout);
`, sandbox);
const expected = vm.runInContext('SCREEN_BIN_FULL_WIRE_SIZE', sandbox);
if (bytes.byteLength !== expected) {
  throw new Error(`Default.layout encodes to ${bytes.byteLength} bytes, expected ${expected}`);
}

fs.writeFileSync(outputPath, Buffer.from(bytes));
console.log(`Wrote ${outputPath} (${bytes.byteLength} bytes)`);