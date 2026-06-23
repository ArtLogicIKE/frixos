// Test helper: encode default.layout with icon_1 enabled, or decode a wire blob
// and print the icon_1 widget. Uses the editor's own encoder/decoder.
//   node tools/icon_roundtrip.js encode <out.bin>
//   node tools/icon_roundtrip.js decode <in.bin>
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const root = path.resolve(__dirname, '..');
const src = fs.readFileSync(path.join(root, 'spiffs/js/screen-editor.js'), 'utf8');
const noop = () => {};
const sb = { window: {}, document: new Proxy({}, { get: () => noop }), navigator: { language: 'en' },
  console, translations: { en: {} }, currentLanguage: 'en', sectionLoaders: {}, TextEncoder, TextDecoder };
sb.window.settings = undefined;
vm.createContext(sb);
vm.runInContext(src, sb, { filename: 'se.js' });

const mode = process.argv[2];
const file = process.argv[3];

if (mode === 'encode') {
  const data = JSON.parse(fs.readFileSync(path.join(root, 'spiffs/default.layout'), 'utf8'));
  sb.DATA = data;
  const u8 = vm.runInContext(
    `const lay = prepareScreenLayout(DATA);
     ensureScreenLayoutMeta(lay);
     ensureScreenProfileShape(lay.profiles.day);
     ensureScreenProfileShape(lay.profiles.night);
     lay.version = SCREEN_LAYOUT_VERSION;
     ['day','night'].forEach(m => {
       const ic = lay.profiles[m].elements.find(e => e.id === 'icon_1');
       ic.enabled = 1; ic.x = 8; ic.y = 40; ic.z = 3;
     });
     const body = encodeScreenLayoutBinary(lay);
     Array.from(new Uint8Array(body.buffer || body));`,
    sb
  );
  fs.writeFileSync(file, Buffer.from(u8));
  console.log(`wrote ${file} (${u8.length} bytes)`);
} else if (mode === 'decode') {
  const buf = fs.readFileSync(file);
  sb.WIRE = new Uint8Array(buf.buffer, buf.byteOffset, buf.byteLength);
  const out = vm.runInContext('JSON.stringify(decodeScreenLayoutBinary(WIRE))', sb);
  const d = JSON.parse(out);
  const ic = d.profiles.day.elements.find(e => e.id === 'icon_1');
  console.log('decoded: version', d.version, '| icon_1 =', JSON.stringify(ic));
} else {
  console.error('usage: encode|decode <file>');
  process.exit(1);
}
