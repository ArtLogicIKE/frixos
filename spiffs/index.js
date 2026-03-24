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

// Helper for element selection
const el = (id) => document.getElementById(id);

// Translations object
const translations = {
    en: {
        menu: {
            settings: 'Settings',
            advanced: 'Advanced',
            integrations: 'Integrations',
            status: 'Status',
            update: 'Update',
            restart: 'Restart'
        },
        common: {
            save_settings: 'Save Settings',
            cancel: 'Cancel'
        },
        settings: {
            connection: {
                title: 'Connection Settings *',
                wifi_ssid: 'WiFi SSID',
                wifi_password: 'WiFi Password',
                hostname: 'Hostname',
                fahrenheit: 'Temperature in Fahrenheit',
                hour12: 'Time in 12-hour format',
                auto_update: 'Automatically check for firmware updates',
                restart_note: '* Changing connection settings will restart the device.'
            },
            wifi: {
                available_networks: 'Available WiFi Networks',
                scanning: 'Scanning for networks...',
                scan_button: 'Scan available networks'
            }
        },
        advanced: {
            display: {
                title: 'Display Settings',
                x_offset: 'X Offset',
                y_offset: 'Y Offset',
                rotation: 'Display Rotation',
                show_scroll: 'Show scrolling information message',
                show_weather: 'Show weather forecast icon and moon phase',
                show_leading_zero: 'Show 0 for single digit hour',
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
            brightness: {
                title: 'Projection brightness settings',
                auto_dimming: 'Automatic dimming settings',
                day_threshold: 'Day Threshold (lux)',
                light_sensitivity: 'Light Sensitivity (lux)',
                sensitivity_help: 'Sensitivity helps avoid unwanted brightness changes. Dark = Threshold - Sensitivity, Bright = Threshold + Sensitivity.',
                full_brightness: 'Maintain full brightness (reduces LED lifespan)',
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
                token_ha: 'Fetch settings from Home Assistant (needs enabled integration)',
                token_stock: 'Fetch stock price quote (needs enabled stock integration with Finnhub.io)',
                token_cgm_glucose: 'Latest glucose reading from your CGM device, formatted, e.g. 157 mg/dl (+3 @ 12:01). (needs active CGM integration)',
                token_cgm_reading: 'Latest glucose reading, plain, e.g. 157',
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
            failed_copy_clipboard: 'Failed to copy to clipboard'
        }
    }
};

// Translation loading cache - tracks which translations have been loaded
const translationsLoaded = { en: true }; // English is always loaded

// Async function to load translations for a specific language from JSON file
async function loadTranslations(lang) {
    if (translationsLoaded[lang] || lang === 'en') return;
    if (window._translationPromises?.[lang]) return window._translationPromises[lang];
    if (!window._translationPromises) window._translationPromises = {};

    return window._translationPromises[lang] = fetch(`/language_${lang}.json`)
        .then(res => res.ok ? res.json() : Promise.reject())
        .then(data => { translations[lang] = data; })
        .catch(() => { translations[lang] = translations.en; })
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

// Setup password visibility toggles
function setupPasswordToggles() {
    document.querySelectorAll('.password-toggle').forEach(button => {
        button.addEventListener('click', function() {
            const input = this.previousElementSibling;
            if (!input) return;

            const isPassword = input.type === 'password';
            input.type = isPassword ? 'text' : 'password';

            // Toggle icon
            this.textContent = isPassword ? '🙈' : '👁️';

            // Update aria-label programmatically to avoid overwriting the icon with translation text
            const labelKey = isPassword ? 'common.hide_password' : 'common.show_password';
            const translation = getNestedTranslation(translations[currentLanguage] || translations.en, labelKey);
            if (translation) {
                this.setAttribute('aria-label', translation);
            }

            // Restore focus to input
            input.focus();
        });
    });
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
let currentLanguage = 'en';

// Translation function - now async to support lazy-loading
// Optimization: Consolidated DOM traversal into a single pass using querySelectorAll and dataset
async function translate(lang) {
    await loadTranslations(lang);
    
    const effectiveLang = (lang !== 'en' && translations[lang] === translations.en) ? 'en' : lang;
    currentLanguage = effectiveLang;
    const trans = translations[effectiveLang];

    document.querySelectorAll('[data-i18n], [data-i18n-placeholder]').forEach(element => {
        const i18nKey = element.dataset.i18n;
        const i18nPlaceholderKey = element.dataset.i18nPlaceholder;

        if (i18nKey) {
            const translation = getNestedTranslation(trans, i18nKey);
            if (translation) element.innerHTML = translation;
        }

        if (i18nPlaceholderKey) {
            const translation = getNestedTranslation(trans, i18nPlaceholderKey);
            if (translation) element.placeholder = translation;
        }
    });

    // Update password toggles ARIA labels
    document.querySelectorAll('.password-toggle').forEach(button => {
        const input = button.previousElementSibling;
        if (input) {
            const isPassword = input.type === 'password';
            const labelKey = isPassword ? 'common.show_password' : 'common.hide_password';
            const translation = getNestedTranslation(trans, labelKey);
            if (translation) {
                button.setAttribute('aria-label', translation);
            }
        }
    });

    const nameElement = el('current-language-name');
    if (nameElement) nameElement.textContent = LANGUAGE_NAMES[effectiveLang] || LANGUAGE_NAMES['en'];

    const hash = window.location.hash.substring(1);
    if (hash && hash !== 'settings') {
        const sectionName = hash.charAt(0).toUpperCase() + hash.slice(1);
        const translatedSection = getNestedTranslation(trans, `menu.${hash}`) || sectionName;
        const pageTitleElement = el('page-title');
        if (pageTitleElement) pageTitleElement.textContent = 'Frixos - ' + translatedSection;
    }
}

// Setup language selector
function setupLanguageSelector() {
    const languageToggle = el('language-toggle');
    const languageDropdown = el('language-dropdown');
    
    // Toggle dropdown on button click
    languageToggle.addEventListener('click', function(e) {
        e.stopPropagation();
        languageDropdown.style.display = languageDropdown.style.display === 'none' ? 'block' : 'none';
    });
    
    // Close dropdown when clicking outside
    document.addEventListener('click', function(e) {
        if (!languageToggle.contains(e.target) && !languageDropdown.contains(e.target)) {
            languageDropdown.style.display = 'none';
        }
    });
    
    // Handle language selection
    document.querySelectorAll('.language-option').forEach(option => {
        option.addEventListener('click', function() {
            const selectedLang = this.getAttribute('data-lang');
            changeLanguage(selectedLang);
            languageDropdown.style.display = 'none';
        });
    });
}

// Change language function
async function changeLanguage(lang) {
    if (!LANGUAGES.includes(lang)) {
        console.error(`Invalid language: ${lang}`);
        return;
    }
    
    // Apply translation (now async to support lazy-loading)
    await translate(lang);
    
    // Save to NVS via API
    const languageIndex = LANGUAGES.indexOf(lang);
    fetch('/api/settings', {
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
    })
    .catch(error => {
        console.error('Error saving language preference:', error);
    });
}

// Track which sections have been initialized and which parameters have been loaded
window.sectionsInitialized = {
    settings: false,
    advanced: false,
    integrations: false,
    status: false,
    update: false,
    restart: false
};

window.settings = {}; // Will be populated as sections are loaded
window.settingsLoaded = {
    theme: false,      // p40, p41 for theme and language
    settings: false,   // p00, p34-p39 for settings section
    advanced: false,   // p01-p24, p42, p43 for advanced section
    integrations: false // p25-p33 for integrations section
};

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
            
            setupAdvancedSection();
            setupStatusSection();
            setupUpdateSection();
            setupRestartSection();
            setupIntegrationsSection();

            // Setup send to support and copy buttons
            setupSupportButtons();

            // Setup password visibility toggles
            setupPasswordToggles();
        })
        .catch(error => {
            console.error('Error during initialization:', error);
        });

    // Add refresh sensor button handler
    const refreshSensorButton = el('refreshSensorButton');
    if (refreshSensorButton) {
        refreshSensorButton.addEventListener('click', function() {
            fetchStatus()
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
    
    // Add click event listeners to menu items
    menuItems.forEach(item => {
        item.addEventListener('click', function(e) {
            e.preventDefault();
            const sectionId = this.getAttribute('href').substring(1);
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
    
    // Get all page sections
    const sections = document.querySelectorAll('.page-section');
    
    // Hide all sections first
    sections.forEach(section => {
        section.style.display = 'none';
    });
    
    // Show the selected section
    const currentSection = el(hash + '-section');
    if (currentSection) {
        currentSection.style.display = 'block';
        
        // Update page title
        el('page-title').textContent = 'Frixos - ' +
            hash.charAt(0).toUpperCase() + hash.slice(1);
        
        // Update active menu item
        document.querySelectorAll('.menu-item').forEach(item => {
            if (item.getAttribute('href') === '#' + hash) {
                item.classList.add('active');
            } else {
                item.classList.remove('active');
            }
        });
        
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
            fetchStatus();
            window.sectionsInitialized.status = true;
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
            fetchStatus();
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
        
        // PWM Frequency: 10-1000000 integer
        pwmFrequencyValidator: function(value) {
            const num = parseInt(value);
            if (isNaN(num)) return "Must be a whole number";
            if (num < 10 || num > 1000000) return "Must be between 10 and 1000000";
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
                error.textContent = errorMessage;
                error.style.display = 'block';
            } else {
                input.classList.remove('input-invalid');
                error.style.display = 'none';
            }
        });
        
        // Also validate on blur (helps catch errors after focus changes)
        input.addEventListener('blur', function() {
            const errorMessage = config.validator.call(this, this.value);
            
            if (errorMessage) {
                input.classList.add('input-invalid');
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
                
                const errorMessage = config.validator.call(input, input.value);
                
                if (errorMessage) {
                    e.preventDefault(); // Prevent form submission
                    input.classList.add('input-invalid');
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
            hostnameError.textContent = errorMessage;
            hostnameError.style.display = 'block';
        } else {
            hostnameInput.classList.remove('input-invalid');
            hostnameError.style.display = 'none';
        }
    });
    
    // For forms that contain hostname - prevent submission if invalid
    const settingsForm = el('settingsForm');
    const advancedForm = el('advancedForm');
    
    [settingsForm, advancedForm].forEach(form => {
        if (form) {
            form.addEventListener('submit', function(e) {
                const hostname = hostnameInput.value;
                const errorMessage = validateHostname(hostname);
                
                if (errorMessage) {
                    e.preventDefault(); // Prevent form submission
                    hostnameInput.classList.add('input-invalid');
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
 * - p07 = quiet_weather (Show weather forecast)
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
 * - p22 = dim_disable (Maintain full brightness)
 * - p23 = brightness_LED (LED brightness array)
 * - p24 = show_leading_zero (Show leading zero)
 * - p42 = pwm_frequency (PWM frequency in Hz, range 10-1000000)
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
 * - p48 = eeprom_sec_time (Alternate time display duration in seconds)
 * - p49 = eeprom_sec_cgm (Alternate CGM display duration in seconds)
 * - p51 = eeprom_glucose_high (High glucose threshold in mg/dL)
 * - p52 = eeprom_glucose_low (Low glucose threshold in mg/dL)
 * - p53 = cgm_unit (Glucose display unit: 0=mg/dL, 1=mmol/L)
 */
function handleFormSubmit(e, formId) {
    e.preventDefault();

    // Show "Saving..." message
    showStatus(getMessage('saving_settings'), 'info');

    // Get the actual form element to verify fields belong to it
    const form = el(formId);
    if (!form) {
        console.error(`Form ${formId} not found`);
        showStatus(getMessage('error_saving_unknown'), 'error');
        return;
    }

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
        
        const dayfontEl = getFieldInForm('dayfont');
        if (dayfontEl && addIfChanged(formData, 'p04', dayfontEl.value, window.settings.p04)) changedCount++;
        
        const nightfontEl = getFieldInForm('nightfont');
        if (nightfontEl && addIfChanged(formData, 'p05', nightfontEl.value, window.settings.p05)) changedCount++;
        
        const quietScrollEl = getFieldInForm('quiet_scroll');
        if (quietScrollEl && addIfChanged(formData, 'p06', quietScrollEl.checked ? 1 : 0, window.settings.p06)) changedCount++;
        
        const quietWeatherEl = getFieldInForm('quiet_weather');
        if (quietWeatherEl && addIfChanged(formData, 'p07', quietWeatherEl.checked ? 1 : 0, window.settings.p07)) changedCount++;
        
        const showGridEl = getFieldInForm('show_grid');
        if (showGridEl && addIfChanged(formData, 'p08', showGridEl.checked ? 1 : 0, window.settings.p08)) changedCount++;
        
        const mirroringEl = getFieldInForm('mirroring');
        if (mirroringEl && addIfChanged(formData, 'p09', mirroringEl.checked ? 1 : 0, window.settings.p09)) changedCount++;
        
        const colorFilterEl = getFieldInForm('color_filter');
        if (colorFilterEl && addIfChanged(formData, 'p10', parseInt(colorFilterEl.value) || 0, window.settings.p10)) changedCount++;
        
        const nightColorFilterEl = getFieldInForm('night_color_filter');
        if (nightColorFilterEl && addIfChanged(formData, 'p11', parseInt(nightColorFilterEl.value) || 0, window.settings.p11)) changedCount++;
        
        const msgColorEl = getFieldInForm('msg_color');
        if (msgColorEl && addIfChanged(formData, 'p12', msgColorEl.value, window.settings.p12)) changedCount++;
        
        const msgFontEl = getFieldInForm('msg_font');
        if (msgFontEl && addIfChanged(formData, 'p13', parseInt(msgFontEl.value) || 0, window.settings.p13)) changedCount++;
        
        const scrollDelayEl = getFieldInForm('scroll_delay');
        if (scrollDelayEl && addIfChanged(formData, 'p14', parseInt(scrollDelayEl.value) || 0, window.settings.p14)) changedCount++;
        
        const nightMsgColorEl = getFieldInForm('night_msg_color');
        if (nightMsgColorEl && addIfChanged(formData, 'p15', nightMsgColorEl.value, window.settings.p15)) changedCount++;
        
        const messageEl = getFieldInForm('message');
        if (messageEl && addIfChanged(formData, 'p16', messageEl.value, window.settings.p16)) changedCount++;
        
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
        
        const dimDisableEl = getFieldInForm('dim_disable');
        if (dimDisableEl && addIfChanged(formData, 'p22', dimDisableEl.checked ? 1 : 0, window.settings.p22)) changedCount++;
        
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
        
        const showLeadingZeroEl = getFieldInForm('show_leading_zero');
        if (showLeadingZeroEl && addIfChanged(formData, 'p24', showLeadingZeroEl.checked ? 1 : 0, window.settings.p24)) changedCount++;
        
        const dotsBreatheEl = getFieldInForm('dots_breathe');
        if (dotsBreatheEl && addIfChanged(formData, 'p50', dotsBreatheEl.checked ? 1 : 0, window.settings.p50)) changedCount++;
        
        const pwmFreqEl = getFieldInForm('pwm_frequency');
        if (pwmFreqEl && addIfChanged(formData, 'p42', parseInt(pwmFreqEl.value) || 0, window.settings.p42)) changedCount++;
        
        const maxPowerEl = getFieldInForm('max_power');
        if (maxPowerEl && addIfChanged(formData, 'p43', parseInt(maxPowerEl.value) || 0, window.settings.p43)) changedCount++;
    }

    // Integrations form fields (integrationsForm)
    if (formId === 'integrationsForm') {
        const haUrlInput = getFieldInForm('eeprom_ha_url');
        if (haUrlInput && addIfChanged(formData, 'p25', haUrlInput.value.trim(), window.settings.p25)) {
            changedCount++;
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

        const secTimeInput = getFieldInForm('eeprom_sec_time');
        if (secTimeInput && addIfChanged(formData, 'p48', parseInt(secTimeInput.value) || 0, window.settings.p48)) {
            changedCount++;
        }

        const secCgmInput = getFieldInForm('eeprom_sec_cgm');
        if (secCgmInput && addIfChanged(formData, 'p49', parseInt(secCgmInput.value) || 0, window.settings.p49)) {
            changedCount++;
        }

        const glucoseUsernameInput = getFieldInForm('eeprom_glucose_username');
        if (glucoseUsernameInput && addIfChanged(formData, 'p31', glucoseUsernameInput.value.trim(), window.settings.p31)) {
            changedCount++;
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
    saveSettings(formData, networkSettingsChanged);
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
    .then(response => {
        return response.json();
    })
    .then(data => {
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
            noScanMessage.textContent = 'Click "Scan available networks" to see WiFi networks';
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
    loadingText.textContent = 'Scanning for networks...';
    networkList.appendChild(loadingText);
}

function startWifiScan() {
    
    // Update UI to show scanning state
    startScanningUI();

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
    const networkList = el('network-list');
    const loadingSpinner = el('loading-spinner');
    const loadingText = el('loading-text');
    
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
        noNetworks.textContent = 'No networks found';
        networkList.appendChild(noNetworks);
        return;
    }

    // Add each network to the list
    networks.forEach(network => {
        const networkItem = document.createElement('div');
        networkItem.className = 'network-item';
        networkItem.onclick = () => selectNetwork(network.ssid);

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
    el('wifi_ssid').value = ssid;
    el('wifi_pass').focus();
}

function showNetworkError(message) {
    const networkList = el('network-list');
    const loadingSpinner = el('loading-spinner');
    const loadingText = el('loading-text');
    
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
        refreshButton.addEventListener('click', fetchStatus);
    }
    
    // Status data will be fetched when the section is shown (in navigateToSection)
}

// Setup support buttons (send email and copy to clipboard)
function setupSupportButtons() {
    const sendToSupportButton = el('sendToSupportButton');
    const copyToClipboardButton = el('copyToClipboardButton');
    
    if (sendToSupportButton) {
        // Remove any existing listeners by cloning the button
        const newButton = sendToSupportButton.cloneNode(true);
        sendToSupportButton.parentNode.replaceChild(newButton, sendToSupportButton);
        
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
        
        newButton.addEventListener('click', function(e) {
            e.preventDefault();
            e.stopPropagation();
            copySystemInfoToClipboard();
        });
    } else {
        console.warn('copyToClipboardButton not found in DOM');
    }
}

function fetchStatus() {
    return fetch('/api/status')
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

// Update (OTA) section functionality
function setupUpdateSection() {
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
        uploadButton.disabled = true;
        firmwareFile.value = '';
        progressContainer.style.display = 'none';
    }
}

// Restart section functionality
function setupRestartSection() {
    const resetButton = el('resetButton');
    const resetModal = el('resetModal');
    const cancelButton = el('cancelButton');
    const confirmButton = el('confirmButton');
    
    // Show modal when reset button is clicked
    resetButton.addEventListener('click', function() {
        resetModal.style.display = 'flex';
    });
    
    // Hide modal when cancel button is clicked
    cancelButton.addEventListener('click', function() {
        resetModal.style.display = 'none';
    });
    
    // Handle device reset when confirm button is clicked
    confirmButton.addEventListener('click', function() {
        resetModal.style.display = 'none';
        resetDevice();
    });
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
function setupAdvancedSection() {
    // Setup event listeners only once
    if (!window.advancedEventListenersSet) {
        window.advancedEventListenersSet = true;
        
        // Handle form submission
        const advancedForm = el('advancedForm');
        if (advancedForm) {
            advancedForm.addEventListener('submit', (e) => handleFormSubmit(e, 'advancedForm'));
        }
    }

    // Populate fields if settings are loaded
    if (window.settings && window.settingsLoaded.advanced) {
        
        if (el('ofs_x') && window.settings.p01 !== undefined) el('ofs_x').value = window.settings.p01 || 0;
        if (el('ofs_y') && window.settings.p02 !== undefined) el('ofs_y').value = window.settings.p02 || 0;
        if (el('rotation') && window.settings.p03 !== undefined) el('rotation').value = window.settings.p03 || 0;
        if (el('dayfont') && window.settings.p04 !== undefined) el('dayfont').value = window.settings.p04 || 'bold';
        if (el('nightfont') && window.settings.p05 !== undefined) el('nightfont').value = window.settings.p05 || 'bold';
        if (el('quiet_scroll') && window.settings.p06 !== undefined) el('quiet_scroll').checked = window.settings.p06;
        if (el('quiet_weather') && window.settings.p07 !== undefined) el('quiet_weather').checked = window.settings.p07;
        if (el('show_grid') && window.settings.p08 !== undefined) el('show_grid').checked = window.settings.p08;
        if (el('mirroring') && window.settings.p09 !== undefined) el('mirroring').checked = window.settings.p09;
        if (el('color_filter') && window.settings.p10 !== undefined) el('color_filter').value = window.settings.p10 || 0;
        if (el('night_color_filter') && window.settings.p11 !== undefined) {
            el('night_color_filter').value = window.settings.p11 || 0;
        }
        if (el('msg_color') && window.settings.p12 !== undefined) el('msg_color').value = window.settings.p12 || '#FFFFFF';
        if (el('msg_font') && window.settings.p13 !== undefined) el('msg_font').value = window.settings.p13 || 0;
        if (el('scroll_delay') && window.settings.p14 !== undefined) el('scroll_delay').value = window.settings.p14 || 65;
        if (el('night_msg_color') && window.settings.p15 !== undefined) el('night_msg_color').value = window.settings.p15 || '#FFFFFF';
        if (el('message') && window.settings.p16 !== undefined) el('message').value = window.settings.p16 || '';
        if (el('lat') && window.settings.p17 !== undefined) el('lat').value = window.settings.p17 || '';
        if (el('lon') && window.settings.p18 !== undefined) el('lon').value = window.settings.p18 || '';
        if (el('timezone') && window.settings.p19 !== undefined) el('timezone').value = window.settings.p19 || '';
        if (el('wifi_start') && window.settings.p46 !== undefined) el('wifi_start').value = window.settings.p46 || 0;
        if (el('wifi_end') && window.settings.p47 !== undefined) el('wifi_end').value = window.settings.p47 || 0;
        if (el('lux_sensitivity') && window.settings.p20 !== undefined) el('lux_sensitivity').value = window.settings.p20 || 2.5;
        if (el('lux_threshold') && window.settings.p21 !== undefined) el('lux_threshold').value = window.settings.p21 || 50;
        if (el('dim_disable') && window.settings.p22 !== undefined) el('dim_disable').checked = window.settings.p22;
        if (el('brightness_LED0') && window.settings.p23 && window.settings.p23[0] !== undefined) el('brightness_LED0').value = window.settings.p23[0];
        if (el('brightness_LED1') && window.settings.p23 && window.settings.p23[1] !== undefined) el('brightness_LED1').value = window.settings.p23[1];
        if (el('show_leading_zero') && window.settings.p24 !== undefined) el('show_leading_zero').checked = window.settings.p24;
        if (el('dots_breathe') && window.settings.p50 !== undefined) el('dots_breathe').checked = window.settings.p50;
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

        if (form) {
            form.addEventListener('submit', function(e) {
                // Trigger validation explicitly before submitting
                if (haUrlInput) haUrlInput.dispatchEvent(new Event('input', { bubbles: true }));
                if (haTokenInput) haTokenInput.dispatchEvent(new Event('input', { bubbles: true }));
                if (stockKeyInput) stockKeyInput.dispatchEvent(new Event('input', { bubbles: true }));

                const urlError = el('eeprom_ha_url-error');
                const tokenError = el('eeprom_ha_token-error');
                const stockKeyError = el('eeprom_stock_key-error');

                const urlErrorText = urlError ? urlError.textContent : '';
                const tokenErrorText = tokenError ? tokenError.textContent : '';
                const stockKeyErrorText = stockKeyError ? stockKeyError.textContent : '';

                if (urlErrorText || tokenErrorText || stockKeyErrorText) {
                    e.preventDefault(); // Prevent submission if there are validation errors
                    showStatus(getMessage('correct_form_before_save'), 'error');
                } else {
                    // If no client-side errors, proceed with generic form submission
                    handleFormSubmit(e, 'integrationsForm');
                }
            });
        }
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

        const secTime = el('eeprom_sec_time');
        if (secTime && typeof window.settings.p48 !== 'undefined') {
            secTime.value = window.settings.p48 || 0;
        }

        const secCgm = el('eeprom_sec_cgm');
        if (secCgm && typeof window.settings.p49 !== 'undefined') {
            secCgm.value = window.settings.p49 || 0;
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

    integrationsForm.addEventListener('submit', function(e) {
        e.preventDefault();
        
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
        ofs_x: window.settings ? window.settings.p01 : (el('ofs_x') ? el('ofs_x').value : ''),
        ofs_y: window.settings ? window.settings.p02 : (el('ofs_y') ? el('ofs_y').value : ''),
        rotation: window.settings ? window.settings.p03 : (el('rotation') ? el('rotation').value : ''),
        dayfont: window.settings ? window.settings.p04 : (el('dayfont') ? el('dayfont').value : ''),
        nightfont: window.settings ? window.settings.p05 : (el('nightfont') ? el('nightfont').value : ''),
        quiet_scroll: window.settings ? (window.settings.p06 ? 'Yes' : 'No') : 'Unknown',
        quiet_weather: window.settings ? (window.settings.p07 ? 'Yes' : 'No') : 'Unknown',
        show_grid: window.settings ? (window.settings.p08 ? 'Yes' : 'No') : 'Unknown',
        mirroring: window.settings ? (window.settings.p09 ? 'Yes' : 'No') : 'Unknown',
        color_filter: window.settings ? window.settings.p10 : (el('color_filter') ? el('color_filter').value : ''),
        night_color_filter: window.settings ? window.settings.p11 : (el('night_color_filter') ? el('night_color_filter').value : ''),
        msg_color: window.settings ? window.settings.p12 : (el('msg_color') ? el('msg_color').value : ''),
        msg_font: window.settings ? window.settings.p13 : (el('msg_font') ? el('msg_font').value : ''),
        night_msg_color: window.settings ? window.settings.p15 : (el('night_msg_color') ? el('night_msg_color').value : ''),
        message: window.settings ? window.settings.p16 : (el('message') ? el('message').value : ''),
        lat: window.settings ? window.settings.p17 : (el('lat') ? el('lat').value : ''),
        lon: window.settings ? window.settings.p18 : (el('lon') ? el('lon').value : ''),
        timezone: window.settings ? window.settings.p19 : (el('timezone') ? el('timezone').value : ''),
        lux_sensitivity: window.settings ? window.settings.p20 : (el('lux_sensitivity') ? el('lux_sensitivity').value : ''),
        lux_threshold: window.settings ? window.settings.p21 : (el('lux_threshold') ? el('lux_threshold').value : ''),
        dim_disable: window.settings ? (window.settings.p22 ? 'Yes' : 'No') : 'Unknown',
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
    emailBody += `Temperature Unit: ${info.settings.fahrenheit === 'Yes' ? 'Fahrenheit' : 'Celsius'}\n`;
    emailBody += `Time Format: ${info.settings.hour12 === 'Yes' ? '12-hour' : '24-hour'}\n`;
    emailBody += `Auto Update: ${info.settings.update_firmware === 'Yes' ? 'Enabled' : 'Disabled'}\n`;
    emailBody += `X Offset: ${info.settings.ofs_x}\n`;
    emailBody += `Y Offset: ${info.settings.ofs_y}\n`;
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
    emailBody += `Dim Disable: ${info.settings.dim_disable === 'Yes' ? 'Enabled' : 'Disabled'}\n`;
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
    promises.push(fetchStatus().catch(err => {
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
    }
}

// Function to copy system information to clipboard
async function copySystemInfoToClipboard(skipDataLoad = false) {
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