// Language configuration
const LANGUAGES = ['en', 'de', 'fr', 'it', 'pt', 'sv', 'da', 'pl', 'es'];

const LANGUAGE_NAMES = {
    'en': 'English',
    'de': 'Deutsch',
    'fr': 'Français',
    'it': 'Italiano',
    'pt': 'Português',
    'sv': 'Svenska',
    'da': 'Dansk',
    'pl': 'Polski',
    'es': 'Español'
};

// DOM Element Cache to minimize redundant lookups
const domCache = new Map();

/**
 * Helper for element selection with O(1) caching
 * @param {string} id - The ID of the element to retrieve
 * @returns {HTMLElement|null}
 */
const el = (id) => {
    if (!id) return null;
    let element = domCache.get(id);
    if (!element || !element.isConnected) {
        element = document.getElementById(id);
        if (element) domCache.set(id, element);
    }
    return element;
};

// Translation element caches to minimize global DOM queries during language switches
let i18nElementsCache = null;
let passwordTogglesCache = null;
let tokenCodesCache = null;
let languageOptionsCache = null;

// Helper to invalidate I18n element caches when dynamic content is added or replaced
function invalidateI18nCache() {
    i18nElementsCache = null;
    passwordTogglesCache = null;
    tokenCodesCache = null;
    languageOptionsCache = null;
}

// Helper to highlight an element (visual feedback for programmatic updates)
function highlightElement(element) {
    if (!element) return;
    element.classList.remove('input-highlight');
    void element.offsetWidth; // Force reflow
    element.classList.add('input-highlight');
}

// Helper for updating character counter visual state
function updateCharCounter(input, counter) {
    if (!input || !counter) return;
    const length = input.value.length;
    const maxLength = input.getAttribute('maxlength') || 511;
    counter.textContent = `${length} / ${maxLength}`;

    counter.classList.toggle('near-limit', length >= 450 && length < maxLength);
    counter.classList.toggle('at-limit', length >= maxLength);
}

// Helper for toggling button loading state
function toggleLoading(btn, isLoading) {
    if (!btn) return;
    if (isLoading) {
        btn.classList.add('is-loading');
        btn.disabled = true;
        btn.setAttribute('aria-busy', 'true');
    } else {
        btn.classList.remove('is-loading');
        btn.disabled = false;
        btn.removeAttribute('aria-busy');
    }
}

// Translations object
const translations = {
    en: {
        menu: {
            settings: 'Settings',
            advanced: 'Advanced',
            screen: 'Layout',
            integrations: 'Integrations',
            status: 'Status',
            files: 'Files',
            update: 'Update',
            restart: 'Restart'
        },
        common: {
            save_settings: 'Save Settings',
            cancel: 'Cancel',
            show_password: 'Show password',
            hide_password: 'Hide password',
            change_language: 'Change language',
            toggle_theme: 'Toggle theme',
            insert: 'Insert'
        },
        screen: {
            title: 'Layout',
            description: 'Place elements on the 128x128 display. Elements can overlap.',
            day: 'Day',
            night: 'Night',
            save: 'Save',
            hint: ' Drag items from the palette onto the grid.',
            palette: 'Palette',
            options: 'Options',
            enabled: 'Enabled',
            message_font: 'Scrolling message font',
            restore_defaults: 'Restore Defaults',
            restore_default: 'Restore Default',
            layout_file_placeholder: 'Select a layout file',
            load_system_layout: 'Load system layout',
            copy_to_day: 'Copy layout to Day',
            copy_to_night: 'Copy layout to Night',
            save_to_file: 'Save to File',
            read_from_file: 'Read from File'
        },
        settings: {
            connection: {
                title: 'Connection Settings *',
                wifi_ssid: 'WiFi SSID',
                wifi_password: 'WiFi Password',
                hostname: 'Hostname',
                fahrenheit: 'US units (F, mph, ...)',
                hour12: 'Time in 12-hour format',
                auto_update: 'Automatically check for firmware updates',
                restart_note: '* Changing connection settings will restart the device.'
            },
            wifi: {
                available_networks: 'Available WiFi Networks',
                scanning: 'Scanning for networks...',
                scan_button: 'Scan available networks',
                no_scan_message: 'Click "Scan available networks" to see WiFi networks',
                no_networks: 'No networks found',
                signal: 'Signal strength',
                secure: 'Secure',
                open: 'Open'
            }
        },
        advanced: {
            display: {
                title: 'Display Settings',
                x_offset: 'X Offset',
                y_offset: 'Y Offset',
                rotation: 'Display Rotation',
                show_scroll: 'Show scrolling information message',
                show_grid: 'Show grid',
                mirror: 'Mirror Display'
            },
            day: {
                title: 'Day Settings',
                font: 'Font',
                color_filter: 'Color Filter',
                message_color: 'Message Color'
            },
            night: {
                title: 'Night Settings',
                font: 'Font',
                color_filter: 'Color Filter',
                message_color: 'Message Color'
            },
            color_filter: {
                none: 'None',
                red: 'Red',
                green: 'Green',
                blue: 'Blue',
                bw: 'Black & White'
            },
            fonts: {
                title: 'Font Samples'
            },
            location: {
                title: 'Location Settings *',
                latitude: 'Latitude',
                longitude: 'Longitude',
                timezone: 'Timezone',
                timezone_placeholder: 'Enter Timezone in POSIX format ONLY if automatic detection doesn\'t work',
                help_text: 'You can find a list of POSIX timezones <a href="https://leo.leung.xyz/wiki/Timezone" target="_blank">here</a>. Lat/Lon only affect weather data.',
                restart_note: '* Changing location settings will restart the device.'
            },
            wifi_hours: {
                title: 'WiFi Active Hours',
                start: 'Start',
                end: 'End'
            },
            brightness: {
                title: 'Projection brightness settings',
                dimming_selection: 'Dimming selection',
                dimming_mode: 'Dimming mode',
                dim_mode_full: 'Always full brightness',
                dim_mode_brightness: 'Based on brightness',
                dim_mode_time: 'Based on time-of-day',
                brightness_dimming: 'Brightness-based dimming',
                timeofday_dimming: 'Time-of-day dimming',
                dim_start: 'Start',
                dim_end: 'End',
                day_threshold: 'Day Threshold (lux)',
                light_sensitivity: 'Sensitivity (lux)',
                sensitivity_help: 'Sensitivity helps avoid unwanted brightness changes. Dark = Threshold - Sensitivity, Bright = Threshold + Sensitivity.',
                led_levels: 'LED Brightness Levels (%)',
                led_day: 'Brightness (Day)',
                led_night: 'Brightness (Night)',
                pwm_frequency: 'PWM Frequency',
                max_power: 'Max Power'
            },
            message: {
                title: 'Scrolling Message',
                font_size: 'Font Size',
                scroll_delay: 'Scroll Delay',
                scroll_delay_help: 'Animation delay in milliseconds (30-255)',
                message: 'Message',
                message_placeholder: 'Enter your message',
                tokens_title: 'Available Tokens:',
                token_device: 'Device name',
                token_greeting: 'Time-based greeting (Good Morning/Afternoon/Evening/Night)',
                token_day: 'Current day of the week',
                token_date: 'Current date',
                token_mon: 'Current month',
                token_temp: 'Current temperature',
                token_hum: 'Current humidity',
                token_high: 'Today\'s high temperature',
                token_low: 'Today\'s low temperature',
                token_rise: 'Sunrise time',
                token_set: 'Sunset time',
                token_wind: 'Wind speed and direction',
                token_gust: 'Wind gust speed and direction',
                token_precip: 'Precipitation amount and probability',
                token_uv: 'UV index',
                token_pressure: 'Sea-level pressure with trend',
                token_3high: 'Highest temperature across 3 days',
                token_3low: 'Lowest temperature across 3 days',
                token_ha: 'Fetch settings from Home Assistant (needs enabled integration)',
                token_stock: 'Fetch stock price quote (needs enabled stock integration with Finnhub.io)',
                token_cgm_glucose: 'Latest glucose reading from your CGM device, formatted, e.g. 157 mg/dl (+3 @ 12:01). (needs active CGM integration)',
                token_cgm_reading: 'Latest glucose reading, plain, e.g. 157',
                token_cgm_time: 'Time of latest CGM measurement, e.g. 12:01pm (needs active CGM integration)',
                tokens_note: 'Note: Tokens are case-sensitive and must be enclosed in square brackets.',
                default_message: 'Default message: [device].local/ [greeting] [day], [date] [mon], now [temp] today [high]-[low], hum. [hum], sun [rise]-[set]'
            }
        },
        status: {
            support: {
                title: 'Share information with support',
                send_email: 'Send email',
                copy_clipboard: 'Copy to clipboard',
                help_text: 'If you need any support, please share the information above with us. Click "send email" to send us a message; if that doesn\'t work, copy the information to your clipboard and send it via email to support@buyfrixos.com.'
            },
            sensor: {
                title: 'Sensor Data',
                light_level: 'Current Light Level (lux)',
                last_time_update: 'Last Time Update',
                timezone: 'Timezone',
                moon_phase: 'Moon Phase',
                last_weather_update: 'Last Weather Update',
                latitude: 'Latitude',
                longitude: 'Longitude',
                uptime: 'Uptime'
            },
            system: {
                title: 'System Information',
                app_name: 'Application Name',
                version: 'Version',
                fw_version: 'Firmware Version',
                power_on_hours: 'Power On Hours',
                mac_address: 'MAC Address',
                ip_address: 'IP Address',
                chip_revision: 'Chip Revision',
                flash_size: 'SPI Flash Size',
                cpu_freq: 'CPU Frequency',
                compile_time: 'Compile Time',
                free_heap: 'Free Heap Memory',
                min_free_heap: 'Min Free Heap'
            },
            logs: {
                title: 'System Logs'
            },
            integration_status: {
                title: 'Integration Status'
            },
            refresh_button: 'Refresh Status'
        },
        integrations: {
            ha: {
                title: 'Home Assistant',
                server_url: 'Server URL',
                token: 'LL Access Token',
                token_placeholder: 'Enter your token',
                refresh_interval: 'Refresh (min.)'
            },
            stock: {
                title: 'Stock Quote Service (via Finnhub.io)',
                api_key: 'API Key',
                api_key_placeholder: 'Enter your Finnhub API key',
                refresh_interval: 'Refresh (min.)'
            },
            dexcom: {
                title: 'Dexcom G5/6',
                region: 'Region',
                disabled: 'Disabled',
                japan: 'Japan',
                rest_of_world: 'Rest of World',
                username: 'Username',
                username_placeholder: 'Enter your Dexcom username',
                password: 'Password',
                password_placeholder: 'Enter your Dexcom password',
                refresh_interval: 'Refresh (min.)',
                high_threshold: 'High Glucose Threshold (mg/dL)',
                low_threshold: 'Low Glucose Threshold (mg/dL)'
            },
            glucose_monitors: {
                title: 'Glucose Monitoring',
                high_threshold: 'High (mg/dL)',
                low_threshold: 'Low (mg/dL)',
                refresh_interval: 'Refresh (min.)',
                validity_duration: 'Validity (min.)',
                username: 'Username',
                username_placeholder: 'Enter your username',
                password: 'Password',
                password_placeholder: 'Enter your password',
                cgm_device_title: 'CGM Device'
            }
        },
        update: {
            refresh_title: 'Refresh',
            refresh_description: 'This downloads and installs again the current version of the firmware and files (spiffs).',
            reinstall_button: 'Reinstall current firmware',
            title: 'Upload Firmware / Files',
            auto_check: 'Your device checks and installs new firmware automatically.',
            manual_info: 'If you have been instructed by our support to manually install a specific firmware version, or if you are looking to upload e.g. a custom font, you can do this here.',
            select_file: 'Select a firmware file (.bin) or other file to upload to the device.',
            upload_button: 'Upload File',
            firmware_instructions_title: 'Firmware Update Instructions',
            firmware_step1: 'Click the "Choose File" button and select the firmware file (.bin) from your computer.',
            firmware_step2: 'Click the "Upload Firmware" button to start the update process.',
            firmware_step3: 'Wait for the update to complete. Do not power off or disconnect the device during the update.',
            firmware_step4: 'The device will automatically restart after the update is complete.',
            firmware_warning: '<strong>Warning:</strong> Uploading incorrect firmware may cause the device to malfunction. Make sure to use the correct firmware for your device.',
            file_instructions_title: 'User file upload instructions',
            file_step1: 'Click the "Choose File" button and select the file (e.g. user-font.jpg) from your computer.',
            file_step2: 'Click the "Upload Firmware" button to start the upload process.',
            file_step3: 'Wait for the upload to complete. Do not power off or disconnect the device during the upload.',
            file_step4: 'You can now upload another file or close this window.'
        },
        files: {
            title: 'SPIFFS Files',
            description: 'Manage files stored on the device SPIFFS filesystem.',
            refresh: 'Refresh',
            rename: 'Rename',
            delete: 'Delete Selected',
            filename: 'Filename',
            size: 'Size',
            not_loaded: 'Open this tab to load files.',
            loading: 'Loading files...',
            empty: 'No files found',
            count: ' files',
            selected: ' selected',
            select_all: 'Select all files',
            rename_prompt: 'Enter a new filename for ',
            delete_confirm: 'Delete selected files?',
            only_one_rename: 'Select exactly one file to rename'
        },
        restart: {
            title: 'Device Reset',
            description: 'Use this option to restart your Frixos device.',
            button: 'Restart Device',
            confirm_title: 'Confirm Restart',
            confirm_message: 'Are you sure you want to restart the device?',
            unavailable_message: 'The device will become temporarily unavailable during restart.',
            confirm_button: 'Restart'
        },
        messages: {
            error_loading_settings: 'Error loading settings',
            sensor_data_refreshed: 'Sensor data refreshed',
            failed_refresh_sensor: 'Failed to refresh sensor data',
            correct_form_errors: 'Please correct the highlighted fields before submitting',
            invalid_hostname: 'Invalid hostname: ',
            saving_settings: 'Saving settings...',
            network_settings_changed: 'Network settings changed. Device will restart...',
            device_will_restart: 'Device will restart in ',
            settings_saved: 'Settings saved successfully',
            error_saving_settings: 'Error saving settings: ',
            error_saving_unknown: 'Error saving settings. Check console for details.',
            failed_fetch_status: 'Failed to fetch status data',
            select_firmware_file: 'Please select a firmware file or other file to upload',
            uploading_file: 'Uploading file... Do not power off the device.',
            firmware_update_success: 'Firmware update successful! Device is rebooting...',
            file_upload_success: 'File upload successful!',
            update_failed: 'Update failed: ',
            invalid_response: 'Invalid response from server',
            upload_failed_status: 'Upload failed with status: ',
            network_error_upload: 'Network error during upload',
            upload_aborted: 'Upload aborted',
            reinstall_sending: 'Starting reinstall...',
            reinstall_started: 'Reinstall started. Do not power off the device.',
            reinstall_failed: 'Failed to start reinstall: ',
            reinstall_wifi_required: 'WiFi must be connected to reinstall firmware',
            reinstall_in_progress: 'An update is already in progress',
            sending_reset: 'Sending reset command...',
            device_restarting: 'Device is restarting. This page will refresh in ',
            device_restarting_seconds: ' seconds.',
            failed_restart: 'Failed to restart device: ',
            error_reset_connection: 'Error sending reset command: Connection lost',
            correct_form_before_save: 'Please correct the form errors before saving.',
            dexcom_credentials_required: 'Username and password are required when enabled',
            dexcom_refresh_range: 'Dexcom refresh interval must be between 1 and 60 minutes',
            glucose_high_range: 'High glucose threshold must be between 1 and 400 mg/dL',
            glucose_low_range: 'Low glucose threshold must be between 1 and 400 mg/dL',
            glucose_low_less_than_high: 'Low glucose threshold must be less than high threshold',
            cgm_only_one: 'Only one CGM source (Dexcom, FreeStyle Libre, or Nightscout URL) can be enabled at a time.',
            error_preparing_info: 'Error preparing system information: ',
            info_copied_clipboard: 'System information copied to clipboard',
            failed_copy_clipboard: 'Failed to copy to clipboard',
            failed_fetch_files: 'Failed to fetch files',
            files_deleted: 'Files deleted',
            failed_delete_files: 'Failed to delete files: ',
            file_renamed: 'File renamed',
            failed_rename_file: 'Failed to rename file: ',
            layout_saved_to_file: 'Layout saved to file',
            layout_read_from_file: 'Layout loaded from file. Click Save to apply to the device.',
            layout_read_invalid: 'Invalid layout file',
            layout_system_loaded: 'System layout loaded. Click Save to apply to the device.',
            layout_system_select_file: 'Select a layout file first'
        }
    }
};

// Translation loading cache - tracks which translations have been loaded
const translationsLoaded = {};

function deepMergeTranslations(target, source) {
    if (!source || typeof source !== 'object') return target;
    Object.keys(source).forEach(key => {
        const value = source[key];
        if (value && typeof value === 'object' && !Array.isArray(value)) {
            if (!target[key] || typeof target[key] !== 'object') target[key] = {};
            deepMergeTranslations(target[key], value);
        } else {
            target[key] = value;
        }
    });
    return target;
}

// Async function to load translations for a specific language from JSON file
async function loadTranslations(lang) {
    if (translationsLoaded[lang]) return;
    if (window._translationPromises?.[lang]) return window._translationPromises[lang];
    if (!window._translationPromises) window._translationPromises = {};

    return window._translationPromises[lang] = fetch(`/language_${lang}.json`)
        .then(res => res.ok ? res.json() : Promise.reject())
        .then(data => {
            if (lang === 'en') {
                deepMergeTranslations(translations.en, data);
            } else {
                translations[lang] = data;
            }
        })
        .catch(() => {
            if (lang !== 'en') translations[lang] = translations.en;
        })
        .finally(() => {
            translationsLoaded[lang] = true;
            delete window._translationPromises[lang];
        });
}

// Helper function to get nested translation
// Optimization: Cache split paths to avoid redundant string operations for 180+ translatable elements
const pathCache = new Map();
function getNestedTranslation(obj, path) {
    let parts = pathCache.get(path);
    if (!parts) {
        parts = path.split('.');
        pathCache.set(path, parts);
    }
    let current = obj;
    for (let i = 0; i < parts.length; i++) {
        current = current && current[parts[i]];
        if (!current) break;
    }
    return current;
}

// Helper function to get translated message
function getMessage(key, ...args) {
    const message = getNestedTranslation(translations[currentLanguage], `messages.${key}`);
    if (!message) {
        console.warn(`Message key not found: ${key}`);
        return getNestedTranslation(translations.en, `messages.${key}`) || key;
    }
    
    // Handle messages with placeholders
    if (args.length > 0) {
        return message + args.join('');
    }
    return message;
}

// CGM mutual exclusivity: only one of Dexcom, FreeStyle Libre, or Nightscout URL can be enabled
function updateCgmExclusivity() {
    const dexcom = el('eeprom_dexcom_region');
    const libre = el('eeprom_libre_region');
    const nsUrl = el('eeprom_ns_url');
    if (!dexcom || !libre || !nsUrl) return;
    const dexcomEnabled = dexcom.value !== '0';
    const libreEnabled = libre.value !== '0';
    const nsEnabled = nsUrl.value.trim() !== '';
    libre.disabled = dexcomEnabled || nsEnabled;
    nsUrl.disabled = dexcomEnabled || libreEnabled;
    dexcom.disabled = libreEnabled || nsEnabled;
}

// Current language (will be loaded from NVS)
let currentLanguage = '';

// Translation function - now async to support lazy-loading
// Optimization: Persistent early return and DOM mutation diffing to minimize overhead
async function translate(lang) {
    await loadTranslations(lang);
    
    const effectiveLang = (lang !== 'en' && translations[lang] === translations.en) ? 'en' : lang;

    // Early return if same language already applied
    if (currentLanguage === effectiveLang) return;

    currentLanguage = effectiveLang;
    const trans = translations[effectiveLang];

    // Optimization: Use cached elements if available, otherwise query and cache them
    if (!i18nElementsCache) {
        i18nElementsCache = document.querySelectorAll('[data-i18n], [data-i18n-placeholder], [data-i18n-aria-label]');
    }
    i18nElementsCache.forEach(element => {
        const i18nKey = element.dataset.i18n;
        const i18nPlaceholderKey = element.dataset.i18nPlaceholder;
        const i18nAriaLabelKey = element.dataset.i18nAriaLabel;

        if (i18nKey) {
            const translation = getNestedTranslation(trans, i18nKey);
            // Optimization: Only update DOM if content actually changed to avoid layout thrashing
            if (translation && element.innerHTML !== translation) {
                element.innerHTML = translation;
            }
        }

        if (i18nPlaceholderKey) {
            const translation = getNestedTranslation(trans, i18nPlaceholderKey);
            // Optimization: Only update DOM if placeholder actually changed
            if (translation && element.placeholder !== translation) {
                element.placeholder = translation;
            }
        }

        if (i18nAriaLabelKey) {
            const translation = getNestedTranslation(trans, i18nAriaLabelKey);
            if (translation && element.getAttribute('aria-label') !== translation) {
                element.setAttribute('aria-label', translation);
            }
        }
    });

    // Update password toggle ARIA labels for accessibility after language change
    if (!passwordTogglesCache) {
        passwordTogglesCache = document.querySelectorAll('.password-toggle');
    }
    passwordTogglesCache.forEach(button => {
        const input = button.previousElementSibling;
        if (input) {
            const isPassword = input.type === 'password';
            const actionKey = isPassword ? 'common.show_password' : 'common.hide_password';
            const translation = getNestedTranslation(trans, actionKey);
            if (translation) {
                button.setAttribute('aria-label', translation);
            }
        }
    });

    // Update token tooltips and ARIA labels after language change
    tokenCodesCache = null;
    document.querySelectorAll('.token-code').forEach(token => updateTokenButtonTooltip(token));

    const nameElement = el('current-language-name');
    if (nameElement) nameElement.textContent = LANGUAGE_NAMES[effectiveLang] || LANGUAGE_NAMES['en'];

    // Update language selection state in dropdown
    if (!languageOptionsCache) {
        languageOptionsCache = document.querySelectorAll('.language-option');
    }
    languageOptionsCache.forEach(option => {
        const isSelected = option.getAttribute('data-lang') === effectiveLang;
        option.classList.toggle('is-active', isSelected);
        option.setAttribute('aria-selected', isSelected.toString());
    });

        const hash = window.location.hash.substring(1);
    if (hash && hash !== 'settings') {
        const sectionName = hash.charAt(0).toUpperCase() + hash.slice(1);
        const translatedSection = getNestedTranslation(trans, `menu.${hash}`) || sectionName;
        const pageTitleElement = el('page-title');
        if (pageTitleElement) pageTitleElement.textContent = 'Frixos - ' + translatedSection;
    }

    refreshScreenEditorI18n();
}

// Setup password visibility toggles
function setupPasswordToggles() {
    document.querySelectorAll('.password-toggle').forEach(button => {
        button.addEventListener('click', function() {
            const input = this.previousElementSibling;
            if (!input) return;
            const isPassword = input.type === 'password';
            input.type = isPassword ? 'text' : 'password';
            this.textContent = isPassword ? '🙈' : '👁️';

            // Localized ARIA label
            const trans = translations[currentLanguage] || translations.en;
            const actionKey = isPassword ? 'common.hide_password' : 'common.show_password';
            const translation = getNestedTranslation(trans, actionKey);
            if (translation) {
                this.setAttribute('aria-label', translation);
            }

            input.focus();
        });
    });
}

// Setup language selector
function setupLanguageSelector() {
    const languageToggle = el('language-toggle');
    const languageDropdown = el('language-dropdown');
    
    // Toggle dropdown on button click
    languageToggle.addEventListener('click', function(e) {
        e.stopPropagation();
        const isExpanded = languageDropdown.style.display !== 'none';
        languageDropdown.style.display = isExpanded ? 'none' : 'block';
        languageToggle.setAttribute('aria-expanded', (!isExpanded).toString());
    });
    
    // Close dropdown when clicking outside
    document.addEventListener('click', function(e) {
        if (!languageToggle.contains(e.target) && !languageDropdown.contains(e.target)) {
            languageDropdown.style.display = 'none';
            languageToggle.setAttribute('aria-expanded', 'false');
        }
    });
    
    // Handle language selection
    document.querySelectorAll('.language-option').forEach(option => {
        const select = function() {
            const selectedLang = option.getAttribute('data-lang');
            changeLanguage(selectedLang);
            languageDropdown.style.display = 'none';
            languageToggle.setAttribute('aria-expanded', 'false');
            languageToggle.focus();
        };

        option.addEventListener('click', select);
        option.addEventListener('keydown', function(e) {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                select();
            }
        });
    });
}

// Change language function
async function changeLanguage(lang) {
    if (!LANGUAGES.includes(lang)) {
        console.error(`Invalid language: ${lang}`);
        return;
    }
    
    const languageToggle = el('language-toggle');
    toggleLoading(languageToggle, true);
    
    try {
        // Apply translation (now async to support lazy-loading)
        await translate(lang);

        // Save to NVS via API
        const languageIndex = LANGUAGES.indexOf(lang);
        await fetch('/api/settings', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                p41: languageIndex
            })
        })
        .then(response => response.json())
        .then(data => {
            if (!data || data.status !== 'ok') {
                console.error('Error saving language preference:', data);
            }
        });
    } catch (error) {
        console.error('Error in changeLanguage:', error);
    } finally {
        toggleLoading(languageToggle, false);
    }
}

// Track which sections have been initialized and which parameters have been loaded
window.sectionsInitialized = {
    settings: false,
    advanced: false,
    screen: false,
    integrations: false,
    status: false,
    files: false,
    update: false,
    restart: false
};

window.filesLoaded = false;
window.spiffsFiles = [];
window.selectedFiles = new Set();
window.filesSort = { key: 'name', direction: 'asc' };

window.settings = {}; // Will be populated as sections are loaded
window.settingsLoaded = {
    theme: false,      // p40, p41 for theme and language
    settings: false,   // p00, p34-p39 for settings section
    advanced: false,   // p01-p24, p42, p43 for advanced section
    integrations: false // p25-p33 for integrations section
};

// Global navigation state to optimize section switching (avoiding O(N) DOM lookups)
let activeSection = null;
let activeMenuItem = null;
const menuItemsMap = new Map();

// Function to fetch minimal parameters for theme and language
async function fetchThemeParams() {
    if (window.settingsLoaded.theme) {
        return window.settings;
    }

    // Use query parameter to fetch only theme-related parameters
    try {
        const response = await fetch('/api/settings?group=theme');
        const data = await response.json();
        
        // Store theme-related parameters
        Object.keys(data).forEach(key => {
            window.settings[key] = data[key];
        });
        window.settingsLoaded.theme = true;

        // Initialize theme using the settings data
        initTheme(data);

        // Load and apply language preference
        const languageIndex = data.p41 !== undefined ? data.p41 : 0;
        const selectedLang = LANGUAGES[languageIndex] || 'en';
        await translate(selectedLang);

        return data;
    } catch (error) {
        console.error('Error loading theme parameters:', error);
        showStatus(getMessage('error_loading_settings'), 'error');
        throw error;
    }
}
// Function to fetch parameters for a specific section
function fetchSectionParams(sectionName) {
    const sectionMap = {
        settings: 'settings',
        advanced: 'advanced',
        integrations: 'integrations'
    };
    
    const mappedSection = sectionMap[sectionName];
    if (!mappedSection) {
        return window.settings;
    }
    
    // Check if already loaded for this section
    if (window.settingsLoaded[mappedSection]) {
        return window.settings;
    }
    
    // Need to fetch parameters (first time a section is shown)
    // Use query parameter to fetch only the parameters needed for this section
    return fetch(`/api/settings?group=${mappedSection}`)
        .then(response => response.json())
        .then(data => {
            // Merge fetched parameters into window.settings
            Object.keys(data).forEach(key => {
                window.settings[key] = data[key];
            });
            window.settingsLoaded[mappedSection] = true;
            return data;
        })
        .catch(error => {
            console.error(`Error loading parameters for ${sectionName} section:`, error);
            showStatus(getMessage('error_loading_settings'), 'error');
            throw error;
        });
}

document.addEventListener('DOMContentLoaded', function() {
    // Load theme and settings params together (settings is default page, so preload it)
    // Other sections (advanced, integrations) load on demand when navigated to
    Promise.all([fetchThemeParams(), fetchSectionParams('settings')])
        .then(() => {
            setupPasswordToggles();

            // Setup language selector
            setupLanguageSelector();
            
            // Setup navigation
            setupNavigation();
            
            // Setup event listeners for sections
            // Settings params already loaded above; advanced/integrations load on demand
            setupSettingsSection();
            window.sectionsInitialized.settings = true;
            
            // Initialize the current section based on URL hash (settings already loaded)
            navigateToSection();
            
            // Setup hostname validation
            setupHostnameValidation();
            
            // Setup additional field validations
            setupFieldValidations();
            
            // Initialize accessibility
            initA11y();

            setupAdvancedSection();
            setupStatusSection();
            setupFilesSection();
            setupUpdateSection();
            setupRestartSection();
            setupIntegrationsSection();
            setupScreenSection();

            // Setup send to support and copy buttons
            setupSupportButtons();
        })
        .catch(error => {
            console.error('Error during initialization:', error);
        });

    // Add refresh sensor button handler
    const refreshSensorButton = el('refreshSensorButton');
    if (refreshSensorButton) {
        refreshSensorButton.addEventListener('click', function() {
            fetchStatus(false)
                .then(data => {
                    // Update sensor data
                    el('lux').textContent = data.lux !== undefined ? data.lux.toFixed(1) : '-';
                    el('lux_sensitivity_val').textContent = data.lux_sensitivity !== undefined ? `${data.lux_sensitivity} lux` : '-';
                    el('lux_threshold_val').textContent = data.lux_threshold !== undefined ? `${data.lux_threshold} lux` : '-';
                    if (data.uptime !== undefined) {
                        el('uptime').textContent = formatUptime(data.uptime);
                    } else {
                        el('uptime').textContent = '-';
                    }
                    showStatus(getMessage('sensor_data_refreshed'), 'success');
                })
                .catch(error => {
                    console.error('Error refreshing sensor data:', error);
                    showStatus(getMessage('failed_refresh_sensor'), 'error');
                });
        });
    }

    // Remove all section toggle buttons
    const sectionToggles = document.querySelectorAll('.section-toggle');
    sectionToggles.forEach(toggle => {
        toggle.remove();
    });

    // Remove all section headers click handlers
    const sectionHeaders = document.querySelectorAll('.section-header');
    sectionHeaders.forEach(header => {
        header.style.cursor = 'default';
    });
});

// Theme management functions
function initTheme(settings) {
    // If eeprom_dark_theme is not undefined, set the theme based on that value
    if (settings.eeprom_dark_theme !== undefined) {
        const isDarkTheme = !!settings.eeprom_dark_theme;
        if (isDarkTheme) {
            document.body.classList.remove('light-theme');
        } else {
            document.body.classList.add('light-theme');
        }
    } else {
        // Default to dark theme if preference is not set
        document.body.classList.remove('light-theme');
    }
    
    // Add event listener to theme toggle button
    const themeToggle = el('theme-toggle');
    if (themeToggle) {
        themeToggle.addEventListener('click', toggleTheme);
    }
}

function toggleTheme() {
    const isDarkTheme = !document.body.classList.contains('light-theme');
    
    if (isDarkTheme) {
        document.body.classList.add('light-theme');
    } else {
        document.body.classList.remove('light-theme');
    }
    
    // Save theme preference to NVS via server API
    fetch('/api/settings', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            eeprom_dark_theme: !isDarkTheme ? 1 : 0
        })
    })
    .then(response => response.json())
    .then(data => {
        if (!data || data.status !== 'ok') {
            console.error('Error saving theme preference:', data);
        }
    })
    .catch(error => {
        console.error('Error saving theme preference:', error);
    });
}

// Navigation setup
function setupNavigation() {
    // Get all menu items
    const menuItems = document.querySelectorAll('.menu-item');
    
    // Add click event listeners to menu items and cache them in Map for O(1) access
    menuItems.forEach(item => {
        const href = item.getAttribute('href');
        menuItemsMap.set(href, item);

        item.addEventListener('click', function(e) {
            e.preventDefault();
            const sectionId = href.substring(1);
            window.location.hash = sectionId;
        });
    });
    
    // Listen for hash changes in URL
    window.addEventListener('hashchange', navigateToSection);
}

// Display section based on URL hash
function navigateToSection() {
    let hash = window.location.hash.substring(1);
    
    // Default to settings if no hash
    if (!hash) {
        hash = 'settings';
        window.location.hash = hash;
    }
    
    // Optimization: Hide only the previously active section instead of O(N) iteration
    if (activeSection) {
        activeSection.style.display = 'none';
    } else {
        // Fallback for first load if state isn't set yet (still faster than global query)
        document.querySelectorAll('.page-section').forEach(s => s.style.display = 'none');
    }
    
    // Show the selected section
    const currentSection = el(hash + '-section');
    if (currentSection) {
        currentSection.style.display = 'block';
        activeSection = currentSection;
        
        // Update page title
        const sectionName = hash.charAt(0).toUpperCase() + hash.slice(1);
        el('page-title').textContent = 'Frixos - ' + sectionName;
        document.title = 'Frixos - ' + sectionName;
        
        // Optimization: Update only previous and current menu items instead of O(N) iteration
        if (activeMenuItem) {
            activeMenuItem.classList.remove('active');
            activeMenuItem.removeAttribute('aria-current');
        } else {
            // Fallback for first load to ensure no other item is active
            document.querySelectorAll('.menu-item').forEach(item => {
                item.classList.remove('active');
                item.removeAttribute('aria-current');
            });
        }

        const nextMenuItem = menuItemsMap.get('#' + hash);
        if (nextMenuItem) {
            nextMenuItem.classList.add('active');
            nextMenuItem.setAttribute('aria-current', 'page');
            activeMenuItem = nextMenuItem;
        }
        
        // Load parameters for the section if not already loaded
        if (hash === 'settings' && !window.sectionsInitialized.settings) {
            fetchSectionParams('settings')
                .then(data => {
                    setupSettingsSection();
                    window.sectionsInitialized.settings = true;
                })
                .catch(error => {
                    console.error('Error loading settings section:', error);
                });
        } else if (hash === 'advanced' && !window.sectionsInitialized.advanced) {
            fetchSectionParams('advanced')
                .then(data => {
                    setupAdvancedSection();
                    window.sectionsInitialized.advanced = true;
                })
                .catch(error => {
                    console.error('Error loading advanced section:', error);
                });
        } else if (hash === 'integrations' && !window.sectionsInitialized.integrations) {
            const integrationsFormEarly = el('integrationsForm');
            const submitWhileLoading = integrationsFormEarly && integrationsFormEarly.querySelector('button[type="submit"]');
            if (submitWhileLoading) submitWhileLoading.disabled = true;
            fetchSectionParams('integrations')
                .then(data => {
                    setupIntegrationsSection();
                    window.sectionsInitialized.integrations = true;
                })
                .catch(error => {
                    console.error('Error loading integrations section:', error);
                });
        } else if (hash === 'status' && !window.sectionsInitialized.status) {
            // Status section uses /api/status, not /api/settings
            fetchStatus(true);
            window.sectionsInitialized.status = true;
        } else if (hash === 'screen' && !window.sectionsInitialized.screen) {
            setupScreenSection();
            fetchScreenLayout();
            window.sectionsInitialized.screen = true;
        } else if (hash === 'files') {
            setupFilesSection();
            fetchFiles();
            window.sectionsInitialized.files = true;
        } else if (hash === 'settings' && window.sectionsInitialized.settings) {
            // Re-initialize with already loaded data
            setupSettingsSection();
        } else if (hash === 'advanced' && window.sectionsInitialized.advanced) {
            // Re-initialize with already loaded data
            setupAdvancedSection();
        } else if (hash === 'integrations' && window.sectionsInitialized.integrations) {
            // Re-initialize with already loaded data
            setupIntegrationsSection();
        } else if (hash === 'status' && window.sectionsInitialized.status) {
            // Re-fetch status data when returning to status page
            fetchStatus(true);
        } else if (hash === 'screen' && window.sectionsInitialized.screen) {
            setupScreenSection();
            refreshScreenLayoutSelect();
        }
    }
}

// Function to show status messages
function showStatus(message, type) {
    const statusElem = el('status-message');
    if (!statusElem) {
        console.error('Status message element not found!');
        return;
    }
    
    statusElem.textContent = message;
    statusElem.className = 'status ' + type;
    statusElem.style.display = 'block';

    // Clear any existing timeout
    if (window.statusTimeout) {
        clearTimeout(window.statusTimeout);
    }

    // Hide the message after 5 seconds (increased from 3 seconds)
    window.statusTimeout = setTimeout(function () {
        statusElem.style.display = 'none';
    }, 5000);
}

// Function to toggle collapsible sections
function toggleSection(header) {
    const section = header.closest('.collapsible');
    section.classList.toggle('collapsed');
}

// Function to initialize accessibility attributes
function initA11y() {
    // Link inputs to error labels, counters, and token masks via aria-describedby
    const selectors = '.input-error, #message-counter, .token-mask, [data-i18n="advanced.message.scroll_delay_help"]';
    document.querySelectorAll(selectors).forEach(el_desc => {
        let inputId = el_desc.id ? el_desc.id.replace(/-(error|counter|mask|help)/, '') : '';
        if (el_desc.dataset.i18n === 'advanced.message.scroll_delay_help') inputId = 'scroll_delay';
        const input = el(inputId);
        if (!input) return;
        if (!el_desc.id) el_desc.id = inputId + '-help';
        const desc = input.getAttribute('aria-describedby') || '';
        if (!desc.includes(el_desc.id)) input.setAttribute('aria-describedby', (desc + ' ' + el_desc.id).trim());
    });
}

// Function to validate input fields according to specified constraints
function setupFieldValidations() {
    // Common validator functions
    const validators = {
        // X/Y Offset: 0-160
        offsetValidator: function(value) {
            if (value === "") return null;
            const num = parseInt(value);
            if (isNaN(num)) return "Must be a number";
            if (num < 0 || num > 160) return "Must be between 0 and 160";
            return null; // Valid
        },
        
        // Latitude/Longitude: Decimal numbers
        coordValidator: function(value) {
            if (value === "") return null; // Empty is OK
            
            // Check for correct format (allows positive/negative decimals)
            if (!/^-?\d+(\.\d+)?$/.test(value)) {
                return "Must be a decimal number (e.g., 40.6892 or -74.0470)";
            }
            
            // Latitude should be between -90 and 90
            if (this.id === 'lat') {
                const num = parseFloat(value);
                if (num < -90 || num > 90) {
                    return "Latitude must be between -90 and 90";
                }
            }
            
            // Longitude should be between -180 and 180
            if (this.id === 'lon') {
                const num = parseFloat(value);
                if (num < -180 || num > 180) {
                    return "Longitude must be between -180 and 180";
                }
            }
            
            return null; // Valid
        },
        
        // Lux Threshold: 0-500 decimal
        luxThresholdValidator: function(value) {
            const num = parseFloat(value);
            if (isNaN(num)) return "Must be a number";
            if (num < 0 || num > 500) return "Must be between 0 and 500";
            return null; // Valid
        },
        
        // Lux Sensitivity: 0-50 positive decimal
        luxSensitivityValidator: function(value) {
            const num = parseFloat(value);
            if (isNaN(num)) return "Must be a number";
            if (num < 0 || num > 50) return "Must be between 0 and 50";
            return null; // Valid
        },
        
        // LED Brightness: 1-100 integer
        brightnessValidator: function(value) {
            const num = parseInt(value);
            if (isNaN(num)) return "Must be a whole number";
            if (num < 1 || num > 100) return "Must be between 1 and 100";
            return null; // Valid
        },
        
        // PWM Frequency: 10-300000 integer
        pwmFrequencyValidator: function(value) {
            const num = parseInt(value);
            if (isNaN(num)) return "Must be a whole number";
            if (num < 10 || num > 300000) return "Must be between 10 and 300000";
            return null; // Valid
        },
        
        // Max Power: 1-1023 integer
        maxPowerValidator: function(value) {
            const num = parseInt(value);
            if (isNaN(num)) return "Must be a whole number";
            if (num < 1 || num > 1023) return "Must be between 1 and 1023";
            return null; // Valid
        }
    };
    
    // Field configurations mapping
    const fieldConfigs = [
        { id: 'ofs_x', validator: validators.offsetValidator },
        { id: 'ofs_y', validator: validators.offsetValidator },
        { id: 'lat', validator: validators.coordValidator },
        { id: 'lon', validator: validators.coordValidator },
        { id: 'lux_threshold', validator: validators.luxThresholdValidator },
        { id: 'lux_sensitivity', validator: validators.luxSensitivityValidator },
        { id: 'brightness_LED0', validator: validators.brightnessValidator },
        { id: 'brightness_LED1', validator: validators.brightnessValidator },
        { id: 'pwm_frequency', validator: validators.pwmFrequencyValidator },
        { id: 'max_power', validator: validators.maxPowerValidator }
    ];
    
    // Setup validation for each field
    fieldConfigs.forEach(config => {
        const input = el(config.id);
        const error = el(`${config.id}-error`);
        
        if (!input || !error) return;
        
        // Validate on input
        input.addEventListener('input', function() {
            const errorMessage = config.validator.call(this, this.value);
            
            if (errorMessage) {
                input.classList.add('input-invalid');
                input.setAttribute('aria-invalid', 'true');
                error.textContent = errorMessage;
                error.style.display = 'block';
            } else {
                input.classList.remove('input-invalid');
                input.setAttribute('aria-invalid', 'false');
                error.style.display = 'none';
            }
        });
        
        // Also validate on blur (helps catch errors after focus changes)
        input.addEventListener('blur', function() {
            const errorMessage = config.validator.call(this, this.value);
            
            if (errorMessage) {
                input.classList.add('input-invalid');
                input.setAttribute('aria-invalid', 'true');
                error.textContent = errorMessage;
                error.style.display = 'block';
            }
        });
    });
    
    // Add form submission validation
    const forms = ['settingsForm', 'advancedForm'];
    forms.forEach(formId => {
        const form = el(formId);
        if (!form) return;
        
        form.addEventListener('submit', function(e) {
            let hasError = false;
            
            // Check each field
            fieldConfigs.forEach(config => {
                const input = el(config.id);
                const error = el(`${config.id}-error`);
                
                if (!input || !error) return;
                // Only validate fields that belong to the form being submitted (both forms share the same IDs in DOM)
                if (!form.contains(input)) return;
                
                const errorMessage = config.validator.call(input, input.value);
                
                if (errorMessage) {
                    e.preventDefault(); // Prevent form submission
                    input.classList.add('input-invalid');
                    input.setAttribute('aria-invalid', 'true');
                    error.textContent = errorMessage;
                    error.style.display = 'block';
                    
                    if (!hasError) {
                        input.focus();
                        hasError = true;
                    }
                }
            });
            
            if (hasError) {
                showStatus(getMessage('correct_form_errors'), 'error');
            }
        });
    });
}

// Function to validate hostnames according to DNS rules
function setupHostnameValidation() {
    const hostnameInput = el('hostname');
    const hostnameError = el('hostname-error');
    
    if (!hostnameInput || !hostnameError) return;
    
    // Regular expression for valid hostname characters (alphanumeric and hyphen)
    const validHostnameRegex = /^[a-zA-Z0-9-]+$/;
    
    function validateHostname(hostname) {
        if (!hostname || hostname.length === 0) {
            return 'Hostname cannot be empty.';
        }
        if (hostname.length > 32) {
            return 'Hostname is too long (max 32 chars).';
        }
        if (hostname.startsWith('-') || hostname.endsWith('-')) {
            return 'Hostname cannot start or end with a hyphen.';
        }
        if (!validHostnameRegex.test(hostname)) {
            return 'Hostname can only contain letters, numbers, and hyphens.';
        }
        return null; // Valid hostname
    }
    
    // Validate as user types
    hostnameInput.addEventListener('input', function() {
        const hostname = this.value;
        const errorMessage = validateHostname(hostname);
        
        if (errorMessage) {
            hostnameInput.classList.add('input-invalid');
            hostnameInput.setAttribute('aria-invalid', 'true');
            hostnameError.textContent = errorMessage;
            hostnameError.style.display = 'block';
        } else {
            hostnameInput.classList.remove('input-invalid');
            hostnameInput.setAttribute('aria-invalid', 'false');
            hostnameError.style.display = 'none';
        }
    });
    
    // Prevent submission if hostname is invalid
    const settingsForm = el('settingsForm');

    [settingsForm].forEach(form => {
        if (form) {
            form.addEventListener('submit', function(e) {
                const hostname = hostnameInput.value;
                const errorMessage = validateHostname(hostname);
                
                if (errorMessage) {
                    e.preventDefault(); // Prevent form submission
                    hostnameInput.classList.add('input-invalid');
                    hostnameInput.setAttribute('aria-invalid', 'true');
                    hostnameError.textContent = errorMessage;
                    hostnameError.style.display = 'block';
                    hostnameInput.focus();
                    
                    // Show error status
                    showStatus(getMessage('invalid_hostname') + errorMessage, 'error');
                }
            });
        }
    });
}

// Helper function to check if a value has changed
// Returns false if oldValue is undefined (section not loaded) to prevent sending unchanged fields
function hasValueChanged(newValue, oldValue) {
    // If oldValue is undefined, it means the section hasn't been loaded yet
    // Don't treat this as a change - skip the field to avoid sending wrong values
    if (oldValue === undefined || oldValue === null) {
        return false;
    }
    
    // If new value is undefined/null but old value exists, it's a change (clearing the field)
    if (newValue === undefined || newValue === null) {
        return oldValue !== undefined && oldValue !== null && String(oldValue).trim() !== '';
    }
    
    // Handle arrays
    if (Array.isArray(newValue) && Array.isArray(oldValue)) {
        if (newValue.length !== oldValue.length) return true;
        for (let i = 0; i < newValue.length; i++) {
            if (newValue[i] !== oldValue[i]) return true;
        }
        return false;
    }
    
    // Handle numbers (convert to same type for comparison)
    if (typeof newValue === 'number' && typeof oldValue === 'number') {
        return newValue !== oldValue;
    }
    
    // Handle strings (trim and compare)
    const newStr = String(newValue).trim();
    const oldStr = String(oldValue).trim();
    return newStr !== oldStr;
}

// Helper function to add field to formData only if it has changed
function addIfChanged(formData, key, newValue, oldValue) {
    if (hasValueChanged(newValue, oldValue)) {
        formData[key] = newValue;
        return true;
    }
    return false;
}

// Shared form submission handler
/*
 * Field Name Mappings (pXX format to reduce HTTP header size):
 * 
 * Basic Settings:
 * - p00 = hostname (Device hostname)
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
 * - p03 = rotation (Display rotation)
 * - p04 = dayfont (Day font)
 * - p05 = nightfont (Night font)
 * - p06 = quiet_scroll (Show scrolling message)
 * - p07 = quiet_weather (legacy; layout controls weather/moon visibility)
 * - p08 = show_grid (Show grid)
 * - p09 = mirroring (Mirror display)
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
 * - p42 = pwm_frequency (PWM frequency in Hz, range 10-300000)
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
    }

    // Advanced form fields (advancedForm)
    if (formId === 'advancedForm') {
        const ofsXEl = getFieldInForm('ofs_x');
        if (ofsXEl && addIfChanged(formData, 'p01', parseInt(ofsXEl.value) || 0, window.settings.p01)) changedCount++;
        
        const ofsYEl = getFieldInForm('ofs_y');
        if (ofsYEl && addIfChanged(formData, 'p02', parseInt(ofsYEl.value) || 0, window.settings.p02)) changedCount++;
        
        const rotationEl = getFieldInForm('rotation');
        if (rotationEl && addIfChanged(formData, 'p03', parseInt(rotationEl.value) || 0, window.settings.p03)) changedCount++;
        
        const showGridEl = getFieldInForm('show_grid');
        if (showGridEl && addIfChanged(formData, 'p08', showGridEl.checked ? 1 : 0, window.settings.p08)) changedCount++;
        
        const mirroringEl = getFieldInForm('mirroring');
        if (mirroringEl && addIfChanged(formData, 'p09', mirroringEl.checked ? 1 : 0, window.settings.p09)) changedCount++;
        
        const latEl = getFieldInForm('lat');
        if (latEl && addIfChanged(formData, 'p17', latEl.value, window.settings.p17)) changedCount++;
        
        const lonEl = getFieldInForm('lon');
        if (lonEl && addIfChanged(formData, 'p18', lonEl.value, window.settings.p18)) changedCount++;
        
        const timezoneEl = getFieldInForm('timezone');
        if (timezoneEl && addIfChanged(formData, 'p19', timezoneEl.value, window.settings.p19)) changedCount++;
        
        const wifiStartEl = getFieldInForm('wifi_start');
        if (wifiStartEl && addIfChanged(formData, 'p46', parseInt(wifiStartEl.value) || 0, window.settings.p46)) changedCount++;
        
        const wifiEndEl = getFieldInForm('wifi_end');
        if (wifiEndEl && addIfChanged(formData, 'p47', parseInt(wifiEndEl.value) || 0, window.settings.p47)) changedCount++;
        
        const luxSensitivityEl = getFieldInForm('lux_sensitivity');
        if (luxSensitivityEl && addIfChanged(formData, 'p20', parseFloat(luxSensitivityEl.value) || 0, window.settings.p20)) changedCount++;
        
        const luxThresholdEl = getFieldInForm('lux_threshold');
        if (luxThresholdEl && addIfChanged(formData, 'p21', parseFloat(luxThresholdEl.value) || 0, window.settings.p21)) changedCount++;
        
        const dimModeEl = getFieldInForm('dim_mode');
        if (dimModeEl && addIfChanged(formData, 'p22', parseInt(dimModeEl.value, 10) || 0, window.settings.p22)) changedCount++;

        const dimStartEl = getFieldInForm('dim_start');
        if (dimStartEl && addIfChanged(formData, 'p55', parseInt(dimStartEl.value, 10) || 0, window.settings.p55)) changedCount++;

        const dimEndEl = getFieldInForm('dim_end');
        if (dimEndEl && addIfChanged(formData, 'p56', parseInt(dimEndEl.value, 10) || 0, window.settings.p56)) changedCount++;
        
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

        const newSched = serializeDisplaySchedule();
        if (addIfChanged(formData, 'p58', newSched, window.settings.p58)) {
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
        formData.p35 !== undefined;


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
    }

    // Populate fields if settings are loaded
    if (window.settings && window.settingsLoaded.settings) {
        const hostnameEl = el('hostname');
        const wifiSsidEl = el('wifi_ssid');
        const wifiPassEl = el('wifi_pass');
        const fahrenheitEl = el('fahrenheit');
        const hour12El = el('hour12');
        const updateFirmwareEl = el('update_firmware');
        
        if (hostnameEl && window.settings.p00 !== undefined) hostnameEl.value = window.settings.p00;
        if (wifiSsidEl && window.settings.p34 !== undefined) wifiSsidEl.value = window.settings.p34;
        if (wifiPassEl && window.settings.p35 !== undefined) wifiPassEl.value = window.settings.p35;
        if (fahrenheitEl && window.settings.p36 !== undefined) fahrenheitEl.checked = window.settings.p36;
        if (hour12El && window.settings.p37 !== undefined) hour12El.checked = window.settings.p37;
        if (updateFirmwareEl && window.settings.p39 !== undefined) updateFirmwareEl.checked = window.settings.p39;
    }
}

// WiFi scanning functions
function startScanningUI() {
    const networkList = el('network-list');
    
    // Safety check - if networkList doesn't exist, we can't proceed
    if (!networkList) {
        console.error('Network list element not found');
        return;
    }
    
    // Clear any existing content
    networkList.innerHTML = '';
    
    // Create and add loading spinner
    const loadingSpinner = document.createElement('div');
    loadingSpinner.id = 'loading-spinner';
    loadingSpinner.className = 'loading-spinner';
    networkList.appendChild(loadingSpinner);
    
    // Create and add loading text
    const loadingText = document.createElement('div');
    loadingText.id = 'loading-text';
    loadingText.className = 'loading-text';
    const trans = translations[currentLanguage] || translations.en;
    loadingText.textContent = getNestedTranslation(trans, 'settings.wifi.scanning') || 'Scanning for networks...';
    networkList.appendChild(loadingText);
}

function startWifiScan() {
    
    // Update UI to show scanning state
    startScanningUI();

    const scanBtn = el('scan-btn');
    toggleLoading(scanBtn, true);

    // Request scan start
    fetch('/api/wifi/scan')
        .then(response => response.json())
        .then(data => {
            if (data.status === 'ok') {
                // Start polling for results
                pollScanResults();
            } else {
                showNetworkError('Failed to start scan');
            }
        })
        .catch(error => {
            console.error('Error starting scan:', error);
            showNetworkError('Network error');
        });
}

function pollScanResults() {
    const checkStatus = () => {
        fetch('/api/wifi/status')
            .then(response => response.json())
            .then(data => {
                if (!data.scanning && data.scan_done) {
                    // Scan complete, display results
                    displayNetworks(data.networks || []);
                } else {
                    // Still scanning, check again in 1 second
                    setTimeout(checkStatus, 1000);
                }
            })
            .catch(error => {
                console.error('Error checking scan status:', error);
                showNetworkError('Error getting scan results');
            });
    };

    // Start checking status
    checkStatus();
}

function displayNetworks(networks) {
    invalidateI18nCache();
    const networkList = el('network-list');
    const loadingSpinner = el('loading-spinner');
    const loadingText = el('loading-text');
    const scanBtn = el('scan-btn');
    toggleLoading(scanBtn, false);
    
    // Safety check - if networkList doesn't exist, we can't proceed
    if (!networkList) {
        console.error('Network list element not found');
        return;
    }
    
    // Hide loading indicators if they exist
    if (loadingSpinner) {
        loadingSpinner.style.display = 'none';
    }
    if (loadingText) {
        loadingText.style.display = 'none';
    }

    // Clear the network list
    networkList.innerHTML = '';

    if (!networks || networks.length === 0) {
        const noNetworks = document.createElement('div');
        noNetworks.className = 'info-text';
        const trans = translations[currentLanguage] || translations.en;
        noNetworks.textContent = getNestedTranslation(trans, 'settings.wifi.no_networks') || 'No networks found';
        networkList.appendChild(noNetworks);
        return;
    }

    // Pre-calculate localized labels for the ARIA labels
    const trans = translations[currentLanguage] || translations.en;
    const signalLabel = getNestedTranslation(trans, 'settings.wifi.signal') || 'Signal strength';
    const secureLabel = getNestedTranslation(trans, 'settings.wifi.secure') || 'Secure';
    const openLabel = getNestedTranslation(trans, 'settings.wifi.open') || 'Open';

    // Add each network to the list
    networks.forEach(network => {
        const networkItem = document.createElement('div');
        networkItem.className = 'network-item';
        networkItem.setAttribute('role', 'button');
        networkItem.setAttribute('tabindex', '0');

        const securityStatus = network.requires_password ? secureLabel : openLabel;
        networkItem.setAttribute('aria-label', `${network.ssid}, ${signalLabel} ${network.signal_strength}%, ${securityStatus}`);

        networkItem.onclick = () => selectNetwork(network.ssid);
        networkItem.onkeydown = (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                selectNetwork(network.ssid);
            }
        };

        // Create network name element
        const nameSpan = document.createElement('span');
        nameSpan.textContent = network.ssid;
        if (network.requires_password) {
            nameSpan.innerHTML += ' <i>🔒</i>';
        }

        // Create signal strength indicator
        const signalStrength = document.createElement('div');
        signalStrength.className = 'signal-strength';

        const signalText = document.createElement('span');
        signalText.textContent = network.signal_strength + '%';

        const signalBar = document.createElement('div');
        signalBar.className = 'signal-bar';

        const signalFill = document.createElement('div');
        signalFill.className = 'signal-fill';
        signalFill.style.width = network.signal_strength + '%';

        signalBar.appendChild(signalFill);
        signalStrength.appendChild(signalBar);
        signalStrength.appendChild(signalText);

        // Add elements to network item
        networkItem.appendChild(nameSpan);
        networkItem.appendChild(signalStrength);

        // Add to network list
        networkList.appendChild(networkItem);
    });
}

function selectNetwork(ssid) {
    const ssidInput = el('wifi_ssid');
    const passInput = el('wifi_pass');

    ssidInput.value = ssid;
    highlightElement(ssidInput);
    highlightElement(passInput);
    passInput.focus();
}

function showNetworkError(message) {
    const networkList = el('network-list');
    const loadingSpinner = el('loading-spinner');
    const loadingText = el('loading-text');
    const scanBtn = el('scan-btn');
    toggleLoading(scanBtn, false);
    
    // Safety check - if networkList doesn't exist, we can't proceed
    if (!networkList) {
        console.error('Network list element not found');
        return;
    }
    
    // Hide loading indicators if they exist
    if (loadingSpinner) {
        loadingSpinner.style.display = 'none';
    }
    if (loadingText) {
        loadingText.style.display = 'none';
    }

    networkList.innerHTML = '';
    const errorDiv = document.createElement('div');
    errorDiv.className = 'info-text error';
    errorDiv.textContent = message;
    networkList.appendChild(errorDiv);
}

// Status section functionality
function setupStatusSection() {
    // Add refresh button handler
    const refreshButton = el('refreshButton');
    if (refreshButton) {
        refreshButton.addEventListener('click', () => fetchStatus(true));
    }
    
    // Status data will be fetched when the section is shown (in navigateToSection)
}

// Setup support buttons (send email and copy to clipboard)
function setupSupportButtons() {
    invalidateI18nCache();
    const sendToSupportButton = el('sendToSupportButton');
    const copyToClipboardButton = el('copyToClipboardButton');
    
    if (sendToSupportButton) {
        // Remove any existing listeners by cloning the button
        const newButton = sendToSupportButton.cloneNode(true);
        sendToSupportButton.parentNode.replaceChild(newButton, sendToSupportButton);
        domCache.delete('sendToSupportButton');
        
        newButton.addEventListener('click', function(e) {
            e.preventDefault();
            e.stopPropagation();
            sendSystemInfoToSupport();
        });
    } else {
        console.warn('sendToSupportButton not found in DOM');
    }
    
    if (copyToClipboardButton) {
        // Remove any existing listeners by cloning the button
        const newButton = copyToClipboardButton.cloneNode(true);
        copyToClipboardButton.parentNode.replaceChild(newButton, copyToClipboardButton);
        domCache.delete('copyToClipboardButton');
        
        newButton.addEventListener('click', function(e) {
            e.preventDefault();
            e.stopPropagation();
            copySystemInfoToClipboard();
        });
    } else {
        console.warn('copyToClipboardButton not found in DOM');
    }
}

function fetchStatus(includeLogs = false) {
    const url = includeLogs ? '/api/status?logs=1' : '/api/status';
    const refreshBtn = el('refreshButton');
    if (includeLogs) toggleLoading(refreshBtn, true);

    return fetch(url)
        .then(response => response.json())
        .then(data => {
            // Update time & weather status
            const timeUpdateStatus = el('time_update_status');
            const weatherUpdateStatus = el('weather_update_status');

            // Update time status
            if (data.time_status) {
                const timestamp = new Date(data.last_time_update * 1000).toLocaleString();
                timeUpdateStatus.innerHTML = `<span class="status-icon status-success"></span> ${timestamp}`;
            } else {
                timeUpdateStatus.innerHTML = '<span class="status-icon status-error"></span> Not synced';
            }

            // Update weather status
            if (data.weather_status) {
                const timestamp = new Date(data.last_weather_update * 1000).toLocaleString();
                weatherUpdateStatus.innerHTML = `<span class="status-icon status-success"></span> ${timestamp}`;
            } else {
                weatherUpdateStatus.innerHTML = '<span class="status-icon status-error"></span> Not synced';
            }

            // Update other weather-related info
            el('moon_status').textContent = data.moon_icon_index !== undefined ? getMoonPhaseName(data.moon_icon_index) : '-';
            el('latitude').textContent = data.latitude || '-';
            el('longitude').textContent = data.longitude || '-';
            el('timezone_val').textContent = data.timezone || '-';

            // Update system information
            el('app').textContent = data.app || '-';
            el('version').textContent = data.version || '-';
            el('fwversion').textContent = data.fwversion || '-';
            el('poh').textContent = data.poh !== undefined ? formatPOH(data.poh) : '-';
            el('mac_address').textContent = data.mac_address || '-';
            el('ip_address').textContent = data.ip_address || '-';
            el('chip_revision').textContent = data.chip_revision || '-';
            el('flash_size').textContent = data.flash_size ? formatBytes(data.flash_size) : '-';
            el('cpu_freq').textContent = data.cpu_freq ? `${(data.cpu_freq / 1000000)} MHz` : '-';
            el('compile_time').textContent = data.compile_time || '-';
            el('free_heap').textContent = data.free_heap ? formatBytes(data.free_heap) : '-';
            el('min_free_heap').textContent = data.min_free_heap ? formatBytes(data.min_free_heap) : '-';

            // Update sensor data
            el('lux').textContent = data.lux !== undefined ? data.lux.toFixed(1) : '-';
            if (data.uptime !== undefined) {
                el('uptime').textContent = formatUptime(data.uptime);
            } else {
                el('uptime').textContent = '-';
            }

            // Update system logs
            const logsTextarea = el('system_logs');
            if (data.system_logs && Array.isArray(data.system_logs)) {
                logsTextarea.value = data.system_logs.join('\n');
            } else {
                logsTextarea.value = 'No logs available';
            }

            // Update HA Status textarea
            if (data.ha_tokens && Array.isArray(data.ha_tokens)) {
                el('ha_status_textarea').value = data.ha_tokens.join('\n');
            } else {
                el('ha_status_textarea').value = 'No Integrations active';
            }

            return data; // Return the data for other functions to use
        })
        .catch(error => {
            console.error('Error fetching status:', error);
            showStatus(getMessage('failed_fetch_status'), 'error');
            throw error; // Re-throw the error for other functions to handle
        })
        .finally(() => {
            if (includeLogs) toggleLoading(refreshBtn, false);
        });
}

function formatBytes(bytes, decimals = 2) {
    if (!bytes) return '0 Bytes';
    
    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    
    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
}

function getMoonPhaseName(index) {
    const phases = [
        'New Moon',
        'Waxing Crescent',
        'First Quarter',
        'Waxing Gibbous',
        'Full Moon',
        'Waning Gibbous',
        'Last Quarter',
        'Waning Crescent'
    ];
    
    if (index >= 0 && index < phases.length) {
        return phases[index];
    } else {
        return 'Unknown (' + index + ')';
    }
}

// Files section functionality
function setupFilesSection() {
    if (window.filesEventListenersSet) return;
    window.filesEventListenersSet = true;

    const refreshButton = el('refreshFilesButton');
    const deleteButton = el('deleteFilesButton');
    const renameButton = el('renameFileButton');
    const selectAll = el('selectAllFiles');
    const sortButtons = document.querySelectorAll('#files-section .table-sort-button');

    if (refreshButton) {
        refreshButton.addEventListener('click', fetchFiles);
    }

    if (deleteButton) {
        deleteButton.addEventListener('click', deleteSelectedFiles);
    }

    if (renameButton) {
        renameButton.addEventListener('click', renameSelectedFile);
    }

    if (selectAll) {
        const trans = translations[currentLanguage] || translations.en;
        selectAll.setAttribute('aria-label', getNestedTranslation(trans, 'files.select_all') || 'Select all files');
        selectAll.addEventListener('change', function() {
            window.selectedFiles.clear();
            if (selectAll.checked) {
                window.spiffsFiles.forEach(file => window.selectedFiles.add(file.name));
            }
            renderFilesTable(window.spiffsFiles);
        });
    }

    sortButtons.forEach(button => {
        button.addEventListener('click', function() {
            const sortKey = button.dataset.sortKey;
            if (!sortKey) return;

            if (window.filesSort.key === sortKey) {
                window.filesSort.direction = window.filesSort.direction === 'asc' ? 'desc' : 'asc';
            } else {
                window.filesSort.key = sortKey;
                window.filesSort.direction = 'asc';
            }

            renderFilesTable(window.spiffsFiles);
        });
    });

    updateFilesSortHeaders();
}

function fetchFiles() {
    const refreshButton = el('refreshFilesButton');
    toggleLoading(refreshButton, true);
    renderFilesLoading();

    return fetch('/api/files')
        .then(response => {
            if (!response.ok) {
                throw new Error(response.statusText || response.status);
            }
            return response.json();
        })
        .then(data => {
            window.spiffsFiles = Array.isArray(data.files) ? data.files : [];
            window.screenSpiffsFileSet = new Set(window.spiffsFiles.map(f => f.name));
            window.selectedFiles.clear();
            window.filesLoaded = true;
            renderFilesTable(window.spiffsFiles);
            return data;
        })
        .catch(error => {
            console.error('Error fetching files:', error);
            showStatus(getMessage('failed_fetch_files'), 'error');
            renderFilesError();
        })
        .finally(() => {
            toggleLoading(refreshButton, false);
        });
}

function renderFilesLoading() {
    const tbody = el('filesTableBody');
    if (!tbody) return;
    const trans = translations[currentLanguage] || translations.en;
    tbody.innerHTML = '';
    const row = document.createElement('tr');
    const cell = document.createElement('td');
    cell.colSpan = 3;
    cell.className = 'files-empty';
    cell.textContent = getNestedTranslation(trans, 'files.loading') || 'Loading files...';
    row.appendChild(cell);
    tbody.appendChild(row);
}

function renderFilesError() {
    const tbody = el('filesTableBody');
    if (!tbody) return;
    tbody.innerHTML = '';
    const row = document.createElement('tr');
    const cell = document.createElement('td');
    cell.colSpan = 3;
    cell.className = 'files-empty error';
    cell.textContent = getMessage('failed_fetch_files');
    row.appendChild(cell);
    tbody.appendChild(row);
    updateFilesActions();
}

function renderFilesTable(files) {
    const tbody = el('filesTableBody');
    const summary = el('files-summary');
    const selectAll = el('selectAllFiles');
    if (!tbody) return;

    const trans = translations[currentLanguage] || translations.en;
    const sortedFiles = getSortedFiles(files || []);
    tbody.innerHTML = '';

    if (sortedFiles.length === 0) {
        const row = document.createElement('tr');
        const cell = document.createElement('td');
        cell.colSpan = 3;
        cell.className = 'files-empty';
        cell.textContent = getNestedTranslation(trans, 'files.empty') || 'No files found';
        row.appendChild(cell);
        tbody.appendChild(row);
    } else {
        sortedFiles.forEach(file => {
            const row = document.createElement('tr');
            const selected = window.selectedFiles.has(file.name);
            row.className = selected ? 'is-selected' : '';

            const selectCell = document.createElement('td');
            const checkbox = document.createElement('input');
            checkbox.type = 'checkbox';
            checkbox.checked = selected;
            checkbox.setAttribute('aria-label', file.name);
            checkbox.addEventListener('change', function() {
                if (checkbox.checked) {
                    window.selectedFiles.add(file.name);
                } else {
                    window.selectedFiles.delete(file.name);
                }
                renderFilesTable(window.spiffsFiles);
            });
            selectCell.appendChild(checkbox);

            const nameCell = document.createElement('td');
            const nameLink = document.createElement('a');
            nameLink.href = '/' + encodeURIComponent(file.name || '');
            nameLink.download = file.name || '';
            nameLink.textContent = file.name || '';
            nameCell.appendChild(nameLink);

            const sizeCell = document.createElement('td');
            sizeCell.textContent = formatBytes(file.size || 0);

            row.appendChild(selectCell);
            row.appendChild(nameCell);
            row.appendChild(sizeCell);
            tbody.appendChild(row);
        });
    }

    if (summary) {
        const countLabel = getNestedTranslation(trans, 'files.count') || ' files';
        const selectedLabel = getNestedTranslation(trans, 'files.selected') || ' selected';
        summary.textContent = `${sortedFiles.length}${countLabel}, ${window.selectedFiles.size}${selectedLabel}`;
    }

    if (selectAll) {
        selectAll.checked = sortedFiles.length > 0 && window.selectedFiles.size === sortedFiles.length;
        selectAll.indeterminate = window.selectedFiles.size > 0 && window.selectedFiles.size < sortedFiles.length;
    }

    updateFilesSortHeaders();
    updateFilesActions();
}

function getSortedFiles(files) {
    const direction = window.filesSort.direction === 'desc' ? -1 : 1;
    const key = window.filesSort.key;

    return [...files].sort((a, b) => {
        if (key === 'size') {
            return (((a.size || 0) - (b.size || 0)) * direction) ||
                String(a.name || '').localeCompare(String(b.name || ''));
        }

        return String(a.name || '').localeCompare(String(b.name || ''), undefined, { sensitivity: 'base' }) * direction;
    });
}

function updateFilesSortHeaders() {
    const headers = {
        name: el('filesNameHeader'),
        size: el('filesSizeHeader')
    };
    const buttons = document.querySelectorAll('#files-section .table-sort-button');

    Object.keys(headers).forEach(key => {
        const header = headers[key];
        if (!header) return;
        const isActive = window.filesSort.key === key;
        header.setAttribute('aria-sort', isActive ? (window.filesSort.direction === 'asc' ? 'ascending' : 'descending') : 'none');
    });

    buttons.forEach(button => {
        const isActive = button.dataset.sortKey === window.filesSort.key;
        button.classList.toggle('is-active', isActive);
        button.dataset.sortDirection = isActive ? window.filesSort.direction : '';
    });
}

function updateFilesActions() {
    const selectedCount = window.selectedFiles.size;
    const deleteButton = el('deleteFilesButton');
    const renameButton = el('renameFileButton');
    if (deleteButton) deleteButton.disabled = selectedCount === 0;
    if (renameButton) renameButton.disabled = selectedCount !== 1;
}

function deleteSelectedFiles() {
    const selected = Array.from(window.selectedFiles);
    if (selected.length === 0) return;

    const trans = translations[currentLanguage] || translations.en;
    const confirmMessage = getNestedTranslation(trans, 'files.delete_confirm') || 'Delete selected files?';
    if (!window.confirm(`${confirmMessage}\n\n${selected.join('\n')}`)) {
        return;
    }

    const deleteButton = el('deleteFilesButton');
    toggleLoading(deleteButton, true);

    fetch('/api/files/delete', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ files: selected })
    })
    .then(response => response.json().then(data => ({ ok: response.ok, data })))
    .then(({ ok, data }) => {
        if (!ok || !data || data.status !== 'ok') {
            throw new Error(data && data.message ? data.message : 'Delete failed');
        }
        showStatus(getMessage('files_deleted'), 'success');
        return fetchFiles();
    })
    .catch(error => {
        console.error('Error deleting files:', error);
        showStatus(getMessage('failed_delete_files') + error.message, 'error');
    })
    .finally(() => {
        toggleLoading(deleteButton, false);
        updateFilesActions();
    });
}

function renameSelectedFile() {
    const selected = Array.from(window.selectedFiles);
    if (selected.length !== 1) {
        const trans = translations[currentLanguage] || translations.en;
        showStatus(getNestedTranslation(trans, 'files.only_one_rename') || 'Select exactly one file to rename', 'error');
        return;
    }

    const oldName = selected[0];
    const trans = translations[currentLanguage] || translations.en;
    const promptText = getNestedTranslation(trans, 'files.rename_prompt') || 'Enter a new filename for ';
    const newName = window.prompt(promptText + oldName, oldName);
    if (!newName || newName === oldName) {
        return;
    }

    const renameButton = el('renameFileButton');
    toggleLoading(renameButton, true);

    fetch('/api/files/rename', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ oldName, newName: newName.trim() })
    })
    .then(response => response.json().then(data => ({ ok: response.ok, data })))
    .then(({ ok, data }) => {
        if (!ok || !data || data.status !== 'ok') {
            throw new Error(data && data.message ? data.message : 'Rename failed');
        }
        showStatus(getMessage('file_renamed'), 'success');
        return fetchFiles();
    })
    .catch(error => {
        console.error('Error renaming file:', error);
        showStatus(getMessage('failed_rename_file') + error.message, 'error');
    })
    .finally(() => {
        toggleLoading(renameButton, false);
        updateFilesActions();
    });
}

// Update (OTA) section functionality
function setupUpdateSection() {
    const reinstallButton = el('reinstallButton');
    if (reinstallButton) {
        reinstallButton.addEventListener('click', reinstallCurrentFirmware);
    }

    const uploadForm = el('uploadForm');
    const firmwareFile = el('firmwareFile');
    const uploadButton = el('uploadButton');
    const progressContainer = el('progressContainer');
    const progressBar = el('progress');
    const progressText = el('progressText');
    
    // Enable upload button when file is selected
    firmwareFile.addEventListener('change', function() {
        uploadButton.disabled = !firmwareFile.files.length;
    });
    
    // Handle form submission
    uploadForm.addEventListener('submit', function(e) {
        e.preventDefault();
        
        if (!firmwareFile.files.length) {
            showStatus(getMessage('select_firmware_file'), 'error');
            return;
        }
        
        const file = firmwareFile.files[0];
                
        // Disable form elements during upload
        firmwareFile.disabled = true;
        uploadButton.disabled = true;
        
        // Show progress container
        progressContainer.style.display = 'block';
        progressBar.style.width = '0%';
        progressText.textContent = '0%';
        
        // Show uploading status
        showStatus(getMessage('uploading_file'), 'info');
        toggleLoading(uploadButton, true);
        
        // Create FormData and append file
        const formData = new FormData();
        formData.append('firmware', file);
        
        // Upload file using XMLHttpRequest to track progress
        const xhr = new XMLHttpRequest();
        
        xhr.upload.addEventListener('progress', function(e) {
            if (e.lengthComputable) {
                const percentComplete = Math.round((e.loaded / e.total) * 100);
                progressBar.style.width = percentComplete + '%';
                progressText.textContent = percentComplete + '%';
            }
        });
        
        xhr.addEventListener('load', function() {
            if (xhr.status === 200) {
                try {
                    const response = JSON.parse(xhr.responseText);
                    if (response.status === 'ok') {
                        // Check if this was a firmware update or regular file upload
                        if (response.message && response.message.includes('rebooting')) {
                            // This was a firmware update
                            showStatus(getMessage('firmware_update_success'), 'success');
                            // Redirect to status page after a delay
                            setTimeout(function() {
                                window.location.hash = 'status';
                                navigateToSection();
                            }, 5000);
                        } else {
                            // This was a regular file upload
                            showStatus(getMessage('file_upload_success'), 'success');
                            resetForm();
                        }
                    } else {
                        showStatus(getMessage('update_failed') + (response.message || 'Unknown error'), 'error');
                        resetForm();
                    }
                } catch (error) {
                    showStatus(getMessage('invalid_response'), 'error');
                    resetForm();
                }
            } else {
                showStatus(getMessage('upload_failed_status') + xhr.status, 'error');
                resetForm();
            }
        });
        
        xhr.addEventListener('error', function() {
            showStatus(getMessage('network_error_upload'), 'error');
            resetForm();
        });
        
        xhr.addEventListener('abort', function() {
            showStatus(getMessage('upload_aborted'), 'error');
            resetForm();
        });
        
        // Open and send the request
        xhr.open('POST', '/api/ota', true);
        xhr.send(formData);
    });
    
    function resetForm() {
        firmwareFile.disabled = false;
        toggleLoading(uploadButton, false);
        uploadButton.disabled = true;
        firmwareFile.value = '';
        progressContainer.style.display = 'none';
    }
}

function reinstallCurrentFirmware() {
    const reinstallButton = el('reinstallButton');
    toggleLoading(reinstallButton, true);
    showStatus(getMessage('reinstall_sending'), 'info');

    fetch('/api/ota/reinstall', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({}),
    })
    .then(response => response.json())
    .then(data => {
        toggleLoading(reinstallButton, false);
        if (data.status === 'ok') {
            showStatus(getMessage('reinstall_started'), 'success');
            setTimeout(function() {
                window.location.hash = 'status';
                navigateToSection();
            }, 3000);
            return;
        }

        let message = data.message || 'Unknown error';
        if (message.includes('WiFi not connected')) {
            message = getMessage('reinstall_wifi_required');
        } else if (message.includes('already in progress')) {
            message = getMessage('reinstall_in_progress');
        }
        showStatus(getMessage('reinstall_failed') + message, 'error');
    })
    .catch(() => {
        toggleLoading(reinstallButton, false);
        showStatus(getMessage('reinstall_failed') + getMessage('error_reset_connection'), 'error');
    });
}

// Restart section functionality
function setupRestartSection() {
    const resetButton = el('resetButton');
    const resetModal = el('resetModal');
    const cancelButton = el('cancelButton');
    const confirmButton = el('confirmButton');
    let lastFocusedElement;

    const openModal = () => {
        lastFocusedElement = document.activeElement;
        resetModal.style.display = 'flex';
        cancelButton.focus();
    };

    const closeModal = () => {
        resetModal.style.display = 'none';
        if (lastFocusedElement) {
            lastFocusedElement.focus();
        }
    };
    
    // Show modal when reset button is clicked
    resetButton.addEventListener('click', openModal);
    
    // Hide modal when cancel button is clicked
    cancelButton.addEventListener('click', closeModal);
    
    // Handle device reset when confirm button is clicked
    confirmButton.addEventListener('click', () => {
        resetModal.style.display = 'none';
        resetDevice();
        if (lastFocusedElement) {
            lastFocusedElement.focus();
        }
    });

    // Keyboard navigation for modal
    resetModal.addEventListener('keydown', function(e) {
        if (e.key === 'Escape') {
            closeModal();
        } else if (e.key === 'Tab') {
            // Basic focus trap between Cancel and Restart buttons
            if (e.shiftKey) { // Shift + Tab
                if (document.activeElement === cancelButton) {
                    e.preventDefault();
                    confirmButton.focus();
                }
            } else { // Tab
                if (document.activeElement === confirmButton) {
                    e.preventDefault();
                    cancelButton.focus();
                }
            }
        }
    });
}

// ----------------
// Screen editor section
// ----------------
const SCREEN_SIZE = 128;
const SCREEN_DRAG_THRESHOLD = 3;
const SCREEN_PREVIEW_MESSAGE = 'Customize your frixos projection clock';
const SCREEN_MESSAGE_FONT_HEIGHTS = {
    0: 8,
    1: 10,
    2: 12,
    3: 14,
    4: 16
};
const SCREEN_MESSAGE_FONT_OPTIONS = [
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
    text_5: 'Rain [precip]'
};
const SCREEN_TOKEN_CODES = [
    '[device]', '[greeting]', '[day]', '[date]', '[mon]', '[temp]', '[hum]', '[high]', '[low]',
    '[rise]', '[set]', '[wind]', '[gust]', '[precip]', '[uv]', '[pressure]', '[3high]', '[3low]',
    '[HA:entity_id:path]', '[$:symbol]', '[CGM:glucose]', '[CGM:reading]', '[CGM:time]'
];
const TOKEN_HELP = {
    '[device]': { i18nKey: 'advanced.message.token_device', example: 'frixos' },
    '[greeting]': { i18nKey: 'advanced.message.token_greeting', example: 'Good Evening' },
    '[day]': { i18nKey: 'advanced.message.token_day', example: 'Saturday' },
    '[date]': { i18nKey: 'advanced.message.token_date', example: 'Jun 6' },
    '[mon]': { i18nKey: 'advanced.message.token_mon', example: 'June' },
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

function updateTokenButtonTooltip(btn) {
    const code = getTokenCode(btn);
    const meta = TOKEN_HELP[code];
    const trans = translations[currentLanguage] || translations.en;
    const insertLabel = getNestedTranslation(trans, 'common.insert') || 'Insert';
    btn.setAttribute('aria-label', `${insertLabel} ${code}`);
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

async function refreshScreenLayoutSelect() {
    await ensureScreenSpiffsFiles(true);
    const select = el('screenLayoutSelect');
    if (!select) return;

    const trans = translations[currentLanguage] || translations.en;
    const placeholderText = getNestedTranslation(trans, 'screen.layout_file_placeholder') || 'Select a layout file';
    const previous = select.value;
    const names = getScreenLayoutFileNames();

    select.innerHTML = '';
    const placeholder = document.createElement('option');
    placeholder.value = '';
    placeholder.textContent = placeholderText;
    select.appendChild(placeholder);

    names.forEach(name => {
        const option = document.createElement('option');
        option.value = name;
        option.textContent = name;
        select.appendChild(option);
    });

    if (names.includes(previous)) {
        select.value = previous;
    }

    const loadBtn = el('screenLoadSystemBtn');
    if (loadBtn) loadBtn.disabled = names.length === 0;
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

function getPreviewImageUrl(def, inMatrix) {
    if (def.id === 'time') {
        const font = inMatrix ? getScreenTimeFont(window.screenEditor.mode) : 'bold';
        return resolveTimeFontPreviewUrl(font);
    }
    const item = SCREEN_THEME_ASSETS[def.id];
    if (item && inMatrix) {
        return resolveThemeAssetUrl(getScreenTimeFont(window.screenEditor.mode), item);
    }
    return def.img;
}

const SCREEN_ELEMENT_DEFS = [
    { id: 'glucose_level', label: 'Glucose Level', w: 14, h: 20, img: 'default-glucose.jpg' },
    { id: 'glucose_trend', label: 'Glucose Trend', w: 12, h: 14, img: 'default-trend.jpg' },
    { id: 'wifi_off', label: 'WiFi off', w: 20, h: 20, img: 'default-wifi-off.jpg' },
    { id: 'weather', label: 'Weather', w: 32, h: 22, img: 'default-weather.jpg' },
    { id: 'moon', label: 'Moon', w: 14, h: 14, img: 'default-moon.jpg' },
    { id: 'time', label: 'Time/Glucose', w: 80, h: 36, img: 'bold.jpg', paletteFitFullImage: true },
    { id: 'glucose', label: 'Glucose', w: 70, h: 36, img: 'glucose.jpg', paletteFitFullImage: true },
    { id: 'ampm', label: 'AM/PM', w: 10, h: 20, img: 'default-ampm.jpg' },
    { id: 'message', label: 'Scrolling Message', w: 128, h: 8, dynamicHeight: true, text: SCREEN_PREVIEW_MESSAGE },
    { id: 'text_1', label: 'Text 1', w: 64, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_1 },
    { id: 'text_2', label: 'Text 2', w: 64, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_2 },
    { id: 'text_3', label: 'Text 3', w: 64, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_3 },
    { id: 'text_4', label: 'Text 4', w: 64, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_4 },
    { id: 'text_5', label: 'Text 5', w: 64, h: 8, dynamicHeight: true, text: SCREEN_DEFAULT_STATIC_TEXTS.text_5 }
];

const SCREEN_TEXT_SLOT_IDS = ['text_1', 'text_2', 'text_3', 'text_4', 'text_5'];

const SCREEN_PALETTE_TEXT_DEF = {
    id: 'text',
    label: 'Text',
    w: 64,
    h: 8,
    dynamicHeight: true
};

function getNextTextSlot(profile) {
    if (!profile || !profile.elements) return SCREEN_TEXT_SLOT_IDS[0];
    for (const id of SCREEN_TEXT_SLOT_IDS) {
        const elem = profile.elements.find(e => e.id === id);
        if (!elem || !elem.enabled) return id;
    }
    return null;
}

function getScreenPaletteDefs() {
    return [
        ...SCREEN_ELEMENT_DEFS.filter(d => !isScreenStaticTextElement(d.id)),
        SCREEN_PALETTE_TEXT_DEF
    ];
}

function resolvePaletteElementId(paletteId, profile) {
    if (paletteId !== 'text') return paletteId;
    return getNextTextSlot(profile);
}

function isScreenTextElement(id) {
    return id === 'message' || /^text_[1-5]$/.test(id);
}

function isScreenStaticTextElement(id) {
    return /^text_[1-5]$/.test(id);
}

function ensureScreenTextOptions(element) {
    if (!element.options) element.options = {};
    if (element.options.font === undefined || element.options.font === 255) element.options.font = 0;
    if (!element.options.color) element.options.color = '#ffffff';
    if (!element.options.bg_color) element.options.bg_color = '#000000';
    if (isScreenStaticTextElement(element.id)) {
        if (element.options.width === undefined) element.options.width = 0;
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
        { id: 'ampm', enabled: 1, x: 101, y: 54, z: 2 },
        { id: 'glucose', enabled: 0, x: 22, y: 47, z: 1 },
        { id: 'message', enabled: 1, x: 0, y: 86, z: 0, options: { font: 0, color: '#ffffff', bg_color: '#000000' } }
    ];
    ['text_1', 'text_2', 'text_3', 'text_4', 'text_5'].forEach((id, index) => {
        elements.push({
            id,
            enabled: 0,
            x: 45,
            y: 90,
            z: Math.min(4, 1 + index),
            options: { font: 0, color: '#ffffff', bg_color: '#000000', width: 0, align: 0 }
        });
    });
    return {
        scroll_text: SCREEN_DEFAULT_SCROLL_TEXT,
        static_texts: { ...SCREEN_DEFAULT_STATIC_TEXTS },
        elements
    };
}

const SCREEN_LAYOUT_VERSION = 7;

const SCREEN_DEFAULT_LAYOUT = {
    version: SCREEN_LAYOUT_VERSION,
    scroll_delay: 65,
    day_font: 'bold',
    night_font: 'bold',
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
    return SCREEN_ELEMENT_DEFS.find(d => d.id === id) || null;
}

function evenPx(value) {
    const rounded = Math.max(2, Math.round(value));
    return rounded % 2 === 0 ? rounded : rounded + 1;
}

function getScreenElementSize(def, elementState) {
    if (!def) return { w: 0, h: 0 };
    let w;
    let h;
    if (isScreenTextElement(def.id)) {
        const font = elementState && elementState.options && elementState.options.font !== undefined ? elementState.options.font : 0;
        w = isScreenStaticTextElement(def.id) && elementState && elementState.options && elementState.options.width > 0
            ? elementState.options.width
            : (isScreenStaticTextElement(def.id) ? def.w : 128);
        h = SCREEN_MESSAGE_FONT_HEIGHTS[font] || 8;
    } else {
        w = def.w;
        h = def.h;
    }
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
        if (id === 'glucose') {
            e = { id, enabled: 0, x: 22, y: 47, z: 1 };
        } else {
            e = { id, enabled: 1, x: 0, y: 0, z: 0 };
        }
        profile.elements.push(e);
    }
    if (e.enabled === undefined) {
        if (id === 'glucose') e.enabled = 0;
        else e.enabled = 1;
    }
    if (id === 'ampm') e.enabled = 1;
    if (e.x === undefined) e.x = 0;
    if (e.y === undefined) e.y = 0;
    ensureElementLayer(e);
    return e;
}

function setupScreenSection() {
    if (window.screenEventListenersSet) return;
    window.screenEventListenersSet = true;

    const btnDay = el('screenModeDay');
    const btnNight = el('screenModeNight');
    const saveBtn = el('screenSaveBtn');
    const restoreDefaultBtn = el('screenRestoreDefaultBtn');
    const loadSystemBtn = el('screenLoadSystemBtn');
    const copyBtn = el('screenCopyBtn');
    const saveToFileBtn = el('screenSaveToFileBtn');
    const readFromFileBtn = el('screenReadFromFileBtn');
    const layoutFileInput = el('screenLayoutFileInput');

    if (btnDay) btnDay.addEventListener('click', () => setScreenMode('day'));
    if (btnNight) btnNight.addEventListener('click', () => setScreenMode('night'));
    if (saveBtn) saveBtn.addEventListener('click', saveScreenLayout);
    if (restoreDefaultBtn) restoreDefaultBtn.addEventListener('click', restoreScreenDefaults);
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

    renderScreenPalette();
    updateScreenModeButtons();
    updateScreenStatusLine();
}

function setScreenMode(mode) {
    window.screenEditor.mode = mode === 'night' ? 'night' : 'day';
    updateScreenModeButtons();
    renderScreenCanvas();
    renderScreenOptions();
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
    if (isScreenStaticTextElement(def.id)) {
        return { w: PALETTE_SWATCH_SIZE, h: PALETTE_SWATCH_SIZE };
    }
    if (def.id === 'message') {
        return { w: PALETTE_SWATCH_SIZE, h: 8 };
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

function setupSpriteBackground(preview, imgUrl, nativeH, displayW, displayH) {
    const scale = displayH / nativeH;
    const sheet = new Image();
    sheet.onload = () => {
        preview.style.backgroundImage = `url(${imgUrl})`;
        preview.style.backgroundRepeat = 'no-repeat';
        preview.style.backgroundPosition = 'left top';
        preview.style.backgroundSize = `${sheet.naturalWidth * scale}px ${displayH}px`;
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
    setupSpriteBackground(preview, imgUrl || def.img, def.h, size.w, size.h);
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
            if (def.paletteFitFullImage) {
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
    if (!inMatrix && def.id === 'text') {
        preview.classList.add('screen-preview-palette-static-label');
        text.classList.add('screen-preview-text-palette-static');
        text.textContent = getScreenElementLabel('text');
    } else if (!inMatrix && isScreenStaticTextElement(def.id)) {
        preview.classList.add('screen-preview-palette-static-label');
        text.classList.add('screen-preview-text-palette-static');
        const label = document.createElement('span');
        label.className = 'screen-preview-text-word';
        label.textContent = 'Text';
        const digit = document.createElement('span');
        digit.className = 'screen-preview-text-digit';
        digit.textContent = def.id.replace('text_', '');
        text.appendChild(label);
        text.appendChild(digit);
    } else {
        text.textContent = def.text || def.label;
        if (isScreenTextElement(def.id)) {
            const font = elementState && elementState.options && elementState.options.font !== undefined ? elementState.options.font : 0;
            const pt = SCREEN_MESSAGE_FONT_HEIGHTS[font] || 8;
            const previewScale = inMatrix ? (matrixScale || getScreenScale()) : 1;
            text.style.fontSize = `${pt * previewScale}px`;
            if (elementState && elementState.options) {
                if (elementState.options.color) text.style.color = elementState.options.color;
                if (elementState.options.bg_color) {
                    preview.style.backgroundColor = elementState.options.bg_color;
                }
            }
            if (isScreenStaticTextElement(def.id)) {
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

        const slotId = def.id === 'text' && profile ? resolvePaletteElementId(def.id, profile) : def.id;
        const unavailable = def.id === 'text' && !slotId;
        if (unavailable) {
            item.classList.add('palette-item-disabled');
            item.setAttribute('aria-disabled', 'true');
            item.tabIndex = -1;
        } else {
            item.tabIndex = 0;
        }

        const swatch = document.createElement('div');
        swatch.className = 'palette-swatch';
        const palettePreview = createScreenPreview(def, false, null);
        swatch.appendChild(palettePreview);

        const label = document.createElement('div');
        label.textContent = getScreenElementLabel(def.id);

        item.appendChild(swatch);
        item.appendChild(label);

        if (!unavailable) {
            item.addEventListener('pointerdown', (e) => {
                e.preventDefault();
                const resolvedId = resolvePaletteElementId(def.id, getProfileObj(window.screenEditor.mode));
                if (!resolvedId) return;
                beginDrag(resolvedId, e, true);
            });
            item.addEventListener('keydown', (e) => {
                if (e.key === 'Enter' || e.key === ' ') {
                    e.preventDefault();
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
            fetch('/api/settings?params=p24,p50'),
            fetch('/api/screen')
        ]);
        if (settingsResp.ok) {
            const settingsData = await settingsResp.json();
            Object.keys(settingsData).forEach(key => {
                window.settings[key] = settingsData[key];
            });
        }
        const data = await response.json();
        window.screenLayout = prepareScreenLayout(data);
        if (!window.screenLayout.profiles) window.screenLayout.profiles = { day: makeDefaultScreenProfile(), night: makeDefaultScreenProfile() };
        if (!window.screenLayout.profiles.day) window.screenLayout.profiles.day = makeDefaultScreenProfile();
        if (!window.screenLayout.profiles.night) window.screenLayout.profiles.night = makeDefaultScreenProfile();
        ensureScreenLayoutMeta(window.screenLayout);
        ensureScreenProfileShape(window.screenLayout.profiles.day);
        ensureScreenProfileShape(window.screenLayout.profiles.night);
        renderScreenCanvas();
        renderScreenOptions();
    } catch (error) {
        console.error('Error loading screen layout:', error);
        showStatus(getMessage('error_loading_settings'), 'error');
    }
}

async function saveScreenTimeDisplaySettings() {
    const leading = el('screen_show_leading_zero');
    const dots = el('screen_dots_breathe');
    if (!leading && !dots) return true;

    const formData = {};
    if (leading) formData.p24 = leading.checked ? 1 : 0;
    if (dots) formData.p50 = dots.checked ? 1 : 0;

    const unchanged = (leading === null || formData.p24 === window.settings.p24)
        && (dots === null || formData.p50 === window.settings.p50);
    if (unchanged) return true;

    try {
        const response = await fetch('/api/settings', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(formData)
        });
        const data = await response.json().catch(() => ({}));
        if (response.ok && data && data.status === 'ok') {
            Object.assign(window.settings, formData);
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
        const response = await fetch('/api/screen', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(window.screenLayout)
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
            showStatus(getMessage('error_saving_settings') + (data && data.error ? data.error : 'Unknown error'), 'error');
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
    if (!filename) {
        showStatus(getMessage('layout_system_select_file'), 'error');
        return;
    }
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
    await saveScreenLayout();
}

function copyScreenLayoutToOtherMode() {
    if (!window.screenLayout || !window.screenLayout.profiles) return;
    const source = window.screenEditor.mode;
    const target = source === 'day' ? 'night' : 'day';
    const sourceProfile = getProfileObj(source);
    if (!sourceProfile) return;

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

function appendScreenGuides(canvas, scale) {
    canvas.appendChild(createScreenGuideLine('v', 64 * scale, 'matrix'));
    canvas.appendChild(createScreenGuideLine('h', 64 * scale, 'matrix'));

    const selectedId = window.screenEditor.selectedId;
    if (!selectedId) return;

    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return;
    const elem = profile.elements.find(item => item.id === selectedId);
    const def = findElementDef(selectedId);
    if (!elem || !def) return;

    const size = getScreenElementSize(def, elem);
    canvas.appendChild(createScreenGuideLine('v', (elem.x + size.w / 2) * scale, 'selection'));
    canvas.appendChild(createScreenGuideLine('h', (elem.y + size.h / 2) * scale, 'selection'));
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
        box.style.left = `${(e.x || 0) * scale}px`;
        box.style.top = `${(e.y || 0) * scale}px`;
        box.style.width = `${size.w * scale}px`;
        box.style.height = `${size.h * scale}px`;
        const layer = getElementLayer(e);
        box.style.zIndex = String(window.screenEditor.selectedId === e.id ? 12 + layer : 3 + layer);

        box.appendChild(createScreenPreview(def, true, e, scale));

        box.addEventListener('pointerdown', (ev) => {
            ev.preventDefault();
            ev.stopPropagation();
            window.screenEditor.selectedId = e.id;
            beginDrag(e.id, ev, false);
            renderScreenCanvas();
            renderScreenOptions();
        });

        canvas.appendChild(box);
    });

    appendScreenGuides(canvas, scale);
    updateScreenStatusLine();
}

async function appendScreenFontSamples(container, selectedFont) {
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
            if (mode === 'night') window.screenLayout.night_font = font;
            else window.screenLayout.day_font = font;
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

function renderScreenOptions() {
    const opt = el('screenOptions');
    if (!opt) return;
    opt.innerHTML = '';
    if (!window.screenLayout || !window.screenEditor.selectedId) return;

    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return;
    const e = ensureElementInProfile(profile, window.screenEditor.selectedId);
    const def = findElementDef(e.id);
    const trans = translations[currentLanguage] || translations.en;

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

    const enabledRow = document.createElement('div');
    enabledRow.className = 'form-group';
    const enabledLabel = document.createElement('label');
    enabledLabel.textContent = getNestedTranslation(trans, 'screen.enabled') || 'Enabled';
    const enabledInput = document.createElement('input');
    enabledInput.type = 'checkbox';
    enabledInput.checked = e.enabled !== 0;
    enabledInput.addEventListener('change', () => {
        e.enabled = enabledInput.checked ? 1 : 0;
        renderScreenCanvas();
        if (isScreenStaticTextElement(e.id)) renderScreenPalette();
    });
    enabledRow.appendChild(enabledLabel);
    enabledRow.appendChild(enabledInput);
    opt.appendChild(enabledRow);

    if (e.id === 'time') {
        const mode = window.screenEditor.mode;
        const fontKey = mode === 'night' ? 'night_font' : 'day_font';
        const filterKey = mode === 'night' ? 'night_color_filter' : 'day_color_filter';
        ensureScreenLayoutMeta(window.screenLayout);

        const modeNote = document.createElement('p');
        modeNote.className = 'screen-options-mode-note';
        modeNote.textContent = mode === 'night'
            ? (getNestedTranslation(trans, 'screen.night_profile') || 'Night profile')
            : (getNestedTranslation(trans, 'screen.day_profile') || 'Day profile');
        opt.appendChild(modeNote);

        const timeFontRow = document.createElement('div');
        timeFontRow.className = 'form-group';
        const timeFontLabel = document.createElement('label');
        timeFontLabel.textContent = getNestedTranslation(trans, 'screen.time_font') || 'Time font';
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
        opt.appendChild(timeFontRow);

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
        });
        filterRow.appendChild(filterLabel);
        filterRow.appendChild(filterSelect);
        opt.appendChild(filterRow);

        appendScreenFontSamples(opt, window.screenLayout[fontKey] || 'bold');

        const leadingRow = document.createElement('div');
        leadingRow.className = 'checkbox-container';
        const leadingInput = document.createElement('input');
        leadingInput.type = 'checkbox';
        leadingInput.id = 'screen_show_leading_zero';
        leadingInput.checked = !!(window.settings && window.settings.p24);
        const leadingLabel = document.createElement('label');
        leadingLabel.htmlFor = 'screen_show_leading_zero';
        leadingLabel.textContent = getNestedTranslation(trans, 'screen.show_leading_zero')
            || 'Show 0 for single digit hour';
        leadingRow.appendChild(leadingInput);
        leadingRow.appendChild(leadingLabel);
        opt.appendChild(leadingRow);

        const breatheRow = document.createElement('div');
        breatheRow.className = 'checkbox-container';
        const breatheInput = document.createElement('input');
        breatheInput.type = 'checkbox';
        breatheInput.id = 'screen_dots_breathe';
        breatheInput.checked = !!(window.settings && window.settings.p50);
        const breatheLabel = document.createElement('label');
        breatheLabel.htmlFor = 'screen_dots_breathe';
        breatheLabel.textContent = getNestedTranslation(trans, 'screen.disable_dots_breathe')
            || 'Disable breathing time dots';
        breatheRow.appendChild(breatheInput);
        breatheRow.appendChild(breatheLabel);
        opt.appendChild(breatheRow);
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

        if (isScreenStaticTextElement(e.id)) {
            const widthRow = document.createElement('div');
            widthRow.className = 'form-group';
            const widthLabel = document.createElement('label');
            widthLabel.textContent = getNestedTranslation(trans, 'screen.text_width') || 'Width (px, 0 = no limit)';
            const widthInput = document.createElement('input');
            widthInput.type = 'number';
            widthInput.min = '0';
            widthInput.max = '127';
            widthInput.value = String(e.options.width || 0);
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
        textArea.rows = e.id === 'message' ? 3 : 2;
        if (e.id === 'message') {
            textArea.value = profile.scroll_text || '';
            textArea.addEventListener('input', () => {
                profile.scroll_text = textArea.value;
                renderScreenCanvas();
            });
        } else {
            if (!profile.static_texts) profile.static_texts = { ...SCREEN_DEFAULT_STATIC_TEXTS };
            textArea.value = profile.static_texts[e.id] || '';
            textArea.addEventListener('input', () => {
                profile.static_texts[e.id] = textArea.value;
                renderScreenCanvas();
            });
        }
        textRow.appendChild(textLabel);
        textRow.appendChild(textArea);
        setupTokenHighlightTextarea(textArea);
        opt.appendChild(textRow);
        appendScreenTokenButtons(opt, textArea);
    }
}

function beginDrag(id, pointerEvent, fromPalette) {
    const canvas = el('screenCanvas');
    if (!canvas) return;
    if (!window.screenLayout) return;
    const profile = getProfileObj(window.screenEditor.mode);
    if (!profile) return;

    const elem = ensureElementInProfile(profile, id);

    const scale = getScreenScale();
    window.screenEditor.scale = scale;

    let startLeft = elem.x * scale;
    let startTop = elem.y * scale;

    if (fromPalette) {
        window.screenEditor.selectedId = id;
        renderScreenCanvas();
        renderScreenOptions();
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
    const target = e.target && e.target.closest ? e.target.closest('.screen-element') : null;
    if (!target) {
        window.screenEditor.selectedId = null;
        renderScreenCanvas();
        renderScreenOptions();
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
        elem.x = clamp(Math.round((e.clientX - rect.left - 10) / scale), 0, SCREEN_SIZE - 1);
        elem.y = clamp(Math.round((e.clientY - rect.top - 10) / scale), 0, SCREEN_SIZE - 1);
    } else {
        const newLeft = d.startLeft + dx;
        const newTop = d.startTop + dy;
        elem.x = clamp(Math.round(newLeft / scale), 0, SCREEN_SIZE - 1);
        elem.y = clamp(Math.round(newTop / scale), 0, SCREEN_SIZE - 1);
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
    }

    // Populate fields if settings are loaded
    if (window.settings && window.settingsLoaded.advanced) {
        const messageInput = el('message');
        const messageCounter = el('message-counter');
        if (el('ofs_x') && window.settings.p01 !== undefined) el('ofs_x').value = window.settings.p01 || 0;
        if (el('ofs_y') && window.settings.p02 !== undefined) el('ofs_y').value = window.settings.p02 || 0;
        if (el('rotation') && window.settings.p03 !== undefined) el('rotation').value = window.settings.p03 || 0;
        if (el('show_grid') && window.settings.p08 !== undefined) el('show_grid').checked = window.settings.p08;
        if (el('mirroring') && window.settings.p09 !== undefined) el('mirroring').checked = window.settings.p09;
        if (el('lat') && window.settings.p17 !== undefined) el('lat').value = window.settings.p17 || '';
        if (el('lon') && window.settings.p18 !== undefined) el('lon').value = window.settings.p18 || '';
        if (el('timezone') && window.settings.p19 !== undefined) el('timezone').value = window.settings.p19 || '';
        if (el('wifi_start') && window.settings.p46 !== undefined) el('wifi_start').value = window.settings.p46 || 0;
        if (el('wifi_end') && window.settings.p47 !== undefined) el('wifi_end').value = window.settings.p47 || 0;
        if (el('lux_sensitivity') && window.settings.p20 !== undefined) el('lux_sensitivity').value = window.settings.p20 || 2.5;
        if (el('lux_threshold') && window.settings.p21 !== undefined) el('lux_threshold').value = window.settings.p21 || 50;
        if (el('dim_mode') && window.settings.p22 !== undefined) el('dim_mode').value = String(window.settings.p22);
        if (el('dim_start') && window.settings.p55 !== undefined) el('dim_start').value = window.settings.p55 || 0;
        if (el('dim_end') && window.settings.p56 !== undefined) el('dim_end').value = window.settings.p56 || 0;
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
    row.style.cssText = 'display:flex;align-items:center;gap:8px;margin-bottom:6px;flex-wrap:wrap;';

    const typeSelect = document.createElement('select');
    typeSelect.style.cssText = 'flex:0 0 auto;';
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
    durInput.style.cssText = 'width:70px;flex:0 0 auto;';
    durInput.title = 'Duration (seconds)';

    const durLabel = document.createElement('span');
    durLabel.textContent = 'sec';
    durLabel.style.cssText = 'flex:0 0 auto;color:#999;font-size:0.9em;';

    const entityInput = document.createElement('input');
    entityInput.type = 'text';
    entityInput.placeholder = 'entity_id (e.g. sensor.temp)';
    entityInput.value = slot.e || '';
    entityInput.style.cssText = 'flex:1 1 160px;min-width:120px;';
    entityInput.style.display = (slot.t === 3) ? '' : 'none';

    const unitInput = document.createElement('input');
    unitInput.type = 'text';
    unitInput.placeholder = 'unit (e.g. °F)';
    unitInput.maxLength = 7;
    unitInput.value = slot.l || '';
    unitInput.style.cssText = 'width:70px;flex:0 0 auto;';
    unitInput.style.display = (slot.t === 3) ? '' : 'none';

    const nameInput = document.createElement('input');
    nameInput.type = 'text';
    nameInput.placeholder = 'name (e.g. Outside Temp)';
    nameInput.maxLength = 31;
    nameInput.value = slot.n || '';
    nameInput.style.cssText = 'flex:1 1 120px;min-width:100px;';
    nameInput.style.display = (slot.t === 2 || slot.t === 3) ? '' : 'none';

    typeSelect.addEventListener('change', () => {
        const t = parseInt(typeSelect.value);
        const isHA = (t === 3);
        const hasName = (t === 2 || t === 3);
        entityInput.style.display = isHA ? '' : 'none';
        unitInput.style.display   = isHA ? '' : 'none';
        nameInput.style.display   = hasName ? '' : 'none';
    });

    const removeBtn = document.createElement('button');
    removeBtn.type = 'button';
    removeBtn.textContent = '✕';
    removeBtn.style.cssText = 'flex:0 0 auto;cursor:pointer;padding:2px 8px;';
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

function renderDisplaySchedule() {
    const list = el('display-schedule-list');
    if (!list) return;
    list.innerHTML = '';

    let slots = [];
    if (typeof window.settings.p58 === 'string' && window.settings.p58.trim().startsWith('[')) {
        try { slots = JSON.parse(window.settings.p58); } catch(e) { slots = []; }
    }
    if (!Array.isArray(slots) || slots.length === 0) {
        // Migrate from legacy p48/p49/p57 defaults so the UI isn't blank
        if ((window.settings.p48 || 0) > 0) slots.push({t:0, d: window.settings.p48});
        if ((window.settings.p49 || 0) > 0) slots.push({t:1, d: window.settings.p49});
        if ((window.settings.p57 || 0) > 0) slots.push({t:2, d: window.settings.p57});
        if (slots.length === 0) slots.push({t:0, d:30});
    }
    slots.forEach(s => list.appendChild(buildSlotRow(s)));

    const addBtn = el('add-schedule-slot');
    if (addBtn && !addBtn._listenerSet) {
        addBtn._listenerSet = true;
        addBtn.addEventListener('click', () => {
            list.appendChild(buildSlotRow({t:0, d:30}));
        });
    }
}

function serializeDisplaySchedule() {
    const list = el('display-schedule-list');
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
            if (nameEl && nameEl.value.trim()) slot.n = nameEl.value.trim();
        } else if (t === 2) {
            if (nameEl && nameEl.value.trim()) slot.n = nameEl.value.trim();
        }
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

        renderDisplaySchedule();

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

// Helper to format uptime
function formatUptime(seconds) {
    const d = Math.floor(seconds / 86400);
    const h = Math.floor((seconds % 86400) / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = Math.floor(seconds % 60);
    return `${d}d ${h}h ${m}m ${s}s`;
}

// Helper to format Power On Hours into user-friendly format
function formatPOH(hours) {
    if (hours === 0) return '0h';
    
    const years = Math.floor(hours / (365 * 24));
    const months = Math.floor((hours % (365 * 24)) / (30 * 24));
    const days = Math.floor((hours % (30 * 24)) / 24);
    const remainingHours = hours % 24;
    
    let result = '';
    if (years > 0) result += `${years}y `;
    if (months > 0) result += `${months}m `;
    if (days > 0) result += `${days}d `;
    if (remainingHours > 0 || result === '') result += `${remainingHours}h`;
    
    return result.trim();
}

// Helper function to safely get text content
function safeGetTextContent(elementId) {
    const element = el(elementId);
    return element ? (element.textContent || element.innerText || '') : 'N/A';
}

// Helper function to safely get input/textarea value
function safeGetValue(elementId) {
    const element = el(elementId);
    return element ? (element.value || '') : 'N/A';
}

// Function to collect all system information
function collectSystemInfo() {
    const info = {
        // Sensor Data
        sensorData: {
            lux: safeGetTextContent('lux'),
            timeUpdate: safeGetTextContent('time_update_status'),
            timezone: safeGetTextContent('timezone_val'),
            moonPhase: safeGetTextContent('moon_status'),
            weatherUpdate: safeGetTextContent('weather_update_status'),
            latitude: safeGetTextContent('latitude'),
            longitude: safeGetTextContent('longitude'),
            uptime: safeGetTextContent('uptime')
        },

        // System Information
        systemInfo: {
            app: safeGetTextContent('app'),
            version: safeGetTextContent('version'),
            fwversion: safeGetTextContent('fwversion'),
            revision: safeGetTextContent('revision'),
            macAddress: safeGetTextContent('mac_address'),
            ipAddress: safeGetTextContent('ip_address'),
            chipRevision: safeGetTextContent('chip_revision'),
            flashSize: safeGetTextContent('flash_size'),
            cpuFreq: safeGetTextContent('cpu_freq'),
            compileTime: safeGetTextContent('compile_time'),
            freeHeap: safeGetTextContent('free_heap'),
            minFreeHeap: safeGetTextContent('min_free_heap')
        },

        // System Logs
        systemLogs: safeGetValue('system_logs'),

        // Integration Status
        integrationStatus: safeGetValue('ha_status_textarea'),

        // Settings Information - Use window.settings for checkbox values to ensure accuracy
        settings: {
                    hostname: window.settings ? window.settings.p00 : (el('hostname') ? el('hostname').value : ''),
        wifi_ssid: window.settings ? window.settings.p34 : (el('wifi_ssid') ? el('wifi_ssid').value : ''),
        fahrenheit: window.settings ? (window.settings.p36 ? 'Yes' : 'No') : 'Unknown',
        hour12: window.settings ? (window.settings.p37 ? 'Yes' : 'No') : 'Unknown',
        update_firmware: window.settings ? (window.settings.p39 ? 'Yes' : 'No') : 'Unknown',
        rotation: window.settings ? window.settings.p03 : (el('rotation') ? el('rotation').value : ''),
        dayfont: window.screenLayout ? window.screenLayout.day_font : (window.settings ? window.settings.p04 : 'bold'),
        nightfont: window.screenLayout ? window.screenLayout.night_font : (window.settings ? window.settings.p05 : 'bold'),
        quiet_scroll: window.settings ? (window.settings.p06 ? 'Yes' : 'No') : 'Unknown',
        quiet_weather: window.settings ? (window.settings.p07 ? 'Yes' : 'No') : 'Unknown',
        show_grid: window.settings ? (window.settings.p08 ? 'Yes' : 'No') : 'Unknown',
        mirroring: window.settings ? (window.settings.p09 ? 'Yes' : 'No') : 'Unknown',
        color_filter: window.screenLayout ? window.screenLayout.day_color_filter : (window.settings ? window.settings.p10 : 0),
        night_color_filter: window.screenLayout ? window.screenLayout.night_color_filter : (window.settings ? window.settings.p11 : 0),
        msg_color: window.settings ? window.settings.p12 : (el('msg_color') ? el('msg_color').value : ''),
        msg_font: window.settings ? window.settings.p13 : (el('msg_font') ? el('msg_font').value : ''),
        night_msg_color: window.settings ? window.settings.p15 : (el('night_msg_color') ? el('night_msg_color').value : ''),
        message: window.settings ? window.settings.p16 : (el('message') ? el('message').value : ''),
        lat: window.settings ? window.settings.p17 : (el('lat') ? el('lat').value : ''),
        lon: window.settings ? window.settings.p18 : (el('lon') ? el('lon').value : ''),
        timezone: window.settings ? window.settings.p19 : (el('timezone') ? el('timezone').value : ''),
        lux_sensitivity: window.settings ? window.settings.p20 : (el('lux_sensitivity') ? el('lux_sensitivity').value : ''),
        lux_threshold: window.settings ? window.settings.p21 : (el('lux_threshold') ? el('lux_threshold').value : ''),
        dim_mode: window.settings ? String(window.settings.p22 ?? 0) : 'Unknown',
        dim_start: window.settings ? String(window.settings.p55 ?? 0) : 'Unknown',
        dim_end: window.settings ? String(window.settings.p56 ?? 0) : 'Unknown',
        brightness_LED0: window.settings && window.settings.p23 ? window.settings.p23[0] : (el('brightness_LED0') ? el('brightness_LED0').value : ''),
        brightness_LED1: window.settings && window.settings.p23 ? window.settings.p23[1] : (el('brightness_LED1') ? el('brightness_LED1').value : ''),
        show_leading_zero: window.settings ? (window.settings.p24 ? 'Yes' : 'No') : 'Unknown',
        eeprom_ha_url: window.settings ? window.settings.p25 : (el('eeprom_ha_url') ? el('eeprom_ha_url').value : ''),
        eeprom_ha_token: window.settings ? (window.settings.p26 ? 'Configured' : 'Not configured') : 'Unknown',
        eeprom_ha_refresh_mins: window.settings ? window.settings.p27 : (el('eeprom_ha_refresh_mins') ? el('eeprom_ha_refresh_mins').value : ''),
        eeprom_stock_key: window.settings ? (window.settings.p28 ? 'Configured' : 'Not configured') : 'Unknown',
        eeprom_stock_refresh_mins: window.settings ? window.settings.p29 : (el('eeprom_stock_refresh_mins') ? el('eeprom_stock_refresh_mins').value : ''),
        eeprom_dexcom_region: window.settings ? window.settings.p30 : (el('eeprom_dexcom_region') ? el('eeprom_dexcom_region').value : ''),
        eeprom_libre_region: window.settings ? window.settings.p44 : (el('eeprom_libre_region') ? el('eeprom_libre_region').value : ''),
        eeprom_ns_url: window.settings ? (window.settings.p54 || '') : (el('eeprom_ns_url') ? el('eeprom_ns_url').value : ''),
        eeprom_glucose_username: window.settings ? (window.settings.p31 || (el('eeprom_glucose_username') ? el('eeprom_glucose_username').value : '')) : (el('eeprom_glucose_username') ? el('eeprom_glucose_username').value : ''),
        eeprom_glucose_password: window.settings ? (window.settings.p32 ? 'Configured' : 'Not configured') : 'Unknown',
        eeprom_glucose_refresh: window.settings ? (window.settings.p33 || (el('eeprom_glucose_refresh') ? el('eeprom_glucose_refresh').value : '')) : (el('eeprom_glucose_refresh') ? el('eeprom_glucose_refresh').value : ''),
        eeprom_glucose_high: window.settings ? window.settings.p51 : (el('eeprom_glucose_high') ? el('eeprom_glucose_high').value : ''),
        eeprom_glucose_low: window.settings ? window.settings.p52 : (el('eeprom_glucose_low') ? el('eeprom_glucose_low').value : '')
        }
    };

    return info;
}

// Function to format system information for email
function formatSystemInfoForEmail(info) {
    let emailBody = 'Frixos System Information Report\n\n';
    
    // Sensor Data
    emailBody += '=== SENSOR DATA ===\n';
    emailBody += `Current Light Level: ${info.sensorData.lux}\n`;
    emailBody += `Last Time Update: ${info.sensorData.timeUpdate}\n`;
    emailBody += `Timezone: ${info.sensorData.timezone}\n`;
    emailBody += `Moon Phase: ${info.sensorData.moonPhase}\n`;
    emailBody += `Last Weather Update: ${info.sensorData.weatherUpdate}\n`;
    emailBody += `Latitude: ${info.sensorData.latitude}\n`;
    emailBody += `Longitude: ${info.sensorData.longitude}\n`;
    emailBody += `Uptime: ${info.sensorData.uptime}\n\n`;
    
    // System Information
    emailBody += '=== SYSTEM INFORMATION ===\n';
    emailBody += `Application Name: ${info.systemInfo.app}\n`;
    emailBody += `Version: ${info.systemInfo.version}\n`;
    emailBody += `Firmware Version: ${info.systemInfo.fwversion}\n`;
    emailBody += `Revision: ${info.systemInfo.revision}\n`;
    emailBody += `MAC Address: ${info.systemInfo.macAddress}\n`;
    emailBody += `IP Address: ${info.systemInfo.ipAddress}\n`;
    emailBody += `Chip Revision: ${info.systemInfo.chipRevision}\n`;
    emailBody += `SPI Flash Size: ${info.systemInfo.flashSize}\n`;
    emailBody += `CPU Frequency: ${info.systemInfo.cpuFreq}\n`;
    emailBody += `Compile Time: ${info.systemInfo.compileTime}\n`;
    emailBody += `Free Heap Memory: ${info.systemInfo.freeHeap}\n`;
    emailBody += `Min Free Heap: ${info.systemInfo.minFreeHeap}\n\n`;
    
    // Settings
    emailBody += '=== SETTINGS ===\n';
    emailBody += `Hostname: ${info.settings.hostname}\n`;
    emailBody += `WiFi SSID: ${info.settings.wifi_ssid}\n`;
    emailBody += `Use US units: ${info.settings.fahrenheit}\n`;
    emailBody += `Time Format: ${info.settings.hour12 === 'Yes' ? '12-hour' : '24-hour'}\n`;
    emailBody += `Auto Update: ${info.settings.update_firmware === 'Yes' ? 'Enabled' : 'Disabled'}\n`;
    emailBody += `Display Rotation: ${info.settings.rotation}°\n`;
    emailBody += `Day Font: ${info.settings.dayfont}\n`;
    emailBody += `Day Color Filter: ${info.settings.color_filter}\n`;
    emailBody += `Night Font: ${info.settings.nightfont}\n`;
    emailBody += `Night Color Filter: ${info.settings.night_color_filter}\n`;
    emailBody += `Quiet Scroll: ${info.settings.quiet_scroll === 'Yes' ? 'Enabled' : 'Disabled'}\n`;
    emailBody += `Quiet Weather: ${info.settings.quiet_weather === 'Yes' ? 'Enabled' : 'Disabled'}\n`;
    emailBody += `Show Leading Zero: ${info.settings.show_leading_zero === 'Yes' ? 'Enabled' : 'Disabled'}\n`;
    emailBody += `Show Grid: ${info.settings.show_grid === 'Yes' ? 'Enabled' : 'Disabled'}\n`;
    emailBody += `Mirror Display: ${info.settings.mirroring === 'Yes' ? 'Enabled' : 'Disabled'}\n`;
    emailBody += `Latitude: ${info.settings.lat}\n`;
    emailBody += `Longitude: ${info.settings.lon}\n`;
    emailBody += `Timezone: ${info.settings.timezone}\n`;
    emailBody += `Lux Threshold: ${info.settings.lux_threshold}\n`;
    emailBody += `Lux Sensitivity: ${info.settings.lux_sensitivity}\n`;
    emailBody += `Dim Mode: ${info.settings.dim_mode}\n`;
    emailBody += `Dim Hours: ${info.settings.dim_start}-${info.settings.dim_end}\n`;
    emailBody += `Brightness (Day): ${info.settings.brightness_LED0}%\n`;
    emailBody += `Brightness (Night): ${info.settings.brightness_LED1}%\n`;
    emailBody += `Message: ${info.settings.message}\n\n`;
    
    // Integration Settings
    emailBody += '=== INTEGRATION SETTINGS ===\n';
    emailBody += `Home Assistant URL: ${info.settings.eeprom_ha_url || 'Not configured'}\n`;
    emailBody += `Home Assistant Token: ${info.settings.eeprom_ha_token ? 'Configured' : 'Not configured'}\n`;
    emailBody += `Home Assistant Refresh Interval: ${info.settings.eeprom_ha_refresh_mins || 'Not configured'} minutes\n`;
    emailBody += `Stock Quote API Key: ${info.settings.eeprom_stock_key}\n`;
    emailBody += `Stock Quote Refresh Interval: ${info.settings.eeprom_stock_refresh_mins || 'Not configured'} minutes\n`;
    emailBody += `Dexcom Region: ${info.settings.eeprom_dexcom_region || 'Not configured'}\n`;
    emailBody += `Libre Region: ${info.settings.eeprom_libre_region || 'Not configured'}\n`;
    emailBody += `Nightscout URL: ${info.settings.eeprom_ns_url || 'Not configured'}\n`;
    emailBody += `Glucose Username: ${info.settings.eeprom_glucose_username || 'Not configured'}\n`;
    emailBody += `Glucose Password: ${info.settings.eeprom_glucose_password}\n`;
    emailBody += `Glucose Refresh Interval: ${info.settings.eeprom_glucose_refresh || 'Not configured'} minutes\n`;
    emailBody += `Glucose High Threshold: ${info.settings.eeprom_glucose_high || 'Not configured'} mg/dL\n`;
    emailBody += `Glucose Low Threshold: ${info.settings.eeprom_glucose_low || 'Not configured'} mg/dL\n\n`;
    
    // System Logs
    emailBody += '=== SYSTEM LOGS ===\n';
    emailBody += info.systemLogs + '\n\n';
    
    // Integration Status
    emailBody += '=== INTEGRATION STATUS ===\n';
    emailBody += info.integrationStatus + '\n\n';
    
    emailBody += '=== END CONTENT ===';
    
    return emailBody;
}

// Function to load all data from all sections before collecting system info
async function loadAllSystemData() {
    const promises = [];
    
    // Always fetch status data (populates DOM elements)
    promises.push(fetchStatus(true).catch(err => {
        console.warn('Error fetching status:', err);
        return null; // Continue even if status fails
    }));
    
    // Load all section parameters if not already loaded
    if (!window.settingsLoaded.settings) {
        promises.push(fetchSectionParams('settings').catch(err => {
            console.warn('Error fetching settings params:', err);
            return null;
        }));
    }
    
    if (!window.settingsLoaded.advanced) {
        promises.push(fetchSectionParams('advanced').catch(err => {
            console.warn('Error fetching advanced params:', err);
            return null;
        }));
    }
    
    if (!window.settingsLoaded.integrations) {
        promises.push(fetchSectionParams('integrations').catch(err => {
            console.warn('Error fetching integrations params:', err);
            return null;
        }));
    }
    
    // Wait for all data to load
    await Promise.all(promises);
}

// Function to send system information to support
async function sendSystemInfoToSupport() {
    const btn = el('sendToSupportButton');
    if (btn) toggleLoading(btn, true);
    try {
        // Show loading message
        showStatus('Loading all system data...', 'info');
        
        // Load all data from all sections first
        await loadAllSystemData();
        
        // Now collect the system info
        const info = collectSystemInfo();
        const emailBody = formatSystemInfoForEmail(info);
        
        // Create mailto link
        const subject = encodeURIComponent("Frixos System Information Report");
        const body = encodeURIComponent(emailBody);
        const mailtoLink = `mailto:support@buyfrixos.com?subject=${subject}&body=${body}`;
        
        // Check if mailto link is too long (most browsers limit to ~2000-8000 chars)
        // If too long, fall back to clipboard method
        if (mailtoLink.length > 2000) {
            // Copy to clipboard and show instructions (skip data load since we already loaded it)
            await copySystemInfoToClipboard(true);
            showStatus('Email content copied to clipboard. Please paste it into your email client and send to support@buyfrixos.com', 'success');
            return;
        }
        
        // Try to open default email client
        try {
            window.location.href = mailtoLink;
            // Give user feedback
            setTimeout(() => {
                showStatus('Opening email client... If it doesn\'t open, use "Copy to clipboard" instead.', 'info');
            }, 100);
        } catch (err) {
            console.error('Failed to open mailto link:', err);
            // Fall back to clipboard (skip data load since we already loaded it)
            await copySystemInfoToClipboard(true);
            showStatus('Email client could not be opened. Content copied to clipboard instead. Please paste it into your email and send to support@buyfrixos.com', 'info');
        }
    } catch (error) {
        console.error('Error in sendSystemInfoToSupport:', error);
        showStatus('Error preparing email: ' + error.message + '. Try using "Copy to clipboard" instead.', 'error');
    } finally {
        if (btn) toggleLoading(btn, false);
    }
}

// Function to copy system information to clipboard
async function copySystemInfoToClipboard(skipDataLoad = false) {
    const btn = el('copyToClipboardButton');
    if (!skipDataLoad && btn) toggleLoading(btn, true);
    try {
        // Load all data from all sections first (unless already loaded)
        if (!skipDataLoad) {
            // Show loading message
            showStatus('Loading all system data...', 'info');
            await loadAllSystemData();
        }
        
        // Now collect the system info
        const info = collectSystemInfo();
        const emailBody = formatSystemInfoForEmail(info);
        
        // Try modern Clipboard API first (more reliable)
        if (navigator.clipboard && navigator.clipboard.writeText) {
            navigator.clipboard.writeText(emailBody).then(() => {
                showStatus(getMessage('info_copied_clipboard') || 'Information copied to clipboard!', 'success');
            }).catch(err => {
                console.error('Clipboard API failed, falling back to execCommand:', err);
                // Fall back to execCommand method
                copyToClipboardFallback(emailBody);
            });
        } else {
            // Fall back to execCommand for older browsers
            copyToClipboardFallback(emailBody);
        }
    } catch (error) {
        console.error('Error in copySystemInfoToClipboard:', error);
        showStatus(getMessage('error_preparing_info') + error.message, 'error');
    } finally {
        if (!skipDataLoad && btn) toggleLoading(btn, false);
    }
}

// Fallback method using execCommand (for older browsers)
function copyToClipboardFallback(text) {
    try {
        // Create a temporary textarea element
        const textArea = document.createElement('textarea');
        textArea.value = text;
        
        // Make the textarea out of viewport
        textArea.style.position = 'fixed';
        textArea.style.left = '-999999px';
        textArea.style.top = '-999999px';
        document.body.appendChild(textArea);
        
        // Select and copy the text
        textArea.focus();
        textArea.select();
        
        try {
            const successful = document.execCommand('copy');
            if (successful) {
                showStatus(getMessage('info_copied_clipboard') || 'Information copied to clipboard!', 'success');
            } else {
                throw new Error('Copy command failed');
            }
        } catch (err) {
            console.error('execCommand copy failed:', err);
            showStatus(getMessage('failed_copy_clipboard') || 'Failed to copy to clipboard. Please select and copy manually.', 'error');
        }
        
        // Clean up
        document.body.removeChild(textArea);
    } catch (error) {
        console.error('Error in copyToClipboardFallback:', error);
        showStatus('Error copying to clipboard: ' + error.message, 'error');
    }
} 