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
/* Unified text widgets (firmware f-text-widget.h). Family 0/1 are bitmap fonts,
   2 = profile digit theme, 3 = profile aux theme, 4+ = SCREEN_TIME_FONTS[family - 4]. */
const SCREEN_TW_IDS = ['tw_1', 'tw_2', 'tw_3', 'tw_4', 'tw_5', 'tw_6', 'tw_7', 'tw_8'];
const SCREEN_TW_TEXT_LEN = 64;
const SCREEN_TW_SLOTS = 3;           // contents a widget can rotate through
const SCREEN_TW_DURATION_DEFAULT = 10;
const SCREEN_TW_DURATION_MAX = 3600;
const SCREEN_TW_SIZE_MIN = 8;
const SCREEN_TW_SIZE_MAX = 36;
const SCREEN_TW_SIZES = [8, 10, 12, 14, 16, 20, 24, 28, 32, 36]; // generated bitmap sizes
const SCREEN_TW_FAMILY = { SANS: 0, BOLD: 1, SPRITE_THEME: 2, SPRITE_AUX: 3, SPRITE_FIRST: 4 };
const SCREEN_TW_FLAG = { SHOW_UNITS: 0x01, TABULAR: 0x02, TRANSPARENT_BG: 0x04, COLOR_OVERRIDE: 0x08, SCROLL: 0x10 };
// [line_height, base_line, cap_height] of main/fonts/tw_<family>_<size>.c. Sans also has the
// 40-52 sizes the device uses for units next to digit sprites.
const SCREEN_TW_METRICS = {
    0: { 8: [10, 2, 5], 10: [11, 2, 7], 12: [16, 3, 9], 14: [18, 3, 10], 16: [21, 4, 12], 20: [22, 4, 14], 24: [26, 5, 17], 28: [29, 5, 20], 32: [33, 6, 22], 36: [37, 7, 25], 40: [43, 8, 29], 44: [48, 9, 31], 48: [50, 9, 34], 52: [55, 10, 37] },
    1: { 8: [11, 2, 6], 10: [14, 3, 7], 12: [16, 4, 9], 14: [18, 4, 10], 16: [20, 5, 12], 20: [21, 5, 15], 24: [26, 6, 17], 28: [31, 7, 20], 32: [35, 9, 23], 36: [38, 9, 26] }
};
// Web copies of the two text fonts (spiffs/tw-*.woff), so previews look like the device.
const SCREEN_TW_CSS_FONTS = {
    0: { family: "TWSans, Montserrat, 'Segoe UI', Arial, sans-serif", weight: '500' },
    1: { family: "TWBold, 'DejaVu Sans', Verdana, sans-serif", weight: '700' }
};
const SCREEN_COLOR_FILTER_OPTIONS = [
    { v: 0, key: 'advanced.color_filter.none', label: 'None' },
    { v: 1, key: 'advanced.color_filter.red', label: 'Red' },
    { v: 2, key: 'advanced.color_filter.green', label: 'Green' },
    { v: 3, key: 'advanced.color_filter.blue', label: 'Blue' },
    { v: 4, key: 'advanced.color_filter.bw', label: 'Black & White' }
];
const SCREEN_DEFAULT_SCROLL_TEXT = '[device]: [greeting] [day], [date] [mon], now [temp] today [high]-[low], hum. [hum], sun [rise]-[set]';
const SCREEN_PALETTE_LABEL_MAX = 24;
const SCREEN_TOKEN_CODES = [
    '[device]', '[greeting]', '[day]', '[date]', '[mon]', '[time]', '[hour12]', '[hour24]', '[min]', '[ampm]',
    '[temp]', '[hum]', '[high]', '[low]',
    '[rise]', '[set]', '[wind]', '[gust]', '[precip]', '[uv]', '[pressure]', '[3high]', '[3low]',
    '[wu:temp]', '[wu:hum]', '[wu:dew]', '[wu:wind]', '[wu:gust]', '[wu:pressure]', '[wu:rain]', '[wu:uv]',
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
    '[CGM:glucose]': { i18nKey: 'advanced.message.token_cgm_glucose', example: '157 mg/dL' },
    '[CGM:reading]': { i18nKey: 'advanced.message.token_cgm_reading', example: '157' },
    '[CGM:time]': { i18nKey: 'advanced.message.token_cgm_time', example: '12:01pm' },
    '[wu:temp]': { i18nKey: 'advanced.message.token_wu_temp', example: '72°F' },
    '[wu:hum]': { i18nKey: 'advanced.message.token_wu_hum', example: '45%' },
    '[wu:dew]': { i18nKey: 'advanced.message.token_wu_dew', example: '50°F' },
    '[wu:wind]': { i18nKey: 'advanced.message.token_wu_wind', example: '11.2 mph NW' },
    '[wu:gust]': { i18nKey: 'advanced.message.token_wu_gust', example: '17.9 mph NW' },
    '[wu:pressure]': { i18nKey: 'advanced.message.token_wu_pressure', example: '29.92 inHg' },
    '[wu:rain]': { i18nKey: 'advanced.message.token_wu_rain', example: '0.06 in.' },
    '[wu:uv]': { i18nKey: 'advanced.message.token_wu_uv', example: '6.2' },
    '[colon]': { i18nKey: 'screen.tw_token_colon', example: ':' }
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
    // skip syncTokenHighlightMetrics here; it's expensive and only needed on resize
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
    const syncMetricsAndRefresh = () => {
        syncTokenHighlightMetrics(textarea, highlight);
        updateTokenHighlightTextarea(textarea);
    };
    const syncScroll = () => {
        highlight.scrollTop = textarea.scrollTop;
        highlight.scrollLeft = textarea.scrollLeft;
    };
    textarea.addEventListener('input', refresh);
    textarea.addEventListener('scroll', syncScroll);
    if (typeof ResizeObserver !== 'undefined') {
        const ro = new ResizeObserver(syncMetricsAndRefresh);
        ro.observe(textarea);
    }
    syncMetricsAndRefresh();
}
function getScreenTranslation(key, fallback) {
    const trans = translations[currentLanguage] || translations.en;
    return getNestedTranslation(trans, key) || fallback;
}

function getScreenElementLabel(id) {
    const named = getScreenInstanceName(id);
    if (named) return named;
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
    sw.setAttribute('role', 'switch');
    sw.setAttribute('aria-checked', checked ? 'true' : 'false');
    sw.setAttribute('aria-label', label);
    sw.addEventListener('click', () => {
        const on = sw.classList.toggle('on');
        sw.setAttribute('aria-checked', on ? 'true' : 'false');
        if (onChange) onChange(on);
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
    renderScreenPresets();
}

let _screenRectCache = null;
let _screenScaleCache = null;

function invalidateScreenCache() {
    _screenRectCache = null;
    _screenScaleCache = null;
}

function getScreenRect() {
    if (_screenRectCache) return _screenRectCache;
    const canvas = el('screenCanvas');
    if (!canvas) return null;
    _screenRectCache = canvas.getBoundingClientRect();
    return _screenRectCache;
}

function canvasCoordsFromPointer(clientX, clientY) {
    const rect = getScreenRect();
    if (!rect) return null;
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
    const multiHint = getScreenTranslation('screen.multi_hint', 'Ctrl+click for multiple selection');

    if (isMultiSelect()) {
        status.textContent = getScreenTranslation('screen.status_multi', '{n} elements selected')
            .replace('{n}', String(getSelectedIds().length)) + ' · ' + multiHint;
        return;
    }

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
                .replace('{h}', String(size.h)) + ' · ' + multiHint;
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
        .replace('{y}', cy !== null ? String(cy) : '—') + ' · ' + multiHint;
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
    moon: 'moon'
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
    const nestedBasenames = new Set(names
        .filter(name => name.includes('/'))
        .map(name => name.split('/').pop().toLowerCase()));
    return [...new Set(names.filter(name => name.includes('/') || !nestedBasenames.has(name.toLowerCase())))]
        .sort((a, b) => a.localeCompare(b));
}

// Human-facing name for a .layout file: extension stripped, separators spaced.
function screenLayoutDisplayName(name) {
    return name.split('/').pop().replace(/\.layout$/i, '').replace(/[-_]/g, ' ');
}

function screenFileUrl(name) {
    return '/' + name.split('/').map(part => encodeURIComponent(part)).join('/');
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
    current.textContent = getScreenTranslation('common.current', 'Current');
    select.appendChild(current);

    names.forEach(name => {
        const option = document.createElement('option');
        option.value = name;
        option.textContent = screenLayoutDisplayName(name);
        select.appendChild(option);
    });

    select.value = names.includes(previous) ? previous : '';
    return names;
}

async function refreshScreenLayoutFiles() {
    // The Presets gallery (renderScreenPresets) is now the only layout picker;
    // this just makes sure the SPIFFS file list is fresh before it renders.
    await ensureScreenSpiffsFiles(true);
}

/* ---------- preset gallery ----------
   One card per .layout file on the device. Clicking a card loads that layout
   into the editor (same path as the Load button); the user then presses Apply. */
async function renderScreenPresets(selectedName) {
    const grid = el('screenPresets');
    if (!grid) return;
    await ensureScreenSpiffsFiles();
    const names = getScreenLayoutFileNames();
    grid.innerHTML = '';
    if (!names.length) {
        const d = document.createElement('div');
        d.className = 'preset-empty';
        d.textContent = getScreenTranslation('screen.presets_none', 'No layout files on the device.');
        grid.appendChild(d);
        return;
    }
    names.forEach(name => {
        const card = document.createElement('div');
        card.className = 'preset-card';
        if (selectedName && name === selectedName) card.classList.add('sel');
        card.setAttribute('role', 'button');
        card.tabIndex = 0;
        const pn = document.createElement('div');
        pn.className = 'pn';
        pn.textContent = screenLayoutDisplayName(name);
        card.appendChild(pn);
        card.title = name; // full filename on hover only
        const act = async () => {
            await loadSystemScreenLayout(name);
            $$('#screenPresets .preset-card').forEach(c => c.classList.toggle('sel', c === card));
        };
        card.addEventListener('click', act);
        card.addEventListener('keydown', e => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); act(); } });
        grid.appendChild(card);
    });
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
        const response = await fetch(screenFileUrl(filename));
        if (!response.ok) { showStatus(getMessage('layout_read_invalid'), 'error'); return; }
        const data = await response.json();
        if (!data || !data.profiles) { showStatus(getMessage('layout_read_invalid'), 'error'); return; }

        const layout = prepareScreenLayout(data);
        layout.version = SCREEN_LAYOUT_VERSION;
        ensureScreenLayoutMeta(layout);
        ensureScreenProfileShape(layout.profiles.day);
        ensureScreenProfileShape(layout.profiles.night);

        const body = encodeScreenLayoutBinary(layout);
        if (body.byteLength !== SCREEN_BIN_FULL_WIRE_SIZE) {
            throw new Error(`screen wire size mismatch: ${body.byteLength} != ${SCREEN_BIN_FULL_WIRE_SIZE}`);
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
        const themed = `themes/${theme}/${item}.${ext}`;
        if (hasSpiffsFile(themed)) return themed;
        const legacy = `${theme}-${item}.${ext}`;
        if (hasSpiffsFile(legacy)) return legacy;
    }
    for (const ext of ['bmp', 'jpg']) {
        const fallback = `themes/default/${item}.${ext}`;
        if (hasSpiffsFile(fallback)) return fallback;
        const legacy = `default-${item}.${ext}`;
        if (hasSpiffsFile(legacy)) return legacy;
    }
    return 'logo.jpg';
}

function resolveTimeFontPreviewUrl(font) {
    const preview = `themes/${font}/preview.jpg`;
    if (hasSpiffsFile(preview)) return preview;
    const legacy = `${font}.jpg`;
    if (hasSpiffsFile(legacy)) return legacy;
    if (font === 'user1' || font === 'user2') return 'themes/bold/preview.jpg';
    return preview;
}

function getScreenAuxFont(mode) {
    if (!window.screenLayout) return 'bold';
    return mode === 'night'
        ? (window.screenLayout.night_aux_font || 'bold')
        : (window.screenLayout.day_aux_font || 'bold');
}

function getPreviewImageUrl(def, inMatrix) {
    if (!inMatrix && def.paletteImg) return def.paletteImg;
    if (isScreenIconElement(def.id)) {
        // Show the uploaded JPG only once it has loaded (dims cached); until then
        // return null so the placeholder "Icon N" label is shown instead.
        const dims = getIconDims(def.id);
        return dims ? iconUrl(def.id) : null;
    }
    const item = SCREEN_THEME_ASSETS[def.id];
    if (item && inMatrix) {
        const theme = def.id === 'weather'
            ? getScreenWeatherFont(window.screenEditor.mode)
            : getScreenTimeFont(window.screenEditor.mode);
        return resolveThemeAssetUrl(theme, item);
    }
    return def.img;
}

const SCREEN_ELEMENT_DEFS = [
    { id: 'glucose_level', label: 'Glucose Level', w: 14, h: 20, img: 'themes/default/glucose.jpg', paletteImg: 'palette-glucose.png', paletteFitFullImage: true, spriteSheet: { index: 2, frames: 4 } },
    { id: 'glucose_trend', label: 'Glucose Trend', w: 12, h: 14, img: 'themes/default/trend.jpg', paletteImg: 'palette-trend.png', paletteFitFullImage: true, spriteSheet: { index: 2, frames: 5 } },
    { id: 'wifi_off', label: 'WiFi off', w: 20, h: 20, img: 'themes/default/wifi-off.jpg', paletteImg: 'palette-wifi-off.png', paletteFitFullImage: true },
    { id: 'weather', label: 'Weather', w: 32, h: 22, img: 'themes/default/weather.jpg' },
    { id: 'moon', label: 'Moon', w: 14, h: 14, img: 'themes/default/moon.jpg', paletteImg: 'palette-moon.jpg', paletteFitFullImage: true, spriteSheet: { index: 2, frames: 8 } },
    { id: 'graph', label: 'Graph', w: 80, h: 36, img: 'themes/default/graph.jpg', paletteImg: 'palette-graph.png', paletteFitFullImage: true },
    { id: 'icon_1', label: 'Icon 1', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_2', label: 'Icon 2', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_3', label: 'Icon 3', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_4', label: 'Icon 4', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_5', label: 'Icon 5', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_6', label: 'Icon 6', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_7', label: 'Icon 7', w: 24, h: 24, isIcon: true, paletteFitFullImage: true },
    { id: 'icon_8', label: 'Icon 8', w: 24, h: 24, isIcon: true },
    { id: 'tw_1', label: 'Text 1', w: 80, h: 21 },
    { id: 'tw_2', label: 'Text 2', w: 80, h: 21 },
    { id: 'tw_3', label: 'Text 3', w: 80, h: 21 },
    { id: 'tw_4', label: 'Text 4', w: 80, h: 21 },
    { id: 'tw_5', label: 'Text 5', w: 80, h: 21 },
    { id: 'tw_6', label: 'Text 6', w: 80, h: 21 },
    { id: 'tw_7', label: 'Text 7', w: 80, h: 21 },
    { id: 'tw_8', label: 'Text 8', w: 80, h: 21 }
];

const SCREEN_ELEMENT_DEFS_MAP = new Map(SCREEN_ELEMENT_DEFS.map(d => [d.id, d]));

const SCREEN_ICON_SLOT_IDS = ['icon_1', 'icon_2', 'icon_3', 'icon_4', 'icon_5', 'icon_6', 'icon_7', 'icon_8'];
const SCREEN_ICON_MAX_DIM = 64; // must match firmware SCREEN_ICON_MAX_DIM

const SCREEN_PALETTE_SETTINGS_DEF = {
    id: 'screen_settings',
    label: 'Screen settings',
    w: 32,
    h: 32,
    img: 'themes/default/settings.jpg',
    paletteFitFullImage: true,
    paletteMeta: true
};

const SCREEN_PALETTE_ORDER = [
    'screen_settings',
    'weather',
    'moon',
    'wifi_off',
    'glucose_level',
    'glucose_trend',
    'graph'
];

const SCREEN_ALIGNMENT_GRID_LINES = [20, 30, 40, 90, 100, 110];

function truncatePaletteLabel(text, maxLen = SCREEN_PALETTE_LABEL_MAX) {
    const s = (text || '').trim();
    if (!s) return '';
    return s.length > maxLen ? s.slice(0, maxLen) + '\u2026' : s;
}

// User-given name of a text/icon widget instance in the current profile ('' when unnamed).
function getScreenInstanceName(id) {
    if (!isScreenTextWidgetElement(id) && !isScreenIconElement(id)) return '';
    const e = screenFindProfileElement(getProfileObj(window.screenEditor.mode), id);
    return e && typeof e.name === 'string' ? e.name.trim() : '';
}

function getTextPaletteLabel(profile, id) {
    const name = getScreenInstanceName(id);
    if (name) return name;
    const emptyLabel = getScreenTranslation('screen.empty_text_label', '(empty)');
    const e = screenFindProfileElement(profile, id);
    return screenDefaultTextWidgetName(e) || emptyLabel;
}

function screenDefaultTextWidgetName(e) {
    if (!e || !isScreenTextWidgetElement(e.id)) return '';
    const options = ensureScreenTwOptions(e);
    if (options.scroll) return 'Scroll';
    const template = String((options.slots[0] && options.slots[0].text) || '').trim();
    if (/\[clock\]/i.test(template)) return 'Clock';
    if (/\[(?:ampm12|ampm)\]/i.test(template)) return 'AM/PM';
    const plain = template.replace(/\[[^\]]+\]/g, ' ').replace(/\s+/g, ' ')
        .replace(/^[\s:;,./-]+|[\s:;,./-]+$/g, '').trim();
    return screenClampUtf8(plain || `Text ${e.id.split('_')[1]}`, SCREEN_BIN_NAME_LEN - 1);
}

function getScreenPaletteItemLabel(def, profile) {
    if (isScreenTextWidgetElement(def.id) && profile) return getTextPaletteLabel(profile, def.id);
    return getScreenElementLabel(def.id);
}

function isPaletteItemSelected(def) {
    return window.screenEditor.selectedId === def.id;
}

// A slot is free when it was never added, or was deleted again.
function isScreenSlotFree(profile, id) {
    const elem = profile && profile.elements ? profile.elements.find(e => e.id === id) : null;
    return !elem || (!elem.instanced && !elem.enabled);
}

function getNextTextSlot(profile) {
    return SCREEN_TW_IDS.find(id => isScreenSlotFree(profile, id)) || null;
}

function getNextIconSlot(profile) {
    return SCREEN_ICON_SLOT_IDS.find(id => isScreenSlotFree(profile, id)) || null;
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
        const def = findElementDef(id);
        if (def && !isScreenTextWidgetElement(def.id) && !isScreenIconElement(def.id)) {
            items.push(def);
        }
    });

    if (profile) {
        SCREEN_INSTANCE_IDS.forEach(id => {
            const elem = profile.elements.find(e => e.id === id);
            if (elem && (elem.instanced || elem.enabled)) {
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

function isScreenTextElement(id) {
    return id === 'message';
}

function ensureScreenTextOptions(element) {
    if (!element.options) element.options = {};
    if (element.options.font === undefined || element.options.font === 255) element.options.font = 0;
    if (!element.options.color) element.options.color = '#ffffff';
    if (!element.options.bg_color) element.options.bg_color = '#000000';
}

/* ---------- unified text widgets (tw_1..tw_8) ---------- */
function isScreenTextWidgetElement(id) {
    return /^tw_[1-8]$/.test(id);
}

function twIsSprite(family) {
    return family >= SCREEN_TW_FAMILY.SPRITE_THEME;
}

// Device picks the largest generated bitmap size <= the requested one.
function twBitmapSize(size) {
    let s = SCREEN_TW_SIZES[0];
    SCREEN_TW_SIZES.forEach(v => { if (v <= size) s = v; });
    return s;
}

function twMetrics(family, size) {
    return SCREEN_TW_METRICS[family === SCREEN_TW_FAMILY.BOLD ? 1 : 0][twBitmapSize(size)];
}

// Sans font the device uses for units/letters next to sprite digits: capitals as tall as
// the digit ink (about 34/36 of the cell height). Mirrors font_registry_match_cap().
function twSpriteFallbackEm(size) {
    const cap = Math.round(size * 34 / 36);
    let best = 8;
    let bestDiff = 1e9;
    Object.keys(SCREEN_TW_METRICS[0]).forEach(k => {
        const d = Math.abs(SCREEN_TW_METRICS[0][k][2] - cap);
        if (d < bestDiff) { bestDiff = d; best = parseInt(k, 10); }
    });
    return best;
}

function twSpriteTheme(family, mode) {
    if (family === SCREEN_TW_FAMILY.SPRITE_THEME) return getScreenTimeFont(mode);
    if (family === SCREEN_TW_FAMILY.SPRITE_AUX) return getScreenAuxFont(mode);
    return SCREEN_TIME_FONTS[family - SCREEN_TW_FAMILY.SPRITE_FIRST] || 'bold';
}

// Sprite digits sit on the baseline of the cap-matched sans fallback (units, letters).
function twLineHeight(o) {
    if (twIsSprite(o.family)) return o.size + SCREEN_TW_METRICS[0][twSpriteFallbackEm(o.size)][1];
    return twMetrics(o.family, o.size)[0];
}

function twMeasureTextBounds(text, family, size) {
    const css = SCREEN_TW_CSS_FONTS[family] || SCREEN_TW_CSS_FONTS[0];
    const canvas = document.createElement('canvas');
    const context = canvas.getContext && canvas.getContext('2d');
    const metrics = SCREEN_TW_METRICS[family === SCREEN_TW_FAMILY.BOLD ? 1 : 0][size] || twMetrics(family, size);
    if (!context) return {
        width: Math.ceil([...String(text)].length * size * 0.6),
        ascent: metrics[2], descent: 0
    };
    context.font = `${css.weight} ${size}px ${css.family}`;
    const measured = context.measureText(String(text));
    return {
        width: Math.max(1, Math.ceil(measured.width)),
        ascent: Number.isFinite(measured.actualBoundingBoxAscent) ? measured.actualBoundingBoxAscent : metrics[2],
        descent: Number.isFinite(measured.actualBoundingBoxDescent) ? measured.actualBoundingBoxDescent : 0
    };
}

function twMeasureTextWidth(text, family, size) {
    return twMeasureTextBounds(text, family, size).width;
}

function twContentHeight(o, text) {
    const value = String(text || '');
    if (!value.trim()) return twLineHeight(o);
    if (!twIsSprite(o.family)) {
        const bounds = twMeasureTextBounds(value, o.family, twBitmapSize(o.size));
        return Math.max(1, Math.ceil(bounds.ascent + bounds.descent));
    }

    let top = Infinity;
    let bottom = -Infinity;
    if (/[0-9:.]/.test(value)) {
        top = 0;
        bottom = o.size;
    }
    const fallbackText = value.replace(/[0-9:.]/g, '');
    if (fallbackText) {
        const bounds = twMeasureTextBounds(fallbackText, SCREEN_TW_FAMILY.SANS, twSpriteFallbackEm(o.size));
        top = Math.min(top, o.size - bounds.ascent);
        bottom = Math.max(bottom, o.size + bounds.descent);
    }
    return Number.isFinite(top) ? Math.max(1, Math.ceil(bottom - top)) : 1;
}

function twWidgetHeight(o, text) {
    return twContentHeight(o, text) + 4;
}

function twContentWidth(o, text) {
    if (!twIsSprite(o.family)) return twMeasureTextWidth(text, o.family, twBitmapSize(o.size));
    const px = v => Math.round(v * o.size / 36);
    const digitWidth = Math.max(1, px(18));
    const separatorWidth = px(1) * 2 + Math.max(1, px(4));
    const fallbackSize = twSpriteFallbackEm(o.size);
    let width = 0;
    for (const ch of String(text)) {
        if (ch >= '0' && ch <= '9') width += digitWidth;
        else if (ch === ':' || ch === '.') width += separatorWidth;
        else width += twMeasureTextWidth(ch, 0, fallbackSize);
    }
    return Math.max(1, width);
}

function updateTwFontAndCenter(e, o, update) {
    const autoWidth = o.width === 0;
    const def = findElementDef(e.id);
    const centerX = autoWidth
        ? (e.x || 0) + getScreenElementSize(def, e).w / 2
        : 0;
    update();
    if (autoWidth) {
        const width = getScreenElementSize(def, e).w;
        e.x = clamp(Math.round(centerX - width / 2), 0, Math.max(0, SCREEN_SIZE - width));
    }
}

function twDefaultOptions() {
    return {
        family: SCREEN_TW_FAMILY.SANS, size: 16, show_units: true, tabular: true, transparent: false,
        color: '#ffffff', color_enabled: false, bg_color: '#000000', width: 0, align: 0, scroll: false,
        slots: [{ text: '', duration: SCREEN_TW_DURATION_DEFAULT }]
    };
}

function twDefaultElement(id) {
    const i = Math.max(0, SCREEN_TW_IDS.indexOf(id));
    return { id, enabled: 0, x: 4, y: 4 + (i % 4) * 30, z: 2, options: twDefaultOptions() };
}

function ensureScreenTwOptions(e) {
    if (!e.options) e.options = {};
    const o = e.options;
    const d = twDefaultOptions();
    const colorModeSpecified = o.color_enabled !== undefined;
    for (const k in d) if (o[k] === undefined) o[k] = d[k];
    if (!colorModeSpecified) o.color_enabled = o.color !== '#ffffff';
    // Layout files written before rotation had a single `text`.
    if (typeof o.text === 'string') {
        if (!Array.isArray(o.slots) || !o.slots.length || (o.slots.length === 1 && !o.slots[0].text)) {
            o.slots = [{ text: o.text, duration: SCREEN_TW_DURATION_DEFAULT }];
        }
        delete o.text;
    }
    if (!Array.isArray(o.slots) || !o.slots.length) o.slots = [{ text: '', duration: SCREEN_TW_DURATION_DEFAULT }];
    o.slots = o.slots.slice(0, SCREEN_TW_SLOTS).map(s => (s && typeof s === 'object') ? s : {});
    // In place: editor handlers keep references to these objects between renders.
    o.slots.forEach(s => {
        s.text = String(s.text || '');
        s.duration = clamp(parseInt(s.duration, 10) || SCREEN_TW_DURATION_DEFAULT, 1, SCREEN_TW_DURATION_MAX);
    });
    o.size = clamp(parseInt(o.size, 10) || 16, SCREEN_TW_SIZE_MIN, SCREEN_TW_SIZE_MAX);
    o.family = clamp(parseInt(o.family, 10) || 0, 0,
        SCREEN_TW_FAMILY.SPRITE_FIRST + SCREEN_TIME_FONTS.length - 1);
    o.scroll = !!o.scroll;
    if (o.scroll) {
        if (o.family > SCREEN_TW_FAMILY.BOLD) o.family = SCREEN_TW_FAMILY.SANS;
        if (o.size > 20) o.size = 20;
    }
    return o;
}

// Longest prefix of `text` that fits a slot without cutting a [token] in half.
function twFitSlotText(text) {
    let s = String(text || '').slice(0, SCREEN_TW_TEXT_LEN - 1);
    const open = s.lastIndexOf('[');
    if (open > s.lastIndexOf(']') && s.length < String(text || '').length) s = s.slice(0, open);
    return s;
}

// Slot being edited / previewed per widget (editor-only state).
function twEditedSlot(e) {
    const o = ensureScreenTwOptions(e);
    const map = window.screenEditor.twSlot = window.screenEditor.twSlot || {};
    return Math.min(map[e.id] || 0, o.slots.length - 1);
}

// Same rule as strip_units() in f-text-widget.c.
function twStripUnits(value) {
    const m = /^\s*([-+]?\d[\d.,:]*)/.exec(value);
    return m ? m[1].replace(/[.,:]+$/, '') : value;
}

// Wall clock of the device (its IANA timezone when known) in the UI language.
function twFormatDate(now, lang, opts) {
    const tz = window.settings && window.settings.tz_iana ? String(window.settings.tz_iana) : undefined;
    try {
        return new Intl.DateTimeFormat(lang, Object.assign({ timeZone: tz }, opts)).format(now);
    } catch (err) {
        return new Intl.DateTimeFormat(lang, opts).format(now);
    }
}

function twClock(now) {
    const lang = (typeof currentLanguage === 'string' && currentLanguage) || 'en';
    const num = opts => parseInt(twFormatDate(now, 'en-US', opts), 10);
    return {
        hour: num({ hour: 'numeric', hourCycle: 'h23' }) % 24,
        min: num({ minute: 'numeric' }),
        mday: num({ day: 'numeric' }),
        weekday: twFormatDate(now, lang, { weekday: 'long' }),
        month: twFormatDate(now, lang, { month: 'long' })
    };
}

// Tokens the device never strips units from (see text_widget_render() in f-text-widget.c).
const SCREEN_TW_RAW_TOKENS = new Set(['[colon]', '[clock]', '[ampm12]']);

// Value of one token as the device would show it; null when the token is unknown.
// Time, date and device tokens are live; data tokens (weather, CGM, ...) show their example.
function twTokenValue(tok, clock) {
    const s = window.settings || {};
    const h12 = !!s.p37;
    const lead = s.p24 === undefined ? true : !!s.p24;
    const pad = n => String(n).padStart(2, '0');
    const hour12 = clock.hour === 0 ? 12 : (clock.hour > 12 ? clock.hour - 12 : clock.hour);
    const ampm = h12 ? (clock.hour >= 12 ? 'pm' : 'am') : '';
    switch (tok) {
        case '[colon]': return ':';
        case '[clock]': {
            const h = h12 ? hour12 : clock.hour;
            return `${lead ? pad(h) : h}:${pad(clock.min)}`;
        }
        case '[ampm12]':
        case '[ampm]': return ampm;
        case '[hour12]': return String(hour12);
        case '[hour24]': return pad(clock.hour);
        case '[min]': return pad(clock.min);
        case '[time]': return h12 ? `${hour12}:${pad(clock.min)}${ampm}` : `${pad(clock.hour)}:${pad(clock.min)}`;
        case '[date]': return String(clock.mday);
        case '[day]': return clock.weekday;
        case '[mon]': return clock.month;
        case '[device]': return s.p00 || TOKEN_HELP[tok].example;
        default:
            if (TOKEN_HELP[tok]) return TOKEN_HELP[tok].example;
            return /^\[(HA|\$):/.test(tok) ? '42' : null;
    }
}

// Expands the tokens of a template the way the device does before drawing it.
function twExpandText(text, showUnits, now) {
    const clock = twClock(now || new Date());
    return String(text || '').replace(/\[[^\]\[]+\]/g, tok => {
        const v = twTokenValue(tok, clock);
        if (v === null) return tok;
        return showUnits || SCREEN_TW_RAW_TOKENS.has(tok) ? v : twStripUnits(v);
    });
}

function twSampleText(o, slot) {
    return twExpandText((o.slots[slot || 0] || o.slots[0]).text, o.show_units);
}

// The canvas shows live values, so repaint when the expanded text of a visible widget changes.
let screenTokenTicker = null;
function refreshScreenTokenPreviews() {
    const canvas = el('screenCanvas');
    if (!window.screenLayout || !canvas || canvas.offsetParent === null || window.screenEditor.dragging) return;
    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile || !Array.isArray(profile.elements)) return;
    const sig = profile.elements
        .filter(e => isScreenTextWidgetElement(e.id) && e.enabled)
        .map(e => {
            const o = ensureScreenTwOptions(e);
            return `${e.id}=${o.scroll ? twExpandText(profile.scroll_text, o.show_units) : twSampleText(o, twEditedSlot(e))}`;
        })
        .join('\n');
    if (sig === window.screenEditor.tokenSig) return;
    window.screenEditor.tokenSig = sig;
    renderScreenCanvas();
}

function startScreenTokenTicker() {
    if (!screenTokenTicker) screenTokenTicker = setInterval(refreshScreenTokenPreviews, 1000);
}

function createTwPreview(def, inMatrix, e, matrixScale) {
    const preview = document.createElement('div');
    preview.className = `screen-preview screen-preview-tw ${inMatrix ? 'screen-preview-matrix' : 'screen-preview-palette'}`;
    preview.setAttribute('aria-label', def.label);
    if (!inMatrix || !e) {
        applyPalettePreviewSize(preview, def);
        const t = document.createElement('div');
        t.className = 'screen-preview-text';
        t.style.fontWeight = '700';
        t.textContent = '12:34';
        preview.appendChild(t);
        return preview;
    }

    const o = ensureScreenTwOptions(e);
    const s = matrixScale || getScreenScale();
    const sprite = twIsSprite(o.family);
    preview.style.display = 'flex';
    preview.style.alignItems = 'flex-start';
    preview.style.overflow = 'hidden';
    preview.style.boxSizing = 'border-box';
    preview.style.padding = `${2 * s}px 0`;
    preview.style.justifyContent = ['flex-start', 'center', 'flex-end'][o.align] || 'flex-start';
    preview.style.backgroundColor = sprite ? '#000000' : (o.transparent ? 'transparent' : o.bg_color);

    const editedSlot = twEditedSlot(e);
    const profile = getProfileObj(window.screenEditor.mode);
    const hasTemplate = o.scroll || !!o.slots[editedSlot].text;
    const sample = o.scroll
        ? twExpandText(profile && profile.scroll_text, o.show_units)
        : (hasTemplate ? twSampleText(o, editedSlot) : '');
    const text = hasTemplate ? sample : getScreenElementLabel(def.id);
    const contentH = twContentHeight(o, sample);
    if (!sprite) {
        const css = SCREEN_TW_CSS_FONTS[o.family] || SCREEN_TW_CSS_FONTS[0];
        const span = document.createElement('div');
        span.textContent = text;
        span.style.fontFamily = css.family;
        span.style.fontWeight = css.weight;
        span.style.fontSize = `${twBitmapSize(o.size) * s}px`;
        span.style.lineHeight = `${contentH * s}px`;
        span.style.color = o.color_enabled ? o.color : '#ffffff';
        span.style.whiteSpace = 'pre';
        span.style.fontVariantNumeric = o.tabular ? 'tabular-nums' : 'normal';
        preview.appendChild(span);
        return preview;
    }

    // Sprite theme: slice the 180x36 sheet like the device does (cells reduced to size/36).
    const theme = twSpriteTheme(o.family, window.screenEditor.mode);
    const sheet = resolveThemeAssetUrl(theme, 'font');
    const dot = resolveThemeAssetUrl(theme, 'dot');
    const h = o.size;
    const px = v => Math.round(v * h / 36);
    const dw = Math.max(1, px(18));
    const dotSize = Math.max(1, px(4));
    const sepW = px(1) * 2 + dotSize;
    const fallbackPx = twSpriteFallbackEm(h);
    const row = document.createElement('div');
    row.style.display = 'flex';
    row.style.alignItems = 'baseline';
    row.style.height = `${contentH * s}px`;
    row.style.whiteSpace = 'pre';
    for (const ch of text) {
        const cell = document.createElement('div');
        cell.style.flex = '0 0 auto';
        if (ch >= '0' && ch <= '9') {
            cell.style.width = `${dw * s}px`;
            cell.style.height = `${h * s}px`;
            cell.style.backgroundImage = `url('${sheet}')`;
            cell.style.backgroundSize = `${dw * 10 * s}px ${h * s}px`;
            cell.style.backgroundPosition = `${-(ch.charCodeAt(0) - 48) * dw * s}px 0`;
        } else if (ch === ':' || ch === '.') {
            cell.style.position = 'relative';
            cell.style.width = `${sepW * s}px`;
            cell.style.height = `${h * s}px`;
            (ch === ':' ? [10, 26] : [30]).forEach(y => {
                const d = document.createElement('div');
                d.style.position = 'absolute';
                d.style.left = `${px(1) * s}px`;
                d.style.top = `${px(y) * s}px`;
                d.style.width = d.style.height = `${dotSize * s}px`;
                d.style.backgroundImage = `url('${dot}')`;
                d.style.backgroundSize = '100% 100%';
                cell.appendChild(d);
            });
        } else {
            cell.textContent = ch;
            cell.style.fontFamily = SCREEN_TW_CSS_FONTS[0].family;
            cell.style.fontSize = `${fallbackPx * s}px`;
            cell.style.lineHeight = '1';
            cell.style.color = o.color_enabled ? o.color : '#ffffff';
        }
        row.appendChild(cell);
    }
    preview.appendChild(row);
    return preview;
}

function appendScreenTwOptions(opt, e, profile, trans) {
    const o = ensureScreenTwOptions(e);
    const T = (k, d) => getNestedTranslation(trans, k) || d;
    let sectionTarget = opt;
    const mkRow = (labelText, control) => {
        const row = document.createElement('div');
        row.className = 'form-group';
        const lab = document.createElement('label');
        lab.textContent = labelText;
        row.appendChild(lab);
        row.appendChild(control);
        sectionTarget.appendChild(row);
        return control;
    };
    const mkSelect = (labelText, items, value, onCh) => {
        const sel = document.createElement('select');
        items.forEach(([v, t]) => {
            const op = document.createElement('option');
            op.value = String(v);
            op.textContent = t;
            sel.appendChild(op);
        });
        sel.value = String(value);
        sel.addEventListener('change', () => onCh(parseInt(sel.value, 10) || 0));
        return mkRow(labelText, sel);
    };
    const mkSection = title => {
        const section = document.createElement('section');
        section.className = 'screen-tw-section';
        const heading = document.createElement('h4');
        heading.className = 'screen-tw-section-title';
        heading.textContent = title;
        section.appendChild(heading);
        opt.appendChild(section);
        return section;
    };
    const generalSection = mkSection(T('screen.tw_section_general', 'General'));
    const fontSection = mkSection(T('screen.tw_section_font', 'Font'));
    const textSection = mkSection(T('screen.tw_section_text', 'Text'));
    sectionTarget = generalSection;
    appendScreenSwitchRow(generalSection, {
        id: 'screen_element_enabled',
        label: T('screen.enabled', 'Enabled'),
        checked: e.enabled !== 0,
        onChange: on => {
            e.enabled = on ? 1 : 0;
            renderScreenCanvas();
            renderScreenPalette();
        }
    });
    const scrolling = !!o.scroll;
    appendScreenSwitchRow(generalSection, {
        id: 'tw_scroll',
        label: T('screen.tw_scroll', 'Scroll text'),
        checked: scrolling,
        onChange: on => {
            o.scroll = on;
            if (on) {
                if (!profile.scroll_text) profile.scroll_text = o.slots[0].text || SCREEN_DEFAULT_SCROLL_TEXT;
                profile.elements.forEach(other => {
                    if (other.id !== e.id && isScreenTextWidgetElement(other.id))
                        ensureScreenTwOptions(other).scroll = false;
                });
            } else if (!o.slots[0].text) {
                // The long text may not fit a slot: keep its leading whole tokens.
                o.slots[0].text = twFitSlotText(profile.scroll_text);
            }
            renderScreenCanvas();
            renderScreenOptions();
            renderScreenPalette();
        }
    });

    const sprite = !scrolling && twIsSprite(o.family);

    sectionTarget = fontSection;
    appendScreenTwFontPicker(fontSection, o, T, e, scrolling);
    if (sprite) {
        // Sprites are reduced from 36 px, so any height in range is exact.
        const inp = document.createElement('input');
        inp.type = 'number';
        inp.min = String(SCREEN_TW_SIZE_MIN);
        inp.max = String(SCREEN_TW_SIZE_MAX);
        inp.value = String(o.size);
        inp.addEventListener('change', () => {
            const size = clamp(parseInt(inp.value, 10) || 36, SCREEN_TW_SIZE_MIN, SCREEN_TW_SIZE_MAX);
            updateTwFontAndCenter(e, o, () => { o.size = size; });
            inp.value = String(o.size);
            renderScreenCanvas();
        });
        mkRow(T('screen.tw_size', 'Size (px)'), inp);
    } else {
        const sizes = scrolling ? SCREEN_TW_SIZES.filter(v => v <= 20) : SCREEN_TW_SIZES;
        mkSelect(T('screen.tw_size', 'Size (px)'), sizes.map(v => [v, `${v} px`]), twBitmapSize(o.size), v => {
            updateTwFontAndCenter(e, o, () => { o.size = v; });
            renderScreenCanvas();
        });
    }

    const colorControl = document.createElement('div');
    colorControl.className = 'screen-tw-color-control';
    colorControl.setAttribute('role', 'radiogroup');
    colorControl.setAttribute('aria-label', sprite
        ? T('screen.tw_units_color', 'Units / text color')
        : T('screen.message_color', 'Color'));
    const makeColorChoice = (id, value, labelText, checked, onChange) => {
        const label = document.createElement('label');
        label.className = 'screen-tw-color-choice';
        const radio = document.createElement('input');
        radio.type = 'radio';
        radio.name = 'tw_color_mode';
        radio.id = id;
        radio.value = value;
        radio.checked = checked;
        radio.addEventListener('change', () => {
            if (radio.checked) onChange();
        });
        const text = document.createElement('span');
        text.textContent = labelText;
        label.appendChild(radio);
        label.appendChild(text);
        return label;
    };
    const color = document.createElement('input');
    color.type = 'color';
    color.value = o.color;
    color.disabled = !o.color_enabled;
    const defaultChoice = makeColorChoice(
        'tw_color_mode_default', 'default', T('screen.tw_color_default', 'Default'),
        !o.color_enabled, () => {
            o.last_custom_color = o.color;
            o.color = '#ffffff';
            o.color_enabled = false;
            color.disabled = true;
            renderScreenCanvas();
        });
    const customGroup = document.createElement('div');
    customGroup.className = 'screen-tw-color-custom';
    const customChoice = makeColorChoice(
        'tw_color_mode_custom', 'custom', T('screen.tw_color_custom', 'Custom'),
        !!o.color_enabled, () => {
            o.color = o.last_custom_color || color.value;
            o.color_enabled = true;
            color.disabled = false;
            renderScreenCanvas();
        });
    color.addEventListener('input', () => {
        o.color = color.value;
        o.last_custom_color = color.value;
        o.color_enabled = true;
        renderScreenCanvas();
    });
    colorControl.appendChild(defaultChoice);
    customGroup.appendChild(customChoice);
    customGroup.appendChild(color);
    colorControl.appendChild(customGroup);
    mkRow(sprite ? T('screen.tw_units_color', 'Units / text color') : T('screen.message_color', 'Color'), colorControl);

    if (sprite) {
        const note = document.createElement('p');
        note.className = 'screen-options-mode-note';
        note.textContent = T('screen.tw_sprite_note',
            'Digit themes keep their own colours on a black background; the colour applies to units and other characters.');
        fontSection.appendChild(note);
    }

    sectionTarget = generalSection;
    const width = document.createElement('input');
    width.type = 'number';
    width.min = '0';
    width.max = String(SCREEN_SIZE);
    width.value = String(o.width || 0);
    width.addEventListener('change', () => {
        o.width = clamp(parseInt(width.value, 10) || 0, 0, SCREEN_SIZE);
        renderScreenCanvas();
    });
    mkRow(T('screen.text_width', 'Width (px, 0 = auto)'), width);

    if (scrolling) {
        const delay = document.createElement('input');
        delay.type = 'number';
        delay.min = '30';
        delay.max = '255';
        delay.value = String(window.screenLayout.scroll_delay || 60);
        delay.addEventListener('change', () => {
            window.screenLayout.scroll_delay = clamp(parseInt(delay.value, 10) || 60, 30, 255);
        });
        mkRow(T('screen.scroll_delay', 'Scroll delay (ms)'), delay);
    }

    sectionTarget = textSection;
    appendScreenSwitchRow(textSection, {
        id: 'tw_show_units',
        label: T('screen.tw_show_units', 'Show units'),
        checked: !!o.show_units,
        onChange: on => { o.show_units = on; renderScreenCanvas(); }
    });
    mkSelect(T('screen.text_align', 'Alignment'), SCREEN_ALIGN_OPTIONS.map(a => [a.v, getScreenAlignLabel(a.v)]), o.align || 0, v => {
        o.align = v;
        renderScreenCanvas();
    });

    if (!sprite) {
        appendScreenSwitchRow(textSection, {
            id: 'tw_transparent',
            label: T('screen.tw_transparent', 'Transparent background'),
            checked: !!o.transparent,
            onChange: on => { o.transparent = on; renderScreenCanvas(); renderScreenOptions(); }
        });
        if (!o.transparent) {
            const bg = document.createElement('input');
            bg.type = 'color';
            bg.value = o.bg_color;
            bg.addEventListener('input', () => { o.bg_color = bg.value; renderScreenCanvas(); });
            mkRow(T('screen.background_color', 'Background Color'), bg);
        }
        appendScreenSwitchRow(textSection, {
            id: 'tw_tabular',
            label: T('screen.tw_tabular', 'Fixed-width digits'),
            checked: !!o.tabular,
            onChange: on => { o.tabular = on; renderScreenCanvas(); }
        });
    }

    if (scrolling) {
        const textArea = document.createElement('textarea');
        textArea.id = 'editor-textarea-' + e.id;
        textArea.maxLength = SCREEN_BIN_SCROLL_LEN - 1;
        textArea.rows = 4;
        textArea.value = profile.scroll_text || '';
        const counter = document.createElement('div');
        counter.id = textArea.id + '-counter';
        counter.className = 'counter';
        counter.setAttribute('aria-live', 'polite');
        textArea.setAttribute('aria-describedby', counter.id);
        textArea.addEventListener('input', () => {
            profile.scroll_text = textArea.value;
            renderScreenCanvas();
            updateCharCounter(textArea, counter);
        });
        const row = document.createElement('div');
        row.className = 'form-group';
        const label = document.createElement('label');
        label.textContent = T('screen.message_text', 'Text');
        row.appendChild(label);
        row.appendChild(textArea);
        row.appendChild(counter);
        textSection.appendChild(row);
        updateCharCounter(textArea, counter);
        setupTokenHighlightTextarea(textArea);
        appendScreenTokenButtons(textSection, textArea);
    } else {
        appendScreenTwSlots(textSection, e, o, T);
    }
}

// Font picker: one card per font showing how it looks. Text fonts and digit themes are
// separate sections, so card names stay short ("Nixie", not "Digits: Nixie").
function appendScreenTwFontPicker(opt, o, T, e, scrollOnly = false) {
    const wrap = document.createElement('div');
    wrap.className = 'screen-font-samples tw-font-picker';
    const cap = s => s.charAt(0).toUpperCase() + s.slice(1);

    const addSection = (title, wide, cards) => {
        const h = document.createElement('div');
        h.className = 'tw-font-group-label';
        h.textContent = title;
        wrap.appendChild(h);
        const grid = document.createElement('div');
        grid.className = 'screen-font-samples-grid' + (wide ? ' tw-font-grid-wide' : '');
        cards.forEach(c => grid.appendChild(c));
        wrap.appendChild(grid);
    };
    const card = (family, name, thumb) => {
        const box = document.createElement('button');
        box.type = 'button';
        box.className = 'screen-font-sample-box' + (o.family === family ? ' selected' : '');
        box.setAttribute('aria-pressed', o.family === family ? 'true' : 'false');
        const label = document.createElement('div');
        label.className = 'screen-font-sample-name';
        label.textContent = name;
        box.appendChild(thumb);
        box.appendChild(label);
        box.addEventListener('click', () => {
            updateTwFontAndCenter(e, o, () => { o.family = family; });
            renderScreenCanvas();
            renderScreenOptions();
        });
        return box;
    };
    const textThumb = family => {
        const t = document.createElement('div');
        t.className = 'screen-font-sample-thumb tw-font-sample';
        const css = SCREEN_TW_CSS_FONTS[family];
        t.style.fontFamily = css.family;
        t.style.fontWeight = css.weight;
        t.textContent = 'Aa 12:34 \u00b0C';
        return t;
    };
    const sheetThumb = theme => {
        const t = document.createElement('div');
        t.className = 'screen-font-sample-thumb tw-sheet-sample';
        t.style.backgroundImage = `url('${resolveThemeAssetUrl(theme, 'font')}')`;
        return t;
    };

    addSection(T('screen.tw_text_fonts', 'Text fonts'), false, [
        card(SCREEN_TW_FAMILY.SANS, T('screen.tw_family_sans', 'Sans'), textThumb(0)),
        card(SCREEN_TW_FAMILY.BOLD, T('screen.tw_family_bold', 'Bold'), textThumb(1))
    ]);

    // User fonts only appear once their sprite sheet has been uploaded.
    const themes = [];
    if (!scrollOnly) SCREEN_TIME_FONTS.forEach((f, i) => {
        const isUser = f === 'user1' || f === 'user2';
        if (isUser && !hasSpiffsFile(`themes/${f}/font.jpg`) && !hasSpiffsFile(`themes/${f}/font.bmp`) &&
            !hasSpiffsFile(`${f}-font.jpg`) && !hasSpiffsFile(`${f}-font.bmp`)) return;
        themes.push(card(SCREEN_TW_FAMILY.SPRITE_FIRST + i, cap(f), sheetThumb(f)));
    });
    if (themes.length) addSection(T('screen.tw_digit_themes', 'Digit themes'), true, themes);
    opt.appendChild(wrap);
}

// Up to SCREEN_TW_SLOTS contents shown in turn, each for its own duration.
function appendScreenTwSlots(opt, e, o, T) {
    const cur = twEditedSlot(e);
    const select = i => {
        window.screenEditor.twSlot[e.id] = i;
        renderScreenCanvas();
        renderScreenOptions();
    };

    const head = document.createElement('div');
    head.className = 'tw-slots-head';
    const title = document.createElement('span');
    title.className = 'tw-slots-label';
    title.textContent = T('screen.tw_contents', 'Content');
    head.appendChild(title);
    const chips = document.createElement('div');
    chips.className = 'tw-slot-chips';
    o.slots.forEach((s, i) => {
        const b = document.createElement('button');
        b.type = 'button';
        b.className = 'tw-slot-chip' + (i === cur ? ' selected' : '');
        b.textContent = String(i + 1);
        b.title = (s.text || '').slice(0, 40);
        b.addEventListener('click', () => select(i));
        chips.appendChild(b);
    });
    if (o.slots.length < SCREEN_TW_SLOTS) {
        const add = document.createElement('button');
        add.type = 'button';
        add.className = 'tw-slot-chip tw-slot-add';
        add.textContent = '+';
        add.title = T('screen.tw_add_content', 'Add alternating content');
        add.addEventListener('click', () => {
            o.slots.push({ text: '', duration: SCREEN_TW_DURATION_DEFAULT });
            select(o.slots.length - 1);
        });
        chips.appendChild(add);
    }
    if (o.slots.length > 1) {
        const del = document.createElement('button');
        del.type = 'button';
        del.className = 'tw-slot-chip tw-slot-del';
        del.textContent = '\u00d7';
        del.title = T('screen.tw_remove_content', 'Remove this content');
        del.addEventListener('click', () => {
            o.slots.splice(cur, 1);
            select(Math.max(0, cur - 1));
        });
        chips.appendChild(del);
    }
    head.appendChild(chips);
    opt.appendChild(head);

    const slot = o.slots[cur];
    const textArea = document.createElement('textarea');
    textArea.id = 'editor-textarea-' + e.id;
    textArea.maxLength = SCREEN_TW_TEXT_LEN - 1;
    textArea.rows = 2;
    textArea.value = slot.text || '';
    const counter = document.createElement('div');
    counter.id = textArea.id + '-counter';
    counter.className = 'counter';
    counter.setAttribute('aria-live', 'polite');
    textArea.setAttribute('aria-describedby', counter.id);
    textArea.addEventListener('input', () => {
        slot.text = textArea.value;
        renderScreenCanvas();
        updateCharCounter(textArea, counter);
    });
    const textRow = document.createElement('div');
    textRow.className = 'form-group';
    const textLabel = document.createElement('label');
    textLabel.textContent = T('screen.message_text', 'Text');
    textRow.appendChild(textLabel);
    textRow.appendChild(textArea);
    textRow.appendChild(counter);
    opt.appendChild(textRow);
    updateCharCounter(textArea, counter);
    setupTokenHighlightTextarea(textArea);

    if (o.slots.length > 1) {
        const dur = document.createElement('input');
        dur.type = 'number';
        dur.min = '1';
        dur.max = String(SCREEN_TW_DURATION_MAX);
        dur.value = String(slot.duration);
        dur.addEventListener('change', () => {
            slot.duration = clamp(parseInt(dur.value, 10) || SCREEN_TW_DURATION_DEFAULT, 1, SCREEN_TW_DURATION_MAX);
            dur.value = String(slot.duration);
        });
        const row = document.createElement('div');
        row.className = 'form-group';
        const lab = document.createElement('label');
        lab.textContent = T('screen.tw_duration', 'Show for (seconds)');
        row.appendChild(lab);
        row.appendChild(dur);
        opt.appendChild(row);
        const note = document.createElement('p');
        note.className = 'screen-options-mode-note';
        note.textContent = T('screen.tw_rotation_note', 'Contents are shown in turn; empty ones are skipped.');
        opt.appendChild(note);
    }
    appendScreenTokenButtons(opt, textArea, ['[colon]']);
}

function screenWriteTextWidgets(view, off, layout) {
    view.setUint32(off, SCREEN_BIN_EXT_MAGIC, true);
    view.setUint8(off + 4, SCREEN_BIN_EXT_VERSION);
    view.setUint8(off + 5, SCREEN_TW_IDS.length);
    view.setUint16(off + 6, SCREEN_BIN_TW_SIZE, true);
    off += SCREEN_BIN_EXT_HEADER_SIZE;
    ['day', 'night'].forEach(mode => {
        const profile = layout.profiles[mode];
        SCREEN_TW_IDS.forEach(id => {
            const e = screenFindProfileElement(profile, id) || twDefaultElement(id);
            const o = ensureScreenTwOptions(e);
            const fg = screenHexToRgb(o.color, { r: 255, g: 255, b: 255 });
            const bg = screenHexToRgb(o.bg_color, { r: 0, g: 0, b: 0 });
            const flags = (o.show_units ? SCREEN_TW_FLAG.SHOW_UNITS : 0) |
                (o.tabular ? SCREEN_TW_FLAG.TABULAR : 0) |
                (o.transparent ? SCREEN_TW_FLAG.TRANSPARENT_BG : 0) |
                (o.color_enabled ? SCREEN_TW_FLAG.COLOR_OVERRIDE : 0) |
                (o.scroll ? SCREEN_TW_FLAG.SCROLL : 0);
            [e.enabled ? 1 : 0, e.x || 0, e.y || 0, e.z || 0, o.width || 0, o.align || 0,
                o.family, o.size, flags, o.slots.length, fg.r, fg.g, fg.b, bg.r, bg.g, bg.b]
                .forEach((v, i) => view.setUint8(off + i, v & 0xff));
            for (let s = 0; s < SCREEN_TW_SLOTS; s++) {
                const slot = o.slots[s];
                view.setUint16(off + 16 + s * 2, slot ? slot.duration : SCREEN_TW_DURATION_DEFAULT, true);
                screenWriteFixedString(view, off + 24 + s * SCREEN_TW_TEXT_LEN, SCREEN_TW_TEXT_LEN, slot ? slot.text : '');
            }
            off += SCREEN_BIN_TW_SIZE;
        });
    });
    screenWriteWidgetMeta(view, off, layout);
}

// Largest prefix of `text` that fits `maxBytes` of UTF-8 without cutting a character.
function screenClampUtf8(text, maxBytes) {
    let out = '';
    let used = 0;
    for (const ch of String(text == null ? '' : text)) {
        const n = new TextEncoder().encode(ch).length;
        if (used + n > maxBytes) break;
        out += ch;
        used += n;
    }
    return out;
}

function screenWriteWidgetMeta(view, off, layout) {
    ['day', 'night'].forEach((mode, p) => {
        const profile = layout.profiles[mode];
        let mask = 0;
        SCREEN_INSTANCE_IDS.forEach((id, i) => {
            const e = screenFindProfileElement(profile, id);
            if (e && (e.instanced || e.enabled)) mask |= 1 << i;
            screenWriteFixedString(view, off + 4 + (p * SCREEN_INSTANCE_IDS.length + i) * SCREEN_BIN_NAME_LEN,
                SCREEN_BIN_NAME_LEN, e ? screenClampUtf8(e.name, SCREEN_BIN_NAME_LEN - 1) : '');
        });
        view.setUint16(off + p * 2, mask, true);
    });
}

function screenReadWidgetMeta(view, off, layout) {
    ['day', 'night'].forEach((mode, p) => {
        const profile = layout.profiles[mode];
        const mask = view.getUint16(off + p * 2, true);
        SCREEN_INSTANCE_IDS.forEach((id, i) => {
            const e = screenFindProfileElement(profile, id);
            if (!e) return;
            e.instanced = ((mask >> i) & 1) || e.enabled ? 1 : 0;
            e.name = screenReadFixedString(view, off + 4 + (p * SCREEN_INSTANCE_IDS.length + i) * SCREEN_BIN_NAME_LEN,
                SCREEN_BIN_NAME_LEN);
        });
    });
}

function screenReadTextWidgets(view, off, layout) {
    const extVersion = view.getUint8(off + 4);
    if (view.getUint32(off, true) !== SCREEN_BIN_EXT_MAGIC ||
        (extVersion !== SCREEN_BIN_EXT_VERSION && extVersion !== SCREEN_BIN_EXT_VERSION_V3 &&
            extVersion !== SCREEN_BIN_EXT_VERSION_V2) ||
        view.getUint8(off + 5) !== SCREEN_TW_IDS.length ||
        view.getUint16(off + 6, true) !== SCREEN_BIN_TW_SIZE) return;
    off += SCREEN_BIN_EXT_HEADER_SIZE;
    ['day', 'night'].forEach(mode => {
        const profile = layout.profiles[mode];
        SCREEN_TW_IDS.forEach(id => {
            const u = i => view.getUint8(off + i);
            const flags = u(8);
            const count = clamp(u(9), 1, SCREEN_TW_SLOTS);
            const slots = [];
            for (let s = 0; s < count; s++) {
                slots.push({
                    text: screenReadFixedString(view, off + 24 + s * SCREEN_TW_TEXT_LEN, SCREEN_TW_TEXT_LEN),
                    duration: view.getUint16(off + 16 + s * 2, true)
                });
            }
            profile.elements = profile.elements.filter(x => x.id !== id);
            const color = screenRgbToHex(u(10), u(11), u(12));
            profile.elements.push({
                id, enabled: u(0), x: u(1), y: u(2), z: u(3),
                options: {
                    width: u(4), align: u(5), family: u(6), size: u(7),
                    show_units: !!(flags & SCREEN_TW_FLAG.SHOW_UNITS),
                    tabular: !!(flags & SCREEN_TW_FLAG.TABULAR),
                    transparent: !!(flags & SCREEN_TW_FLAG.TRANSPARENT_BG),
                    scroll: !!(flags & SCREEN_TW_FLAG.SCROLL),
                    color_enabled: extVersion >= SCREEN_BIN_EXT_VERSION ? !!(flags & SCREEN_TW_FLAG.COLOR_OVERRIDE) : color !== '#ffffff',
                    color,
                    bg_color: screenRgbToHex(u(13), u(14), u(15)),
                    slots
                }
            });
            off += SCREEN_BIN_TW_SIZE;
        });
    });
    if (extVersion >= SCREEN_BIN_EXT_VERSION_V3) screenReadWidgetMeta(view, off, layout);
}

function makeDefaultScreenProfile() {
    const elements = [
        { id: 'glucose_level', enabled: 1, x: 27, y: 27, z: 4 },
        { id: 'glucose_trend', enabled: 1, x: 42, y: 30, z: 4 },
        { id: 'wifi_off', enabled: 1, x: 22, y: 27, z: 4 },
        { id: 'weather', enabled: 1, x: 54, y: 24, z: 3 },
        { id: 'moon', enabled: 1, x: 87, y: 29, z: 3 },
    ];
    SCREEN_ICON_SLOT_IDS.forEach(id => {
        // Icons carry no text styling; only enabled/x/y/z are used. Source is
        // S:/icon<N>.jpg, uploaded by the user.
        elements.push({ id, enabled: 0, x: 48, y: 48, z: 2 });
    });
    SCREEN_TW_IDS.forEach(id => elements.push(twDefaultElement(id)));
    // Factory clock (same as text_widgets_defaults() in the firmware): profile digit theme
    // and the AM/PM mark that stays empty in 24 h mode.
    const clock = elements.find(e => e.id === 'tw_1');
    Object.assign(clock, { enabled: 1, x: 22, y: 47, z: 1, name: 'Clock' });
    Object.assign(clock.options, {
        family: SCREEN_TW_FAMILY.SPRITE_THEME, size: 36, show_units: false, tabular: false,
        width: 0, align: 1,
        slots: [{ text: '[clock]', duration: SCREEN_TW_DURATION_DEFAULT }]
    });
    const ampm = elements.find(e => e.id === 'tw_2');
    Object.assign(ampm, { enabled: 1, x: 101, y: 54, z: 2, name: 'AMPM' });
    Object.assign(ampm.options, {
        family: SCREEN_TW_FAMILY.SANS, size: 12, show_units: true, tabular: false, transparent: true,
        width: 25,
        slots: [{ text: '[ampm12]', duration: SCREEN_TW_DURATION_DEFAULT }]
    });
    const scroll = elements.find(e => e.id === 'tw_3');
    Object.assign(scroll, { enabled: 1, x: 0, y: 86, z: 0, name: 'Scroll' });
    Object.assign(scroll.options, { scroll: true, family: SCREEN_TW_FAMILY.SANS, size: 16, width: SCREEN_SIZE, align: 0 });
    return {
        scroll_text: SCREEN_DEFAULT_SCROLL_TEXT,
        weather_theme: 0,
        elements
    };
}

/* Layouts saved before the unified text widget (version <= 10) hold digit displays, digit
   labels, the AM/PM sprite and static text lines. They are converted when loaded, with the same
   rules as screen_layout_migrate_v10() in the firmware. */
const SCREEN_LEGACY_ELEMENT_IDS = ['time', 'time_aux', 'ampm', 'digit_label', 'digit_label_aux',
    'text_1', 'text_2', 'text_3', 'text_4', 'text_5', 'text_6', 'text_7', 'text_8'];
const SCREEN_LEGACY_FONT_PX = [8, 10, 12, 14, 16, 8];
const SCREEN_LEGACY_DIGIT_MIN_SIZE = 9;

function screenLegacyTextOptions(src, family, size, text) {
    const o = twDefaultOptions();
    const so = (src && src.options) || {};
    o.family = family;
    o.size = size;
    o.width = so.width || 0;
    o.align = so.align || 0;
    o.color = so.color || '#ffffff';
    o.bg_color = so.bg_color || '#000000';
    o.slots = [{ text: String(text || '').slice(0, SCREEN_TW_TEXT_LEN - 1), duration: SCREEN_TW_DURATION_DEFAULT }];
    return o;
}

function screenConvertLegacyProfile(profile) {
    if (!profile || !Array.isArray(profile.elements)) return;
    const find = id => profile.elements.find(e => e && e.id === id);
    const legacyMessage = find('message');
    if (!SCREEN_LEGACY_ELEMENT_IDS.some(id => find(id)) && !legacyMessage) {
        delete profile.static_texts;
        return;
    }
    const texts = profile.static_texts || {};
    const on = id => { const e = find(id); return !!(e && e.enabled !== 0); };
    const made = [];
    const make = (src, options) => made.push({ enabled: 1, x: src.x || 0, y: src.y || 0, z: src.z || 0, options });
    const digitSize = src => {
        const f = src.options && src.options.font;
        return (f >= SCREEN_LEGACY_DIGIT_MIN_SIZE && f < SCREEN_TW_SIZE_MAX) ? f : SCREEN_TW_SIZE_MAX;
    };
    const digits = (src, family) => {
        const o = screenLegacyTextOptions(src, family, digitSize(src), '[clock]');
        o.show_units = false;
        o.tabular = false;
        o.color = '#ffffff';
        o.bg_color = '#000000';
        o.width = 0;
        o.align = 0;
        return o;
    };
    const label = (src, text) => {
        const o = screenLegacyTextOptions(src, SCREEN_TW_FAMILY.SANS,
            SCREEN_LEGACY_FONT_PX[(src.options && src.options.font) <= 5 ? src.options.font : 0] || 8, text);
        o.tabular = false;
        return o;
    };

    if (on('time')) make(find('time'), digits(find('time'), SCREEN_TW_FAMILY.SPRITE_THEME));
    if (on('time_aux')) make(find('time_aux'), digits(find('time_aux'), SCREEN_TW_FAMILY.SPRITE_AUX));
    if (on('time') && find('ampm') && find('ampm').enabled !== 0) {
        const a = find('ampm');
        const o = twDefaultOptions();
        o.size = 12; o.tabular = false; o.transparent = true;
        o.slots = [{ text: '[ampm12]', duration: SCREEN_TW_DURATION_DEFAULT }];
        make(a, o);
    }
    if (on('digit_label')) make(find('digit_label'), label(find('digit_label'), texts.digit_label));
    if (on('digit_label_aux')) make(find('digit_label_aux'), label(find('digit_label_aux'), texts.digit_label_aux));
    for (let i = 1; i <= 8; i++) {
        if (on(`text_${i}`)) make(find(`text_${i}`), label(find(`text_${i}`), texts[`text_${i}`]));
    }

    profile.elements = profile.elements.filter(e => e && !SCREEN_LEGACY_ELEMENT_IDS.includes(e.id));
    // Widgets written by the earlier text-widget editor keep their slot; converted ones fill the rest.
    SCREEN_TW_IDS.forEach(id => {
        let e = find(id);
        if (!e) { e = twDefaultElement(id); profile.elements.push(e); }
        if (e.enabled || !made.length) return;
        const m = made.shift();
        e.enabled = 1; e.x = m.x; e.y = m.y; e.z = m.z; e.options = m.options;
    });
    if (legacyMessage) {
        let target = SCREEN_TW_IDS.map(id => find(id)).find(e => e && !e.enabled && !e.instanced) || null;
        if (!target) {
            const clock = SCREEN_TW_IDS.map(id => find(id)).find(e => e &&
                (e.options.slots || []).some(s => String(s.text || '').includes('[clock]')));
            const ampm = SCREEN_TW_IDS.map(id => find(id)).find(e => e &&
                (e.options.slots || []).some(s => String(s.text || '').includes('[ampm12]')));
            if (clock && ampm && clock !== ampm) {
                const slot = clock.options.slots.find(s => String(s.text || '').includes('[clock]'));
                if (slot && slot.text.length + '[ampm12]'.length < SCREEN_TW_TEXT_LEN) {
                    slot.text += '[ampm12]';
                    ampm.enabled = 0;
                    target = ampm;
                }
            }
        }
        if (target) {
            const old = legacyMessage.options || {};
            const px = SCREEN_LEGACY_FONT_PX[old.font <= 5 ? old.font : 0] || 8;
            target.enabled = legacyMessage.enabled === 0 ? 0 : 1;
            target.x = legacyMessage.x || 0;
            target.y = legacyMessage.y || 0;
            target.z = legacyMessage.z || 0;
            target.name = 'Scroll';
            target.options = Object.assign(twDefaultOptions(), {
                scroll: true,
                family: SCREEN_TW_FAMILY.SANS,
                size: Math.min(20, px),
                width: old.width || 0,
                align: old.align || 0,
                color: old.color || '#ffffff',
                color_enabled: true,
                bg_color: old.bg_color || '#000000',
                slots: [{ text: '', duration: SCREEN_TW_DURATION_DEFAULT }]
            });
        } else {
            console.warn('Legacy scrolling message has no available text widget slot');
        }
    }
    profile.elements = profile.elements.filter(e => e && e.id !== 'message');
    delete profile.static_texts;
}

const SCREEN_LAYOUT_VERSION = 12;

/* Compact binary wire format for /api/screen (must match f-screen-layout-bin.h). */
const SCREEN_BIN_MAGIC = 0x4653584C;
const SCREEN_BIN_FORMAT = 1;
const SCREEN_BIN_FONT_LEN = 12;
const SCREEN_BIN_WIDGET_SIZE = 13;
const SCREEN_BIN_WIDGET_COUNT = 15; // glucose..graph (7) + 8 icon slots
const SCREEN_BIN_SCROLL_LEN = 512;
const SCREEN_BIN_GRAPH_SIZE = 88; // screen_graph_cfg_t (token[64] + 24)
const SCREEN_BIN_GRAPH_TOKEN_LEN = 64;
const SCREEN_BIN_HEADER_SIZE = 64;
const SCREEN_BIN_PROFILE_SIZE =
    SCREEN_BIN_WIDGET_COUNT * SCREEN_BIN_WIDGET_SIZE +
    SCREEN_BIN_SCROLL_LEN +
    SCREEN_BIN_GRAPH_SIZE;
const SCREEN_BIN_WIRE_SIZE = SCREEN_BIN_HEADER_SIZE + SCREEN_BIN_PROFILE_SIZE * 2;
const SCREEN_BIN_MIME = 'application/vnd.frixos.screen-layout+1';

/* Text-widget section appended after the base wire (screen_layout_ext_wire_t). Version 3 adds the
   widget meta: per profile a bitmask of existing slots and a 16-byte name for each slot. */
const SCREEN_BIN_EXT_MAGIC = 0x54585346;
const SCREEN_BIN_EXT_VERSION = 4;
const SCREEN_BIN_EXT_VERSION_V3 = 3;
const SCREEN_BIN_EXT_VERSION_V2 = 2;
const SCREEN_BIN_EXT_HEADER_SIZE = 8;
const SCREEN_BIN_TW_SIZE = 216; // text_widget_cfg_t
const SCREEN_BIN_NAME_LEN = 16;
const SCREEN_INSTANCE_IDS = SCREEN_TW_IDS.concat(SCREEN_ICON_SLOT_IDS); // widget_meta_t slot order
const SCREEN_BIN_META_SIZE = 2 * 2 + 2 * SCREEN_INSTANCE_IDS.length * SCREEN_BIN_NAME_LEN;
const SCREEN_BIN_EXT_V2_SIZE = SCREEN_BIN_EXT_HEADER_SIZE + SCREEN_BIN_TW_SIZE * SCREEN_TW_IDS.length * 2;
const SCREEN_BIN_EXT_SIZE = SCREEN_BIN_EXT_V2_SIZE + SCREEN_BIN_META_SIZE;
const SCREEN_BIN_FULL_WIRE_SIZE_V2 = SCREEN_BIN_WIRE_SIZE + SCREEN_BIN_EXT_V2_SIZE;
const SCREEN_BIN_FULL_WIRE_SIZE = SCREEN_BIN_WIRE_SIZE + SCREEN_BIN_EXT_SIZE;

const GRAPH_VAL_UNSET = -32768; // int16 sentinel for "not set"
const GRAPH_FLAG = { AUTOSCALE: 0x01, SHOW_AXIS: 0x02, BAND: 0x04, BACKFILL: 0x08, SHOW_VALUE: 0x10, BOOLEAN: 0x20, THICK: 0x40, SHOW_XAXIS: 0x80 };

/* Firmware widget[] index order (screen_element_id_t). */
const SCREEN_WIRE_ELEM_IDS = [
    'glucose_level', 'glucose_trend', 'wifi_off', 'weather', 'moon', 'message', 'graph',
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
    return profile.elements.find(e => e && e.id === id);
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
        show_xaxis: !!(flags & GRAPH_FLAG.SHOW_XAXIS),
        band_on: !!(flags & GRAPH_FLAG.BAND),
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
    if (o.show_xaxis) flags |= GRAPH_FLAG.SHOW_XAXIS;
    if (o.band_on) flags |= GRAPH_FLAG.BAND;
    flags |= GRAPH_FLAG.BACKFILL; // always on: keeps older firmware backfilling
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
        scroll_text: ''
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

    // Per-profile graph config -> merge into the graph element's options.
    const gcfg = screenReadGraphCfg(view, off);
    off += SCREEN_BIN_GRAPH_SIZE;
    const gelem = profile.elements.find(e => e && e.id === 'graph');
    if (gelem) gelem.options = Object.assign({}, gelem.options || {}, gcfg);

    return { profile, offset: off };
}

function decodeScreenLayoutBinary(bytes) {
    if (!bytes || (bytes.byteLength !== SCREEN_BIN_WIRE_SIZE &&
        bytes.byteLength !== SCREEN_BIN_FULL_WIRE_SIZE_V2 && bytes.byteLength !== SCREEN_BIN_FULL_WIRE_SIZE)) {
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
    layout.profiles.day.weather_theme = clamp(view.getUint8(9), 0, SCREEN_TIME_FONTS.length);
    layout.profiles.night.weather_theme = clamp(view.getUint8(10), 0, SCREEN_TIME_FONTS.length);
    // Older firmware sends no ext section: ensureScreenProfileShape adds default widgets.
    if (bytes.byteLength !== SCREEN_BIN_WIRE_SIZE) {
        screenReadTextWidgets(view, SCREEN_BIN_WIRE_SIZE, layout);
    }
    return layout;
}

function encodeScreenLayoutBinary(layout) {
    ensureScreenLayoutMeta(layout);
    ensureScreenProfileShape(layout.profiles.day);
    ensureScreenProfileShape(layout.profiles.night);

    const buf = new ArrayBuffer(SCREEN_BIN_FULL_WIRE_SIZE);
    const view = new DataView(buf);
    const bytes = new Uint8Array(buf);
    bytes.fill(0);

    view.setUint32(0, SCREEN_BIN_MAGIC, true);
    view.setUint8(4, SCREEN_BIN_FORMAT);
    view.setUint8(5, SCREEN_LAYOUT_VERSION);
    view.setUint8(6, layout.scroll_delay != null ? layout.scroll_delay : 60);
    view.setUint8(7, layout.day_color_filter != null ? layout.day_color_filter : 0);
    view.setUint8(8, layout.night_color_filter != null ? layout.night_color_filter : 0);
    screenWriteFixedString(view, 12, SCREEN_BIN_FONT_LEN, layout.day_font || 'bold');
    screenWriteFixedString(view, 24, SCREEN_BIN_FONT_LEN, layout.night_font || 'light');
    screenWriteFixedString(view, 36, SCREEN_BIN_FONT_LEN, layout.day_aux_font || layout.day_font || 'bold');
    screenWriteFixedString(view, 48, SCREEN_BIN_FONT_LEN, layout.night_aux_font || layout.night_font || 'light');
    view.setUint8(9, clamp(parseInt(layout.profiles.day.weather_theme, 10) || 0, 0, SCREEN_TIME_FONTS.length));
    view.setUint8(10, clamp(parseInt(layout.profiles.night.weather_theme, 10) || 0, 0, SCREEN_TIME_FONTS.length));
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
        const gelem = screenFindProfileElement(profile, 'graph');
        screenWriteGraphCfg(view, off, gelem && gelem.options ? gelem.options : {});
        off += SCREEN_BIN_GRAPH_SIZE;
    });

    screenWriteTextWidgets(view, off, layout);
    return bytes;
}

const SCREEN_DEFAULT_LAYOUT = {
    version: SCREEN_LAYOUT_VERSION,
    scroll_delay: 60,
    day_font: 'bold',
    night_font: 'light',
    day_aux_font: 'bold',
    night_aux_font: 'light',
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
    dragRafHandle: null,
    dragging: null, // { id, originX, originY, startLeft, startTop, fromPalette, members }
    selectedId: null,   // primary/anchor selection (last clicked) — drives the options panel
    selectedIds: [],    // full selection set; >1 means a multi-selection (no per-element options)
    cursorX: null,
    cursorY: null
};

/* ---------- selection helpers (single + multi) ---------- */
function getSelectedIds() {
    const ids = window.screenEditor.selectedIds;
    return Array.isArray(ids) ? ids : [];
}
function isElementSelected(id) {
    return getSelectedIds().indexOf(id) !== -1;
}
function isMultiSelect() {
    return getSelectedIds().length > 1;
}
// Sets the whole selection. `selectedId` mirrors the anchor (last id) so all the
// existing single-selection code (options, status, palette highlight) keeps working.
function setScreenSelection(ids) {
    const arr = (Array.isArray(ids) ? ids : (ids ? [ids] : [])).filter(Boolean);
    window.screenEditor.selectedIds = arr;
    window.screenEditor.selectedId = arr.length ? arr[arr.length - 1] : null;
}
// Ctrl+Click toggles one element in/out of the selection.
function toggleScreenSelection(id) {
    const arr = getSelectedIds().slice();
    const i = arr.indexOf(id);
    if (i === -1) arr.push(id); else arr.splice(i, 1);
    setScreenSelection(arr);
}

function getScreenScale() {
    if (_screenScaleCache !== null) return _screenScaleCache;
    const rect = getScreenRect();
    if (!rect) return 4;
    _screenScaleCache = Math.max(1, Math.round(rect.width / SCREEN_SIZE));
    return _screenScaleCache;
}

function getProfileObj(mode) {
    return window.screenLayout && window.screenLayout.profiles ? window.screenLayout.profiles[mode] : null;
}

function findElementDef(id) {
    if (id === 'screen_settings') return SCREEN_PALETTE_SETTINGS_DEF;
    return SCREEN_ELEMENT_DEFS_MAP.get(id) || null;
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
        return { w: 128, h: getScreenLabelBoxHeight(font) };
    }
    if (isScreenIconElement(def.id)) {
        const d = getIconDims(def.id); // null until the JPG (if any) has loaded
        return { w: evenPx(d ? d.w : def.w), h: evenPx(d ? d.h : def.h) };
    }
    if (isScreenTextWidgetElement(def.id)) {
        if (!elementState) return { w: evenPx(def.w), h: evenPx(def.h) };
        const o = ensureScreenTwOptions(elementState);
        const slot = o.slots[twEditedSlot(elementState)];
        const text = slot && slot.text ? twSampleText(o, twEditedSlot(elementState)) : '';
        const widthText = text || ' ';
        return { w: o.width > 0 ? o.width : twContentWidth(o, widthText), h: twWidgetHeight(o, text) };
    }
    if (def.id === 'graph') {
        // Use the configured graph size so the editor preview tracks Width/Height.
        const o = elementState && elementState.options ? elementState.options : {};
        return { w: evenPx(o.gwidth || def.w), h: evenPx(o.gheight || def.h) };
    }
    w = def.w;
    h = def.h;
    return { w: evenPx(w), h: evenPx(h) };
}

function getScreenTimeFont(mode) {
    if (!window.screenLayout) return 'bold';
    return mode === 'night'
        ? (window.screenLayout.night_font || 'light')
        : (window.screenLayout.day_font || 'bold');
}

function getScreenWeatherFont(mode) {
    const profile = window.screenLayout && window.screenLayout.profiles && window.screenLayout.profiles[mode];
    const override = clamp(parseInt(profile && profile.weather_theme, 10) || 0, 0, SCREEN_TIME_FONTS.length);
    return override ? SCREEN_TIME_FONTS[override - 1] : getScreenTimeFont(mode);
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
    if (!layout.night_font) layout.night_font = (window.settings && window.settings.p05) || 'light';
    if (!layout.day_aux_font) layout.day_aux_font = layout.day_font || 'bold';
    if (!layout.night_aux_font) layout.night_aux_font = layout.night_font || 'light';
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
    profile.weather_theme = clamp(parseInt(profile.weather_theme, 10) || 0, 0, SCREEN_TIME_FONTS.length);

    SCREEN_ELEMENT_DEFS.forEach(def => {
        const element = ensureElementInProfile(profile, def.id);
        ensureElementLayer(element);
        if (isScreenTextElement(def.id)) ensureScreenTextOptions(element);
        if (isScreenTextWidgetElement(def.id)) ensureScreenTwOptions(element);
    });
    dedupeScreenProfileElements(profile);
    let scrollFound = false;
    SCREEN_TW_IDS.forEach(id => {
        const e = screenFindProfileElement(profile, id);
        if (!e) return;
        const o = ensureScreenTwOptions(e);
        if (!o.scroll) return;
        if (scrollFound) {
            o.scroll = false;
            return;
        }
        scrollFound = true;
    });
}

function cloneScreenDefaults() {
    return JSON.parse(JSON.stringify(SCREEN_DEFAULT_LAYOUT));
}

function prepareScreenLayout(layout) {
    if (!layout || !layout.profiles) return layout;
    if (!layout.scroll_delay) {
        layout.scroll_delay = (window.settings && window.settings.p14) || 60;
    }
    ['day', 'night'].forEach(mode => {
        const profile = layout.profiles[mode];
        if (!profile || !Array.isArray(profile.elements)) return;
        screenConvertLegacyProfile(profile);
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
        if (isScreenIconElement(id)) {
            e = { id, enabled: 0, x: 48, y: 48, z: 2 };
        } else if (isScreenTextWidgetElement(id)) {
            e = twDefaultElement(id);
        } else {
            e = { id, enabled: 1, x: 0, y: 0, z: 0 };
        }
        profile.elements.push(e);
    }
    if (e.enabled === undefined) {
        e.enabled = (isScreenIconElement(id) || isScreenTextWidgetElement(id)) ? 0 : 1;
    }
    if (e.x === undefined) e.x = 0;
    if (e.y === undefined) e.y = 0;
    ensureElementLayer(e);
    if (isScreenIconElement(id) || isScreenTextWidgetElement(id)) {
        // Layouts without instance data: every enabled widget exists.
        if (e.instanced === undefined) e.instanced = e.enabled ? 1 : 0;
        else if (e.enabled) e.instanced = 1;
        if (typeof e.name !== 'string') e.name = '';
        if (isScreenTextWidgetElement(id) && e.instanced && !e.name.trim()) {
            const o = ensureScreenTwOptions(e);
            const template = String((o.slots[0] && o.slots[0].text) || '').trim();
            if (/^\[clock\]$/i.test(template)) {
                o.width = 0;
                o.align = 1;
            }
            e.name = screenDefaultTextWidgetName(e);
        }
    }
    return e;
}

function screenFreshInstanceElement(id) {
    const e = isScreenIconElement(id) ? { id, enabled: 0, x: 48, y: 48, z: 2 } : twDefaultElement(id);
    e.instanced = 0;
    e.name = '';
    return e;
}

// Adds a text or icon widget in the first free slot, centred on the canvas and selected.
function addScreenWidgetInstance(kind) {
    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return false;
    const isIcon = kind === 'icon';
    const id = isIcon ? getNextIconSlot(profile) : getNextTextSlot(profile);
    if (!id) {
        showStatus(isIcon
            ? getScreenTranslation('screen.no_free_icon', 'All 8 icon slots are in use')
            : getScreenTranslation('screen.no_free_text', 'All 8 text slots are in use'), 'warning');
        return false;
    }
    ensureElementInProfile(profile, id);
    const e = screenFreshInstanceElement(id);
    profile.elements[profile.elements.findIndex(x => x.id === id)] = e;
    e.instanced = 1;
    e.enabled = 1;
    const base = isIcon ? getScreenTranslation('screen.default_icon_name', 'Icon')
        : getScreenTranslation('screen.default_text_name', 'Text');
    e.name = screenClampUtf8(`${base} ${id.split('_')[1]}`, SCREEN_BIN_NAME_LEN - 1);

    const def = findElementDef(id);
    let size;
    if (isIcon) {
        size = getScreenElementSize(def, e);
    } else {
        const o = ensureScreenTwOptions(e);
        const slot = o.slots[twEditedSlot(e)];
        const text = slot && slot.text ? twSampleText(o, twEditedSlot(e)) : getScreenElementLabel(def.id);
        size = { w: def.w, h: twWidgetHeight(o, text) };
    }
    e.x = clamp(Math.round((SCREEN_SIZE - size.w) / 2), 0, SCREEN_SIZE - 1);
    e.y = visualYToLayoutY(clamp(Math.round((SCREEN_SIZE - size.h) / 2), 0, SCREEN_SIZE - 1), id);

    setScreenSelection([id]);
    renderScreenCanvas();
    renderScreenOptions();
    renderScreenPalette();
    updateScreenStatusLine();
    return true;
}

function removeScreenWidgetInstance(id) {
    const profile = getProfileObj(window.screenEditor.mode);
    const idx = profile && profile.elements ? profile.elements.findIndex(x => x.id === id) : -1;
    if (idx < 0) return;
    profile.elements[idx] = screenFreshInstanceElement(id);
    if (window.screenEditor.twSlot) delete window.screenEditor.twSlot[id];
    setScreenSelection([]);
    renderScreenCanvas();
    renderScreenOptions();
    renderScreenPalette();
    updateScreenStatusLine();
}

function updateScreenPaletteMenu() {
    const profile = getProfileObj(window.screenEditor.mode);
    const textBtn = el('screenAddTextBtn');
    const iconBtn = el('screenAddIconBtn');
    if (textBtn) textBtn.disabled = !getNextTextSlot(profile);
    if (iconBtn) iconBtn.disabled = !getNextIconSlot(profile);
}

function setupScreenPaletteMenu() {
    const btn = el('screenPaletteAddBtn');
    const menu = el('screenPaletteAddMenu');
    if (!btn || !menu) return;
    const setOpen = open => {
        menu.hidden = !open;
        btn.setAttribute('aria-expanded', open ? 'true' : 'false');
        if (open) updateScreenPaletteMenu();
    };
    btn.addEventListener('click', ev => {
        if (ev && ev.stopPropagation) ev.stopPropagation();
        setOpen(menu.hidden);
    });
    [['screenAddTextBtn', 'text'], ['screenAddIconBtn', 'icon']].forEach(([id, kind]) => {
        const item = el(id);
        if (item) item.addEventListener('click', () => { setOpen(false); addScreenWidgetInstance(kind); });
    });
    document.addEventListener('pointerdown', ev => {
        const inside = ev.target && ev.target.closest && ev.target.closest('.palette-add');
        if (!menu.hidden && !inside) setOpen(false);
    });
    document.addEventListener('keydown', ev => {
        if (ev.key === 'Escape' && !menu.hidden) { setOpen(false); btn.focus(); }
    });
}

function appendScreenInstanceNameRow(container, e, trans) {
    const row = document.createElement('div');
    row.className = 'form-group';
    const label = document.createElement('label');
    label.textContent = getNestedTranslation(trans, 'screen.widget_name') || 'Name';
    const input = document.createElement('input');
    input.type = 'text';
    input.id = 'screen_widget_name';
    input.maxLength = SCREEN_BIN_NAME_LEN - 1;
    input.value = e.name || '';
    input.setAttribute('autocomplete', 'off');
    input.addEventListener('input', () => {
        const clamped = screenClampUtf8(input.value, SCREEN_BIN_NAME_LEN - 1);
        if (clamped !== input.value) input.value = clamped;
        e.name = clamped;
        renderScreenPalette();
        updateScreenStatusLine();
    });
    row.appendChild(label);
    row.appendChild(input);
    container.appendChild(row);
}

function appendScreenDeleteRow(container, e, trans) {
    const row = document.createElement('div');
    row.className = 'form-group screen-delete-widget-row';
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'btn screen-delete-widget-btn';
    btn.id = 'screen_widget_delete';
    btn.setAttribute('aria-label', getNestedTranslation(trans, 'screen.delete_widget') || 'Delete widget');
    const icon = document.createElement('span');
    icon.className = 'screen-delete-widget-icon';
    icon.setAttribute('aria-hidden', 'true');
    const label = document.createElement('span');
    label.textContent = getNestedTranslation(trans, 'screen.delete_widget') || 'Delete widget';
    btn.appendChild(icon);
    btn.appendChild(label);
    btn.addEventListener('click', () => removeScreenWidgetInstance(e.id));
    row.appendChild(btn);
    container.appendChild(row);
}

function screenCanTrashDraggedWidget(drag) {
    return !!(drag && !drag.fromPalette && drag.elem && !isScreenMetaElement(drag.id) &&
        (!drag.members || drag.members.length === 1));
}

function screenDeletesDraggedInstance(drag) {
    return !!(screenCanTrashDraggedWidget(drag) && drag.elem.instanced &&
        (isScreenTextWidgetElement(drag.id) || isScreenIconElement(drag.id)));
}

function screenPointerOverTrash(clientX, clientY, rect) {
    const width = 80;
    const height = 60;
    const inset = 12;
    const left = rect.left + rect.width - inset - width;
    const top = rect.top + rect.height - inset - height;
    return clientX >= left && clientX <= left + width && clientY >= top && clientY <= top + height;
}

function appendScreenTrashTarget(canvas, drag) {
    if (!screenCanTrashDraggedWidget(drag)) return;
    const target = document.createElement('div');
    target.className = 'screen-canvas-trash-target' + (drag.deleteTarget ? ' active' : '');
    target.setAttribute('aria-hidden', 'true');
    const icon = document.createElement('span');
    icon.className = 'screen-canvas-trash-icon';
    const label = document.createElement('span');
    label.textContent = screenDeletesDraggedInstance(drag)
        ? getScreenTranslation('screen.delete_widget', 'Delete widget')
        : getScreenTranslation('screen.disable_widget', 'Disable widget');
    target.appendChild(icon);
    target.appendChild(label);
    canvas.appendChild(target);
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
    const ids = getSelectedIds().filter(id => id && !isScreenMetaElement(id));
    if (!ids.length) return false;

    const nudge = shiftKey ? 10 : 1;
    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return false;

    // Snapshot each member's start position in visual space.
    const members = ids.map(id => {
        const elem = ensureElementInProfile(profile, id);
        return { id, elem, x: elem.x || 0, visualY: layoutYToVisualY(elem.y || 0, id) };
    });

    let dx = 0, dy = 0;
    if (key === 'ArrowLeft') dx = -nudge;
    else if (key === 'ArrowRight') dx = nudge;
    else if (key === 'ArrowUp') dy = -nudge;
    else if (key === 'ArrowDown') dy = nudge;
    else return false;

    // Clamp the delta for the whole group so it moves rigidly and stops as a unit
    // when any member reaches an edge.
    const { dx: cdx, dy: cdy } = clampGroupDelta(members, dx, dy);
    members.forEach(m => {
        m.elem.x = m.x + cdx;
        m.elem.y = visualYToLayoutY(m.visualY + cdy, m.id);
    });

    renderScreenCanvas();
    renderScreenOptions();
    updateScreenStatusLine();
    return true;
}

// Clamp a (dx, dy) delta (layout units; y is visual-space) so that every member's
// top-left corner stays within [0, SCREEN_SIZE-1] — keeps a multi-selection rigid.
function clampGroupDelta(members, dx, dy) {
    let minX = Infinity, maxX = -Infinity, minVY = Infinity, maxVY = -Infinity;
    members.forEach(m => {
        if (m.x < minX) minX = m.x;
        if (m.x > maxX) maxX = m.x;
        if (m.visualY < minVY) minVY = m.visualY;
        if (m.visualY > maxVY) maxVY = m.visualY;
    });
    return {
        dx: clamp(dx, -minX, (SCREEN_SIZE - 1) - maxX),
        dy: clamp(dy, -minVY, (SCREEN_SIZE - 1) - maxVY)
    };
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
    const copyBtn = el('screenCopyBtn');
    const saveToFileBtn = el('screenSaveToFileBtn');
    const saveToDeviceBtn = el('screenSaveToDeviceBtn');
    const readFromFileBtn = el('screenReadFromFileBtn');
    const layoutFileInput = el('screenLayoutFileInput');

    if (btnDay) btnDay.addEventListener('click', () => setScreenMode('day'));
    if (btnNight) btnNight.addEventListener('click', () => setScreenMode('night'));
    if (saveBtn) saveBtn.addEventListener('click', saveScreenLayout);
    if (copyBtn) copyBtn.addEventListener('click', copyScreenLayoutToOtherMode);
    if (saveToFileBtn) saveToFileBtn.addEventListener('click', saveScreenLayoutToFile);
    if (saveToDeviceBtn) saveToDeviceBtn.addEventListener('click', openLayoutSaveModal);
    const layoutSaveModal = el('layoutSaveModal');
    const layoutSaveName = el('layoutSaveName');
    const layoutSaveCancel = el('layoutSaveCancel');
    const layoutSaveConfirm = el('layoutSaveConfirm');
    if (layoutSaveCancel) layoutSaveCancel.addEventListener('click', closeLayoutSaveModal);
    if (layoutSaveConfirm) layoutSaveConfirm.addEventListener('click', confirmLayoutSave);
    if (layoutSaveModal) layoutSaveModal.addEventListener('click', e => { if (e.target === layoutSaveModal) closeLayoutSaveModal(); });
    if (layoutSaveName) {
        layoutSaveName.addEventListener('keydown', e => {
            if (e.key === 'Enter') { e.preventDefault(); confirmLayoutSave(); }
        });
        layoutSaveName.addEventListener('input', () => {
            if (window.layoutSavePending) window.layoutSavePending.filename = '';
            const warn = el('layoutSaveWarn');
            const err = el('layoutSaveError');
            if (warn) warn.hidden = true;
            if (err) err.classList.remove('show');
            layoutSaveName.removeAttribute('aria-invalid');
        });
    }
    document.addEventListener('keydown', e => {
        if (e.key === 'Escape' && layoutSaveModal && layoutSaveModal.classList.contains('open')) closeLayoutSaveModal();
    });
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

        if (typeof ResizeObserver !== 'undefined') {
            const ro = new ResizeObserver(() => invalidateScreenCache());
            ro.observe(canvas);
        }
        window.addEventListener('scroll', invalidateScreenCache, { passive: true });
        window.addEventListener('resize', invalidateScreenCache, { passive: true });
    }

    document.addEventListener('keydown', onScreenArrowKeyDown);

    setupScreenPaletteMenu();
    startScreenTokenTicker();
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

function getPalettePreviewSize(def) {
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
    const fallback = (font === 'user1' || font === 'user2') ? 'themes/bold/preview.jpg' : null;
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
    if (isScreenTextWidgetElement(def.id)) return createTwPreview(def, inMatrix, elementState, matrixScale);
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
    text.textContent = def.text || def.label;
    if (isScreenTextElement(def.id)) {
        const font = elementState && elementState.options && elementState.options.font !== undefined ? elementState.options.font : 0;
        const pt = SCREEN_MESSAGE_FONT_SIZES[font] || 8;
        const previewScale = inMatrix ? (matrixScale || getScreenScale()) : 1;
        text.style.fontSize = `${pt * previewScale}px`;
        text.style.lineHeight = '1';
        if (inMatrix) {
            preview.style.alignItems = 'flex-start';
            preview.style.justifyContent = 'flex-start';
            text.style.textAlign = 'left';
        }
        if (elementState && elementState.options) {
            if (elementState.options.color) text.style.color = elementState.options.color;
            if (elementState.options.bg_color) {
                preview.style.backgroundColor = elementState.options.bg_color;
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
        item.tabIndex = 0;
        const instance = isScreenTextWidgetElement(def.id) || isScreenIconElement(def.id)
            ? screenFindProfileElement(profile, def.id) : null;
        if (instance && !instance.enabled) {
            item.classList.add('palette-item-off');
            item.setAttribute('aria-label', `${getScreenPaletteItemLabel(def, profile)} (${getScreenTranslation('screen.disabled', 'disabled')})`);
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

        const selectPaletteItem = () => {
            if (def.paletteMeta) {
                setScreenSelection([def.id]);
                renderScreenCanvas();
                renderScreenOptions();
                renderScreenPalette();
                updateScreenStatusLine();
                focusScreenCanvas();
                return;
            }
            const activeProfile = getProfileObj(window.screenEditor.mode);
            if (!activeProfile) return;
            const elObj = ensureElementInProfile(activeProfile, def.id);
            elObj.enabled = 1;
            setScreenSelection([def.id]);
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
                beginDrag(def.id, e, true);
            });
        }
        item.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                selectPaletteItem();
            }
        });

        list.appendChild(item);
    });
    updateScreenPaletteMenu();
}

async function fetchScreenLayout() {
    try {
        await refreshScreenLayoutFiles();
        const [settingsResp, response] = await Promise.all([
            fetchSettingsJson('/api/settings?params=p00,p08,p19,p24,p37,p50', {
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
        if (body.byteLength !== SCREEN_BIN_FULL_WIRE_SIZE) {
            throw new Error(`screen wire size mismatch: ${body.byteLength} != ${SCREEN_BIN_FULL_WIRE_SIZE}`);
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
    setScreenSelection([]);
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

// Built-in presets ship in the firmware image. A same-named save is allowed,
// but the next file reconcile puts the shipped copy back.
const SHIPPED_LAYOUT_NAMES = new Set([
    'layouts/default.layout',
    'layouts/diabetic.layout',
    'layouts/homeassistant.layout',
    'layouts/weather.layout',
    'layouts/hagraph.layout'
]);

// Layout saves may target the root or the shipped layouts directory.
function normalizeDeviceLayoutName(raw) {
    const trimmed = String(raw == null ? '' : raw).trim();
    if (!trimmed) return '';
    const file = /\.layout$/i.test(trimmed) ? trimmed : trimmed + '.layout';
    if (file.length > 110 || /^\.layout$/i.test(file)) return null;
    const parts = file.split('/');
    if (file.includes('..') || file.includes('\\') || parts.some(part => !part || part === '.') ||
        (parts.length > 1 && parts[0].toLowerCase() !== 'layouts') || /[\u0000-\u001f]/.test(file)) return null;
    return file;
}

function openLayoutSaveModal() {
    const layout = prepareScreenLayoutForExport();
    if (!layout) return;
    window.layoutSavePending = { layout, filename: '' };
    const input = el('layoutSaveName');
    const err = el('layoutSaveError');
    const warn = el('layoutSaveWarn');
    if (input) { input.value = ''; input.removeAttribute('aria-invalid'); }
    if (err) { err.textContent = ''; err.classList.remove('show'); }
    if (warn) { warn.hidden = true; warn.textContent = ''; }
    const modal = el('layoutSaveModal');
    if (modal) modal.classList.add('open');
    if (input) input.focus();
}

function closeLayoutSaveModal() {
    const modal = el('layoutSaveModal');
    if (modal) modal.classList.remove('open');
    window.layoutSavePending = null;
}

async function confirmLayoutSave() {
    const pending = window.layoutSavePending;
    if (!pending) return;
    const input = el('layoutSaveName');
    const err = el('layoutSaveError');
    const warn = el('layoutSaveWarn');
    const filename = normalizeDeviceLayoutName(input && input.value);
    if (!filename) {
        if (input) input.setAttribute('aria-invalid', 'true');
        if (err) { err.textContent = getMessage('layout_name_invalid'); err.classList.add('show'); }
        if (warn) warn.hidden = true;
        pending.filename = '';
        return;
    }
    if (pending.filename !== filename) {
        const confirmBtn = el('layoutSaveConfirm');
        toggleLoading(confirmBtn, true);
        try { await ensureScreenSpiffsFiles(true); }
        finally { toggleLoading(confirmBtn, false); }
        if (window.layoutSavePending !== pending) return;
        const exists = (window.screenSpiffsFileList || []).some(f => f.name === filename);
        if (exists) {
            pending.filename = filename;
            const shipped = SHIPPED_LAYOUT_NAMES.has(filename.toLowerCase());
            const key = shipped ? 'layout_overwrite_shipped' : 'layout_overwrite_confirm';
            if (warn) {
                warn.textContent = getMessage(key).replace('{name}', filename);
                warn.hidden = false;
            }
            if (err) err.classList.remove('show');
            if (input) input.removeAttribute('aria-invalid');
            return;
        }
    }
    const layout = pending.layout;
    closeLayoutSaveModal();
    await uploadScreenLayoutToDevice(layout, filename);
}

async function uploadScreenLayoutToDevice(layout, filename) {
    const btn = el('screenSaveToDeviceBtn');
    const body = JSON.stringify(layout, null, 2);
    const controller = new AbortController();
    try {
        toggleLoading(btn, true);
        showStatus(getMessage('saving_settings'), 'info');
        // The handler stores the file, then redraws the clock on the HTTP task
        // before that reply finishes. A matching file is the success signal.
        const post = fetch('/api/ota', {
            method: 'POST',
            headers: { 'X-Filename': filename, 'Content-Type': 'application/json' },
            body,
            signal: controller.signal
        }).then(async resp => {
            if (!resp.ok) return false;
            const data = await Promise.race([
                resp.json().catch(() => null),
                new Promise(resolve => setTimeout(() => resolve(null), 3000))
            ]);
            return !!(data && data.status === 'ok');
        }).catch(() => false);

        let landed = false;
        for (let attempt = 0; attempt < 10 && !landed; attempt++) {
            if (attempt) await new Promise(r => setTimeout(r, 400));
            const text = await Promise.race([
                fetch(screenFileUrl(filename) + '?t=' + Date.now(), { cache: 'no-store' })
                    .then(check => check.ok ? check.text() : '')
                    .catch(() => ''),
                new Promise(resolve => setTimeout(() => resolve(''), 4000))
            ]);
            landed = text === body;
        }
        const bodyOk = await Promise.race([
            post,
            new Promise(resolve => setTimeout(() => resolve(false), 300))
        ]);
        controller.abort();
        if (landed || bodyOk) {
            for (let attempt = 0; attempt < 5; attempt++) {
                await ensureScreenSpiffsFiles(true);
                if ((window.screenSpiffsFileList || []).some(f => f.name === filename)) break;
                await new Promise(r => setTimeout(r, 400));
            }
            await renderScreenPresets(filename);
            showStatus(getMessage('layout_saved_to_device'), 'success');
        } else {
            showStatus(getMessage('layout_save_device_failed'), 'error');
        }
    } catch (error) {
        console.error('Error saving layout to device:', error);
        showStatus(getMessage('layout_save_device_failed'), 'error');
    } finally {
        controller.abort();
        toggleLoading(btn, false);
    }
}

async function loadSystemScreenLayout(filename) {
    if (!filename) return;
    try {
        const response = await fetch(screenFileUrl(filename));
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
    setScreenSelection([]);
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
    if (!selectedId || isMultiSelect()) return; // no crosshair for multi-selection

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

    // Use a DocumentFragment to batch DOM insertions and reduce layout thrashing.
    const fragment = document.createDocumentFragment();

    // Render in z order (ascending), but allow overlap
    const elems = (profile.elements || []).slice().sort((a, b) => (a.z || 0) - (b.z || 0));

    elems.forEach(e => {
        if (e.enabled === 0) return;

        const def = findElementDef(e.id);
        if (!def) return;
        const size = getScreenElementSize(def, e);

        const box = document.createElement('div');
        const selected = isElementSelected(e.id);
        box.className = 'screen-element' + (selected ? ' selected' : '');
        box.setAttribute('data-id', e.id);
        const visualY = layoutYToVisualY(e.y || 0, e.id);
        box.style.left = `${(e.x || 0) * scale}px`;
        box.style.top = `${visualY * scale}px`;
        box.style.width = `${size.w * scale}px`;
        box.style.height = `${size.h * scale}px`;
        const layer = getElementLayer(e);
        box.style.zIndex = String(selected ? 12 + layer : 3 + layer);

        box.appendChild(createScreenPreview(def, true, e, scale));

        box.addEventListener('pointerdown', (ev) => {
            ev.preventDefault();
            ev.stopPropagation();
            focusScreenCanvas();
            const additive = ev.ctrlKey || ev.metaKey;
            if (additive) {
                // Ctrl/Cmd+Click toggles this element in/out of the selection.
                toggleScreenSelection(e.id);
                if (isElementSelected(e.id)) beginDrag(e.id, ev, false);
                renderScreenCanvas();
                renderScreenOptions();
                renderScreenPalette();
                return;
            }
            // Plain click: if it's already part of a multi-selection, keep the group
            // (so you can drag them all) and collapse to just this one if released
            // without dragging; otherwise select only this element.
            let clickSelectId = null;
            if (isElementSelected(e.id) && getSelectedIds().length > 1) {
                clickSelectId = e.id;
            } else {
                setScreenSelection([e.id]);
            }
            beginDrag(e.id, ev, false);
            if (window.screenEditor.dragging) window.screenEditor.dragging.clickSelectId = clickSelectId;
            renderScreenCanvas();
            renderScreenOptions();
            renderScreenPalette();
        });

        fragment.appendChild(box);
    });

    appendScreenGuides(fragment, scale);
    canvas.appendChild(fragment);
    appendScreenTrashTarget(canvas, window.screenEditor.dragging);
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

    const auxFontKey = mode === 'night' ? 'night_aux_font' : 'day_aux_font';
    const auxFontRow = document.createElement('div');
    auxFontRow.className = 'form-group';
    const auxFontLabel = document.createElement('label');
    auxFontLabel.textContent = getNestedTranslation(trans, 'screen.aux_time_font') || 'Aux digit theme';
    const auxFontSelect = document.createElement('select');
    SCREEN_TIME_FONTS.forEach(font => {
        const option = document.createElement('option');
        option.value = font;
        option.textContent = font.charAt(0).toUpperCase() + font.slice(1);
        auxFontSelect.appendChild(option);
    });
    auxFontSelect.value = window.screenLayout[auxFontKey] || 'bold';
    auxFontSelect.addEventListener('change', () => {
        window.screenLayout[auxFontKey] = auxFontSelect.value;
        renderScreenCanvas();
    });
    auxFontRow.appendChild(auxFontLabel);
    auxFontRow.appendChild(auxFontSelect);
    container.appendChild(auxFontRow);

    appendScreenSwitchRow(container, {
        id: 'screen_show_leading_zero',
        label: getNestedTranslation(trans, 'screen.show_leading_zero') || 'Show 0 for single digit hour',
        checked: !!(window.settings && window.settings.p24),
        onChange: () => saveScreenTimeDisplaySettings()
    });
    appendScreenSwitchRow(container, {
        id: 'screen_dots_breathe',
        label: getNestedTranslation(trans, 'screen.disable_dots_breathe') || 'Disable breathing time dots',
        checked: !!(window.settings && window.settings.p50),
        onChange: () => saveScreenTimeDisplaySettings()
    });

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

/* ---------- Fonts shortcut (Device > Display) ----------
   A beginner-friendly control that changes only the clock font for the day
   and/or night layout: it loads the current screen layout, sets day_font /
   night_font, and saves it to the device (same effect as picking a font in
   the Layout editor and pressing Apply) - without opening the editor.
   `mode` is 'day' or 'night'; p04/p05 are the mirrored settings for each. */
async function applyFontShortcut(mode, font, box, grid) {
    if (!window.screenLayout || !window.screenLayout.profiles) {
        await fetchScreenLayout(); // populate window.screenLayout from the device
    }
    if (!window.screenLayout) {
        toast(getMessage('error_saving_settings'), 'err');
        return;
    }
    const fontKey = mode === 'day' ? 'day_font' : 'night_font';
    window.screenLayout[fontKey] = font;
    window.screenLayout.version = SCREEN_LAYOUT_VERSION;
    ensureScreenLayoutMeta(window.screenLayout);
    ensureScreenProfileShape(window.screenLayout.profiles.day);
    ensureScreenProfileShape(window.screenLayout.profiles.night);
    grid.querySelectorAll('.screen-font-sample-box').forEach(b => b.classList.toggle('selected', b === box));
    try {
        const body = encodeScreenLayoutBinary(window.screenLayout);
        const resp = await fetch('/api/screen', {
            method: 'POST',
            headers: { 'Content-Type': 'application/octet-stream' },
            body
        });
        const data = await resp.json().catch(() => ({}));
        if (resp.ok && data && data.status === 'ok') {
            if (window.settings) window.settings[mode === 'day' ? 'p04' : 'p05'] = font; // keep mirror in sync
            toast(getMessage('font_applied'), 'ok');
        } else {
            toast(getMessage('error_saving_settings'), 'err');
        }
    } catch (e) {
        console.error('Font apply failed:', e);
        toast(getMessage('error_saving_unknown'), 'err');
    }
}

function buildFontSampleGrid(mode, current) {
    const grid = document.createElement('div');
    grid.className = 'screen-font-samples-grid';
    SCREEN_TIME_FONTS.forEach(font => {
        const box = document.createElement('button');
        box.type = 'button';
        box.className = 'screen-font-sample-box' + (font === current ? ' selected' : '');
        const name = document.createElement('div');
        name.className = 'screen-font-sample-name';
        name.textContent = font.charAt(0).toUpperCase() + font.slice(1);
        box.appendChild(name);
        box.appendChild(createFontSamplePreview(font));
        box.addEventListener('click', () => applyFontShortcut(mode, font, box, grid));
        grid.appendChild(box);
    });
    return grid;
}

async function renderFontsShortcut() {
    const dayHost = el('dayFontSamples');
    const nightHost = el('nightFontSamples');
    if (!dayHost || !nightHost || dayHost.dataset.rendered) return;
    dayHost.dataset.rendered = '1';
    await ensureScreenSpiffsFiles();
    if (!window.screenLayout || !window.screenLayout.profiles) {
        await fetchScreenLayout();
    }
    dayHost.innerHTML = '';
    dayHost.appendChild(buildFontSampleGrid('day', (window.screenLayout && window.screenLayout.day_font) || 'bold'));
    nightHost.innerHTML = '';
    nightHost.appendChild(buildFontSampleGrid('night', (window.screenLayout && window.screenLayout.night_font) || 'bold'));
}

function initFontsShortcut() {
    const box = el('fontsBox');
    if (!box || box.dataset.wired) return;
    box.dataset.wired = '1';
    // Render the samples lazily the first time the section is expanded.
    box.addEventListener('toggle', () => { if (box.open) renderFontsShortcut(); });
}

function appendScreenTokenButtons(container, textarea, extraCodes) {
    const tokenWrap = document.createElement('div');
    tokenWrap.className = 'message-tokens screen-message-tokens';
    const tokenTitle = document.createElement('h4');
    const trans = translations[currentLanguage] || translations.en;
    tokenTitle.textContent = getNestedTranslation(trans, 'screen.tokens') || 'Tokens';
    tokenWrap.appendChild(tokenTitle);
    const tokenList = document.createElement('div');
    tokenList.className = 'screen-token-list';
    SCREEN_TOKEN_CODES.concat(extraCodes || []).forEach(code => {
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
        autoscale: true, show_axis: true, show_xaxis: true, show_value: true,
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
        '[precip]', '[uv]', '[pressure]', '[3high]', '[3low]', '[CGM:reading]',
        '[wu:temp]', '[wu:hum]', '[wu:dew]', '[wu:wind]', '[wu:gust]',
        '[wu:pressure]', '[wu:rain]', '[wu:uv]'];
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
            (p.elements || []).forEach(e => {
                if (isScreenTextWidgetElement(e.id) && e.options && Array.isArray(e.options.slots)) {
                    e.options.slots.forEach(sl => scrape(sl.text));
                }
            });
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

function appendScreenWeatherThemePicker(container, profile, trans) {
    const row = document.createElement('div');
    row.className = 'form-group';
    const label = document.createElement('label');
    label.htmlFor = 'screen_weather_theme';
    label.textContent = getNestedTranslation(trans, 'screen.weather_theme') || 'Weather theme';
    const select = document.createElement('select');
    select.id = 'screen_weather_theme';
    const inherited = document.createElement('option');
    inherited.value = '0';
    inherited.textContent = getNestedTranslation(trans, 'screen.weather_theme_default') || 'Follow screen theme';
    select.appendChild(inherited);
    SCREEN_TIME_FONTS.forEach((font, i) => {
        const option = document.createElement('option');
        option.value = String(i + 1);
        option.textContent = font.charAt(0).toUpperCase() + font.slice(1);
        select.appendChild(option);
    });
    select.value = String(clamp(parseInt(profile.weather_theme, 10) || 0, 0, SCREEN_TIME_FONTS.length));
    select.addEventListener('change', () => {
        profile.weather_theme = clamp(parseInt(select.value, 10) || 0, 0, SCREEN_TIME_FONTS.length);
        renderScreenCanvas();
    });
    row.appendChild(label);
    row.appendChild(select);
    container.appendChild(row);
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

    // Multi-selection: no per-element options, just a hint on how to move them.
    if (isMultiSelect()) {
        const note = document.createElement('p');
        note.className = 'screen-options-mode-note';
        note.textContent = getScreenTranslation(
            'screen.multi_selected',
            '{n} elements selected — drag or use the arrow keys to move them together. Ctrl+Click to add or remove.'
        ).replace('{n}', String(getSelectedIds().length));
        opt.appendChild(note);
        return;
    }

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

    if (isScreenTextWidgetElement(e.id) || isScreenIconElement(e.id)) appendScreenInstanceNameRow(opt, e, trans);
    appendScreenLayerRow(opt, e);

    if (!isScreenTextWidgetElement(e.id)) {
        appendScreenSwitchRow(opt, {
            id: 'screen_element_enabled',
            label: getNestedTranslation(trans, 'screen.enabled') || 'Enabled',
            checked: e.enabled !== 0,
            onChange: (on) => {
                e.enabled = on ? 1 : 0;
                renderScreenCanvas();
                if (isScreenIconElement(e.id)) renderScreenPalette();
            }
        });
    }

    if (e.id === 'weather') appendScreenWeatherThemePicker(opt, profile, trans);

    if (isScreenIconElement(e.id)) {
        appendScreenIconOptions(opt, e);
        if (e.instanced) appendScreenDeleteRow(opt, e, trans);
    }

    if (isScreenTextWidgetElement(e.id)) {
        appendScreenTwOptions(opt, e, profile, trans);
        if (e.instanced) appendScreenDeleteRow(opt, e, trans);
        return;
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
            delayInput.value = String(window.screenLayout.scroll_delay || 60);
            delayInput.addEventListener('change', () => {
                window.screenLayout.scroll_delay = clamp(parseInt(delayInput.value, 10) || 60, 30, 255);
            });
            delayRow.appendChild(delayLabel);
            delayRow.appendChild(delayInput);
            opt.appendChild(delayRow);
        }

        const textRow = document.createElement('div');
        textRow.className = 'form-group';
        const textLabel = document.createElement('label');
        textLabel.textContent = getNestedTranslation(trans, 'screen.message_text') || 'Text';
        const textArea = document.createElement('textarea');
        const maxLen = 511;
        textArea.maxLength = maxLen;
        textArea.id = 'editor-textarea-' + e.id;
        textArea.rows = 3;

        const counter = document.createElement('div');
        counter.id = textArea.id + '-counter';
        counter.className = 'counter';

        textArea.value = profile.scroll_text || '';
        textArea.addEventListener('input', () => {
            profile.scroll_text = textArea.value;
            renderScreenCanvas();
            updateCharCounter(textArea, counter);
        });
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

        // Rows/sections append to `target`: the panel itself for the common
        // controls, then the "More options" disclosure for expert tuning.
        let target = opt;

        const mkRow = (labelText, control) => {
            const row = document.createElement('div');
            row.className = 'form-group';
            const lab = document.createElement('label');
            lab.textContent = labelText;
            row.appendChild(lab);
            row.appendChild(control);
            target.appendChild(row);
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

        // Subsection heading (declutters the now-long graph panel -- #191).
        const mkSection = (title) => {
            const h = document.createElement('div');
            h.className = 'subtitle';
            h.textContent = title;
            target.appendChild(h);
        };

        // --- Data ---
        mkSection(T('screen.graph_sec_data', 'Data'));
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
        mkNumber(T('screen.graph_interval', 'Sample interval (min)'), o.interval_min, 1, 1440, v => o.interval_min = clamp(parseInt(v, 10) || 5, 1, 1440));
        mkNumber(T('screen.graph_points', 'Data points'), o.points, 2, 100, v => o.points = clamp(parseInt(v, 10) || 60, 2, 100));

        // --- Size ---
        mkSection(T('screen.graph_sec_size', 'Size'));
        mkNumber(T('screen.graph_width', 'Width (px)'), o.gwidth, 60, 80, v => o.gwidth = clamp(parseInt(v, 10) || 80, 60, 80));
        mkNumber(T('screen.graph_height', 'Height (px)'), o.gheight, 28, 36, v => o.gheight = clamp(parseInt(v, 10) || 36, 28, 36));

        // Everything below is expert tuning — folded behind "More options".
        // The open state survives the re-renders triggered by toggles inside.
        const adv = document.createElement('details');
        adv.className = 'adv-box';
        adv.open = !!window._graphAdvOpen;
        adv.addEventListener('toggle', () => { window._graphAdvOpen = adv.open; });
        const advSummary = document.createElement('summary');
        advSummary.textContent = T('screen.graph_more', 'More options');
        adv.appendChild(advSummary);
        const advBody = document.createElement('div');
        advBody.className = 'adv-body';
        adv.appendChild(advBody);
        opt.appendChild(adv);
        target = advBody;

        // --- Scale ---
        mkSection(T('screen.graph_sec_scale', 'Scale'));
        appendScreenSwitchRow(target, { id: 'graph_autoscale', label: T('screen.graph_autoscale', 'Auto Y-scale'), checked: !!o.autoscale, onChange: (on) => { o.autoscale = on; renderScreenOptions(); } });
        if (!o.autoscale) {
            mkNumber(T('screen.graph_ymin', 'Y min'), o.y_min, -32767, 32767, v => o.y_min = (v === '' ? null : (parseInt(v, 10) || 0)));
            mkNumber(T('screen.graph_ymax', 'Y max'), o.y_max, -32767, 32767, v => o.y_max = (v === '' ? null : (parseInt(v, 10) || 0)));
        }

        // --- Axes & value ---
        mkSection(T('screen.graph_sec_axes', 'Axes & value'));
        appendScreenSwitchRow(target, { id: 'graph_axis', label: T('screen.graph_axis', 'Show value axis (Y)'), checked: !!o.show_axis, onChange: (on) => { o.show_axis = on; renderScreenCanvas(); } });
        appendScreenSwitchRow(target, { id: 'graph_xaxis', label: T('screen.graph_xaxis', 'Show time axis (X)'), checked: !!o.show_xaxis, onChange: (on) => { o.show_xaxis = on; renderScreenCanvas(); } });
        appendScreenSwitchRow(target, { id: 'graph_value', label: T('screen.graph_value', 'Show current value'), checked: !!o.show_value, onChange: (on) => { o.show_value = on; renderScreenCanvas(); } });

        // --- Band ---
        mkSection(T('screen.graph_sec_band', 'Band'));
        appendScreenSwitchRow(target, { id: 'graph_band', label: T('screen.graph_band', 'Show low/high band'), checked: !!o.band_on, onChange: (on) => { o.band_on = on; renderScreenOptions(); } });
        if (o.band_on) {
            mkNumber(T('screen.graph_band_low', 'Band low'), o.band_low, -32767, 32767, v => o.band_low = (v === '' ? null : (parseInt(v, 10) || 0)));
            mkNumber(T('screen.graph_band_high', 'Band high'), o.band_high, -32767, 32767, v => o.band_high = (v === '' ? null : (parseInt(v, 10) || 0)));
            mkColor(T('screen.graph_band_color', 'Band color'), o.band_color || '#283c28', v => o.band_color = v);
            mkColor(T('screen.graph_warn_color', 'Out-of-band color'), o.warn_color || '#ff5050', v => o.warn_color = v);
        }

        // --- Style ---
        mkSection(T('screen.graph_sec_style', 'Style'));
        mkColor(T('screen.graph_line_color', 'Line color'), o.color || '#00dcff', v => o.color = v);
        mkColor(T('screen.background_color', 'Background Color'), o.bg_color || '#000000', v => o.bg_color = v);
        mkColor(T('screen.graph_axis_color', 'Axis color'), o.axis_color || '#787878', v => o.axis_color = v);
        appendScreenSwitchRow(target, { id: 'graph_thick', label: T('screen.graph_thick', 'Thick line (2px)'), checked: !!o.thick, onChange: (on) => { o.thick = on; renderScreenCanvas(); } });
        appendScreenSwitchRow(target, { id: 'graph_boolean', label: T('screen.graph_boolean', 'Boolean (step 0/1)'), checked: !!o.boolean, onChange: (on) => { o.boolean = on; renderScreenCanvas(); } });
    }
}

function beginDrag(id, pointerEvent, fromPalette) {
    const canvas = el('screenCanvas');
    if (!canvas) return;
    focusScreenCanvas();
    if (!window.screenLayout) return;
    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return;

    // Cache the reference to the dragged element to avoid redundant lookups
    // during high-frequency pointermove events.
    const elem = ensureElementInProfile(profile, id);

    const scale = getScreenScale();
    window.screenEditor.scale = scale;

    const def = findElementDef(id);
    let startLeft = elem.x * scale;
    let startTop = layoutYToVisualY(elem.y || 0, def ? def.id : id) * scale;

    if (fromPalette) {
        setScreenSelection([id]);
        renderScreenCanvas();
        renderScreenOptions();
        renderScreenPalette();
    }

    // Build the set of elements that move together. When grabbing an element that's
    // part of a multi-selection, every selected (non-meta) element moves; otherwise
    // just the grabbed one. Each member snapshots its start position in visual space.
    let memberIds;
    if (!fromPalette && isElementSelected(id) && getSelectedIds().length > 1) {
        memberIds = getSelectedIds().filter(mid => mid && !isScreenMetaElement(mid));
        if (memberIds.indexOf(id) === -1) memberIds.push(id);
    } else {
        memberIds = [id];
    }
    const members = memberIds.map(mid => {
        const me = ensureElementInProfile(profile, mid);
        return { id: mid, elem: me, x: me.x || 0, visualY: layoutYToVisualY(me.y || 0, mid) };
    });

    canvas.setPointerCapture(pointerEvent.pointerId);
    window.screenEditor.dragging = {
        id,
        elem,
        originX: pointerEvent.clientX,
        originY: pointerEvent.clientY,
        startLeft,
        startTop,
        fromPalette,
        moved: false,
        members,
        // Cache the canvas bounding rectangle and scale to eliminate layout-thrashing
        // getBoundingClientRect() calls during high-frequency pointer moves.
        rect: canvas.getBoundingClientRect(),
        scale
    };
    updateScreenStatusLine();
}

function onScreenPointerDown(e) {
    if (window.screenEditor.dragging) return;
    trackScreenCanvasPointer(e);

    focusScreenCanvas();

    const target = e.target && e.target.closest ? e.target.closest('.screen-element') : null;
    if (!target) {
        setScreenSelection([]);
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
    const elem = d.elem;
    const scale = d.scale;

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
        const rect = d.rect;
        const def = findElementDef(d.id);
        const elementId = def ? def.id : d.id;
        elem.x = clamp(Math.round((e.clientX - rect.left - 10) / scale), 0, SCREEN_SIZE - 1);
        const visualY = clamp(Math.round((e.clientY - rect.top - 10) / scale), 0, SCREEN_SIZE - 1);
        elem.y = visualYToLayoutY(visualY, elementId);
    } else {
        d.moved = true;
        // Move every member of the (possibly multi-element) selection by the same,
        // group-clamped delta so the selection stays rigid.
        const members = d.members && d.members.length
            ? d.members
            : [{ id: d.id, elem, x: elem.x || 0, visualY: layoutYToVisualY(elem.y || 0, d.id) }];
        const { dx: cdx, dy: cdy } = clampGroupDelta(members, Math.round(dx / scale), Math.round(dy / scale));
        members.forEach(m => {
            m.elem.x = m.x + cdx;
            m.elem.y = visualYToLayoutY(m.visualY + cdy, m.id);
        });
    }
    d.deleteTarget = screenCanTrashDraggedWidget(d) && screenPointerOverTrash(e.clientX, e.clientY, d.rect);

    // Throttle rendering to the display's refresh rate (typically 60Hz) using RAF.
    // This prevents redundant DOM re-renders and reduces CPU/GPU load during dragging.
    if (!window.screenEditor.dragRafHandle) {
        window.screenEditor.dragRafHandle = requestAnimationFrame(() => {
            window.screenEditor.dragRafHandle = null;
            renderScreenCanvas();
        });
    }
}

function onScreenPointerUp(e) {
    const d = window.screenEditor.dragging;
    if (!d) return;
    if (window.screenEditor.dragRafHandle) {
        cancelAnimationFrame(window.screenEditor.dragRafHandle);
        window.screenEditor.dragRafHandle = null;
    }
    const placedFromPalette = d.fromPalette && d.moved;
    const droppedOnTrash = d.deleteTarget && screenCanTrashDraggedWidget(d);
    const deleteWidget = droppedOnTrash && screenDeletesDraggedInstance(d);
    // A plain click (no drag) on a member of a multi-selection collapses to just it.
    const collapseTo = (!d.moved && d.clickSelectId) ? d.clickSelectId : null;
    window.screenEditor.dragging = null;
    if (droppedOnTrash) {
        if (deleteWidget) {
            removeScreenWidgetInstance(d.id);
        } else {
            d.elem.enabled = 0;
            renderScreenCanvas();
            renderScreenOptions();
            renderScreenPalette();
            updateScreenStatusLine();
        }
        return;
    }
    if (collapseTo) setScreenSelection([collapseTo]);
    renderScreenCanvas();
    renderScreenOptions();
    updateScreenStatusLine();
    if (placedFromPalette || collapseTo) renderScreenPalette();
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
            // Show the reboot overlay and reload once the device is reachable again.
            waitForReboot();
        } else {
            showStatus(getMessage('failed_restart') + (data.message || 'Unknown error'), 'error');
        }
    })
    .catch(error => {
        // A connection error here almost always means the device already began
        // rebooting before it could answer, so wait for it to come back.
        waitForReboot();
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
    renderScreenPresets();
};

