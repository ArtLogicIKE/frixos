// Advanced settings section functionality
function updateDimmingModeSections() {
    const mode = parseInt(el('dim_mode')?.value, 10);
    const brightnessSection = el('brightness_dimming_section');
    const timeSection = el('timeofday_dimming_section');
    if (brightnessSection) brightnessSection.style.display = mode === 0 ? '' : 'none';
    if (timeSection) timeSection.style.display = mode === 2 ? '' : 'none';
}

function setupAdvancedSection() {
    // Setup event listeners only once
    if (!window.advancedEventListenersSet) {
        window.advancedEventListenersSet = true;
        
        // Handle form submission
        const advancedForm = el('advancedForm');
        if (advancedForm) {
            advancedForm.addEventListener('submit', (e) => handleFormSubmit(e, 'advancedForm'));
        }

        const dimModeSelect = el('dim_mode');
        if (dimModeSelect) {
            dimModeSelect.addEventListener('change', updateDimmingModeSections);
        }

        // Setup message character counter and interactive tokens
        const messageInput = el('message');
        const messageCounter = el('message-counter');
        if (messageInput && messageCounter) {
            messageInput.addEventListener('input', () => updateCharCounter(messageInput, messageCounter));
            document.querySelectorAll('.token-code').forEach(t => {
                decorateTokenButton(t, getTokenCode(t));
                bindTokenInsert(t, messageInput, () => highlightElement(messageInput));
            });
        }

        // Setup interactive font samples
        document.querySelectorAll('.font-sample-box').forEach(box => {
            box.setAttribute('role', 'button');
            box.setAttribute('tabindex', '0');

            const selectFont = () => {
                const nameEl = box.querySelector('.font-sample-name');
                if (!nameEl) return;
                const fontValue = nameEl.textContent.trim().toLowerCase();
                const dayFont = el('dayfont');
                const nightFont = el('nightfont');

                if (dayFont) {
                    dayFont.value = fontValue;
                    highlightElement(dayFont);
                }
                if (nightFont) {
                    nightFont.value = fontValue;
                    highlightElement(nightFont);
                }
                highlightElement(box);
            };

            box.addEventListener('click', selectFont);
            box.addEventListener('keydown', (e) => {
                if (e.key === 'Enter' || e.key === ' ') {
                    e.preventDefault();
                    selectFont();
                }
            });
        });
    }

    // Populate fields if settings are loaded
    if (window.settings && window.settingsLoaded.advanced) {
        const messageInput = el('message');
        const messageCounter = el('message-counter');
        if (el('ofs_x') && window.settings.p01 !== undefined) el('ofs_x').value = window.settings.p01 || 0;
        if (el('ofs_y') && window.settings.p02 !== undefined) el('ofs_y').value = window.settings.p02 || 0;
        if (el('lat') && window.settings.p17 !== undefined) el('lat').value = window.settings.p17 || '';
        if (el('lon') && window.settings.p18 !== undefined) el('lon').value = window.settings.p18 || '';
        if (el('timezone') && window.settings.p19 !== undefined) el('timezone').value = window.settings.p19 || '';
        if (el('wifi_start') && window.settings.p46 !== undefined) el('wifi_start').value = formatMinsToTimeString(window.settings.p46);
        if (el('wifi_end') && window.settings.p47 !== undefined) el('wifi_end').value = formatMinsToTimeString(window.settings.p47);
        if (el('lux_sensitivity') && window.settings.p20 !== undefined) el('lux_sensitivity').value = window.settings.p20 || 2.5;
        if (el('lux_threshold') && window.settings.p21 !== undefined) el('lux_threshold').value = window.settings.p21 || 50;
        if (el('dim_mode') && window.settings.p22 !== undefined) el('dim_mode').value = String(window.settings.p22);
        if (el('dim_start') && window.settings.p55 !== undefined) el('dim_start').value = formatMinsToTimeString(window.settings.p55);
        if (el('dim_end') && window.settings.p56 !== undefined) el('dim_end').value = formatMinsToTimeString(window.settings.p56);
        updateDimmingModeSections();
        if (el('brightness_LED0') && window.settings.p23 && window.settings.p23[0] !== undefined) el('brightness_LED0').value = window.settings.p23[0];
        if (el('brightness_LED1') && window.settings.p23 && window.settings.p23[1] !== undefined) el('brightness_LED1').value = window.settings.p23[1];
        if (el('pwm_frequency') && window.settings.p42 !== undefined) el('pwm_frequency').value = window.settings.p42 || 200;
        if (el('max_power') && window.settings.p43 !== undefined) el('max_power').value = window.settings.p43 || 1023;
    }
}

// Function to update mmol/l conversion labels
function updateMmolLabels() {
    const glucoseHigh = el('eeprom_glucose_high');
    const glucoseLow = el('eeprom_glucose_low');
    const highMmolLabel = el('glucose_high_mmol');
    const lowMmolLabel = el('glucose_low_mmol');
    
    if (glucoseHigh && highMmolLabel) {
        const highValue = parseInt(glucoseHigh.value) || 0;
        if (highValue > 0) {
            const mmolValue = (highValue / 18.0182).toFixed(1);
            highMmolLabel.textContent = `${mmolValue} mmol/L`;
        } else {
            highMmolLabel.textContent = '';
        }
    }
    
    if (glucoseLow && lowMmolLabel) {
        const lowValue = parseInt(glucoseLow.value) || 0;
        if (lowValue > 0) {
            const mmolValue = (lowValue / 18.0182).toFixed(1);
            lowMmolLabel.textContent = `${mmolValue} mmol/L`;
        } else {
            lowMmolLabel.textContent = '';
        }
    }
}

/* ---- Display Schedule UI ---- */

const SLOT_TYPES = [
    { value: 0, label: 'Time' },
    { value: 1, label: 'CGM (glucose)' },
    { value: 2, label: 'Outside Temperature' },
    { value: 3, label: 'Home Assistant entity' },
];

function buildSlotRow(slot) {
    const row = document.createElement('div');
    row.className = 'schedule-slot-row';

    const typeSelect = document.createElement('select');
    SLOT_TYPES.forEach(opt => {
        const o = document.createElement('option');
        o.value = opt.value;
        o.textContent = opt.label;
        if (slot.t === opt.value) o.selected = true;
        typeSelect.appendChild(o);
    });

    const durInput = document.createElement('input');
    durInput.type = 'number';
    durInput.min = 1;
    durInput.max = 3600;
    durInput.value = slot.d || 30;
    durInput.title = 'Duration (seconds)';

    const durLabel = document.createElement('span');
    durLabel.className = 'schedule-slot-sec';
    durLabel.textContent = 'sec';

    const entityInput = document.createElement('input');
    entityInput.type = 'text';
    entityInput.className = 'schedule-slot-entity';
    entityInput.placeholder = 'entity_id (e.g. sensor.temp)';
    entityInput.value = slot.e || '';
    entityInput.hidden = slot.t !== 3;

    const unitInput = document.createElement('input');
    unitInput.type = 'text';
    unitInput.className = 'schedule-slot-unit';
    unitInput.placeholder = 'unit (e.g. °F)';
    unitInput.maxLength = 7;
    unitInput.value = slot.l || '';
    unitInput.hidden = slot.t !== 3;

    const nameInput = document.createElement('input');
    nameInput.type = 'text';
    nameInput.className = 'schedule-slot-label';
    nameInput.placeholder = getScreenTranslation('screen.display_schedule.label_placeholder', 'label (e.g. Outside Temp)');
    nameInput.maxLength = 25;
    nameInput.value = slot.n || '';

    typeSelect.addEventListener('change', () => {
        const t = parseInt(typeSelect.value);
        const isHA = (t === 3);
        entityInput.hidden = !isHA;
        unitInput.hidden = !isHA;
    });

    const removeBtn = document.createElement('button');
    removeBtn.type = 'button';
    removeBtn.textContent = '✕';
    removeBtn.addEventListener('click', () => row.remove());

    row.appendChild(typeSelect);
    row.appendChild(durInput);
    row.appendChild(durLabel);
    row.appendChild(entityInput);
    row.appendChild(unitInput);
    row.appendChild(nameInput);
    row.appendChild(removeBtn);
    return row;
}

function appendDisplayScheduleSection(container, options = {}) {
    const trans = translations[currentLanguage] || translations.en;
    const listId = options.listId || 'display-schedule-list';
    const titleKey = options.titleKey || 'screen.display_schedule.title';
    const descKey = options.descriptionKey || 'screen.display_schedule.description';
    const settingsKey = options.settingsKey || 'p58';

    const section = document.createElement('div');
    section.className = 'screen-display-schedule';

    const heading = document.createElement('h4');
    heading.textContent = getNestedTranslation(trans, titleKey) || 'Display Schedule';

    const desc = document.createElement('p');
    desc.className = 'screen-options-mode-note';
    desc.textContent = getNestedTranslation(trans, descKey)
        || 'Define what appears on the display and for how long. Slots cycle in order. A single Time slot means the clock always shows.';

    const list = document.createElement('div');
    list.id = listId;
    list.className = 'display-schedule-list';

    const addBtn = document.createElement('button');
    addBtn.type = 'button';
    addBtn.className = 'button screen-schedule-add';
    addBtn.textContent = getNestedTranslation(trans, 'screen.display_schedule.add_slot') || '+ Add Slot';
    addBtn.addEventListener('click', () => {
        list.appendChild(buildSlotRow({ t: 0, d: 30 }));
    });

    section.appendChild(heading);
    section.appendChild(desc);
    section.appendChild(list);
    section.appendChild(addBtn);
    container.appendChild(section);

    renderDisplaySchedule(list, settingsKey);
}

function renderDisplaySchedule(listEl, settingsKey = 'p58') {
    const list = listEl || el('display-schedule-list');
    if (!list) return;
    list.innerHTML = '';

    let slots = [];
    const raw = window.settings && window.settings[settingsKey];
    if (typeof raw === 'string' && raw.trim().startsWith('[')) {
        try { slots = JSON.parse(raw); } catch (e) { slots = []; }
    }
    if (settingsKey === 'p58' && (!Array.isArray(slots) || slots.length === 0)) {
        if ((window.settings.p48 || 0) > 0) slots.push({ t: 0, d: window.settings.p48 });
        if ((window.settings.p49 || 0) > 0) slots.push({ t: 1, d: window.settings.p49 });
        if ((window.settings.p57 || 0) > 0) slots.push({ t: 2, d: window.settings.p57 });
        if (slots.length === 0) slots.push({ t: 0, d: 30 });
    }
    if (settingsKey === 'p59' && (!Array.isArray(slots) || slots.length === 0)) {
        slots.push({ t: 0, d: 30 });
    }
    slots.forEach(s => list.appendChild(buildSlotRow(s)));
}

function serializeDisplaySchedule(listId = 'display-schedule-list') {
    const list = el(listId);
    if (!list) return '[]';
    const slots = [];
    list.querySelectorAll('.schedule-slot-row').forEach(row => {
        const typeEl    = row.querySelector('select');
        const durEl     = row.querySelectorAll('input[type="number"]')[0];
        const textInputs = row.querySelectorAll('input[type="text"]');
        const entEl     = textInputs[0];
        const unitEl    = textInputs[1];
        const nameEl    = textInputs[2];
        const t = parseInt(typeEl ? typeEl.value : 0);
        const d = parseInt(durEl  ? durEl.value  : 30) || 1;
        const slot = {t, d};
        if (t === 3) {
            if (entEl  && entEl.value.trim())  slot.e = entEl.value.trim();
            if (unitEl && unitEl.value.trim()) slot.l = unitEl.value.trim();
        }
        if (nameEl && nameEl.value.trim()) slot.n = nameEl.value.trim();
        if (t !== 3 || slot.e) slots.push(slot);
    });
    return JSON.stringify(slots);
}

function setupIntegrationsSection() {
    const form = el('integrationsForm');
    const haUrlInput = el('eeprom_ha_url');
    const haTokenInput = el('eeprom_ha_token');
    const haTokenMask = el('eeprom_ha_token-mask');
    const stockKeyInput = el('eeprom_stock_key');
    const stockKeyMask = el('eeprom_stock_key-mask');

    // Function to update token mask
    function updateTokenMask(token, maskElement) {
        if (token && token.length > 0 && maskElement) {
            const firstFive = token.substring(0, 5);
            const lastFive = token.substring(token.length - 5);
            maskElement.textContent = `${firstFive}.....${lastFive} (${token.length} bytes)`;
        } else if (maskElement) {
            maskElement.textContent = '';
        }
    }

    // Setup event listeners only once
    if (!window.integrationsEventListenersSet) {
        window.integrationsEventListenersSet = true;

        // Add input event listeners for token masking
        if (haTokenInput && haTokenMask) {
            haTokenInput.addEventListener('input', function() {
                updateTokenMask(this.value, haTokenMask);
            });
        }
        if (stockKeyInput && stockKeyMask) {
            stockKeyInput.addEventListener('input', function() {
                updateTokenMask(this.value, stockKeyMask);
            });
        }

        // Add event listeners for shared glucose monitoring fields
        const glucoseRefresh = el('eeprom_glucose_refresh');
        const glucosePassword = el('eeprom_glucose_password');
        const glucosePasswordMask = el('eeprom_glucose_password-mask');
        const dexcomRegion = el('eeprom_dexcom_region');
        const libreRegion = el('eeprom_libre_region');
        const glucoseHigh = el('eeprom_glucose_high');
        const glucoseLow = el('eeprom_glucose_low');
        
        // Add event listeners to update mmol/l labels when values change
        if (glucoseHigh) {
            glucoseHigh.addEventListener('input', updateMmolLabels);
        }
        if (glucoseLow) {
            glucoseLow.addEventListener('input', updateMmolLabels);
        }

        // Shared glucose refresh field - no sync needed since it's saved directly to both devices when enabled

        // Add event listener for shared glucose password masking
        if (glucosePassword && glucosePasswordMask) {
            glucosePassword.addEventListener('input', function() {
                updateTokenMask(this.value, glucosePasswordMask);
            });
        }

        // Region changes should NOT modify username/password fields
        // These fields are shared between Dexcom and Libre and should only be changed by user input

        setupIntegrationsValidation();
    }

    // Populate fields if settings are loaded
    if (window.settings && window.settingsLoaded.integrations) {
        if (haUrlInput && typeof window.settings.p25 !== 'undefined') {
            haUrlInput.value = window.settings.p25;
        }
        if (haTokenInput && typeof window.settings.p26 !== 'undefined') {
            haTokenInput.value = window.settings.p26;
            updateTokenMask(window.settings.p26, haTokenMask);
        }
        const haRefreshMins = el('eeprom_ha_refresh_mins');
        if (haRefreshMins && typeof window.settings.p27 !== 'undefined') {
            haRefreshMins.value = window.settings.p27 || 5;
        }
        if (stockKeyInput && typeof window.settings.p28 !== 'undefined') {
            stockKeyInput.value = window.settings.p28;
            updateTokenMask(window.settings.p28, stockKeyMask);
        }
        const stockRefreshMins = el('eeprom_stock_refresh_mins');
        if (stockRefreshMins && typeof window.settings.p29 !== 'undefined') {
            stockRefreshMins.value = window.settings.p29 || 5;
        }

        // Load Dexcom region
        const dexcomRegion = el('eeprom_dexcom_region');
        if (dexcomRegion && typeof window.settings.p30 !== 'undefined') {
            dexcomRegion.value = window.settings.p30;
        }

        // Load Libre/Freestyle region
        const libreRegion = el('eeprom_libre_region');
        if (libreRegion && typeof window.settings.p44 !== 'undefined') {
            libreRegion.value = window.settings.p44;
        }

        const nsUrlInput = el('eeprom_ns_url');
        if (nsUrlInput && typeof window.settings.p54 !== 'undefined') {
            nsUrlInput.value = window.settings.p54 || '';
        }
        updateCgmExclusivity();

        // Load shared Glucose Monitoring thresholds
        const glucoseHigh = el('eeprom_glucose_high');
        if (glucoseHigh && typeof window.settings.p51 !== 'undefined') {
            glucoseHigh.value = window.settings.p51;
        }

        const glucoseLow = el('eeprom_glucose_low');
        if (glucoseLow && typeof window.settings.p52 !== 'undefined') {
            glucoseLow.value = window.settings.p52;
        }

        // Load glucose unit
        const glucoseUnit = el('eeprom_glucose_unit');
        if (glucoseUnit && typeof window.settings.p53 !== 'undefined') {
            glucoseUnit.value = window.settings.p53 || 0;
        }
        
        // Update mmol/l labels when settings are loaded
        updateMmolLabels();

        // Load shared refresh, username, and password - always from p31, p32, p33
        const glucoseRefresh = el('eeprom_glucose_refresh');
        const glucoseUsername = el('eeprom_glucose_username');
        const glucosePassword = el('eeprom_glucose_password');
        const glucosePasswordMask = el('eeprom_glucose_password-mask');

        // Load shared refresh (p33)
        if (glucoseRefresh && typeof window.settings.p33 !== 'undefined') {
            glucoseRefresh.value = window.settings.p33 || 5;
        }

        const glucoseValidity = el('glucose_validity_duration');
        if (glucoseValidity && typeof window.settings.p45 !== 'undefined') {
            glucoseValidity.value = window.settings.p45 || 30;
        }

        // Load shared username (p31)
        if (glucoseUsername && typeof window.settings.p31 !== 'undefined') {
            glucoseUsername.value = window.settings.p31;
        }

        // Load shared password (p32)
        if (glucosePassword && typeof window.settings.p32 !== 'undefined') {
            glucosePassword.value = window.settings.p32;
            updateTokenMask(window.settings.p32, glucosePasswordMask);
        }
    }

    if (form) {
        const submitBtnIntegrations = form.querySelector('button[type="submit"]');
        if (submitBtnIntegrations) {
            submitBtnIntegrations.disabled = !window.settingsLoaded.integrations;
        }
    }
}

// Function to handle Home Assistant Integration settings
function setupIntegrationsValidation() {
    const integrationsForm = el('integrationsForm');
    if (!integrationsForm) return;

    // Home Assistant validation
    const haUrl = el('eeprom_ha_url');
    const haToken = el('eeprom_ha_token');
    const haRefresh = el('eeprom_ha_refresh_mins');

    // Stock validation
    const stockKey = el('eeprom_stock_key');
    const stockRefresh = el('eeprom_stock_refresh_mins');

    // Glucose monitor validation (Dexcom, FreeStyle Libre, Nightscout URL - only one at a time)
    const dexcomRegion = el('eeprom_dexcom_region');
    const libreRegion = el('eeprom_libre_region');
    const nsUrlInput = el('eeprom_ns_url');
    const glucoseUsername = el('eeprom_glucose_username');
    const glucosePassword = el('eeprom_glucose_password');
    const glucoseRefresh = el('eeprom_glucose_refresh');

    if (dexcomRegion && libreRegion && nsUrlInput && glucoseUsername && glucosePassword && glucoseRefresh) {
        dexcomRegion.addEventListener('change', function() {
            if (dexcomRegion.value !== '0') {
                libreRegion.value = '0';
                nsUrlInput.value = '';
            }
            updateCgmExclusivity();
            validateGlucoseMonitors();
        });
        libreRegion.addEventListener('change', function() {
            if (libreRegion.value !== '0') {
                dexcomRegion.value = '0';
                nsUrlInput.value = '';
            }
            updateCgmExclusivity();
            validateGlucoseMonitors();
        });
        nsUrlInput.addEventListener('input', function() {
            if (nsUrlInput.value.trim() !== '') {
                dexcomRegion.value = '0';
                libreRegion.value = '0';
            }
            updateCgmExclusivity();
            validateGlucoseMonitors();
        });
        glucoseUsername.addEventListener('input', validateGlucoseMonitors);
        glucosePassword.addEventListener('input', validateGlucoseMonitors);
        glucoseRefresh.addEventListener('input', validateGlucoseMonitors);
    }

    function validateGlucoseMonitors() {
        const dexcomRegionVal = el('eeprom_dexcom_region').value;
        const libreRegionVal = el('eeprom_libre_region').value;
        const nsUrlVal = (el('eeprom_ns_url') && el('eeprom_ns_url').value) ? el('eeprom_ns_url').value.trim() : '';
        const username = el('eeprom_glucose_username').value;
        const password = el('eeprom_glucose_password').value;
        const refresh = el('eeprom_glucose_refresh').value;
        const glucoseHigh = el('eeprom_glucose_high').value;
        const glucoseLow = el('eeprom_glucose_low').value;

        const dexcomEnabled = dexcomRegionVal !== '0';
        const libreEnabled = libreRegionVal !== '0';
        const nsEnabled = nsUrlVal !== '';
        const enabledCount = (dexcomEnabled ? 1 : 0) + (libreEnabled ? 1 : 0) + (nsEnabled ? 1 : 0);

        if (enabledCount > 1) {
            showStatus(getMessage('cgm_only_one'), 'error');
            return false;
        }

        if (enabledCount === 0) {
            return true;
        }

        // Exactly one enabled: Dexcom or Libre need username/password/refresh/thresholds; Nightscout URL does not
        if (dexcomEnabled || libreEnabled) {
            if (!username || !password) {
                showStatus(getMessage('dexcom_credentials_required'), 'error');
                return false;
            }
            const refreshNum = parseInt(refresh);
            if (isNaN(refreshNum) || refreshNum < 1 || refreshNum > 60) {
                showStatus(getMessage('dexcom_refresh_range'), 'error');
                return false;
            }
            const highNum = parseInt(glucoseHigh);
            const lowNum = parseInt(glucoseLow);
            if (isNaN(highNum) || highNum < 1 || highNum > 400) {
                showStatus(getMessage('glucose_high_range'), 'error');
                return false;
            }
            if (isNaN(lowNum) || lowNum < 1 || lowNum > 400) {
                showStatus(getMessage('glucose_low_range'), 'error');
                return false;
            }
            if (lowNum >= highNum) {
                showStatus(getMessage('glucose_low_less_than_high'), 'error');
                return false;
            }
        }

        return true;
    }

    function validateHomeAssistant() {
        const url = haUrl.value.trim();
        const token = haToken.value.trim();
        const refresh = haRefresh ? parseInt(haRefresh.value) : 1;

        // If URL is provided, validate format
        if (url && !/^https?:\/\/.+/.test(url)) {
            showStatus('Home Assistant URL must start with http:// or https://', 'error');
            return false;
        }

        // If URL is set but no token, or token set but no URL
        if (url && !token) {
            showStatus('Home Assistant requires both URL and token', 'error');
            return false;
        }

        if (haRefresh && (isNaN(refresh) || refresh < 1 || refresh > 7200)) {
            showStatus('Home Assistant refresh interval must be between 1 and 7200 minutes', 'error');
            return false;
        }

        return true;
    }

    function validateStock() {
        const refresh = stockRefresh ? parseInt(stockRefresh.value) : 5;

        if (stockRefresh && (isNaN(refresh) || refresh < 1 || refresh > 1440)) {
            showStatus('Stock refresh interval must be between 1 and 1440 minutes', 'error');
            return false;
        }

        return true;
    }

    integrationsForm.addEventListener('submit', function(e) {
        e.preventDefault();

        // Trigger field-level validation before checking
        if (haUrl) haUrl.dispatchEvent(new Event('input', { bubbles: true }));
        if (haToken) haToken.dispatchEvent(new Event('input', { bubbles: true }));
        if (stockKey) stockKey.dispatchEvent(new Event('input', { bubbles: true }));

        // Check for existing field-level validation errors
        const urlError = el('eeprom_ha_url-error');
        const tokenError = el('eeprom_ha_token-error');
        const stockKeyError = el('eeprom_stock_key-error');

        if ((urlError && urlError.textContent) || (tokenError && tokenError.textContent) || (stockKeyError && stockKeyError.textContent)) {
            showStatus(getMessage('correct_form_before_save'), 'error');
            return;
        }

        let isValid = true;

        // Validate Home Assistant if enabled
        if (haUrl.value.trim() || haToken.value.trim()) {
            isValid = validateHomeAssistant() && isValid;
        }

        // Validate Stock if enabled
        if (stockKey.value.trim()) {
            isValid = validateStock() && isValid;
        }

        // Validate CGM (only one of Dexcom, FreeStyle Libre, or Nightscout URL can be enabled)
        isValid = validateGlucoseMonitors() && isValid;

        if (isValid) {
            handleFormSubmit(e, 'integrationsForm');
        }
    });
}

