// ----------------
// Screen editor section
// ----------------
const SCREEN_SIZE = 128;
const SCREEN_DRAG_THRESHOLD = 3;
const SCREEN_PREVIEW_MESSAGE = 'Welcome to frixos';
// LVGL label object line heights on device (frixos_* fonts).
const SCREEN_MESSAGE_FONT_LINE_HEIGHTS = {
    0: 12,
    1: 16,
    2: 16,
    3: 17,
    4: 21,
    5: 10 // 5pt Tiny5 (full ASCII), device line height 7
};
// Visible label/bg height on device (line box minus unused rows above/below glyphs).
const SCREEN_LABEL_VISIBLE_HEIGHTS = {
    0: 9,
    1: 13,
    2: 13,
    3: 14,
    4: 18,
    5: 7 // 5pt Tiny5
};
// Stored layout Y is above the visible label; match on-screen bg top inset.
const SCREEN_LABEL_LAYOUT_TRIM = 3;
// Nominal point sizes for preview glyph scale.
const SCREEN_MESSAGE_FONT_SIZES = {
    0: 8,
    1: 10,
    2: 12,
    3: 14,
    4: 16,
    5: 6 // 5pt Tiny5 preview scale
};
const SCREEN_MESSAGE_FONT_HEIGHTS = SCREEN_MESSAGE_FONT_LINE_HEIGHTS;
const SCREEN_MESSAGE_FONT_OPTIONS = [
    { v: 5, t: '5pt' },
    { v: 0, t: '8pt' },
    { v: 1, t: '10pt' },
    { v: 2, t: '12pt' },
    { v: 3, t: '14pt' },
    { v: 4, t: '16pt' }
];
const SCREEN_ALIGN_OPTIONS = [
    { v: 0, t: 'Left' },
    { v: 1, t: 'Center' },
    { v: 2, t: 'Right' }
];
const SCREEN_LAYER_COUNT = 5;
const SCREEN_LAYER_OPTIONS = [
    { v: 0, key: 'screen.layers.back', fallback: '1 (back)' },
    { v: 1, key: 'screen.layers.2', fallback: '2' },
    { v: 2, key: 'screen.layers.3', fallback: '3' },
    { v: 3, key: 'screen.layers.4', fallback: '4' },
    { v: 4, key: 'screen.layers.front', fallback: '5 (front)' }
];
const SCREEN_TIME_FONTS = [
    'bold', 'light', 'lcd', 'nixie', 'robrito', 'ficasso',
    'lichten', 'kablame', 'kablamo', 'kaboom', 'user1', 'user2'
];
const SCREEN_COLOR_FILTER_OPTIONS = [
    { v: 0, key: 'advanced.color_filter.none', label: 'None' },
    { v: 1, key: 'advanced.color_filter.red', label: 'Red' },
    { v: 2, key: 'advanced.color_filter.green', label: 'Green' },
    { v: 3, key: 'advanced.color_filter.blue', label: 'Blue' },
    { v: 4, key: 'advanced.color_filter.bw', label: 'Black & White' }
];
const SCREEN_DEFAULT_SCROLL_TEXT = '[device]: [greeting] [day], [date] [mon], now [temp] today [high]-[low], hum. [hum], sun [rise]-[set]';
const SCREEN_DEFAULT_STATIC_TEXTS = {
    text_1: 'UV [uv]',
    text_2: '[pressure]',
    text_3: 'Wind [wind]',
    text_4: 'Gust [gust]',
    text_5: 'Rain [precip]',
    text_6: '',
    text_7: '',
    text_8: '',
    digit_label: '',
    digit_label_aux: ''
};
const SCREEN_TEXT_SLOT_MAX = 8;
const SCREEN_PALETTE_LABEL_MAX = 24;
const SCREEN_TOKEN_CODES = [
    '[device]', '[greeting]', '[day]', '[date]', '[mon]', '[time]', '[hour12]', '[hour24]', '[min]', '[ampm]',
    '[temp]', '[hum]', '[high]', '[low]',
    '[rise]', '[set]', '[wind]', '[gust]', '[precip]', '[uv]', '[pressure]', '[3high]', '[3low]',
    '[HA:entity_id:path]', '[$:symbol]', '[CGM:glucose]', '[CGM:reading]', '[CGM:time]'
];
const TOKEN_HELP = {
    '[device]': { i18nKey: 'advanced.message.token_device', example: 'frixos' },
    '[greeting]': { i18nKey: 'advanced.message.token_greeting', example: 'Good Evening' },
    '[day]': { i18nKey: 'advanced.message.token_day', example: 'Saturday' },
    '[date]': { i18nKey: 'advanced.message.token_date', example: 'Jun 6' },
    '[mon]': { i18nKey: 'advanced.message.token_mon', example: 'June' },
    '[time]': { i18nKey: 'advanced.message.token_time', example: '3:45pm' },
    '[hour12]': { i18nKey: 'advanced.message.token_hour12', example: '3' },
    '[hour24]': { i18nKey: 'advanced.message.token_hour24', example: '15' },
    '[min]': { i18nKey: 'advanced.message.token_min', example: '45' },
    '[ampm]': { i18nKey: 'advanced.message.token_ampm', example: 'pm' },
    '[temp]': { i18nKey: 'advanced.message.token_temp', example: '72°F' },
    '[hum]': { i18nKey: 'advanced.message.token_hum', example: '45%' },
    '[high]': { i18nKey: 'advanced.message.token_high', example: '78°F' },
    '[low]': { i18nKey: 'advanced.message.token_low', example: '62°F' },
    '[rise]': { i18nKey: 'advanced.message.token_rise', example: '6:42am' },
    '[set]': { i18nKey: 'advanced.message.token_set', example: '8:15pm' },
    '[wind]': { i18nKey: 'advanced.message.token_wind', example: '12mph NW' },
    '[gust]': { i18nKey: 'advanced.message.token_gust', example: '18mph NW' },
    '[precip]': { i18nKey: 'advanced.message.token_precip', example: '0.2in 30%' },
    '[uv]': { i18nKey: 'advanced.message.token_uv', example: '6' },
    '[pressure]': { i18nKey: 'advanced.message.token_pressure', example: '1013 hPa ↑' },
    '[3high]': { i18nKey: 'advanced.message.token_3high', example: '82°F' },
    '[3low]': { i18nKey: 'advanced.message.token_3low', example: '58°F' },
    '[HA:entity_id:path]': { i18nKey: 'advanced.message.token_ha', example: '72' },
    '[$:symbol]': { i18nKey: 'advanced.message.token_stock', example: '$182.45' },
    '[CGM:glucose]': { i18nKey: 'advanced.message.token_cgm_glucose', example: '157 mg/dl (+3 @ 12:01)' },
    '[CGM:reading]': { i18nKey: 'advanced.message.token_cgm_reading', example: '157' },
    '[CGM:time]': { i18nKey: 'advanced.message.token_cgm_time', example: '12:01pm' }
};

function getTokenCode(btn) {
    return btn.dataset.token || btn.querySelector('.token-code-label')?.textContent || btn.textContent.trim();
}

function updateTokenButtonTooltip(btn, insertLabel) {
    const code = getTokenCode(btn);
    const meta = TOKEN_HELP[code];
    const trans = translations[currentLanguage] || translations.en;
    const effectiveInsertLabel = insertLabel ?? (getNestedTranslation(trans, 'common.insert') || 'Insert');
    btn.setAttribute('aria-label', `${effectiveInsertLabel} ${code}`);
    if (!meta) return;
    const description = getNestedTranslation(trans, meta.i18nKey) || '';
    const exampleEl = btn.querySelector('.token-tooltip-example');
    const descEl = btn.querySelector('.token-tooltip-desc');
    if (exampleEl) exampleEl.textContent = meta.example;
    if (descEl) descEl.textContent = description;
}

function decorateTokenButton(btn, code) {
    if (!btn.dataset.token) btn.dataset.token = code;
    if (!btn.querySelector('.token-code-label')) {
        const label = document.createElement('span');
        label.className = 'token-code-label';
        label.textContent = code;
        btn.textContent = '';
        btn.appendChild(label);
    }
    if (!btn.querySelector('.token-tooltip')) {
        btn.classList.add('token-has-tooltip');
        const tooltip = document.createElement('span');
        tooltip.className = 'token-tooltip';
        tooltip.setAttribute('role', 'tooltip');
        const exampleEl = document.createElement('span');
        exampleEl.className = 'token-tooltip-example';
        const descEl = document.createElement('span');
        descEl.className = 'token-tooltip-desc';
        tooltip.appendChild(exampleEl);
        tooltip.appendChild(descEl);
        btn.appendChild(tooltip);
    }
    updateTokenButtonTooltip(btn);
}

function createTokenButton(code) {
    const btn = document.createElement('code');
    btn.className = 'token-code token-has-tooltip';
    btn.tabIndex = 0;
    btn.setAttribute('role', 'button');
    btn.dataset.token = code;
    const label = document.createElement('span');
    label.className = 'token-code-label';
    label.textContent = code;
    btn.appendChild(label);
    const tooltip = document.createElement('span');
    tooltip.className = 'token-tooltip';
    tooltip.setAttribute('role', 'tooltip');
    const exampleEl = document.createElement('span');
    exampleEl.className = 'token-tooltip-example';
    const descEl = document.createElement('span');
    descEl.className = 'token-tooltip-desc';
    tooltip.appendChild(exampleEl);
    tooltip.appendChild(descEl);
    btn.appendChild(tooltip);
    updateTokenButtonTooltip(btn);
    return btn;
}

function bindTokenInsert(btn, textarea, afterInsert) {
    const insert = () => {
        const code = getTokenCode(btn);
        const start = textarea.selectionStart;
        const end = textarea.selectionEnd;
        const value = textarea.value;
        textarea.value = value.slice(0, start) + code + value.slice(end);
        textarea.setSelectionRange(start + code.length, start + code.length);
        textarea.dispatchEvent(new Event('input', { bubbles: true }));
        updateTokenHighlightTextarea(textarea);
        if (afterInsert) afterInsert();
        textarea.focus();
    };
    btn.onclick = insert;
    btn.onkeydown = (ev) => {
        if (ev.key === 'Enter' || ev.key === ' ') {
            ev.preventDefault();
            insert();
        }
    };
}

function escapeHtmlForTokenHighlight(text) {
    return text
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;');
}

function isValidTokenInText(token) {
    if (TOKEN_HELP[token]) return true;
    if (/^\[HA:[^:\]]+:[^\]]+\]$/.test(token)) return true;
    if (/^\[\$:[^\]]+\]$/.test(token)) return true;
    return false;
}

function formatTextWithTokenHighlights(text) {
    let result = '';
    let i = 0;
    while (i < text.length) {
        if (text[i] === '[') {
            const end = text.indexOf(']', i + 1);
            if (end !== -1) {
                const candidate = text.slice(i, end + 1);
                if (isValidTokenInText(candidate)) {
                    result += `<span class="token-in-text">${escapeHtmlForTokenHighlight(candidate)}</span>`;
                    i = end + 1;
                    continue;
                }
            }
        }
        result += escapeHtmlForTokenHighlight(text[i]);
        i++;
    }
    return result;
}

function syncTokenHighlightMetrics(textarea, highlight) {
    const cs = getComputedStyle(textarea);
    highlight.style.fontFamily = cs.fontFamily;
    highlight.style.fontSize = cs.fontSize;
    highlight.style.fontWeight = cs.fontWeight;
    highlight.style.lineHeight = cs.lineHeight;
    highlight.style.letterSpacing = cs.letterSpacing;
    highlight.style.wordSpacing = cs.wordSpacing;
    highlight.style.padding = cs.padding;
    highlight.style.borderWidth = cs.borderWidth;
    highlight.style.borderStyle = 'solid';
    highlight.style.borderColor = 'transparent';
    highlight.style.boxSizing = cs.boxSizing;
    highlight.style.tabSize = cs.tabSize;
}

function updateTokenHighlightTextarea(textarea) {
    const wrap = textarea.closest('.textarea-token-wrap');
    if (!wrap) return;
    const highlight = wrap.querySelector('.textarea-token-highlight');
    if (!highlight) return;
    syncTokenHighlightMetrics(textarea, highlight);
    highlight.innerHTML = formatTextWithTokenHighlights(textarea.value);
    highlight.scrollTop = textarea.scrollTop;
    highlight.scrollLeft = textarea.scrollLeft;
}

function setupTokenHighlightTextarea(textarea) {
    if (textarea.dataset.tokenHighlight === '1') return;
    textarea.dataset.tokenHighlight = '1';

    const wrap = document.createElement('div');
    wrap.className = 'textarea-token-wrap';
    textarea.parentNode.insertBefore(wrap, textarea);
    wrap.appendChild(textarea);

    const highlight = document.createElement('div');
    highlight.className = 'textarea-token-highlight';
    highlight.setAttribute('aria-hidden', 'true');
    wrap.insertBefore(highlight, textarea);

    textarea.classList.add('textarea-token-input');

    const refresh = () => updateTokenHighlightTextarea(textarea);
    const syncScroll = () => {
        highlight.scrollTop = textarea.scrollTop;
        highlight.scrollLeft = textarea.scrollLeft;
    };
    textarea.addEventListener('input', refresh);
    textarea.addEventListener('scroll', syncScroll);
    if (typeof ResizeObserver !== 'undefined') {
        const ro = new ResizeObserver(refresh);
        ro.observe(textarea);
    }
    refresh();
}
function getScreenTranslation(key, fallback) {
    const trans = translations[currentLanguage] || translations.en;
    return getNestedTranslation(trans, key) || fallback;
}

function getScreenElementLabel(id) {
    const def = findElementDef(id);
    return getScreenTranslation(`screen.elements.${id}`, def ? def.label : id);
}

function getScreenAlignLabel(v) {
    const keys = ['left', 'center', 'right'];
    const key = keys[v];
    const fallback = SCREEN_ALIGN_OPTIONS[v] ? SCREEN_ALIGN_OPTIONS[v].t : '';
    return key ? getScreenTranslation(`screen.align.${key}`, fallback) : fallback;
}

function normalizeElementLayer(z) {
    if (z === undefined || z === null) return 0;
    const n = parseInt(z, 10);
    if (isNaN(n)) return 0;
    return Math.min(SCREEN_LAYER_COUNT - 1, Math.max(0, n));
}

function getElementLayer(element) {
    return normalizeElementLayer(element && element.z);
}

function ensureElementLayer(element) {
    if (!element) return;
    element.z = getElementLayer(element);
}

function appendScreenLayerRow(container, element) {
    ensureElementLayer(element);
    const layerRow = document.createElement('div');
    layerRow.className = 'form-group';
    const layerLabel = document.createElement('label');
    layerLabel.textContent = getScreenTranslation('screen.layer', 'Layer');
    const layerSelect = document.createElement('select');
    SCREEN_LAYER_OPTIONS.forEach(o => {
        const option = document.createElement('option');
        option.value = String(o.v);
        option.textContent = getScreenTranslation(o.key, o.fallback);
        layerSelect.appendChild(option);
    });
    layerSelect.value = String(getElementLayer(element));
    layerSelect.addEventListener('change', () => {
        element.z = normalizeElementLayer(parseInt(layerSelect.value, 10));
        renderScreenCanvas();
    });
    layerRow.appendChild(layerLabel);
    layerRow.appendChild(layerSelect);
    container.appendChild(layerRow);
}

function isScreenSwitchOn(id) {
    const sw = el(id);
    return !!(sw && sw.classList.contains('on'));
}

function appendScreenSwitchRow(container, { id, label, checked, onChange }) {
    const row = document.createElement('div');
    row.className = 'form-group';
    const labelEl = document.createElement('label');
    labelEl.htmlFor = id;
    labelEl.textContent = label;
    const sw = document.createElement('button');
    sw.type = 'button';
    sw.className = 'gswitch' + (checked ? ' on' : '');
    sw.id = id;
    sw.setAttribute('aria-label', label);
    sw.addEventListener('click', () => {
        sw.classList.toggle('on');
        if (onChange) onChange(sw.classList.contains('on'));
    });
    row.appendChild(labelEl);
    row.appendChild(sw);
    container.appendChild(row);
    return sw;
}

function refreshScreenEditorI18n() {
    if (!window.sectionsInitialized.screen) return;
    updateScreenModeButtons();
    renderScreenPalette();
    renderScreenOptions();
    updateScreenStatusLine();
}

function canvasCoordsFromPointer(clientX, clientY) {
    const canvas = el('screenCanvas');
    if (!canvas) return null;
    const rect = canvas.getBoundingClientRect();
    if (clientX < rect.left || clientX > rect.right || clientY < rect.top || clientY > rect.bottom) {
        return null;
    }
    const scale = getScreenScale();
    return {
        x: clamp(Math.round((clientX - rect.left) / scale), 0, SCREEN_SIZE - 1),
        y: clamp(Math.round((clientY - rect.top) / scale), 0, SCREEN_SIZE - 1)
    };
}

function updateScreenStatusLine() {
    const status = el('screenStatus');
    if (!status) return;

    const trans = translations[currentLanguage] || translations.en;
    const profile = getProfileObj(window.screenEditor.mode);
    const selectedId = window.screenEditor.selectedId;

    if (selectedId === 'screen_settings') {
        status.textContent = `Selected: ${getScreenElementLabel('screen_settings')}`;
        return;
    }

    if (selectedId && profile) {
        const elem = profile.elements.find(item => item.id === selectedId);
        const def = findElementDef(selectedId);
        if (elem && def) {
            const size = getScreenElementSize(def, elem);
            const name = getScreenElementLabel(selectedId);
            const template = getScreenTranslation(
                'screen.status_selected_size',
                'Selected: {name} @ ({x},{y}) size ({w},{h})'
            );
            status.textContent = template
                .replace('{name}', name)
                .replace('{x}', String(elem.x ?? 0))
                .replace('{y}', String(elem.y ?? 0))
                .replace('{w}', String(size.w))
                .replace('{h}', String(size.h));
            return;
        }
    }

    const noneLabel = getScreenTranslation('screen.status_none_label', '<None>');
    const template = getScreenTranslation('screen.status_none', 'Selected: {none} @ ({x},{y})');
    const cx = window.screenEditor.cursorX;
    const cy = window.screenEditor.cursorY;
    status.textContent = template
        .replace('{none}', noneLabel)
        .replace('{x}', cx !== null ? String(cx) : '—')
        .replace('{y}', cy !== null ? String(cy) : '—');
}

function trackScreenCanvasPointer(e) {
    const coords = canvasCoordsFromPointer(e.clientX, e.clientY);
    if (coords) {
        window.screenEditor.cursorX = coords.x;
        window.screenEditor.cursorY = coords.y;
    }
    updateScreenStatusLine();
}

function clearScreenCanvasPointer() {
    window.screenEditor.cursorX = null;
    window.screenEditor.cursorY = null;
    updateScreenStatusLine();
}

const SCREEN_THEME_ASSETS = {
    glucose_level: 'glucose',
    glucose_trend: 'trend',
    wifi_off: 'wifi-off',
    weather: 'weather',
    moon: 'moon',
    ampm: 'ampm'
};

window.screenSpiffsFileSet = null;

async function ensureScreenSpiffsFiles(forceRefresh) {
    if (window.screenSpiffsFileSet && !forceRefresh) return;
    try {
        const response = await fetch('/api/files');
        const data = await response.json();
        const files = Array.isArray(data.files) ? data.files : [];
        window.screenSpiffsFileSet = new Set(files.map(f => f.name));
        window.screenSpiffsFileList = files;
    } catch (error) {
        console.warn('Could not load SPIFFS file list for layout editor:', error);
        if (!window.screenSpiffsFileSet) window.screenSpiffsFileSet = new Set();
        if (!window.screenSpiffsFileList) window.screenSpiffsFileList = [];
    }
}

function getScreenLayoutFileNames() {
    const names = (window.screenSpiffsFileList || [])
        .map(f => f.name)
        .filter(name => name && name.toLowerCase().endsWith('.layout'));
    return [...new Set(names)].sort((a, b) => a.localeCompare(b));
}

// Fill a <select> with "Current" (value '') followed by every .layout file on
// the device. Returns the list of file names. Shared by the layout editor and
// the Settings > Display dropdown.
function fillLayoutSelect(selectId) {
    const select = el(selectId);
    if (!select) return [];
    const previous = select.value;
    const names = getScreenLayoutFileNames();

    select.innerHTML = '';
    const current = document.createElement('option');
    current.value = '';
    current.textContent = getScreenTranslation('screen.layout_current', 'Current');
    select.appendChild(current);

    names.forEach(name => {
        const option = document.createElement('option');
        option.value = name;
        option.textContent = name;
        select.appendChild(option);
    });

    select.value = names.includes(previous) ? previous : '';
    return names;
}

async function refreshScreenLayoutSelect() {
    await ensureScreenSpiffsFiles(true);
    const names = fillLayoutSelect('screenLayoutSelect');
    const loadBtn = el('screenLoadSystemBtn');
    if (loadBtn) loadBtn.disabled = names.length === 0;
}

// Settings > Display "Layout" dropdown: same file list, but selecting a file and
// pressing Load applies it straight to the device (there is no editor there).
async function populateSettingsLayoutSelect() {
    await ensureScreenSpiffsFiles(true);
    const names = fillLayoutSelect('settingsLayoutSelect');
    const loadBtn = el('settingsLayoutLoadBtn');
    if (loadBtn) loadBtn.disabled = names.length === 0;
}

async function applySettingsLayoutToDevice() {
    const select = el('settingsLayoutSelect');
    const btn = el('settingsLayoutLoadBtn');
    const filename = select && select.value;
    if (!filename) return; // "Current" selected -> nothing to load

    try {
        toggleLoading(btn, true);
        showStatus(getMessage('saving_settings'), 'info');
        const response = await fetch('/' + encodeURIComponent(filename));
        if (!response.ok) { showStatus(getMessage('layout_read_invalid'), 'error'); return; }
        const data = await response.json();
        if (!data || !data.profiles) { showStatus(getMessage('layout_read_invalid'), 'error'); return; }

        const layout = prepareScreenLayout(data);
        layout.version = SCREEN_LAYOUT_VERSION;
        ensureScreenLayoutMeta(layout);
        ensureScreenProfileShape(layout.profiles.day);
        ensureScreenProfileShape(layout.profiles.night);

        const body = encodeScreenLayoutBinary(layout);
        if (body.byteLength !== SCREEN_BIN_WIRE_SIZE) {
            throw new Error(`screen wire size mismatch: ${body.byteLength} != ${SCREEN_BIN_WIRE_SIZE}`);
        }
        const post = await fetch('/api/screen', {
            method: 'POST',
            headers: { 'Content-Type': 'application/octet-stream' },
            body
        });
        const pdata = await post.json().catch(() => ({}));
        if (post.ok && pdata && pdata.status === 'ok') {
            // Keep the editor and cached settings in sync with what we pushed.
            window.screenLayout = layout;
            if (window.settings) {
                window.settings.p04 = layout.day_font;
                window.settings.p05 = layout.night_font;
                window.settings.p10 = layout.day_color_filter;
                window.settings.p11 = layout.night_color_filter;
            }
            showStatus(getMessage('layout_applied'), 'success');
        } else {
            const detail = pdata && pdata.error ? pdata.error : 'Unknown error';
            showStatus(getMessage('layout_apply_failed') + detail, 'error');
        }
    } catch (error) {
        console.error('Error applying layout:', error);
        showStatus(getMessage('layout_apply_failed') + (error.message || ''), 'error');
    } finally {
        toggleLoading(btn, false);
    }
}

function hasSpiffsFile(name) {
    return window.screenSpiffsFileSet && window.screenSpiffsFileSet.has(name);
}

function resolveThemeAssetUrl(theme, item) {
    for (const ext of ['bmp', 'jpg']) {
        const themed = `${theme}-${item}.${ext}`;
        if (hasSpiffsFile(themed)) return themed;
    }
    for (const ext of ['bmp', 'jpg']) {
        const fallback = `default-${item}.${ext}`;
        if (hasSpiffsFile(fallback)) return fallback;
    }
    return 'logo.jpg';
}

function resolveTimeFontPreviewUrl(font) {
    if (hasSpiffsFile(`${font}.jpg`)) return `${font}.jpg`;
    if (font === 'user1' || font === 'user2') return 'bold.jpg';
    return `${font}.jpg`;
}

function getScreenAuxFont(mode) {
    if (!window.screenLayout) return 'bold';
    return mode === 'night'
        ? (window.screenLayout.night_aux_font || 'bold')
        : (window.screenLayout.day_aux_font || 'bold');
}

function getPreviewImageUrl(def, inMatrix) {
    if (!inMatrix && def.paletteImg) return def.paletteImg;
    if (def.id === 'time') {
        const font = inMatrix ? getScreenTimeFont(window.screenEditor.mode) : 'bold';
        return resolveTimeFontPreviewUrl(font);
    }
    if (def.id === 'time_aux') {
        const font = inMatrix ? getScreenAuxFont(window.screenEditor.mode) : 'bold';
        return resolveTimeFontPreviewUrl(font);
    }
    if (isScreenIconElement(def.id)) {
        // Show the uploaded JPG only once it has loaded (dims cached); until then
        // return null so the placeholder "Icon N" label is shown instead.
        const dims = getIconDims(def.id);
        return dims ? iconUrl(def.id) : null;
    }
    const item = SCREEN_THEME_ASSETS[def.id];
    if (item && inMatrix) {
        return resolveThemeAssetUrl(getScreenTimeFont(window.screenEditor.mode), item);
    }
    return def.img;
}

const SCREEN_ELEMENT_DEFS = [
    { id: 'glucose_level', label: 'Glucose Level', w: 14, h: 20, img: 'default-glucose.jpg', paletteImg: 'palette-glucose.png', paletteFitFullImage: true, spriteSheet: { index: 2, frames: 4 } },
    { id: 'glucose_trend', label: 'Glucose Trend', w: 12, h: 14, img: 'default-trend.jpg', paletteImg: 'palette-trend.png', paletteFitFullImage: true, spriteSheet: { index: 2, frames: 5 } },
    { id: 'wifi_off', label: 'WiFi off', w: 20, h: 20, img: 'default-wifi-off.jpg', paletteImg: 'palette-wifi-off.png', paletteFitFullImage: true },
    { id: 'weather', label: 'Weather', w: 32, h: 22, img: 'default-weather.jpg' },
    { id: 'moon', label: 'Moon', w: 14, h: 14, img: 'default-moon.jpg', paletteImg: 'palette-moon.jpg', paletteFitFullImage: true, spriteSheet: { index: 2, frames: 8 } },
    { id: 'time', label: 'Digit Display', w: 80, h: 36, img: 'bold.jpg', paletteFitFullImage: true },
    { id: 'digit_label', label: 'Digit Label', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.digit_label, img: 'bold-digit-label.jpg', paletteFitFullImage: true },
    { id: 'time_aux', label: 'Digit Display (Aux)', w: 80, h: 36, img: 'bold.jpg', paletteFitFullImage: true },
    { id: 'digit_label_aux', label: 'Digit Label (Aux)', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.digit_label_aux, img: 'bold-digit-label.jpg', paletteFitFullImage: true },
    { id: 'ampm', label: 'AM/PM', w: 10, h: 20, img: 'default-ampm.jpg' },
    { id: 'message', label: 'Scrolling Message', w: 128, h: 8, dynamicHeight: true, text: SCREEN_PREVIEW_MESSAGE, paletteImg: 'palette-message.png', paletteFitFullImage: true },
    { id: 'text_1', label: 'Text 1', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_1 },
    { id: 'text_2', label: 'Text 2', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_2 },
    { id: 'text_3', label: 'Text 3', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_3 },
    { id: 'text_4', label: 'Text 4', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_4 },
    { id: 'text_5', label: 'Text 5', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_5 },
    { id: 'text_6', label: 'Text 6', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_6 },
    { id: 'text_7', label: 'Text 7', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_7 },
    { id: 'text_8', label: 'Text 8', w: 80, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_8 },
    { id: 'graph', label: 'Graph', w: 80, h: 36, img: 'default-graph.jpg', paletteImg: 'palette-graph.png', paletteFitFullImage: true },
    { id: 'icon_1', label: 'Icon 1', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_2', label: 'Icon 2', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_3', label: 'Icon 3', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_4', label: 'Icon 4', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_5', label: 'Icon 5', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_6', label: 'Icon 6', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_7', label: 'Icon 7', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_8', label: 'Icon 8', w: 24, h: 24, isIcon: true }
];

const SCREEN_TEXT_SLOT_IDS = ['text_1', 'text_2', 'text_3', 'text_4', 'text_5', 'text_6', 'text_7', 'text_8'];

const SCREEN_PALETTE_TEXT_DEF = {
    id: 'text',
    label: 'New Text Line',
    w: 80,
    h: 8,
    dynamicHeight: true
};

const SCREEN_ICON_SLOT_IDS = ['icon_1', 'icon_2', 'icon_3', 'icon_4', 'icon_5', 'icon_6', 'icon_7', 'icon_8'];
const SCREEN_ICON_MAX_DIM = 64; // must match firmware SCREEN_ICON_MAX_DIM

const SCREEN_PALETTE_ICON_DEF = {
    id: 'icon',
    label: 'New Icon',
    w: 24,
    h: 24,
    isIcon: true
};

const SCREEN_PALETTE_SETTINGS_DEF = {
    id: 'screen_settings',
    label: 'Screen settings',
    w: 32,
    h: 32,
    img: 'default-settings.jpg',
    paletteFitFullImage: true,
    paletteMeta: true
};

const SCREEN_PALETTE_ORDER = [
    'screen_settings',
    'time',
    'digit_label',
    'ampm',
    'message',
    'time_aux',
    'digit_label_aux',
    'weather',
    'moon',
    'wifi_off',
    'glucose_level',
    'glucose_trend',
    'graph',
    'text',
    'icon'
];

const SCREEN_ALIGNMENT_GRID_LINES = [20, 30, 40, 90, 100, 110];

function getPaletteNewTextIconLabel() {
    return getScreenTranslation('screen.palette_text_icon', 'Text');
}

function truncatePaletteLabel(text, maxLen = SCREEN_PALETTE_LABEL_MAX) {
    const s = (text || '').trim();
    if (!s) return '';
    return s.length > maxLen ? s.slice(0, maxLen) + '\u2026' : s;
}

function getStaticTextContent(profile, id) {
    if (!profile) return '';
    if (profile.static_texts && profile.static_texts[id] !== undefined) {
        return profile.static_texts[id];
    }
    return SCREEN_DEFAULT_STATIC_TEXTS[id] || '';
}

function getTextPaletteLabel(profile, id) {
    const emptyLabel = getScreenTranslation('screen.empty_text_label', '(empty)');
    const content = getStaticTextContent(profile, id).trim();
    return truncatePaletteLabel(content) || emptyLabel;
}

function getScreenPaletteItemLabel(def, profile) {
    if (def.id === 'text') return getScreenElementLabel('text');
    if (def.id === 'icon') return getScreenElementLabel('icon');
    if (isScreenStaticTextElement(def.id) && profile) return getTextPaletteLabel(profile, def.id);
    return getScreenElementLabel(def.id);
}

function isPaletteItemSelected(def) {
    return window.screenEditor.selectedId === def.id;
}

function getNextTextSlot(profile) {
    if (!profile || !profile.elements) return SCREEN_TEXT_SLOT_IDS[0];
    for (const id of SCREEN_TEXT_SLOT_IDS) {
        const elem = profile.elements.find(e => e.id === id);
        if (!elem || !elem.enabled) return id;
    }
    return null;
}

function getNextIconSlot(profile) {
    if (!profile || !profile.elements) return SCREEN_ICON_SLOT_IDS[0];
    for (const id of SCREEN_ICON_SLOT_IDS) {
        const elem = profile.elements.find(e => e.id === id);
        if (!elem || !elem.enabled) return id;
    }
    return null;
}

function isScreenIconElement(id) {
    return /^icon_[1-8]$/.test(id);
}

// Slot id 'icon_3' -> SPIFFS file name 'icon3.jpg' (and URL '/icon3.jpg').
function iconFileName(id) {
    const m = /^icon_([1-8])$/.exec(id);
    return m ? `icon${m[1]}.jpg` : null;
}

function getScreenPaletteDefs() {
    const profile = getProfileObj(window.screenEditor.mode);
    const items = [];

    SCREEN_PALETTE_ORDER.forEach(id => {
        if (id === 'screen_settings') {
            items.push(SCREEN_PALETTE_SETTINGS_DEF);
            return;
        }
        if (id === 'text') {
            if (getNextTextSlot(profile) !== null) {
                items.push(SCREEN_PALETTE_TEXT_DEF);
            }
            return;
        }
        if (id === 'icon') {
            if (getNextIconSlot(profile) !== null) {
                items.push(SCREEN_PALETTE_ICON_DEF);
            }
            return;
        }
        const def = findElementDef(id);
        if (def && !isScreenStaticTextElement(def.id) && !isScreenIconElement(def.id)) {
            items.push(def);
        }
    });

    if (profile) {
        SCREEN_TEXT_SLOT_IDS.forEach(id => {
            const elem = profile.elements.find(e => e.id === id);
            if (elem && elem.enabled) {
                const def = findElementDef(id);
                if (def) items.push({ ...def });
            }
        });
        SCREEN_ICON_SLOT_IDS.forEach(id => {
            const elem = profile.elements.find(e => e.id === id);
            if (elem && elem.enabled) {
                const def = findElementDef(id);
                if (def) items.push({ ...def });
            }
        });
    }

    return items;
}

function isScreenSettingsElement(id) {
    return id === 'screen_settings';
}

function isScreenMetaElement(id) {
    return isScreenSettingsElement(id);
}

// Last grid (p08) state known to be persisted on the device. Tracked separately
// from window.settings.p08 because the toggle mutates window.settings.p08 live
// (for the canvas preview) before the save runs -- if the save diffed against
// window.settings.p08 it would always see "no change" and never POST p08.
let screenGridDeviceState = 0;

function getScreenGridEnabled() {
    return !!(window.settings && window.settings.p08);
}

function setScreenGridEnabled(enabled) {
    if (!window.settings) window.settings = {};
    window.settings.p08 = enabled ? 1 : 0;
}

function resolvePaletteElementId(paletteId, profile) {
    if (paletteId === 'text') return getNextTextSlot(profile);
    if (paletteId === 'icon') return getNextIconSlot(profile);
    return paletteId;
}

function isScreenDigitLabelElement(id) {
    return id === 'digit_label' || id === 'digit_label_aux';
}

function isScreenTextElement(id) {
    return id === 'message' || /^text_[1-8]$/.test(id) || isScreenDigitLabelElement(id);
}

function isScreenStaticTextElement(id) {
    return /^text_[1-8]$/.test(id);
}

function isScreenClippedTextElement(id) {
    return isScreenStaticTextElement(id) || isScreenDigitLabelElement(id);
}

function getDigitLabelTextKey(id) {
    return id;
}

function ensureScreenTextOptions(element) {
    if (!element.options) element.options = {};
    if (element.options.font === undefined || element.options.font === 255) element.options.font = 0;
    if (!element.options.color) element.options.color = '#ffffff';
    if (!element.options.bg_color) element.options.bg_color = '#000000';
    if (isScreenStaticTextElement(element.id)) {
        if (element.options.width === undefined) element.options.width = 80;
        if (element.options.align === undefined) element.options.align = 0;
    }
    if (isScreenDigitLabelElement(element.id)) {
        if (element.options.width === undefined) element.options.width = 80;
        if (element.options.align === undefined) element.options.align = 0;
    }
}

function makeDefaultScreenProfile() {
    const elements = [
        { id: 'glucose_level', enabled: 1, x: 27, y: 27, z: 4 },
        { id: 'glucose_trend', enabled: 1, x: 42, y: 30, z: 4 },
        { id: 'wifi_off', enabled: 1, x: 22, y: 27, z: 4 },
        { id: 'weather', enabled: 1, x: 54, y: 24, z: 3 },
        { id: 'moon', enabled: 1, x: 87, y: 29, z: 3 },
        { id: 'time', enabled: 1, x: 22, y: 47, z: 1 },
        { id: 'digit_label', enabled: 0, x: 22, y: 84, z: 1, options: { font: 0, color: '#ffffff', bg_color: '#000000', width: 80, align: 0 } },
        { id: 'ampm', enabled: 1, x: 101, y: 54, z: 2 },
        { id: 'time_aux', enabled: 0, x: 22, y: 47, z: 1 },
        { id: 'digit_label_aux', enabled: 0, x: 22, y: 84, z: 1, options: { font: 0, color: '#ffffff', bg_color: '#000000', width: 80, align: 0 } },
        { id: 'message', enabled: 1, x: 0, y: 86, z: 0, options: { font: 0, color: '#ffffff', bg_color: '#000000' } }
    ];
    SCREEN_TEXT_SLOT_IDS.forEach((id, index) => {
        elements.push({
            id,
            enabled: 0,
            x: 45,
            y: 90,
            z: Math.min(4, 1 + index),
            options: { font: 0, color: '#ffffff', bg_color: '#000000', width: 80, align: 0 }
        });
    });
    SCREEN_ICON_SLOT_IDS.forEach(id => {
        // Icons carry no text styling; only enabled/x/y/z are used. Source is
        // S:/icon<N>.jpg, uploaded by the user.
        elements.push({ id, enabled: 0, x: 48, y: 48, z: 2 });
    });
    return {
        scroll_text: SCREEN_DEFAULT_SCROLL_TEXT,
        static_texts: { ...SCREEN_DEFAULT_STATIC_TEXTS },
        elements
    };
}

const SCREEN_LAYOUT_VERSION = 10;

/* Compact binary wire format for /api/screen (must match f-screen-layout-bin.h). */
const SCREEN_BIN_MAGIC = 0x4653584C;
const SCREEN_BIN_FORMAT = 1;
const SCREEN_BIN_FONT_LEN = 12;
const SCREEN_BIN_WIDGET_SIZE = 13;
const SCREEN_BIN_WIDGET_COUNT = 28; // glucose..graph (20) + 8 icon slots
const SCREEN_BIN_SCROLL_LEN = 512;
const SCREEN_BIN_STATIC_TEXT_LEN = 96;
const SCREEN_BIN_STATIC_TEXT_COUNT = 8;
const SCREEN_BIN_GRAPH_SIZE = 88; // screen_graph_cfg_t (token[64] + 24)
const SCREEN_BIN_GRAPH_TOKEN_LEN = 64;
const SCREEN_BIN_HEADER_SIZE = 64;
const SCREEN_BIN_PROFILE_SIZE =
    SCREEN_BIN_WIDGET_COUNT * SCREEN_BIN_WIDGET_SIZE +
    SCREEN_BIN_SCROLL_LEN +
    SCREEN_BIN_STATIC_TEXT_COUNT * SCREEN_BIN_STATIC_TEXT_LEN +
    SCREEN_BIN_STATIC_TEXT_LEN * 2 +
    SCREEN_BIN_GRAPH_SIZE;
const SCREEN_BIN_WIRE_SIZE = SCREEN_BIN_HEADER_SIZE + SCREEN_BIN_PROFILE_SIZE * 2;
const SCREEN_BIN_MIME = 'application/vnd.frixos.screen-layout+1';

const GRAPH_VAL_UNSET = -32768; // int16 sentinel for "not set"
const GRAPH_FLAG = { AUTOSCALE: 0x01, SHOW_AXIS: 0x02, BAND: 0x04, BACKFILL: 0x08, SHOW_VALUE: 0x10, BOOLEAN: 0x20, THICK: 0x40 };

/* Firmware widget[] index order (screen_element_id_t). */
const SCREEN_WIRE_ELEM_IDS = [
    'glucose_level', 'glucose_trend', 'wifi_off', 'weather', 'moon', 'time', 'message',
    'text_1', 'text_2', 'text_3', 'text_4', 'text_5', 'text_6', 'text_7', 'text_8',
    'ampm', 'time_aux', 'digit_label', 'digit_label_aux', 'graph',
    'icon_1', 'icon_2', 'icon_3', 'icon_4', 'icon_5', 'icon_6', 'icon_7', 'icon_8'
];

function screenHexToRgb(hex, fallback) {
    if (!hex || hex.length !== 7 || hex[0] !== '#') return fallback;
    const n = parseInt(hex.slice(1), 16);
    if (Number.isNaN(n)) return fallback;
    return { r: (n >> 16) & 0xff, g: (n >> 8) & 0xff, b: n & 0xff };
}

function screenRgbToHex(r, g, b) {
    const h = (v) => v.toString(16).padStart(2, '0');
    return `#${h(r)}${h(g)}${h(b)}`;
}

function screenReadFixedString(view, offset, len) {
    const bytes = new Uint8Array(view.buffer, view.byteOffset + offset, len);
    let end = bytes.indexOf(0);
    if (end < 0) end = len;
    return new TextDecoder().decode(bytes.subarray(0, end));
}

function screenWriteFixedString(view, offset, len, value) {
    const bytes = new Uint8Array(view.buffer, view.byteOffset + offset, len);
    bytes.fill(0);
    const text = value == null ? '' : String(value);
    const encoded = new TextEncoder().encode(text);
    bytes.set(encoded.subarray(0, Math.min(encoded.length, len - 1)));
}

function screenFindProfileElement(profile, id) {
    if (!profile || !Array.isArray(profile.elements)) return null;
    let elem = profile.elements.find(e => e && e.id === id);
    if (!elem && id === 'time_aux') {
        elem = profile.elements.find(e => e && e.id === 'glucose');
    }
    return elem;
}

function screenReadWidget(view, offset) {
    return {
        enabled: view.getUint8(offset) ? 1 : 0,
        x: view.getUint8(offset + 1),
        y: view.getUint8(offset + 2),
        z: view.getUint8(offset + 3),
        font: view.getUint8(offset + 4),
        width: view.getUint8(offset + 5),
        align: view.getUint8(offset + 6),
        color_r: view.getUint8(offset + 7),
        color_g: view.getUint8(offset + 8),
        color_b: view.getUint8(offset + 9),
        bg_r: view.getUint8(offset + 10),
        bg_g: view.getUint8(offset + 11),
        bg_b: view.getUint8(offset + 12)
    };
}

function screenWriteWidget(view, offset, elem) {
    const opts = elem && elem.options ? elem.options : {};
    const fg = screenHexToRgb(opts.color, { r: 255, g: 255, b: 255 });
    const bg = screenHexToRgb(opts.bg_color, { r: 0, g: 0, b: 0 });
    view.setUint8(offset, elem && elem.enabled ? 1 : 0);
    view.setUint8(offset + 1, elem && elem.x != null ? elem.x : 0);
    view.setUint8(offset + 2, elem && elem.y != null ? elem.y : 0);
    view.setUint8(offset + 3, elem && elem.z != null ? elem.z : 0);
    view.setUint8(offset + 4, opts.font != null ? opts.font : 0);
    view.setUint8(offset + 5, opts.width != null ? opts.width : 0);
    view.setUint8(offset + 6, opts.align != null ? opts.align : 0);
    view.setUint8(offset + 7, fg.r);
    view.setUint8(offset + 8, fg.g);
    view.setUint8(offset + 9, fg.b);
    view.setUint8(offset + 10, bg.r);
    view.setUint8(offset + 11, bg.g);
    view.setUint8(offset + 12, bg.b);
}

// Read/write the per-profile screen_graph_cfg_t (56 bytes, packed LE) into an
// editor-friendly options object. GRAPH_VAL_UNSET maps to null.
// Field offsets within screen_graph_cfg_t (after the token[64] block).
const GCFG = { TOKEN: 0, INTERVAL: 64, POINTS: 66, WIDTH: 67, HEIGHT: 68, FLAGS: 69,
    BAND_LOW: 70, BAND_HIGH: 72, Y_MIN: 74, Y_MAX: 76, BAND_RGB: 78, WARN_RGB: 81, AXIS_RGB: 84 };

function screenReadGraphCfg(view, off) {
    const i16 = (o) => { const v = view.getInt16(o, true); return v === GRAPH_VAL_UNSET ? null : v; };
    const flags = view.getUint8(off + GCFG.FLAGS);
    return {
        token: screenReadFixedString(view, off + GCFG.TOKEN, SCREEN_BIN_GRAPH_TOKEN_LEN),
        interval_min: view.getUint16(off + GCFG.INTERVAL, true) || 5,
        points: view.getUint8(off + GCFG.POINTS) || 60,
        gwidth: view.getUint8(off + GCFG.WIDTH) || 80,
        gheight: view.getUint8(off + GCFG.HEIGHT) || 36,
        autoscale: !!(flags & GRAPH_FLAG.AUTOSCALE),
        show_axis: !!(flags & GRAPH_FLAG.SHOW_AXIS),
        band_on: !!(flags & GRAPH_FLAG.BAND),
        backfill: !!(flags & GRAPH_FLAG.BACKFILL),
        show_value: !!(flags & GRAPH_FLAG.SHOW_VALUE),
        boolean: !!(flags & GRAPH_FLAG.BOOLEAN),
        thick: !!(flags & GRAPH_FLAG.THICK),
        band_low: i16(off + GCFG.BAND_LOW),
        band_high: i16(off + GCFG.BAND_HIGH),
        y_min: i16(off + GCFG.Y_MIN),
        y_max: i16(off + GCFG.Y_MAX),
        band_color: screenRgbToHex(view.getUint8(off + GCFG.BAND_RGB), view.getUint8(off + GCFG.BAND_RGB + 1), view.getUint8(off + GCFG.BAND_RGB + 2)),
        warn_color: screenRgbToHex(view.getUint8(off + GCFG.WARN_RGB), view.getUint8(off + GCFG.WARN_RGB + 1), view.getUint8(off + GCFG.WARN_RGB + 2)),
        axis_color: screenRgbToHex(view.getUint8(off + GCFG.AXIS_RGB), view.getUint8(off + GCFG.AXIS_RGB + 1), view.getUint8(off + GCFG.AXIS_RGB + 2))
    };
}

function screenWriteGraphCfg(view, off, o) {
    o = o || {};
    const i16 = (v) => (v == null ? GRAPH_VAL_UNSET : (v | 0));
    screenWriteFixedString(view, off + GCFG.TOKEN, SCREEN_BIN_GRAPH_TOKEN_LEN, o.token || '');
    view.setUint16(off + GCFG.INTERVAL, o.interval_min != null ? o.interval_min : 5, true);
    view.setUint8(off + GCFG.POINTS, o.points != null ? o.points : 60);
    view.setUint8(off + GCFG.WIDTH, o.gwidth != null ? o.gwidth : 80);
    view.setUint8(off + GCFG.HEIGHT, o.gheight != null ? o.gheight : 36);
    let flags = 0;
    if (o.autoscale) flags |= GRAPH_FLAG.AUTOSCALE;
    if (o.show_axis) flags |= GRAPH_FLAG.SHOW_AXIS;
    if (o.band_on) flags |= GRAPH_FLAG.BAND;
    if (o.backfill) flags |= GRAPH_FLAG.BACKFILL;
    if (o.show_value) flags |= GRAPH_FLAG.SHOW_VALUE;
    if (o.boolean) flags |= GRAPH_FLAG.BOOLEAN;
    if (o.thick) flags |= GRAPH_FLAG.THICK;
    view.setUint8(off + GCFG.FLAGS, flags);
    view.setInt16(off + GCFG.BAND_LOW, i16(o.band_low), true);
    view.setInt16(off + GCFG.BAND_HIGH, i16(o.band_high), true);
    view.setInt16(off + GCFG.Y_MIN, i16(o.y_min), true);
    view.setInt16(off + GCFG.Y_MAX, i16(o.y_max), true);
    const bc = screenHexToRgb(o.band_color, { r: 40, g: 60, b: 40 });
    const wc = screenHexToRgb(o.warn_color, { r: 255, g: 80, b: 80 });
    const ac = screenHexToRgb(o.axis_color, { r: 120, g: 120, b: 120 });
    view.setUint8(off + GCFG.BAND_RGB, bc.r); view.setUint8(off + GCFG.BAND_RGB + 1, bc.g); view.setUint8(off + GCFG.BAND_RGB + 2, bc.b);
    view.setUint8(off + GCFG.WARN_RGB, wc.r); view.setUint8(off + GCFG.WARN_RGB + 1, wc.g); view.setUint8(off + GCFG.WARN_RGB + 2, wc.b);
    view.setUint8(off + GCFG.AXIS_RGB, ac.r); view.setUint8(off + GCFG.AXIS_RGB + 1, ac.g); view.setUint8(off + GCFG.AXIS_RGB + 2, ac.b);
    view.setUint8(off + 87, 0); // reserved
}

function screenWireProfileToJson(view, offset) {
    const profile = {
        elements: [],
        scroll_text: '',
        static_texts: { ...SCREEN_DEFAULT_STATIC_TEXTS }
    };
    let off = offset;

    SCREEN_WIRE_ELEM_IDS.forEach(id => {
        const w = screenReadWidget(view, off);
        off += SCREEN_BIN_WIDGET_SIZE;
        const elem = { id, enabled: w.enabled, x: w.x, y: w.y, z: w.z };
        if (isScreenTextElement(id)) {
            elem.options = {
                font: w.font,
                color: screenRgbToHex(w.color_r, w.color_g, w.color_b),
                bg_color: screenRgbToHex(w.bg_r, w.bg_g, w.bg_b)
            };
            if (isScreenClippedTextElement(id) || id === 'message') {
                elem.options.width = w.width;
                elem.options.align = w.align;
            }
        } else if (id === 'graph') {
            // line colour reuses the widget colour, background reuses bg
            elem.options = {
                color: screenRgbToHex(w.color_r, w.color_g, w.color_b),
                bg_color: screenRgbToHex(w.bg_r, w.bg_g, w.bg_b)
            };
        }
        profile.elements.push(elem);
    });

    profile.scroll_text = screenReadFixedString(view, off, SCREEN_BIN_SCROLL_LEN);
    off += SCREEN_BIN_SCROLL_LEN;
    SCREEN_TEXT_SLOT_IDS.forEach(id => {
        profile.static_texts[id] = screenReadFixedString(view, off, SCREEN_BIN_STATIC_TEXT_LEN);
        off += SCREEN_BIN_STATIC_TEXT_LEN;
    });
    profile.static_texts.digit_label = screenReadFixedString(view, off, SCREEN_BIN_STATIC_TEXT_LEN);
    off += SCREEN_BIN_STATIC_TEXT_LEN;
    profile.static_texts.digit_label_aux = screenReadFixedString(view, off, SCREEN_BIN_STATIC_TEXT_LEN);
    off += SCREEN_BIN_STATIC_TEXT_LEN;

    // Per-profile graph config -> merge into the graph element's options.
    const gcfg = screenReadGraphCfg(view, off);
    off += SCREEN_BIN_GRAPH_SIZE;
    const gelem = profile.elements.find(e => e && e.id === 'graph');
    if (gelem) gelem.options = Object.assign({}, gelem.options || {}, gcfg);

    return { profile, offset: off };
}

function decodeScreenLayoutBinary(bytes) {
    if (!bytes || bytes.byteLength !== SCREEN_BIN_WIRE_SIZE) {
        throw new Error('invalid screen wire size');
    }
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
    if (view.getUint32(0, true) !== SCREEN_BIN_MAGIC) {
        throw new Error('invalid screen wire magic');
    }
    if (view.getUint8(4) !== SCREEN_BIN_FORMAT) {
        throw new Error('unsupported screen wire format');
    }

    const layout = {
        version: view.getUint8(5),
        scroll_delay: view.getUint8(6),
        day_color_filter: view.getUint8(7),
        night_color_filter: view.getUint8(8),
        day_font: screenReadFixedString(view, 12, SCREEN_BIN_FONT_LEN),
        night_font: screenReadFixedString(view, 24, SCREEN_BIN_FONT_LEN),
        day_aux_font: screenReadFixedString(view, 36, SCREEN_BIN_FONT_LEN),
        night_aux_font: screenReadFixedString(view, 48, SCREEN_BIN_FONT_LEN),
        w: view.getUint16(60, true),
        h: view.getUint16(62, true),
        profiles: {}
    };

    let off = SCREEN_BIN_HEADER_SIZE;
    const day = screenWireProfileToJson(view, off);
    off = day.offset;
    const night = screenWireProfileToJson(view, off);
    layout.profiles.day = day.profile;
    layout.profiles.night = night.profile;
    return layout;
}

function encodeScreenLayoutBinary(layout) {
    ensureScreenLayoutMeta(layout);
    ensureScreenProfileShape(layout.profiles.day);
    ensureScreenProfileShape(layout.profiles.night);

    const buf = new ArrayBuffer(SCREEN_BIN_WIRE_SIZE);
    const view = new DataView(buf);
    const bytes = new Uint8Array(buf);
    bytes.fill(0);

    view.setUint32(0, SCREEN_BIN_MAGIC, true);
    view.setUint8(4, SCREEN_BIN_FORMAT);
    view.setUint8(5, SCREEN_LAYOUT_VERSION);
    view.setUint8(6, layout.scroll_delay != null ? layout.scroll_delay : 65);
    view.setUint8(7, layout.day_color_filter != null ? layout.day_color_filter : 0);
    view.setUint8(8, layout.night_color_filter != null ? layout.night_color_filter : 0);
    screenWriteFixedString(view, 12, SCREEN_BIN_FONT_LEN, layout.day_font || 'bold');
    screenWriteFixedString(view, 24, SCREEN_BIN_FONT_LEN, layout.night_font || 'bold');
    screenWriteFixedString(view, 36, SCREEN_BIN_FONT_LEN, layout.day_aux_font || layout.day_font || 'bold');
    screenWriteFixedString(view, 48, SCREEN_BIN_FONT_LEN, layout.night_aux_font || layout.night_font || 'bold');
    view.setUint16(60, layout.w || SCREEN_SIZE, true);
    view.setUint16(62, layout.h || SCREEN_SIZE, true);

    let off = SCREEN_BIN_HEADER_SIZE;
    ['day', 'night'].forEach(mode => {
        const profile = layout.profiles[mode];
        SCREEN_WIRE_ELEM_IDS.forEach(id => {
            screenWriteWidget(view, off, screenFindProfileElement(profile, id));
            off += SCREEN_BIN_WIDGET_SIZE;
        });
        screenWriteFixedString(view, off, SCREEN_BIN_SCROLL_LEN, profile.scroll_text || '');
        off += SCREEN_BIN_SCROLL_LEN;
        SCREEN_TEXT_SLOT_IDS.forEach(id => {
            const text = profile.static_texts && profile.static_texts[id] != null
                ? profile.static_texts[id]
                : '';
            screenWriteFixedString(view, off, SCREEN_BIN_STATIC_TEXT_LEN, text);
            off += SCREEN_BIN_STATIC_TEXT_LEN;
        });
        screenWriteFixedString(view, off, SCREEN_BIN_STATIC_TEXT_LEN,
            profile.static_texts && profile.static_texts.digit_label != null ? profile.static_texts.digit_label : '');
        off += SCREEN_BIN_STATIC_TEXT_LEN;
        screenWriteFixedString(view, off, SCREEN_BIN_STATIC_TEXT_LEN,
            profile.static_texts && profile.static_texts.digit_label_aux != null ? profile.static_texts.digit_label_aux : '');
        off += SCREEN_BIN_STATIC_TEXT_LEN;
        const gelem = screenFindProfileElement(profile, 'graph');
        screenWriteGraphCfg(view, off, gelem && gelem.options ? gelem.options : {});
        off += SCREEN_BIN_GRAPH_SIZE;
    });

    return bytes;
}

const SCREEN_DEFAULT_LAYOUT = {
    version: SCREEN_LAYOUT_VERSION,
    scroll_delay: 65,
    day_font: 'bold',
    night_font: 'bold',
    day_aux_font: 'bold',
    night_aux_font: 'bold',
    day_color_filter: 0,
    night_color_filter: 0,
    w: 128,
    h: 128,
    profiles: {
        day: makeDefaultScreenProfile(),
        night: makeDefaultScreenProfile()
    }
};

window.screenLayout = null;
window.screenEditor = {
    mode: 'day', // 'day' | 'night'
    scale: 4,
    dragging: null, // { id, originX, originY, startLeft, startTop, fromPalette }
    selectedId: null,
    cursorX: null,
    cursorY: null
};

function getScreenScale() {
    const canvas = el('screenCanvas');
    if (!canvas) return 4;
    const rect = canvas.getBoundingClientRect();
    return Math.max(1, Math.round(rect.width / SCREEN_SIZE));
}

function getProfileObj(mode) {
    return window.screenLayout && window.screenLayout.profiles ? window.screenLayout.profiles[mode] : null;
}

function findElementDef(id) {
    if (id === 'text') return SCREEN_PALETTE_TEXT_DEF;
    if (id === 'icon') return SCREEN_PALETTE_ICON_DEF;
    if (id === 'screen_settings') return SCREEN_PALETTE_SETTINGS_DEF;
    return SCREEN_ELEMENT_DEFS.find(d => d.id === id) || null;
}

function evenPx(value) {
    const rounded = Math.max(2, Math.round(value));
    return rounded % 2 === 0 ? rounded : rounded + 1;
}

function getScreenLabelTopTrim(id) {
    return isScreenTextElement(id) ? SCREEN_LABEL_LAYOUT_TRIM : 0;
}

function layoutYToVisualY(layoutY, id) {
    return layoutY + getScreenLabelTopTrim(id);
}

function visualYToLayoutY(visualY, id) {
    return Math.max(0, visualY - getScreenLabelTopTrim(id));
}

function getScreenLabelLineHeight(fontIndex) {
    return SCREEN_MESSAGE_FONT_LINE_HEIGHTS[fontIndex] || 12;
}

function getScreenLabelBoxHeight(fontIndex) {
    if (SCREEN_LABEL_VISIBLE_HEIGHTS[fontIndex] !== undefined) {
        return SCREEN_LABEL_VISIBLE_HEIGHTS[fontIndex];
    }
    return Math.max(1, getScreenLabelLineHeight(fontIndex) - SCREEN_LABEL_LAYOUT_TRIM);
}

// Natural dimensions (clamped to the device max) of an uploaded icon, cached.
// Probes /icon<N>.jpg once and re-renders the canvas when it loads, so the box
// matches what the device draws (icons render at their native JPG size).
const screenIconDims = {};    // id -> {w,h}
const screenIconProbing = {}; // id -> true while a probe is in flight
let screenIconBust = 0;       // bumped on upload/delete to defeat the browser cache
function iconUrl(id) {
    const f = iconFileName(id);
    if (!f) return null;
    return '/' + f + (screenIconBust ? '?b=' + screenIconBust : '');
}
function getIconDims(id) {
    if (screenIconDims[id]) return screenIconDims[id];
    if (!iconFileName(id) || screenIconProbing[id]) return null;
    screenIconProbing[id] = true;
    const img = new Image();
    img.onload = () => {
        screenIconProbing[id] = false;
        screenIconDims[id] = {
            w: Math.min(SCREEN_ICON_MAX_DIM, img.naturalWidth || 24),
            h: Math.min(SCREEN_ICON_MAX_DIM, img.naturalHeight || 24)
        };
        renderScreenCanvas();
    };
    img.onerror = () => { screenIconProbing[id] = false; };
    img.src = iconUrl(id);
    return null;
}
function invalidateIconDims(id) {
    delete screenIconDims[id];
    delete screenIconProbing[id];
}

function getScreenElementSize(def, elementState) {
    if (!def) return { w: 0, h: 0 };
    let w;
    let h;
    if (isScreenTextElement(def.id)) {
        const font = elementState && elementState.options && elementState.options.font !== undefined ? elementState.options.font : 0;
        w = isScreenClippedTextElement(def.id) && elementState && elementState.options && elementState.options.width > 0
            ? elementState.options.width
            : (isScreenClippedTextElement(def.id) ? def.w : 128);
        h = getScreenLabelBoxHeight(font);
        return { w: evenPx(w), h };
    }
    if (isScreenIconElement(def.id)) {
        const d = getIconDims(def.id); // null until the JPG (if any) has loaded
        return { w: evenPx(d ? d.w : def.w), h: evenPx(d ? d.h : def.h) };
    }
    w = def.w;
    h = def.h;
    return { w: evenPx(w), h: evenPx(h) };
}

function getScreenTimeFont(mode) {
    if (!window.screenLayout) return 'bold';
    return mode === 'night'
        ? (window.screenLayout.night_font || 'bold')
        : (window.screenLayout.day_font || 'bold');
}

function getScreenColorFilter(mode) {
    if (!window.screenLayout) return 0;
    return mode === 'night'
        ? (window.screenLayout.night_color_filter || 0)
        : (window.screenLayout.day_color_filter || 0);
}

function ensureScreenLayoutMeta(layout) {
    if (!layout) return;
    if (!layout.day_font) layout.day_font = (window.settings && window.settings.p04) || 'bold';
    if (!layout.night_font) layout.night_font = (window.settings && window.settings.p05) || 'bold';
    if (!layout.day_aux_font) layout.day_aux_font = layout.day_font || 'bold';
    if (!layout.night_aux_font) layout.night_aux_font = layout.night_font || 'bold';
    if (layout.day_color_filter === undefined) layout.day_color_filter = (window.settings && window.settings.p10) || 0;
    if (layout.night_color_filter === undefined) layout.night_color_filter = (window.settings && window.settings.p11) || 0;
}

function dedupeScreenProfileElements(profile) {
    if (!profile || !Array.isArray(profile.elements)) return;
    const seen = new Set();
    profile.elements = profile.elements.filter(e => {
        if (!e || !e.id || seen.has(e.id)) return false;
        seen.add(e.id);
        return true;
    });
}

function ensureScreenProfileShape(profile) {
    if (!profile) return;
    if (!profile.elements) profile.elements = [];
    if (!profile.scroll_text) profile.scroll_text = SCREEN_DEFAULT_SCROLL_TEXT;
    if (!profile.static_texts) profile.static_texts = { ...SCREEN_DEFAULT_STATIC_TEXTS };

    SCREEN_ELEMENT_DEFS.forEach(def => {
        const element = ensureElementInProfile(profile, def.id);
        ensureElementLayer(element);
        if (isScreenTextElement(def.id)) ensureScreenTextOptions(element);
    });
    dedupeScreenProfileElements(profile);
}

function cloneScreenDefaults() {
    return JSON.parse(JSON.stringify(SCREEN_DEFAULT_LAYOUT));
}

function prepareScreenLayout(layout) {
    if (!layout || !layout.profiles) return layout;
    if (!layout.scroll_delay) {
        layout.scroll_delay = (window.settings && window.settings.p14) || 65;
    }
    ['day', 'night'].forEach(mode => {
        const profile = layout.profiles[mode];
        if (!profile || !Array.isArray(profile.elements)) return;
        profile.elements.forEach(elem => {
            if (elem && elem.id === 'glucose') elem.id = 'time_aux';
        });
    });
    ensureScreenLayoutMeta(layout);
    ['day', 'night'].forEach(mode => {
        const profile = layout.profiles[mode];
        if (profile) ensureScreenProfileShape(profile);
    });
    layout.version = SCREEN_LAYOUT_VERSION;
    return layout;
}

function ensureElementInProfile(profile, id) {
    if (!profile.elements) profile.elements = [];
    let e = profile.elements.find(x => x.id === id);
    if (!e) {
        if (id === 'time_aux') {
            e = { id, enabled: 0, x: 22, y: 47, z: 1 };
        } else if (id === 'digit_label') {
            e = { id, enabled: 0, x: 22, y: 84, z: 1, options: { font: 0, color: '#ffffff', bg_color: '#000000', width: 80, align: 0 } };
        } else if (id === 'digit_label_aux') {
            e = { id, enabled: 0, x: 22, y: 84, z: 1, options: { font: 0, color: '#ffffff', bg_color: '#000000', width: 80, align: 0 } };
        } else if (isScreenIconElement(id)) {
            e = { id, enabled: 0, x: 48, y: 48, z: 2 };
        } else {
            e = { id, enabled: 1, x: 0, y: 0, z: 0 };
        }
        profile.elements.push(e);
    }
    if (e.enabled === undefined) {
        if (id === 'time_aux' || id === 'digit_label' || id === 'digit_label_aux' || isScreenIconElement(id)) e.enabled = 0;
        else e.enabled = 1;
    }
    if (id === 'ampm') e.enabled = 1;
    if (e.x === undefined) e.x = 0;
    if (e.y === undefined) e.y = 0;
    ensureElementLayer(e);
    return e;
}

function isScreenSectionVisible() {
    const section = el('tab-layout');
    return !!(section && section.classList.contains('active'));
}

function focusScreenCanvas() {
    const canvas = el('screenCanvas');
    if (canvas && typeof canvas.focus === 'function') {
        canvas.focus({ preventScroll: true });
    }
}

function shouldIgnoreScreenKeyboardNudge() {
    if (window.screenEditor.dragging) return true;
    const active = document.activeElement;
    if (!active) return false;
    const tag = active.tagName;
    if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return true;
    if (active.isContentEditable) return true;
    if (active.closest && active.closest('.palette-item')) return true;
    return false;
}

function nudgeSelectedScreenElement(key, shiftKey) {
    const selectedId = window.screenEditor.selectedId;
    if (!selectedId || isScreenMetaElement(selectedId)) return false;

    const nudge = shiftKey ? 10 : 1;
    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return false;
    const elem = ensureElementInProfile(profile, selectedId);

    let changed = false;
    if (key === 'ArrowLeft') {
        elem.x = clamp(elem.x - nudge, 0, SCREEN_SIZE - 1);
        changed = true;
    } else if (key === 'ArrowRight') {
        elem.x = clamp(elem.x + nudge, 0, SCREEN_SIZE - 1);
        changed = true;
    } else if (key === 'ArrowUp') {
        const visualY = layoutYToVisualY(elem.y, selectedId);
        const newVisualY = clamp(visualY - nudge, 0, SCREEN_SIZE - 1);
        elem.y = visualYToLayoutY(newVisualY, selectedId);
        changed = true;
    } else if (key === 'ArrowDown') {
        const visualY = layoutYToVisualY(elem.y, selectedId);
        const newVisualY = clamp(visualY + nudge, 0, SCREEN_SIZE - 1);
        elem.y = visualYToLayoutY(newVisualY, selectedId);
        changed = true;
    }

    if (changed) {
        renderScreenCanvas();
        renderScreenOptions();
        updateScreenStatusLine();
    }
    return changed;
}

function onScreenArrowKeyDown(e) {
    if (!isScreenSectionVisible()) return;
    if (!['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown'].includes(e.key)) return;
    if (shouldIgnoreScreenKeyboardNudge()) return;
    if (nudgeSelectedScreenElement(e.key, e.shiftKey)) {
        e.preventDefault();
    }
}

function setupScreenSection() {
    if (window.screenEventListenersSet) return;
    window.screenEventListenersSet = true;

    const btnDay = el('screenModeDay');
    const btnNight = el('screenModeNight');
    const saveBtn = el('screenSaveBtn');
    const loadSystemBtn = el('screenLoadSystemBtn');
    const copyBtn = el('screenCopyBtn');
    const saveToFileBtn = el('screenSaveToFileBtn');
    const readFromFileBtn = el('screenReadFromFileBtn');
    const layoutFileInput = el('screenLayoutFileInput');

    if (btnDay) btnDay.addEventListener('click', () => setScreenMode('day'));
    if (btnNight) btnNight.addEventListener('click', () => setScreenMode('night'));
    if (saveBtn) saveBtn.addEventListener('click', saveScreenLayout);
    if (loadSystemBtn) loadSystemBtn.addEventListener('click', loadSystemScreenLayout);
    if (copyBtn) copyBtn.addEventListener('click', copyScreenLayoutToOtherMode);
    if (saveToFileBtn) saveToFileBtn.addEventListener('click', saveScreenLayoutToFile);
    if (readFromFileBtn) readFromFileBtn.addEventListener('click', () => layoutFileInput && layoutFileInput.click());
    if (layoutFileInput) layoutFileInput.addEventListener('change', onScreenLayoutFileSelected);

    const canvas = el('screenCanvas');
    if (canvas) {
        canvas.addEventListener('pointerdown', onScreenPointerDown);
        canvas.addEventListener('pointermove', (e) => {
            trackScreenCanvasPointer(e);
            onScreenPointerMove(e);
        });
        canvas.addEventListener('pointerup', onScreenPointerUp);
        canvas.addEventListener('pointercancel', onScreenPointerUp);
        canvas.addEventListener('pointerleave', clearScreenCanvasPointer);
    }

    document.addEventListener('keydown', onScreenArrowKeyDown);

    renderScreenPalette();
    updateScreenModeButtons();
    updateScreenStatusLine();
}

function setScreenMode(mode) {
    window.screenEditor.mode = mode === 'night' ? 'night' : 'day';
    updateScreenModeButtons();
    renderScreenCanvas();
    renderScreenOptions();
    renderScreenPalette();
    updateScreenStatusLine();
}

function updateScreenModeButtons() {
    const btnDay = el('screenModeDay');
    const btnNight = el('screenModeNight');
    if (!btnDay || !btnNight) return;
    const isDay = window.screenEditor.mode === 'day';
    btnDay.classList.toggle('active', isDay);
    btnNight.classList.toggle('active', !isDay);
    btnDay.setAttribute('aria-pressed', isDay ? 'true' : 'false');
    btnNight.setAttribute('aria-pressed', !isDay ? 'true' : 'false');

    const copyBtn = el('screenCopyBtn');
    if (copyBtn) {
        const trans = translations[currentLanguage] || translations.en;
        const key = isDay ? 'screen.copy_to_night' : 'screen.copy_to_day';
        copyBtn.textContent = getNestedTranslation(trans, key) || (isDay ? 'Copy layout to Night' : 'Copy layout to Day');
    }
}

const PALETTE_SWATCH_SIZE = 56;
// Aspect of bold-digit-label.jpg (must match tools/gen_palette_icons.py).
const DIGIT_LABEL_PALETTE_ASPECT = 94 / 154;

function getPalettePreviewSize(def) {
    if (isScreenStaticTextElement(def.id)) {
        return { w: PALETTE_SWATCH_SIZE, h: PALETTE_SWATCH_SIZE };
    }
    if (def.paletteFitFullImage && isScreenDigitLabelElement(def.id)) {
        return { w: PALETTE_SWATCH_SIZE, h: evenPx(PALETTE_SWATCH_SIZE * DIGIT_LABEL_PALETTE_ASPECT) };
    }
    if (def.paletteFitFullImage && (def.id === 'time' || def.id === 'time_aux')) {
        return { w: PALETTE_SWATCH_SIZE, h: evenPx(PALETTE_SWATCH_SIZE * 36 / 80) };
    }
    if (def.id === 'message') {
        return { w: PALETTE_SWATCH_SIZE, h: evenPx(PALETTE_SWATCH_SIZE * 60 / 160) };
    }

    const frameW = def.w;
    const frameH = def.h;
    if (frameW > frameH) {
        return {
            w: PALETTE_SWATCH_SIZE,
            h: evenPx(PALETTE_SWATCH_SIZE * frameH / frameW)
        };
    }
    if (frameW < frameH) {
        return {
            w: evenPx(PALETTE_SWATCH_SIZE * frameW / frameH),
            h: PALETTE_SWATCH_SIZE
        };
    }
    return { w: PALETTE_SWATCH_SIZE, h: PALETTE_SWATCH_SIZE };
}

function applyPalettePreviewSize(preview, def) {
    const size = getPalettePreviewSize(def);
    preview.style.width = `${size.w}px`;
    preview.style.height = `${size.h}px`;
    return size;
}

function setupSpriteBackground(preview, imgUrl, nativeH, displayW, displayH, spriteIndex = 0, spriteFrames = 1) {
    const scale = displayH / nativeH;
    const sheet = new Image();
    sheet.onload = () => {
        const frames = Math.max(1, spriteFrames);
        const frameW = sheet.naturalWidth / frames;
        const scaledSheetW = sheet.naturalWidth * scale;
        const xOffset = -(spriteIndex * frameW * scale);
        preview.style.backgroundImage = `url("${imgUrl}")`;
        preview.style.backgroundRepeat = 'no-repeat';
        preview.style.backgroundPosition = `${xOffset}px top`;
        preview.style.backgroundSize = `${scaledSheetW}px ${displayH}px`;
    };
    sheet.src = imgUrl;
}

function setupContainedBackground(preview, imgUrl, fallbackUrl) {
    const apply = (url) => {
        preview.style.backgroundImage = `url("${url}")`;
        preview.style.backgroundRepeat = 'no-repeat';
        preview.style.backgroundPosition = 'center center';
        preview.style.backgroundSize = 'contain';
        preview.classList.remove('screen-font-sample-thumb-missing');
    };
    apply(imgUrl);
    const probe = new Image();
    probe.onerror = () => {
        if (fallbackUrl && imgUrl !== fallbackUrl) {
            apply(fallbackUrl);
        } else {
            preview.classList.add('screen-font-sample-thumb-missing');
        }
    };
    probe.src = imgUrl;
}

function createFontSamplePreview(font) {
    const thumb = document.createElement('div');
    thumb.className = 'screen-font-sample-thumb';
    thumb.setAttribute('role', 'img');
    thumb.setAttribute('aria-label', `${font} font sample`);
    const url = resolveTimeFontPreviewUrl(font);
    const fallback = (font === 'user1' || font === 'user2') ? 'bold.jpg' : null;
    setupContainedBackground(thumb, url, fallback);
    return thumb;
}

function setupPaletteSpriteBackground(preview, def, imgUrl) {
    const size = applyPalettePreviewSize(preview, def);
    const sheet = def.spriteSheet;
    setupSpriteBackground(
        preview, imgUrl || def.img, def.h, size.w, size.h,
        sheet ? sheet.index : 0,
        sheet ? sheet.frames : 1
    );
}

function setupPaletteContainedBackground(preview, def) {
    applyPalettePreviewSize(preview, def);
    setupContainedBackground(preview, def.img);
}

function createScreenPreview(def, inMatrix, elementState, matrixScale) {
    const preview = document.createElement('div');
    const size = getScreenElementSize(def, elementState);
    const orientation = size.w > size.h ? 'wide' : (size.w < size.h ? 'tall' : 'square');
    preview.className = `screen-preview screen-preview-${def.id} screen-preview-${orientation}` +
        (inMatrix ? ' screen-preview-matrix' : ' screen-preview-palette');

    const previewImg = getPreviewImageUrl(def, inMatrix);
    if (previewImg) {
        if (inMatrix) {
            const pxW = size.w * matrixScale;
            const pxH = size.h * matrixScale;
            preview.setAttribute('aria-label', def.label);
            if (def.spriteSheet) {
                preview.classList.add('screen-preview-sprite');
                setupSpriteBackground(
                    preview, previewImg, def.h, pxW, pxH,
                    def.spriteSheet.index, def.spriteSheet.frames
                );
            } else if (def.paletteFitFullImage) {
                preview.classList.add('screen-preview-contained');
                setupContainedBackground(preview, previewImg);
            } else {
                preview.classList.add('screen-preview-sprite');
                setupSpriteBackground(preview, previewImg, def.h, pxW, pxH);
            }
            return preview;
        }
        if (def.paletteFitFullImage) {
            preview.classList.add('screen-preview-contained');
            preview.setAttribute('aria-label', def.label);
            applyPalettePreviewSize(preview, def);
            setupContainedBackground(preview, previewImg);
            return preview;
        }
        preview.classList.add('screen-preview-sprite');
        preview.setAttribute('aria-label', def.label);
        setupPaletteSpriteBackground(preview, def);
        return preview;
    }

    if (!inMatrix) {
        applyPalettePreviewSize(preview, def);
    }

    const text = document.createElement('div');
    text.className = 'screen-preview-text screen-preview-text-' + def.id;
    if (!inMatrix && (def.id === 'text' || (isScreenStaticTextElement(def.id) && !inMatrix && !elementState))) {
        preview.classList.add('screen-preview-palette-static-label');
        text.classList.add('screen-preview-text-palette-static');
        text.textContent = getPaletteNewTextIconLabel();
    } else {
        text.textContent = def.text || def.label;
        if (isScreenTextElement(def.id)) {
            const font = elementState && elementState.options && elementState.options.font !== undefined ? elementState.options.font : 0;
            const pt = SCREEN_MESSAGE_FONT_SIZES[font] || 8;
            const previewScale = inMatrix ? (matrixScale || getScreenScale()) : 1;
            text.style.fontSize = `${pt * previewScale}px`;
            text.style.lineHeight = '1';
            if (inMatrix) {
                preview.style.alignItems = 'flex-start';
                const align = elementState && elementState.options ? elementState.options.align : 0;
                if (align === 1) {
                    text.style.textAlign = 'center';
                    preview.style.justifyContent = 'center';
                } else if (align === 2) {
                    text.style.textAlign = 'right';
                    preview.style.justifyContent = 'flex-end';
                } else {
                    text.style.textAlign = 'left';
                    preview.style.justifyContent = 'flex-start';
                }
            }
            if (elementState && elementState.options) {
                if (elementState.options.color) text.style.color = elementState.options.color;
                if (elementState.options.bg_color) {
                    preview.style.backgroundColor = elementState.options.bg_color;
                }
            }
            if (isScreenStaticTextElement(def.id) || isScreenDigitLabelElement(def.id)) {
                const profile = getProfileObj(window.screenEditor.mode);
                const staticText = profile && profile.static_texts ? profile.static_texts[def.id] : def.text;
                text.textContent = staticText || def.text || def.label;
            }
        }
    }
    preview.appendChild(text);
    return preview;
}

function renderScreenPalette() {
    const list = el('screenPaletteItems');
    if (!list) return;
    list.innerHTML = '';

    const profile = getProfileObj(window.screenEditor.mode);

    getScreenPaletteDefs().forEach(def => {
        const item = document.createElement('div');
        item.className = 'palette-item';
        item.setAttribute('data-id', def.id);
        item.setAttribute('role', 'button');

        const slotId = def.id === 'text' ? resolvePaletteElementId(def.id, profile) : def.id;
        const unavailable = def.id === 'text' && !slotId;
        if (unavailable) {
            item.classList.add('palette-item-disabled');
            item.setAttribute('aria-disabled', 'true');
            item.tabIndex = -1;
        } else {
            item.tabIndex = 0;
        }
        if (isPaletteItemSelected(def)) {
            item.classList.add('palette-item-selected');
        }
        if (def.paletteMeta) {
            item.classList.add('palette-item-meta');
        }

        const swatch = document.createElement('div');
        swatch.className = 'palette-swatch';
        const palettePreview = createScreenPreview(def, false, null);
        swatch.appendChild(palettePreview);

        const label = document.createElement('div');
        label.textContent = getScreenPaletteItemLabel(def, profile);

        item.appendChild(swatch);
        item.appendChild(label);

        if (!unavailable) {
            const selectPaletteItem = () => {
                if (def.paletteMeta) {
                    window.screenEditor.selectedId = def.id;
                    renderScreenCanvas();
                    renderScreenOptions();
                    renderScreenPalette();
                    updateScreenStatusLine();
                    focusScreenCanvas();
                    return;
                }
                const activeProfile = getProfileObj(window.screenEditor.mode);
                if (!activeProfile) return;
                const resolvedId = resolvePaletteElementId(def.id, activeProfile);
                if (!resolvedId) return;
                const elObj = ensureElementInProfile(activeProfile, resolvedId);
                elObj.enabled = 1;
                window.screenEditor.selectedId = resolvedId;
                renderScreenCanvas();
                renderScreenOptions();
                renderScreenPalette();
                focusScreenCanvas();
            };

            if (def.paletteMeta) {
                item.addEventListener('pointerdown', (e) => {
                    e.preventDefault();
                    selectPaletteItem();
                });
            } else {
                item.addEventListener('pointerdown', (e) => {
                    e.preventDefault();
                    const resolvedId = resolvePaletteElementId(def.id, getProfileObj(window.screenEditor.mode));
                    if (!resolvedId) return;
                    beginDrag(resolvedId, e, true);
                });
            }
            item.addEventListener('keydown', (e) => {
                if (e.key === 'Enter' || e.key === ' ') {
                    e.preventDefault();
                    if (def.paletteMeta) {
                        selectPaletteItem();
                        return;
                    }
                    const activeProfile = getProfileObj(window.screenEditor.mode);
                    if (!activeProfile) return;
                    const resolvedId = resolvePaletteElementId(def.id, activeProfile);
                    if (!resolvedId) return;
                    const elObj = ensureElementInProfile(activeProfile, resolvedId);
                    elObj.enabled = 1;
                    window.screenEditor.selectedId = resolvedId;
                    renderScreenCanvas();
                    renderScreenOptions();
                    renderScreenPalette();
                    focusScreenCanvas();
                }
            });
        }

        list.appendChild(item);
    });
}

async function fetchScreenLayout() {
    try {
        await refreshScreenLayoutSelect();
        const [settingsResp, response] = await Promise.all([
            fetchSettingsJson('/api/settings?params=p08,p24,p50,p48,p49,p57,p58,p59', {
                fallbackUrl: '/api/settings'
            }).catch(() => fetchSettingsJson('/api/settings')),
            fetch('/api/screen')
        ]);
        if (settingsResp) {
            mergeSettingsData(settingsResp);
        }
        screenGridDeviceState = (window.settings && window.settings.p08) ? 1 : 0;
        if (!response.ok) {
            throw new Error(`Screen layout fetch failed: HTTP ${response.status}`);
        }
        const wire = new Uint8Array(await response.arrayBuffer());
        const data = decodeScreenLayoutBinary(wire);
        window.screenLayout = prepareScreenLayout(data);
        if (!window.screenLayout.profiles) window.screenLayout.profiles = { day: makeDefaultScreenProfile(), night: makeDefaultScreenProfile() };
        if (!window.screenLayout.profiles.day) window.screenLayout.profiles.day = makeDefaultScreenProfile();
        if (!window.screenLayout.profiles.night) window.screenLayout.profiles.night = makeDefaultScreenProfile();
        ensureScreenLayoutMeta(window.screenLayout);
        ensureScreenProfileShape(window.screenLayout.profiles.day);
        ensureScreenProfileShape(window.screenLayout.profiles.night);
        renderScreenCanvas();
        renderScreenOptions();
        renderScreenPalette();
    } catch (error) {
        console.error('Error loading screen layout:', error);
        showStatus(getMessage('error_loading_settings'), 'error');
    }
}

async function saveScreenTimeDisplaySettings() {
    const leading = el('screen_show_leading_zero');
    const dots = el('screen_dots_breathe');
    const scheduleList = el('display-schedule-list');
    const auxScheduleList = el('display-schedule-aux-list');

    const formData = {};
    const gridEnabled = getScreenGridEnabled() ? 1 : 0;
    if (gridEnabled !== screenGridDeviceState) {
        formData.p08 = gridEnabled;
    }
    if (leading) {
        const p24 = isScreenSwitchOn('screen_show_leading_zero') ? 1 : 0;
        if (p24 !== (window.settings && window.settings.p24 || 0)) formData.p24 = p24;
    }
    if (dots) {
        const p50 = isScreenSwitchOn('screen_dots_breathe') ? 1 : 0;
        if (p50 !== (window.settings && window.settings.p50 || 0)) formData.p50 = p50;
    }
    if (scheduleList) {
        const newSched = serializeDisplaySchedule('display-schedule-list');
        if (newSched !== (window.settings.p58 || '')) {
            formData.p58 = newSched;
        }
    }
    if (auxScheduleList) {
        const newAuxSched = serializeDisplaySchedule('display-schedule-aux-list');
        if (newAuxSched !== (window.settings.p59 || '')) {
            formData.p59 = newAuxSched;
        }
    }

    if (Object.keys(formData).length === 0) return true;

    try {
        const response = await fetch('/api/settings', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(formData)
        });
        const data = await response.json().catch(() => ({}));
        if (response.ok && data && data.status === 'ok') {
            Object.assign(window.settings, formData);
            if ('p08' in formData) screenGridDeviceState = formData.p08;
            return true;
        }
        return false;
    } catch (error) {
        console.error('Error saving time display settings:', error);
        return false;
    }
}

async function saveScreenLayout() {
    if (!window.screenLayout) return;
    window.screenLayout.version = SCREEN_LAYOUT_VERSION;
    ensureScreenLayoutMeta(window.screenLayout);
    ensureScreenProfileShape(window.screenLayout.profiles.day);
    ensureScreenProfileShape(window.screenLayout.profiles.night);
    const saveBtn = el('screenSaveBtn');
    try {
        toggleLoading(saveBtn, true);
        showStatus(getMessage('saving_settings'), 'info');
        const body = encodeScreenLayoutBinary(window.screenLayout);
        if (body.byteLength !== SCREEN_BIN_WIRE_SIZE) {
            throw new Error(`screen wire size mismatch: ${body.byteLength} != ${SCREEN_BIN_WIRE_SIZE}`);
        }
        const response = await fetch('/api/screen', {
            method: 'POST',
            headers: { 'Content-Type': 'application/octet-stream' },
            body
        });
        const data = await response.json().catch(() => ({}));
        if (response.ok && data && data.status === 'ok') {
            if (window.settings) {
                window.settings.p04 = window.screenLayout.day_font;
                window.settings.p05 = window.screenLayout.night_font;
                window.settings.p10 = window.screenLayout.day_color_filter;
                window.settings.p11 = window.screenLayout.night_color_filter;
            }
            const timeSettingsOk = await saveScreenTimeDisplaySettings();
            if (timeSettingsOk) {
                showStatus(getMessage('settings_saved'), 'success');
            } else {
                showStatus(getMessage('error_saving_settings'), 'error');
            }
        } else {
            let detail = data && data.error ? data.error : 'Unknown error';
            if (data && data.got != null && data.expected != null) {
                detail += ` (${data.got} bytes, expected ${data.expected})`;
            }
            if (data && data.error === 'bad_content_type') {
                detail += ' — hard-refresh the page (Ctrl+F5)';
            }
            showStatus(getMessage('error_saving_settings') + detail, 'error');
        }
    } catch (error) {
        console.error('Error saving screen layout:', error);
        showStatus(getMessage('error_saving_unknown'), 'error');
    } finally {
        toggleLoading(saveBtn, false);
    }
}

function prepareScreenLayoutForExport() {
    if (!window.screenLayout) return null;
    window.screenLayout.version = SCREEN_LAYOUT_VERSION;
    ensureScreenLayoutMeta(window.screenLayout);
    ensureScreenProfileShape(window.screenLayout.profiles.day);
    ensureScreenProfileShape(window.screenLayout.profiles.night);
    return window.screenLayout;
}

function applyScreenLayoutFromData(data) {
    window.screenLayout = prepareScreenLayout(data);
    if (!window.screenLayout.profiles) {
        window.screenLayout.profiles = { day: makeDefaultScreenProfile(), night: makeDefaultScreenProfile() };
    }
    if (!window.screenLayout.profiles.day) window.screenLayout.profiles.day = makeDefaultScreenProfile();
    if (!window.screenLayout.profiles.night) window.screenLayout.profiles.night = makeDefaultScreenProfile();
    ensureScreenLayoutMeta(window.screenLayout);
    ensureScreenProfileShape(window.screenLayout.profiles.day);
    ensureScreenProfileShape(window.screenLayout.profiles.night);
    window.screenEditor.selectedId = null;
    renderScreenCanvas();
    renderScreenOptions();
    renderScreenPalette();
    updateScreenStatusLine();
}

function saveScreenLayoutToFile() {
    const layout = prepareScreenLayoutForExport();
    if (!layout) return;
    const json = JSON.stringify(layout, null, 2);
    const blob = new Blob([json], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'frixos-layout.layout';
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);
    showStatus(getMessage('layout_saved_to_file'), 'success');
}

async function loadSystemScreenLayout() {
    const select = el('screenLayoutSelect');
    const loadBtn = el('screenLoadSystemBtn');
    const filename = select && select.value;
    if (!filename) return; // "Current" selected -> editor already shows it
    try {
        toggleLoading(loadBtn, true);
        const response = await fetch('/' + encodeURIComponent(filename));
        if (!response.ok) {
            showStatus(getMessage('layout_read_invalid'), 'error');
            return;
        }
        const data = await response.json();
        if (!data || !data.profiles) {
            showStatus(getMessage('layout_read_invalid'), 'error');
            return;
        }
        applyScreenLayoutFromData(data);
        showStatus(getMessage('layout_system_loaded'), 'success');
    } catch (error) {
        console.error('Error loading system layout:', error);
        showStatus(getMessage('layout_read_invalid'), 'error');
    } finally {
        toggleLoading(loadBtn, false);
    }
}

async function onScreenLayoutFileSelected(e) {
    const input = e.target;
    const file = input.files && input.files[0];
    input.value = '';
    if (!file) return;
    try {
        const text = await file.text();
        const data = JSON.parse(text);
        if (!data || !data.profiles) {
            showStatus(getMessage('layout_read_invalid'), 'error');
            return;
        }
        applyScreenLayoutFromData(data);
        showStatus(getMessage('layout_read_from_file'), 'success');
    } catch (error) {
        console.error('Error reading layout file:', error);
        showStatus(getMessage('layout_read_invalid'), 'error');
    }
}

async function restoreScreenDefaults() {
    window.screenLayout = cloneScreenDefaults();
    window.screenEditor.selectedId = null;
    renderScreenCanvas();
    renderScreenOptions();
    renderScreenPalette();
    await saveScreenLayout();
}

function copyScreenLayoutToOtherMode() {
    if (!window.screenLayout || !window.screenLayout.profiles) return;
    const source = window.screenEditor.mode;
    const target = source === 'day' ? 'night' : 'day';
    const sourceProfile = getProfileObj(source);
    if (!sourceProfile) return;

    // Copy the layout profile (element placement, texts, graph) only. The
    // per-mode Theme/Font and colour filter are intentionally kept as-is, since
    // day and night are meant to differ there (e.g. a lighter font at night).
    window.screenLayout.profiles[target] = JSON.parse(JSON.stringify(sourceProfile));
    const trans = translations[currentLanguage] || translations.en;
    const sourceLabel = getNestedTranslation(trans, `screen.${source}`) || source;
    const targetLabel = getNestedTranslation(trans, `screen.${target}`) || target;
    const template = getNestedTranslation(trans, 'messages.layout_copied') || 'Layout copied from {source} to {target}';
    showStatus(template.replace('{source}', sourceLabel).replace('{target}', targetLabel), 'success');
}

function createScreenGuideLine(orientation, posPx, kind) {
    const line = document.createElement('div');
    line.className = `screen-guide screen-guide-${orientation} screen-guide-${kind}`;
    if (orientation === 'v') {
        line.style.left = `${posPx}px`;
    } else {
        line.style.top = `${posPx}px`;
    }
    return line;
}

function appendScreenAlignmentGrid(canvas, scale) {
    SCREEN_ALIGNMENT_GRID_LINES.forEach(pos => {
        canvas.appendChild(createScreenGuideLine('v', pos * scale, 'matrix'));
        canvas.appendChild(createScreenGuideLine('h', pos * scale, 'matrix'));
    });
}

function appendScreenCenterCross(canvas, scale, kind) {
    canvas.appendChild(createScreenGuideLine('v', 64 * scale, kind));
    canvas.appendChild(createScreenGuideLine('h', 64 * scale, kind));
}

function appendScreenGuides(canvas, scale) {
    if (getScreenGridEnabled()) {
        appendScreenAlignmentGrid(canvas, scale);
        appendScreenCenterCross(canvas, scale, 'center');
    } else {
        appendScreenCenterCross(canvas, scale, 'matrix');
    }

    const selectedId = window.screenEditor.selectedId;
    if (!selectedId) return;

    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return;
    const elem = profile.elements.find(item => item.id === selectedId);
    const def = findElementDef(selectedId);
    if (!elem || !def) return;

    const size = getScreenElementSize(def, elem);
    const visualY = layoutYToVisualY(elem.y || 0, selectedId);
    canvas.appendChild(createScreenGuideLine('v', (elem.x + size.w / 2) * scale, 'selection'));
    canvas.appendChild(createScreenGuideLine('h', (visualY + size.h / 2) * scale, 'selection'));
}

function renderScreenCanvas() {
    const canvas = el('screenCanvas');
    if (!canvas || !window.screenLayout) return;
    canvas.innerHTML = '';

    const mode = window.screenEditor.mode;
    const profile = getProfileObj(mode);
    if (!profile) return;

    const scale = getScreenScale();
    window.screenEditor.scale = scale;

    // Render in z order (ascending), but allow overlap
    const elems = (profile.elements || []).slice().sort((a, b) => (a.z || 0) - (b.z || 0));

    elems.forEach(e => {
        if (e.enabled === 0 && e.id !== 'ampm') return;

        const def = findElementDef(e.id);
        if (!def) return;
        const size = getScreenElementSize(def, e);

        const box = document.createElement('div');
        box.className = 'screen-element' + (window.screenEditor.selectedId === e.id ? ' selected' : '');
        box.setAttribute('data-id', e.id);
        const visualY = layoutYToVisualY(e.y || 0, e.id);
        box.style.left = `${(e.x || 0) * scale}px`;
        box.style.top = `${visualY * scale}px`;
        box.style.width = `${size.w * scale}px`;
        box.style.height = `${size.h * scale}px`;
        const layer = getElementLayer(e);
        box.style.zIndex = String(window.screenEditor.selectedId === e.id ? 12 + layer : 3 + layer);

        box.appendChild(createScreenPreview(def, true, e, scale));

        box.addEventListener('pointerdown', (ev) => {
            ev.preventDefault();
            ev.stopPropagation();
            window.screenEditor.selectedId = e.id;
            focusScreenCanvas();
            beginDrag(e.id, ev, false);
            renderScreenCanvas();
            renderScreenOptions();
            renderScreenPalette();
        });

        canvas.appendChild(box);
    });

    appendScreenGuides(canvas, scale);
    updateScreenStatusLine();
}

async function appendScreenThemeFilterOptions(container, trans) {
    const mode = window.screenEditor.mode;
    const fontKey = mode === 'night' ? 'night_font' : 'day_font';
    const filterKey = mode === 'night' ? 'night_color_filter' : 'day_color_filter';
    ensureScreenLayoutMeta(window.screenLayout);

    const modeNote = document.createElement('p');
    modeNote.className = 'screen-options-mode-note';
    modeNote.textContent = mode === 'night'
        ? (getNestedTranslation(trans, 'screen.night_profile') || 'Night profile')
        : (getNestedTranslation(trans, 'screen.day_profile') || 'Day profile');
    container.appendChild(modeNote);

    const timeFontRow = document.createElement('div');
    timeFontRow.className = 'form-group';
    const timeFontLabel = document.createElement('label');
    timeFontLabel.textContent = getNestedTranslation(trans, 'screen.theme_font') || 'Theme/Font';
    const timeFontSelect = document.createElement('select');
    SCREEN_TIME_FONTS.forEach(font => {
        const option = document.createElement('option');
        option.value = font;
        option.textContent = font.charAt(0).toUpperCase() + font.slice(1);
        timeFontSelect.appendChild(option);
    });
    timeFontSelect.value = window.screenLayout[fontKey] || 'bold';
    timeFontSelect.addEventListener('change', () => {
        window.screenLayout[fontKey] = timeFontSelect.value;
        renderScreenCanvas();
        renderScreenOptions();
    });
    timeFontRow.appendChild(timeFontLabel);
    timeFontRow.appendChild(timeFontSelect);
    container.appendChild(timeFontRow);

    const filterRow = document.createElement('div');
    filterRow.className = 'form-group';
    const filterLabel = document.createElement('label');
    filterLabel.textContent = getNestedTranslation(trans, 'screen.color_filter') || 'Color filter';
    const filterSelect = document.createElement('select');
    SCREEN_COLOR_FILTER_OPTIONS.forEach(o => {
        const option = document.createElement('option');
        option.value = String(o.v);
        option.textContent = getNestedTranslation(trans, o.key) || o.label;
        filterSelect.appendChild(option);
    });
    filterSelect.value = String(window.screenLayout[filterKey] || 0);
    filterSelect.addEventListener('change', () => {
        window.screenLayout[filterKey] = parseInt(filterSelect.value, 10) || 0;
        renderScreenCanvas();
    });
    filterRow.appendChild(filterLabel);
    filterRow.appendChild(filterSelect);
    container.appendChild(filterRow);

    await appendScreenFontSamples(container, window.screenLayout[fontKey] || 'bold', fontKey);
}

function appendScreenGridOption(container, trans) {
    appendScreenSwitchRow(container, {
        id: 'screen_show_grid',
        label: getScreenTranslation('screen.show_grid', 'Screen Grid'),
        checked: getScreenGridEnabled(),
        onChange: (on) => {
            setScreenGridEnabled(on);
            renderScreenCanvas();
            updateScreenStatusLine();
            saveScreenTimeDisplaySettings();
        }
    });
}

async function appendScreenFontSamples(container, selectedFont, fontLayoutKey) {
    await ensureScreenSpiffsFiles();
    const wrap = document.createElement('div');
    wrap.className = 'screen-font-samples';
    const title = document.createElement('h4');
    const trans = translations[currentLanguage] || translations.en;
    title.textContent = getNestedTranslation(trans, 'screen.font_selection')
        || getNestedTranslation(trans, 'screen.font_samples')
        || 'Font Selection';
    wrap.appendChild(title);

    const grid = document.createElement('div');
    grid.className = 'screen-font-samples-grid';
    SCREEN_TIME_FONTS.forEach(font => {
        const box = document.createElement('button');
        box.type = 'button';
        box.className = 'screen-font-sample-box' + (font === selectedFont ? ' selected' : '');
        const name = document.createElement('div');
        name.className = 'screen-font-sample-name';
        name.textContent = font.charAt(0).toUpperCase() + font.slice(1);
        box.appendChild(name);
        box.appendChild(createFontSamplePreview(font));
        box.addEventListener('click', () => {
            const mode = window.screenEditor.mode;
            if (fontLayoutKey === 'night_aux_font' || fontLayoutKey === 'day_aux_font') {
                window.screenLayout[fontLayoutKey] = font;
            } else if (fontLayoutKey === 'night_font' || fontLayoutKey === 'day_font') {
                window.screenLayout[fontLayoutKey] = font;
            } else if (mode === 'night') {
                window.screenLayout.night_font = font;
            } else {
                window.screenLayout.day_font = font;
            }
            renderScreenCanvas();
            renderScreenOptions();
        });
        grid.appendChild(box);
    });
    wrap.appendChild(grid);
    container.appendChild(wrap);
}

function appendScreenTokenButtons(container, textarea) {
    const tokenWrap = document.createElement('div');
    tokenWrap.className = 'message-tokens screen-message-tokens';
    const tokenTitle = document.createElement('h4');
    const trans = translations[currentLanguage] || translations.en;
    tokenTitle.textContent = getNestedTranslation(trans, 'screen.tokens') || 'Tokens';
    tokenWrap.appendChild(tokenTitle);
    const tokenList = document.createElement('div');
    tokenList.className = 'screen-token-list';
    SCREEN_TOKEN_CODES.forEach(code => {
        const btn = createTokenButton(code);
        bindTokenInsert(btn, textarea);
        tokenList.appendChild(btn);
    });
    tokenWrap.appendChild(tokenList);
    container.appendChild(tokenWrap);
}

// Default options for a freshly placed graph element.
function ensureScreenGraphOptions(e) {
    if (!e.options) e.options = {};
    const d = {
        token: '', interval_min: 5, points: 60, gwidth: 80, gheight: 36,
        autoscale: true, show_axis: true, show_value: true, backfill: true,
        band_on: false, boolean: false, thick: false,
        band_low: null, band_high: null, y_min: null, y_max: null,
        color: '#00dcff', bg_color: '#000000',
        band_color: '#283c28', warn_color: '#ff5050', axis_color: '#787878'
    };
    for (const k in d) if (e.options[k] === undefined) e.options[k] = d[k];
    return e.options;
}

// Suggested numeric tokens for the graph picker. Free text is also allowed
// (e.g. an [HA:...] token), so this is only a convenience datalist.
function getGraphNumericTokens() {
    const base = ['[temp]', '[hum]', '[high]', '[low]', '[wind]', '[gust]',
        '[precip]', '[uv]', '[pressure]', '[3high]', '[3low]', '[CGM:reading]'];
    const found = new Set(base);
    // Scrape [HA:...] and stock-style tokens from the configured message + texts.
    const scrape = (s) => {
        if (!s) return;
        const m = String(s).match(/\[[^\]]+\]/g);
        if (m) m.forEach(t => found.add(t));
    };
    try {
        if (window.settings && window.settings.p16) scrape(window.settings.p16);
        ['day', 'night'].forEach(mode => {
            const p = window.screenLayout && window.screenLayout.profiles && window.screenLayout.profiles[mode];
            if (!p) return;
            scrape(p.scroll_text);
            if (p.static_texts) Object.values(p.static_texts).forEach(scrape);
        });
    } catch (_) { /* best effort */ }
    return Array.from(found);
}

// Options panel for an icon element: a help note, a live preview of the current
// JPG, and Upload / Delete controls. The icon is stored on the device as
// icon<N>.jpg (uploaded via /api/ota X-Filename, deleted via /api/files/delete).
function appendScreenIconOptions(container, e) {
    const fname = iconFileName(e.id);
    if (!fname) return;

    const note = document.createElement('p');
    note.className = 'screen-options-mode-note';
    note.textContent = getScreenTranslation('screen.icon_help',
        `Upload a JPG up to ${SCREEN_ICON_MAX_DIM}x${SCREEN_ICON_MAX_DIM}px. Stored on the device as ${fname} and shown at its own size. Shared by Day and Night.`);
    container.appendChild(note);

    const previewWrap = document.createElement('div');
    previewWrap.className = 'form-group';
    const thumb = document.createElement('img');
    thumb.className = 'screen-icon-thumb';
    thumb.alt = e.id;
    thumb.style.imageRendering = 'pixelated';
    thumb.style.maxWidth = '64px';
    thumb.style.maxHeight = '64px';
    thumb.style.background = '#000';
    thumb.style.border = '1px solid var(--line, #333)';
    thumb.onerror = () => { thumb.style.display = 'none'; };
    thumb.onload = () => { thumb.style.display = ''; };
    thumb.src = iconUrl(e.id);
    previewWrap.appendChild(thumb);
    container.appendChild(previewWrap);

    const row = document.createElement('div');
    row.className = 'form-group city-row';
    const fileInput = document.createElement('input');
    fileInput.type = 'file';
    fileInput.accept = 'image/jpeg,.jpg,.jpeg';
    fileInput.style.display = 'none';
    fileInput.addEventListener('change', (ev) => onScreenIconFileSelected(ev, e.id));
    const uploadBtn = document.createElement('button');
    uploadBtn.type = 'button';
    uploadBtn.className = 'btn sm ghost';
    uploadBtn.textContent = getScreenTranslation('screen.icon_upload', 'Upload JPG');
    uploadBtn.addEventListener('click', () => fileInput.click());
    const delBtn = document.createElement('button');
    delBtn.type = 'button';
    delBtn.className = 'btn sm ghost';
    delBtn.textContent = getScreenTranslation('screen.icon_delete', 'Delete');
    delBtn.addEventListener('click', () => deleteScreenIcon(e.id));
    row.appendChild(uploadBtn);
    row.appendChild(delBtn);
    row.appendChild(fileInput);
    container.appendChild(row);
}

async function onScreenIconFileSelected(ev, id) {
    const input = ev.target;
    const file = input.files && input.files[0];
    input.value = '';
    if (!file) return;
    const fname = iconFileName(id);
    if (!fname) return;

    if (file.type !== 'image/jpeg' && !/\.jpe?g$/i.test(file.name)) {
        showStatus(getScreenTranslation('screen.icon_bad_type', 'Icon must be a JPG image'), 'error');
        return;
    }
    // Validate dimensions client-side (the firmware decodes at native size).
    const dataUrl = await new Promise(res => { const r = new FileReader(); r.onload = () => res(r.result); r.onerror = () => res(null); r.readAsDataURL(file); });
    const dims = dataUrl ? await new Promise(res => { const im = new Image(); im.onload = () => res({ w: im.naturalWidth, h: im.naturalHeight }); im.onerror = () => res(null); im.src = dataUrl; }) : null;
    if (!dims) { showStatus(getScreenTranslation('screen.icon_bad_type', 'Icon must be a JPG image'), 'error'); return; }
    if (dims.w > SCREEN_ICON_MAX_DIM || dims.h > SCREEN_ICON_MAX_DIM) {
        showStatus(getScreenTranslation('screen.icon_too_big', `Icon must be at most ${SCREEN_ICON_MAX_DIM}x${SCREEN_ICON_MAX_DIM} pixels`), 'error');
        return;
    }
    try {
        showStatus(getMessage('saving_settings'), 'info');
        const body = await file.arrayBuffer();
        const resp = await fetch('/api/ota', {
            method: 'POST',
            headers: { 'X-Filename': fname, 'Content-Type': 'application/octet-stream' },
            body
        });
        const data = await resp.json().catch(() => ({}));
        if (resp.ok && data && data.status === 'ok') {
            screenIconBust++;
            invalidateIconDims(id);
            ensureScreenSpiffsFiles(true);
            showStatus(getScreenTranslation('screen.icon_uploaded', 'Icon uploaded. Click Apply to push the layout.'), 'success');
            renderScreenOptions();
            renderScreenCanvas();
            renderScreenPalette();
        } else {
            showStatus(getScreenTranslation('screen.icon_upload_failed', 'Icon upload failed'), 'error');
        }
    } catch (err) {
        console.error('Icon upload error:', err);
        showStatus(getScreenTranslation('screen.icon_upload_failed', 'Icon upload failed'), 'error');
    }
}

async function deleteScreenIcon(id) {
    const fname = iconFileName(id);
    if (!fname) return;
    try {
        const resp = await fetch('/api/files/delete', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ files: [fname] })
        });
        if (resp.ok) {
            screenIconBust++;
            invalidateIconDims(id);
            ensureScreenSpiffsFiles(true);
            showStatus(getScreenTranslation('screen.icon_deleted', 'Icon deleted'), 'success');
            renderScreenOptions();
            renderScreenCanvas();
            renderScreenPalette();
        } else {
            showStatus(getScreenTranslation('screen.icon_delete_failed', 'Delete failed'), 'error');
        }
    } catch (err) {
        console.error('Icon delete error:', err);
        showStatus(getScreenTranslation('screen.icon_delete_failed', 'Delete failed'), 'error');
    }
}

function renderScreenOptions() {
    const opt = el('screenOptions');
    if (!opt) return;
    opt.innerHTML = '';
    if (!window.screenLayout || !window.screenEditor.selectedId) return;

    const trans = translations[currentLanguage] || translations.en;

    if (isScreenSettingsElement(window.screenEditor.selectedId)) {
        const title = document.createElement('h3');
        title.textContent = (getNestedTranslation(trans, 'screen.options') || 'Options') + `: ${getScreenElementLabel('screen_settings')}`;
        opt.appendChild(title);
        appendScreenThemeFilterOptions(opt, trans);
        appendScreenGridOption(opt, trans);
        return;
    }

    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return;
    const e = ensureElementInProfile(profile, window.screenEditor.selectedId);
    const def = findElementDef(e.id);

    const title = document.createElement('h3');
    title.textContent = (getNestedTranslation(trans, 'screen.options') || 'Options') + `: ${getScreenElementLabel(e.id)}`;
    opt.appendChild(title);

    appendScreenLayerRow(opt, e);

    if (e.id === 'ampm') {
        e.enabled = 1;
        const note = document.createElement('p');
        note.className = 'screen-options-mode-note';
        note.textContent = getNestedTranslation(trans, 'screen.ampm_note')
            || 'AM/PM indicator is only visible when 12-hour format is used (from Settings).';
        opt.appendChild(note);
        return;
    }

    appendScreenSwitchRow(opt, {
        id: 'screen_element_enabled',
        label: getNestedTranslation(trans, 'screen.enabled') || 'Enabled',
        checked: e.enabled !== 0,
        onChange: (on) => {
            e.enabled = on ? 1 : 0;
            renderScreenCanvas();
            if (isScreenStaticTextElement(e.id) || isScreenIconElement(e.id)) renderScreenPalette();
        }
    });

    if (isScreenIconElement(e.id)) {
        appendScreenIconOptions(opt, e);
    }

    if (e.id === 'time') {
        appendScreenSwitchRow(opt, {
            id: 'screen_show_leading_zero',
            label: getNestedTranslation(trans, 'screen.show_leading_zero') || 'Show 0 for single digit hour',
            checked: !!(window.settings && window.settings.p24),
            onChange: () => saveScreenTimeDisplaySettings()
        });

        appendScreenSwitchRow(opt, {
            id: 'screen_dots_breathe',
            label: getNestedTranslation(trans, 'screen.disable_dots_breathe') || 'Disable breathing time dots',
            checked: !!(window.settings && window.settings.p50),
            onChange: () => saveScreenTimeDisplaySettings()
        });

        appendDisplayScheduleSection(opt, {
            listId: 'display-schedule-list',
            titleKey: 'screen.display_schedule.title',
            descriptionKey: 'screen.display_schedule.description',
            settingsKey: 'p58'
        });
    }

    if (e.id === 'time_aux') {
        const mode = window.screenEditor.mode;
        const fontKey = mode === 'night' ? 'night_aux_font' : 'day_aux_font';
        ensureScreenLayoutMeta(window.screenLayout);

        const modeNote = document.createElement('p');
        modeNote.className = 'screen-options-mode-note';
        modeNote.textContent = mode === 'night'
            ? (getNestedTranslation(trans, 'screen.night_profile') || 'Night profile')
            : (getNestedTranslation(trans, 'screen.day_profile') || 'Day profile');
        opt.appendChild(modeNote);

        const auxFontRow = document.createElement('div');
        auxFontRow.className = 'form-group';
        const auxFontLabel = document.createElement('label');
        auxFontLabel.textContent = getNestedTranslation(trans, 'screen.aux_time_font') || 'Digit font';
        const auxFontSelect = document.createElement('select');
        SCREEN_TIME_FONTS.forEach(font => {
            const option = document.createElement('option');
            option.value = font;
            option.textContent = font.charAt(0).toUpperCase() + font.slice(1);
            auxFontSelect.appendChild(option);
        });
        auxFontSelect.value = window.screenLayout[fontKey] || 'bold';
        auxFontSelect.addEventListener('change', () => {
            window.screenLayout[fontKey] = auxFontSelect.value;
            renderScreenCanvas();
            renderScreenOptions();
        });
        auxFontRow.appendChild(auxFontLabel);
        auxFontRow.appendChild(auxFontSelect);
        opt.appendChild(auxFontRow);

        appendScreenFontSamples(opt, window.screenLayout[fontKey] || 'bold', fontKey);

        appendDisplayScheduleSection(opt, {
            listId: 'display-schedule-aux-list',
            titleKey: 'screen.display_schedule_aux.title',
            descriptionKey: 'screen.display_schedule_aux.description',
            settingsKey: 'p59'
        });
    }

    if (isScreenTextElement(e.id)) {
        ensureScreenTextOptions(e);

        const fontRow = document.createElement('div');
        fontRow.className = 'form-group';
        const fontLabel = document.createElement('label');
        fontLabel.textContent = getNestedTranslation(trans, 'screen.message_font') || 'Font';
        const fontSelect = document.createElement('select');
        SCREEN_MESSAGE_FONT_OPTIONS.forEach(o => {
            const option = document.createElement('option');
            option.value = String(o.v);
            option.textContent = o.t;
            fontSelect.appendChild(option);
        });
        fontSelect.value = String(e.options.font || 0);
        fontSelect.addEventListener('change', () => {
            e.options.font = parseInt(fontSelect.value, 10) || 0;
            renderScreenCanvas();
        });
        fontRow.appendChild(fontLabel);
        fontRow.appendChild(fontSelect);
        opt.appendChild(fontRow);

        const colorRow = document.createElement('div');
        colorRow.className = 'form-group';
        const colorLabel = document.createElement('label');
        colorLabel.textContent = getNestedTranslation(trans, 'screen.message_color') || 'Color';
        const colorInput = document.createElement('input');
        colorInput.type = 'color';
        colorInput.value = e.options.color || '#ffffff';
        colorInput.addEventListener('input', () => {
            e.options.color = colorInput.value;
            renderScreenCanvas();
        });
        colorRow.appendChild(colorLabel);
        colorRow.appendChild(colorInput);
        opt.appendChild(colorRow);

        const bgColorRow = document.createElement('div');
        bgColorRow.className = 'form-group';
        const bgColorLabel = document.createElement('label');
        bgColorLabel.textContent = getNestedTranslation(trans, 'screen.background_color') || 'Background Color';
        const bgColorInput = document.createElement('input');
        bgColorInput.type = 'color';
        bgColorInput.value = e.options.bg_color || '#000000';
        bgColorInput.addEventListener('input', () => {
            e.options.bg_color = bgColorInput.value;
            renderScreenCanvas();
        });
        bgColorRow.appendChild(bgColorLabel);
        bgColorRow.appendChild(bgColorInput);
        opt.appendChild(bgColorRow);

        if (e.id === 'message') {
            const delayRow = document.createElement('div');
            delayRow.className = 'form-group';
            const delayLabel = document.createElement('label');
            delayLabel.textContent = getNestedTranslation(trans, 'screen.scroll_delay') || 'Scroll delay (ms)';
            const delayInput = document.createElement('input');
            delayInput.type = 'number';
            delayInput.min = '30';
            delayInput.max = '255';
            delayInput.value = String(window.screenLayout.scroll_delay || 65);
            delayInput.addEventListener('change', () => {
                window.screenLayout.scroll_delay = clamp(parseInt(delayInput.value, 10) || 65, 30, 255);
            });
            delayRow.appendChild(delayLabel);
            delayRow.appendChild(delayInput);
            opt.appendChild(delayRow);
        }

        if (isScreenClippedTextElement(e.id)) {
            const widthRow = document.createElement('div');
            widthRow.className = 'form-group';
            const widthLabel = document.createElement('label');
            widthLabel.textContent = isScreenDigitLabelElement(e.id)
                ? (getNestedTranslation(trans, 'screen.digit_label_width') || 'Width (px)')
                : (getNestedTranslation(trans, 'screen.text_width') || 'Width (px, 0 = no limit)');
            const widthInput = document.createElement('input');
            widthInput.type = 'number';
            widthInput.min = '0';
            widthInput.max = '127';
            widthInput.value = String(e.options.width !== undefined
                ? e.options.width
                : (isScreenDigitLabelElement(e.id) ? 80 : 0));
            widthInput.addEventListener('change', () => {
                e.options.width = clamp(parseInt(widthInput.value, 10) || 0, 0, 127);
                renderScreenCanvas();
            });
            widthRow.appendChild(widthLabel);
            widthRow.appendChild(widthInput);
            opt.appendChild(widthRow);

            const alignRow = document.createElement('div');
            alignRow.className = 'form-group';
            const alignLabel = document.createElement('label');
            alignLabel.textContent = getNestedTranslation(trans, 'screen.text_align') || 'Alignment';
            const alignSelect = document.createElement('select');
            SCREEN_ALIGN_OPTIONS.forEach(o => {
                const option = document.createElement('option');
                option.value = String(o.v);
                option.textContent = getScreenAlignLabel(o.v);
                alignSelect.appendChild(option);
            });
            alignSelect.value = String(e.options.align || 0);
            alignSelect.addEventListener('change', () => {
                e.options.align = parseInt(alignSelect.value, 10) || 0;
                renderScreenCanvas();
            });
            alignRow.appendChild(alignLabel);
            alignRow.appendChild(alignSelect);
            opt.appendChild(alignRow);
        }

        const textRow = document.createElement('div');
        textRow.className = 'form-group';
        const textLabel = document.createElement('label');
        textLabel.textContent = getNestedTranslation(trans, 'screen.message_text') || 'Text';
        const textArea = document.createElement('textarea');
        const maxLen = e.id === 'message' ? 511 : 96;
        textArea.maxLength = maxLen;
        textArea.id = 'editor-textarea-' + e.id;
        textArea.rows = e.id === 'message' ? 3 : 2;

        const counter = document.createElement('div');
        counter.id = textArea.id + '-counter';
        counter.className = 'counter';

        if (e.id === 'message') {
            textArea.value = profile.scroll_text || '';
            textArea.addEventListener('input', () => {
                profile.scroll_text = textArea.value;
                renderScreenCanvas();
                updateCharCounter(textArea, counter);
            });
        } else {
            if (!profile.static_texts) profile.static_texts = { ...SCREEN_DEFAULT_STATIC_TEXTS };
            const textKey = getDigitLabelTextKey(e.id);
            textArea.value = profile.static_texts[textKey] || '';
            textArea.addEventListener('input', () => {
                profile.static_texts[textKey] = textArea.value;
                renderScreenCanvas();
                if (isScreenStaticTextElement(e.id)) renderScreenPalette();
                updateCharCounter(textArea, counter);
            });
        }
        textRow.appendChild(textLabel);
        textRow.appendChild(textArea);
        textRow.appendChild(counter);
        textArea.setAttribute('aria-describedby', (textArea.getAttribute('aria-describedby') || '') + ' ' + counter.id);
        counter.setAttribute('aria-live', 'polite');
        updateCharCounter(textArea, counter);
        setupTokenHighlightTextarea(textArea);
        opt.appendChild(textRow);
        appendScreenTokenButtons(opt, textArea);
    }

    if (e.id === 'graph') {
        const o = ensureScreenGraphOptions(e);
        const T = (k, d) => getNestedTranslation(trans, k) || d;

        const mkRow = (labelText, control) => {
            const row = document.createElement('div');
            row.className = 'form-group';
            const lab = document.createElement('label');
            lab.textContent = labelText;
            row.appendChild(lab);
            row.appendChild(control);
            opt.appendChild(row);
            return control;
        };
        const mkNumber = (labelText, val, min, max, onCh) => {
            const inp = document.createElement('input');
            inp.type = 'number';
            inp.min = String(min);
            inp.max = String(max);
            inp.value = (val == null ? '' : String(val));
            inp.addEventListener('change', () => { onCh(inp.value); renderScreenCanvas(); });
            return mkRow(labelText, inp);
        };
        const mkColor = (labelText, val, onCh) => {
            const inp = document.createElement('input');
            inp.type = 'color';
            inp.value = val;
            inp.addEventListener('input', () => { onCh(inp.value); renderScreenCanvas(); });
            return mkRow(labelText, inp);
        };

        // Token picker (datalist suggestions; free text allows any token).
        const tokenInput = document.createElement('input');
        tokenInput.type = 'text';
        tokenInput.setAttribute('list', 'graphTokenList');
        tokenInput.placeholder = '[temp]';
        tokenInput.value = o.token || '';
        const dl = document.createElement('datalist');
        dl.id = 'graphTokenList';
        getGraphNumericTokens().forEach(t => { const op = document.createElement('option'); op.value = t; dl.appendChild(op); });
        tokenInput.addEventListener('change', () => { o.token = tokenInput.value.trim(); renderScreenCanvas(); });
        mkRow(T('screen.graph_token', 'Token to plot'), tokenInput);
        opt.appendChild(dl);

        mkColor(T('screen.graph_line_color', 'Line color'), o.color || '#00dcff', v => o.color = v);
        mkColor(T('screen.background_color', 'Background Color'), o.bg_color || '#000000', v => o.bg_color = v);

        mkNumber(T('screen.graph_interval', 'Sample interval (min)'), o.interval_min, 1, 1440, v => o.interval_min = clamp(parseInt(v, 10) || 5, 1, 1440));
        mkNumber(T('screen.graph_points', 'Data points'), o.points, 2, 100, v => o.points = clamp(parseInt(v, 10) || 60, 2, 100));
        mkNumber(T('screen.graph_width', 'Width (px)'), o.gwidth, 60, 80, v => o.gwidth = clamp(parseInt(v, 10) || 80, 60, 80));
        mkNumber(T('screen.graph_height', 'Height (px)'), o.gheight, 28, 36, v => o.gheight = clamp(parseInt(v, 10) || 36, 28, 36));

        appendScreenSwitchRow(opt, { id: 'graph_autoscale', label: T('screen.graph_autoscale', 'Auto Y-scale'), checked: !!o.autoscale, onChange: (on) => { o.autoscale = on; renderScreenOptions(); } });
        if (!o.autoscale) {
            mkNumber(T('screen.graph_ymin', 'Y min'), o.y_min, -32767, 32767, v => o.y_min = (v === '' ? null : (parseInt(v, 10) || 0)));
            mkNumber(T('screen.graph_ymax', 'Y max'), o.y_max, -32767, 32767, v => o.y_max = (v === '' ? null : (parseInt(v, 10) || 0)));
        }
        appendScreenSwitchRow(opt, { id: 'graph_axis', label: T('screen.graph_axis', 'Show axis & time labels'), checked: !!o.show_axis, onChange: (on) => { o.show_axis = on; renderScreenOptions(); } });
        appendScreenSwitchRow(opt, { id: 'graph_value', label: T('screen.graph_value', 'Show current value'), checked: !!o.show_value, onChange: (on) => { o.show_value = on; renderScreenCanvas(); } });
        appendScreenSwitchRow(opt, { id: 'graph_boolean', label: T('screen.graph_boolean', 'Boolean (step 0/1)'), checked: !!o.boolean, onChange: (on) => { o.boolean = on; renderScreenCanvas(); } });
        appendScreenSwitchRow(opt, { id: 'graph_thick', label: T('screen.graph_thick', 'Thick line (2px)'), checked: !!o.thick, onChange: (on) => { o.thick = on; renderScreenCanvas(); } });
        appendScreenSwitchRow(opt, { id: 'graph_backfill', label: T('screen.graph_backfill', 'Backfill history (HA/CGM)'), checked: !!o.backfill, onChange: (on) => { o.backfill = on; } });
        appendScreenSwitchRow(opt, { id: 'graph_band', label: T('screen.graph_band', 'Show low/high band'), checked: !!o.band_on, onChange: (on) => { o.band_on = on; renderScreenOptions(); } });
        if (o.band_on) {
            mkNumber(T('screen.graph_band_low', 'Band low'), o.band_low, -32767, 32767, v => o.band_low = (v === '' ? null : (parseInt(v, 10) || 0)));
            mkNumber(T('screen.graph_band_high', 'Band high'), o.band_high, -32767, 32767, v => o.band_high = (v === '' ? null : (parseInt(v, 10) || 0)));
            mkColor(T('screen.graph_band_color', 'Band color'), o.band_color || '#283c28', v => o.band_color = v);
            mkColor(T('screen.graph_warn_color', 'Out-of-band color'), o.warn_color || '#ff5050', v => o.warn_color = v);
        }
        if (o.show_axis) {
            mkColor(T('screen.graph_axis_color', 'Axis color'), o.axis_color || '#787878', v => o.axis_color = v);
        }
    }
}

function beginDrag(id, pointerEvent, fromPalette) {
    const canvas = el('screenCanvas');
    if (!canvas) return;
    focusScreenCanvas();
    if (!window.screenLayout) return;
    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return;

    const elem = ensureElementInProfile(profile, id);

    const scale = getScreenScale();
    window.screenEditor.scale = scale;

    const def = findElementDef(id);
    let startLeft = elem.x * scale;
    let startTop = layoutYToVisualY(elem.y || 0, def ? def.id : id) * scale;

    if (fromPalette) {
        window.screenEditor.selectedId = id;
        renderScreenCanvas();
        renderScreenOptions();
        renderScreenPalette();
    }

    canvas.setPointerCapture(pointerEvent.pointerId);
    window.screenEditor.dragging = {
        id,
        originX: pointerEvent.clientX,
        originY: pointerEvent.clientY,
        startLeft,
        startTop,
        fromPalette,
        moved: false
    };
    updateScreenStatusLine();
}

function onScreenPointerDown(e) {
    if (window.screenEditor.dragging) return;
    trackScreenCanvasPointer(e);

    focusScreenCanvas();

    const target = e.target && e.target.closest ? e.target.closest('.screen-element') : null;
    if (!target) {
        window.screenEditor.selectedId = null;
        renderScreenCanvas();
        renderScreenOptions();
        renderScreenPalette();
    } else {
        updateScreenStatusLine();
    }
}

function onScreenPointerMove(e) {
    const d = window.screenEditor.dragging;
    if (!d) return;
    const canvas = el('screenCanvas');
    if (!canvas) return;
    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return;
    const elem = ensureElementInProfile(profile, d.id);
    const scale = window.screenEditor.scale || getScreenScale();

    const dx = e.clientX - d.originX;
    const dy = e.clientY - d.originY;

    if (d.fromPalette) {
        if (!d.moved && Math.abs(dx) < SCREEN_DRAG_THRESHOLD && Math.abs(dy) < SCREEN_DRAG_THRESHOLD) {
            return;
        }
        if (!d.moved) {
            elem.enabled = 1;
        }
        d.moved = true;
        const rect = canvas.getBoundingClientRect();
        const def = findElementDef(d.id);
        const elementId = def ? def.id : d.id;
        elem.x = clamp(Math.round((e.clientX - rect.left - 10) / scale), 0, SCREEN_SIZE - 1);
        const visualY = clamp(Math.round((e.clientY - rect.top - 10) / scale), 0, SCREEN_SIZE - 1);
        elem.y = visualYToLayoutY(visualY, elementId);
    } else {
        const newLeft = d.startLeft + dx;
        const newTop = d.startTop + dy;
        const def = findElementDef(d.id);
        const elementId = def ? def.id : d.id;
        elem.x = clamp(Math.round(newLeft / scale), 0, SCREEN_SIZE - 1);
        const visualY = clamp(Math.round(newTop / scale), 0, SCREEN_SIZE - 1);
        elem.y = visualYToLayoutY(visualY, elementId);
    }

    renderScreenCanvas();
}

function onScreenPointerUp(e) {
    const d = window.screenEditor.dragging;
    if (!d) return;
    const placedFromPalette = d.fromPalette && d.moved && isScreenStaticTextElement(d.id);
    window.screenEditor.dragging = null;
    updateScreenStatusLine();
    if (placedFromPalette) renderScreenPalette();
}

function clamp(v, lo, hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

function resetDevice() {
    showStatus(getMessage('sending_reset'), 'warning');
    
    fetch('/api/reset', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({}),
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'ok') {
            showStatus(getMessage('device_restarting') + '15' + getMessage('device_restarting_seconds'), 'success');
            
            // Countdown timer for reconnecting
            let countdown = 15;
            const countdownInterval = setInterval(function() {
                countdown--;
                if (countdown <= 0) {
                    clearInterval(countdownInterval);
                    window.location.reload();
                } else {
                    showStatus(getMessage('device_restarting') + countdown + getMessage('device_restarting_seconds'), 'success');
                }
            }, 1000);
        } else {
            showStatus(getMessage('failed_restart') + (data.message || 'Unknown error'), 'error');
        }
    })
    .catch(error => {
        showStatus(getMessage('error_reset_connection'), 'error');

        // Even if we got an error, likely the device is rebooting, so try to reconnect
        setTimeout(function() {
            window.location.reload();
        }, 15000);
    });
}

/* ---------- wire the editor into the glass tab system ----------
   Loaded once when the Layout tab is first opened (showTab guards via
   loadedSections), so unsaved edits survive switching tabs. */
sectionLoaders.layout = function () {
    if (!window.sectionsInitialized.screen) {
        window.sectionsInitialized.screen = true;
        setupScreenSection();
    }
    fetchScreenLayout();
};

