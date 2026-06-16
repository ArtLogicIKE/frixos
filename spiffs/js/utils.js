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
let fontSamplesCache = null;

// Helper to invalidate I18n element caches when dynamic content is added or replaced
function invalidateI18nCache() {
    i18nElementsCache = null;
    passwordTogglesCache = null;
    tokenCodesCache = null;
    languageOptionsCache = null;
    fontSamplesCache = null;
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
            insert: 'Insert',
            select_font: 'Select {name} font'
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
            read_from_file: 'Load from File'
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

    return window._translationPromises[lang] = fetch(`/i18n/language_${lang}.json`)
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


// Helper to parse time string HH:MM to minutes
function parseTimeStringToMins(timeStr) {
    if (!timeStr) return 0;
    const parts = timeStr.split(':');
    return (parseInt(parts[0] || '0', 10) * 60) + parseInt(parts[1] || '0', 10);
}

// Helper to format minutes to HH:MM string
function formatMinsToTimeString(mins) {
    const totalMins = parseInt(mins, 10) || 0;
    const h = Math.floor(totalMins / 60) % 24;
    const m = totalMins % 60;
    return String(h).padStart(2, '0') + ':' + String(m).padStart(2, '0');
}
