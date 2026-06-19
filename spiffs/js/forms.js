// Shared form submission handler
/*
 * Field Name Mappings (pXX format to reduce HTTP header size):
 * 
 * Basic Settings:
 * - p00 = hostname (Device hostname)
 * - p03 = rotation (Display rotation)
 * - p08 = show_grid (Screen grid overlay — Layout palette)
 * - p09 = mirroring (Mirror display)
 * - p34 = wifi_ssid (WiFi SSID)
 * - p35 = wifi_pass (WiFi password)
 * - p36 = fahrenheit (Temperature unit)
 * - p37 = hour12 (12-hour format)
 * - p38 = scroll_speed (Scroll speed)
 * - p39 = update_firmware (Auto firmware update)
 * 
 * Theme Settings:
 * - p40 = dark_theme (Dark theme)
 * - p41 = language (Language index: 0=en, 1=de, 2=fr, 3=it, 4=pt, 5=sv, 6=da, 7=pl)
 * 
 * Advanced Settings:
 * - p01 = ofs_x (X offset)
 * - p02 = ofs_y (Y offset) 
 * - p04 = dayfont (Day font)
 * - p05 = nightfont (Night font)
 * - p06 = quiet_scroll (Show scrolling message)
 * - p07 = quiet_weather (legacy; layout controls weather/moon visibility)
 * - p10 = color_filter (Day color filter)
 * - p11 = night_color_filter (Night color filter)
 * - p12 = msg_color (Message color)
 * - p13 = msg_font (Message font)
 * - p14 = scroll_delay (Scroll delay)
 * - p15 = night_msg_color (Night message color)
 * - p16 = message (Scrolling message)
 * - p17 = lat (Latitude)
 * - p18 = lon (Longitude)
 * - p19 = timezone (Timezone)
 * - p20 = lux_sensitivity (Light sensitivity)
 * - p21 = lux_threshold (Light threshold)
 * - p22 = dim_mode (0=brightness, 1=full, 2=time-of-day)
 * - p55 = dim_start (Time-of-day dimming start hour)
 * - p56 = dim_end (Time-of-day dimming end hour)
 * - p23 = brightness_LED (LED brightness array)
 * - p24 = show_leading_zero (Show leading zero)
 * - p42 = pwm_frequency (PWM frequency in Hz, range 60-50000)
 * - p43 = max_power (Max power, range 1-1023)
 * - p46 = wifi_start (WiFi Active Hours Start, 0-23)
 * - p47 = wifi_end (WiFi Active Hours End, 0-23)
 * - p50 = dots_breathe (Disable breathing time dots)
 * 
 * Integration Settings:
 * - p25 = eeprom_ha_url (Home Assistant URL)
 * - p26 = eeprom_ha_token (Home Assistant token)
 * - p27 = eeprom_ha_refresh_mins (HA refresh interval)
 * - p28 = eeprom_stock_key (Stock API key)
 * - p29 = eeprom_stock_refresh_mins (Stock refresh interval)
 * - p30 = eeprom_dexcom_region (Dexcom region)
 * - p31 = eeprom_glucose_username (Shared glucose username - used by both Dexcom and Libre)
 * - p32 = eeprom_glucose_password (Shared glucose password - used by both Dexcom and Libre)
 * - p33 = eeprom_glucose_refresh (Shared glucose refresh - used by both Dexcom and Libre)
 * - p44 = eeprom_libre_region (Libre region)
 * - p54 = eeprom_ns_url (Nightscout URL, max 100 chars)
 * - p45 = glucose_validity_duration (Glucose data validity duration in minutes)
 * - p48 = eeprom_sec_time (Alternate time display duration in seconds — legacy, migration only)
 * - p49 = eeprom_sec_cgm (Alternate CGM display duration in seconds — legacy, migration only)
 * - p57 = eeprom_sec_weather (Alternate weather temperature display duration — legacy, migration only)
 * - p58 = eeprom_disp_sched (Display schedule JSON)
 * - p59 = eeprom_disp_sched_aux (Aux digit display schedule JSON)
 * - p51 = eeprom_glucose_high (High glucose threshold in mg/dL)
 * - p52 = eeprom_glucose_low (Low glucose threshold in mg/dL)
 * - p53 = cgm_unit (Glucose display unit: 0=mg/dL, 1=mmol/L)
 */
function handleFormSubmit(e, formId) {
    e.preventDefault();

    // Get the actual form element to verify fields belong to it
    const form = el(formId);
    if (!form) {
        console.error(`Form ${formId} not found`);
        return;
    }

    // Show "Saving..." message
    showStatus(getMessage('saving_settings'), 'info');

    // Toggle loading state on the submit button
    const submitBtn = form.querySelector('button[type="submit"]');
    toggleLoading(submitBtn, true);

    // Helper function to check if an element exists and belongs to this form
    function getFieldInForm(fieldId) {
        const field = el(fieldId);
        // Check if field exists and is within this form
        if (field && (form.contains(field) || (field.form && field.form === form) || field.closest(`form#${formId}`))) {
            return field;
        }
        return null;
    }

    // Only include fields that have changed and belong to this form
    const formData = {};
    let changedCount = 0;

    // Settings form fields (settingsForm)
    if (formId === 'settingsForm') {
        const hostnameEl = getFieldInForm('hostname');
        if (hostnameEl && addIfChanged(formData, 'p00', hostnameEl.value, window.settings.p00)) changedCount++;
        
        const wifiSsidEl = getFieldInForm('wifi_ssid');
        if (wifiSsidEl && addIfChanged(formData, 'p34', wifiSsidEl.value, window.settings.p34)) changedCount++;
        
        const wifiPassEl = getFieldInForm('wifi_pass');
        // For password fields, only send if value has changed and is not empty
        if (wifiPassEl) {
            const newPass = wifiPassEl.value.trim();
            const oldPass = window.settings.p35;
            // Send if value changed (including clearing: non-empty -> empty), OR (old value is undefined/empty AND new value is not empty)
            if (hasValueChanged(newPass, oldPass) || (!oldPass && newPass !== '')) {
                formData.p35 = newPass;
                changedCount++;
            }
        }
        
        const fahrenheitEl = getFieldInForm('fahrenheit');
        if (fahrenheitEl && addIfChanged(formData, 'p36', fahrenheitEl.checked ? 1 : 0, window.settings.p36)) changedCount++;
        
        const hour12El = getFieldInForm('hour12');
        if (hour12El && addIfChanged(formData, 'p37', hour12El.checked ? 1 : 0, window.settings.p37)) changedCount++;
        
        const updateFirmwareEl = getFieldInForm('update_firmware');
        if (updateFirmwareEl && addIfChanged(formData, 'p39', updateFirmwareEl.checked ? 1 : 0, window.settings.p39)) {
            changedCount++;
        }

        // Static IP settings (p60-p63)
        const useStaticIpEl = getFieldInForm('use_static_ip');
        if (useStaticIpEl) {
            if (useStaticIpEl.checked) {
                // User wants static IP — collect field values
                const staticIpEl  = getFieldInForm('static_ip');
                const staticGwEl  = getFieldInForm('static_gw');
                const staticNmEl  = getFieldInForm('static_nm');
                const staticDns1El = getFieldInForm('static_dns1');
                const staticDns2El = getFieldInForm('static_dns2');
                const newIp  = staticIpEl  ? staticIpEl.value.trim()  : '';
                const newGw  = staticGwEl  ? staticGwEl.value.trim()  : '';
                const newNm  = staticNmEl  ? staticNmEl.value.trim()  : '255.255.255.0';
                const dns1   = staticDns1El ? staticDns1El.value.trim() : '';
                const dns2   = staticDns2El ? staticDns2El.value.trim() : '';
                const newDns = [dns1, dns2].filter(Boolean).join(',');
                if (addIfChanged(formData, 'p60', newIp,  window.settings.p60)) changedCount++;
                if (addIfChanged(formData, 'p61', newGw,  window.settings.p61)) changedCount++;
                if (addIfChanged(formData, 'p62', newNm,  window.settings.p62)) changedCount++;
                if (addIfChanged(formData, 'p63', newDns, window.settings.p63)) changedCount++;
            } else {
                // User disabled static IP — send empty string to revert to DHCP
                if (window.settings.p60 !== '') {
                    formData.p60 = '';
                    formData.p61 = '';
                    formData.p62 = '';
                    formData.p63 = '';
                    changedCount++;
                }
            }
        }

        const rotationEl = getFieldInForm('rotation');
        if (rotationEl && addIfChanged(formData, 'p03', parseInt(rotationEl.value) || 0, window.settings.p03)) changedCount++;

        const mirroringEl = getFieldInForm('mirroring');
        if (mirroringEl && addIfChanged(formData, 'p09', mirroringEl.checked ? 1 : 0, window.settings.p09)) changedCount++;
    }

    // Advanced form fields (advancedForm)
    if (formId === 'advancedForm') {
        const ofsXEl = getFieldInForm('ofs_x');
        if (ofsXEl && addIfChanged(formData, 'p01', parseInt(ofsXEl.value) || 0, window.settings.p01)) changedCount++;
        
        const ofsYEl = getFieldInForm('ofs_y');
        if (ofsYEl && addIfChanged(formData, 'p02', parseInt(ofsYEl.value) || 0, window.settings.p02)) changedCount++;
        
        const latEl = getFieldInForm('lat');
        if (latEl && addIfChanged(formData, 'p17', latEl.value, window.settings.p17)) changedCount++;
        
        const lonEl = getFieldInForm('lon');
        if (lonEl && addIfChanged(formData, 'p18', lonEl.value, window.settings.p18)) changedCount++;
        
        const timezoneEl = getFieldInForm('timezone');
        if (timezoneEl && addIfChanged(formData, 'p19', timezoneEl.value, window.settings.p19)) changedCount++;

        const tzIanaEl = getFieldInForm('tz_iana');
        if (tzIanaEl && addIfChanged(formData, 'tz_iana', tzIanaEl.value, window.settings.tz_iana)) changedCount++;

        const wifiStartEl = getFieldInForm('wifi_start');
        if (wifiStartEl && addIfChanged(formData, 'p46', parseTimeStringToMins(wifiStartEl.value), window.settings.p46)) changedCount++;
        
        const wifiEndEl = getFieldInForm('wifi_end');
        if (wifiEndEl && addIfChanged(formData, 'p47', parseTimeStringToMins(wifiEndEl.value), window.settings.p47)) changedCount++;
        
        const luxSensitivityEl = getFieldInForm('lux_sensitivity');
        if (luxSensitivityEl && addIfChanged(formData, 'p20', parseFloat(luxSensitivityEl.value) || 0, window.settings.p20)) changedCount++;
        
        const luxThresholdEl = getFieldInForm('lux_threshold');
        if (luxThresholdEl && addIfChanged(formData, 'p21', parseFloat(luxThresholdEl.value) || 0, window.settings.p21)) changedCount++;
        
        const dimModeEl = getFieldInForm('dim_mode');
        if (dimModeEl && addIfChanged(formData, 'p22', parseInt(dimModeEl.value, 10) || 0, window.settings.p22)) changedCount++;

        const dimStartEl = getFieldInForm('dim_start');
        if (dimStartEl && addIfChanged(formData, 'p55', parseTimeStringToMins(dimStartEl.value), window.settings.p55)) changedCount++;

        const dimEndEl = getFieldInForm('dim_end');
        if (dimEndEl && addIfChanged(formData, 'p56', parseTimeStringToMins(dimEndEl.value), window.settings.p56)) changedCount++;
        
        // Handle brightness array - check if any element changed
        // Backend expects exactly 2 elements [day, night], range 1-100. Do not send a third element.
        const brightnessLED0El = getFieldInForm('brightness_LED0');
        const brightnessLED1El = getFieldInForm('brightness_LED1');
        if (brightnessLED0El && brightnessLED1El) {
            const newBrightness = [
                parseInt(brightnessLED0El.value) || 100,
                parseInt(brightnessLED1El.value) || 30
            ];
            if (hasValueChanged(newBrightness, window.settings.p23)) {
                formData.p23 = newBrightness;
                changedCount++;
            }
        }
        
        const pwmFreqEl = getFieldInForm('pwm_frequency');
        if (pwmFreqEl && addIfChanged(formData, 'p42', parseInt(pwmFreqEl.value) || 0, window.settings.p42)) changedCount++;
        
        const maxPowerEl = getFieldInForm('max_power');
        if (maxPowerEl && addIfChanged(formData, 'p43', parseInt(maxPowerEl.value) || 0, window.settings.p43)) changedCount++;
    }

    // Integrations form fields (integrationsForm)
    if (formId === 'integrationsForm') {
        const haUrlInput = getFieldInForm('eeprom_ha_url');
        if (haUrlInput) {
            const newUrl = haUrlInput.value.trim();
            const oldUrl = window.settings.p25;
            if (hasValueChanged(newUrl, oldUrl) || ((oldUrl === undefined || oldUrl === null) && newUrl !== '')) {
                formData.p25 = newUrl;
                changedCount++;
            }
        }
        
        const haTokenInput = getFieldInForm('eeprom_ha_token');
        // For password/token fields, send if:
        // 1. The value has changed from the old value (including clearing: non-empty -> empty), OR
        // 2. The old value is undefined/empty and new value is not empty (first time setting)
        if (haTokenInput) {
            const newToken = haTokenInput.value.trim();
            const oldToken = window.settings.p26;
            // Send if value changed, OR (old value is undefined/empty AND new value is not empty)
            if (hasValueChanged(newToken, oldToken) || (!oldToken && newToken !== '')) {
                formData.p26 = newToken;
                changedCount++;
            }
        }

        const haRefreshInput = getFieldInForm('eeprom_ha_refresh_mins');
        if (haRefreshInput && addIfChanged(formData, 'p27', parseInt(haRefreshInput.value) || 1, window.settings.p27)) {
            changedCount++;
        }

        const stockKeyInput = getFieldInForm('eeprom_stock_key');
        // For password/token fields, send if:
        // 1. The value has changed from the old value (including clearing: non-empty -> empty), OR
        // 2. The old value is undefined/empty and new value is not empty (first time setting)
        if (stockKeyInput) {
            const newKey = stockKeyInput.value.trim();
            const oldKey = window.settings.p28;
            // Send if value changed, OR (old value is undefined/empty AND new value is not empty)
            if (hasValueChanged(newKey, oldKey) || (!oldKey && newKey !== '')) {
                formData.p28 = newKey;
                changedCount++;
            }
        }
        
        const stockRefreshInput = getFieldInForm('eeprom_stock_refresh_mins');
        if (stockRefreshInput && addIfChanged(formData, 'p29', parseInt(stockRefreshInput.value) || 5, window.settings.p29)) {
            changedCount++;
        }

        const dexcomRegionInput = getFieldInForm('eeprom_dexcom_region');
        if (dexcomRegionInput && addIfChanged(formData, 'p30', parseInt(dexcomRegionInput.value) || 0, window.settings.p30)) {
            changedCount++;
        }

        const libreRegionInput = getFieldInForm('eeprom_libre_region');
        if (libreRegionInput && addIfChanged(formData, 'p44', parseInt(libreRegionInput.value) || 0, window.settings.p44)) {
            changedCount++;
        }

        const nsUrlInput = getFieldInForm('eeprom_ns_url');
        if (nsUrlInput && addIfChanged(formData, 'p54', nsUrlInput.value.trim(), window.settings.p54 || '')) {
            changedCount++;
        }

        const glucoseHighInput = getFieldInForm('eeprom_glucose_high');
        if (glucoseHighInput && addIfChanged(formData, 'p51', parseInt(glucoseHighInput.value) || 175, window.settings.p51)) {
            changedCount++;
        }

        const glucoseLowInput = getFieldInForm('eeprom_glucose_low');
        if (glucoseLowInput && addIfChanged(formData, 'p52', parseInt(glucoseLowInput.value) || 70, window.settings.p52)) {
            changedCount++;
        }

        const glucoseRefreshInput = getFieldInForm('eeprom_glucose_refresh');
        if (glucoseRefreshInput && addIfChanged(formData, 'p33', parseInt(glucoseRefreshInput.value) || 5, window.settings.p33)) {
            changedCount++;
        }

        const glucoseUnitInput = getFieldInForm('eeprom_glucose_unit');
        if (glucoseUnitInput && addIfChanged(formData, 'p53', parseInt(glucoseUnitInput.value) || 0, window.settings.p53)) {
            changedCount++;
        }

        const glucoseValidityInput = getFieldInForm('glucose_validity_duration');
        if (glucoseValidityInput && addIfChanged(formData, 'p45', parseInt(glucoseValidityInput.value) || 30, window.settings.p45)) {
            changedCount++;
        }

        const glucoseUsernameInput = getFieldInForm('eeprom_glucose_username');
        if (glucoseUsernameInput) {
            const newUser = glucoseUsernameInput.value.trim();
            const oldUser = window.settings.p31;
            if (hasValueChanged(newUser, oldUser) || ((oldUser === undefined || oldUser === null) && newUser !== '')) {
                formData.p31 = newUser;
                changedCount++;
            }
        }

        const glucosePasswordInput = getFieldInForm('eeprom_glucose_password');
        if (glucosePasswordInput) {
            const passwordValue = glucosePasswordInput.value.trim();
            const oldPassword = window.settings.p32;
            // Send if value changed (including clearing: non-empty -> empty), OR (old value is undefined/empty AND new value is not empty)
            if (hasValueChanged(passwordValue, oldPassword) || (!oldPassword && passwordValue !== '')) {
                formData.p32 = passwordValue;
                changedCount++;
            }
        }
    }

    // Theme setting is global and should only be preserved if it was explicitly changed
    // Don't add it automatically - only if it was part of the form submission

    // Check if network settings have changed
    const networkSettingsChanged = 
        formData.p00 !== undefined ||
        formData.p34 !== undefined ||
        formData.p35 !== undefined ||
        formData.p60 !== undefined;


    // Save settings using shared function
    // Only network settings require restart notification
    saveSettings(formData, networkSettingsChanged)
        .finally(() => {
            toggleLoading(submitBtn, false);
        });
}

// Shared function to save settings
function saveSettings(formData, isNetworkSettings = false) {
    
    return fetch('/api/settings', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formData)
    })
    .then(async (response) => {
        const text = await response.text();
        let data = null;
        if (text) {
            try {
                data = JSON.parse(text);
            } catch (parseErr) {
                const preview = text.length > 160 ? text.slice(0, 160) + '…' : text;
                console.error('Non-JSON settings response:', preview);
                showStatus(getMessage('error_saving_settings') + preview, 'error');
                return;
            }
        }
        if (data && data.status === 'ok') {
            // Update window.settings with new values
            window.settings = { ...window.settings, ...formData };
            
            if (isNetworkSettings) {
                showStatus(getMessage('network_settings_changed'), 'success');
                let countdown = 5;
                const countdownInterval = setInterval(() => {
                    countdown--;
                    showStatus(getMessage('device_will_restart') + countdown + '...', 'success');
                    if (countdown <= 0) {
                        clearInterval(countdownInterval);
                        setTimeout(() => {
                            window.location.reload();
                        }, 2000);
                    }
                }, 1000);
            } else {
                showStatus(getMessage('settings_saved'), 'success');
            }
        } else {
            showStatus(getMessage('error_saving_settings') + (data && data.message ? data.message : 'Unknown error'), 'error');
        }
    })
    .catch(error => {
        console.error('Error:', error);
        showStatus(getMessage('error_saving_unknown'), 'error');
    });
}

// Settings section functionality
function setupSettingsSection() {
    // Setup event listeners only once
    if (!window.settingsEventListenersSet) {
        window.settingsEventListenersSet = true;
        
        // Setup WiFi scanning functionality
        const networkList = el('network-list');
        const loadingSpinner = el('loading-spinner');
        const loadingText = el('loading-text');
        const scanButton = el('scan-btn');

        if (networkList) {
            // Hide loading indicators by default
            if (loadingSpinner) loadingSpinner.style.display = 'none';
            if (loadingText) loadingText.style.display = 'none';
            
            // Add a placeholder message when no scanning is in progress
            const noScanMessage = document.createElement('div');
            noScanMessage.id = 'no-scan-message';
            noScanMessage.className = 'info-text';
            const trans = translations[currentLanguage] || translations.en;
            noScanMessage.textContent = getNestedTranslation(trans, 'settings.wifi.no_scan_message') || 'Click "Scan available networks" to see WiFi networks';
            networkList.appendChild(noScanMessage);
        }

        // Add event listener for scan button
        if (scanButton) {
            scanButton.addEventListener('click', startWifiScan);
        }

        // Handle form submission
        const settingsForm = el('settingsForm');
        if (settingsForm) {
            settingsForm.addEventListener('submit', (e) => handleFormSubmit(e, 'settingsForm'));
        }

        // Static IP toggle: show / hide the panel when the checkbox changes
        // When toggled ON and fields are empty, pre-fill with current DHCP values
        const useStaticIpEl = el('use_static_ip');
        const staticIpFields = el('static-ip-fields');
        if (useStaticIpEl && staticIpFields) {
            useStaticIpEl.addEventListener('change', function () {
                const show = this.checked;
                staticIpFields.style.display = show ? 'block' : 'none';
                if (show) {
                    const ipEl   = el('static_ip');
                    const gwEl   = el('static_gw');
                    const nmEl   = el('static_nm');
                    const dns1El = el('static_dns1');
                    const dns2El = el('static_dns2');
                    // Only pre-fill if all IP fields are empty (no previous config)
                    if (ipEl && !ipEl.value && gwEl && !gwEl.value) {
                        fetch('/api/status')
                            .then(r => r.json())
                            .then(status => {
                                if (ipEl   && !ipEl.value   && status.ip_address && status.ip_address !== '0.0.0.0') ipEl.value   = status.ip_address;
                                if (gwEl   && !gwEl.value   && status.ip_gw      && status.ip_gw      !== '0.0.0.0') gwEl.value   = status.ip_gw;
                                if (nmEl   && !nmEl.value)  nmEl.value   = (status.ip_nm && status.ip_nm !== '0.0.0.0') ? status.ip_nm : '255.255.255.0';
                                if (dns1El && !dns1El.value && status.ip_dns1    && status.ip_dns1 !== '0.0.0.0')     dns1El.value = status.ip_dns1;
                                if (dns2El && !dns2El.value && status.ip_dns2    && status.ip_dns2 !== '0.0.0.0')     dns2El.value = status.ip_dns2;
                            })
                            .catch(() => {
                                // Fallback: leave empty, user fills manually
                                if (nmEl && !nmEl.value) nmEl.value = '255.255.255.0';
                            });
                    }
                }
            });
        }
    }

    // Populate fields if settings are loaded
    if (window.settings && window.settingsLoaded.settings) {
        const hostnameEl = el('hostname');
        const wifiSsidEl = el('wifi_ssid');
        const wifiPassEl = el('wifi_pass');
        const fahrenheitEl = el('fahrenheit');
        const hour12El = el('hour12');
        const updateFirmwareEl = el('update_firmware');
        const rotationEl = el('rotation');
        const mirroringEl = el('mirroring');
        
        if (hostnameEl && window.settings.p00 !== undefined) hostnameEl.value = window.settings.p00;
        if (wifiSsidEl && window.settings.p34 !== undefined) wifiSsidEl.value = window.settings.p34;
        if (wifiPassEl && window.settings.p35 !== undefined) wifiPassEl.value = window.settings.p35;
        if (rotationEl && window.settings.p03 !== undefined) rotationEl.value = window.settings.p03 || 0;
        if (mirroringEl && window.settings.p09 !== undefined) mirroringEl.checked = window.settings.p09;
        if (fahrenheitEl && window.settings.p36 !== undefined) fahrenheitEl.checked = window.settings.p36;
        if (hour12El && window.settings.p37 !== undefined) hour12El.checked = window.settings.p37;
        if (updateFirmwareEl && window.settings.p39 !== undefined) updateFirmwareEl.checked = window.settings.p39;

        // Populate static IP fields
        const hasStaticIp = window.settings.p60 !== undefined && window.settings.p60 !== '';
        const useStaticIpEl2 = el('use_static_ip');
        const staticIpFields2 = el('static-ip-fields');
        if (useStaticIpEl2) {
            useStaticIpEl2.checked = hasStaticIp;
            if (staticIpFields2) staticIpFields2.style.display = hasStaticIp ? 'block' : 'none';
        }
        if (hasStaticIp) {
            const staticIpEl  = el('static_ip');
            const staticGwEl  = el('static_gw');
            const staticNmEl  = el('static_nm');
            const staticDns1El = el('static_dns1');
            const staticDns2El = el('static_dns2');
            if (staticIpEl  && window.settings.p60 !== undefined) staticIpEl.value  = window.settings.p60;
            if (staticGwEl  && window.settings.p61 !== undefined) staticGwEl.value  = window.settings.p61;
            if (staticNmEl) staticNmEl.value = (window.settings.p62 !== undefined && window.settings.p62 !== '')
                                               ? window.settings.p62 : '255.255.255.0';
            if (window.settings.p63 !== undefined && window.settings.p63 !== '') {
                const dnsParts = window.settings.p63.split(',');
                if (staticDns1El && dnsParts[0]) staticDns1El.value = dnsParts[0];
                if (staticDns2El && dnsParts[1]) staticDns2El.value = dnsParts[1];
            }
        }
    }
}

