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

    // Optimization: Use cached elements with pre-parsed keys if available
    if (!i18nElementsCache) {
        i18nElementsCache = Array.from(document.querySelectorAll('[data-i18n], [data-i18n-placeholder], [data-i18n-aria-label]'))
            .map(el => ({
                el,
                key: el.dataset.i18n,
                placeholderKey: el.dataset.i18nPlaceholder,
                ariaLabelKey: el.dataset.i18nAriaLabel
            }));
    }

    i18nElementsCache.forEach(item => {
        const { el, key, placeholderKey, ariaLabelKey } = item;

        if (key) {
            const translation = getNestedTranslation(trans, key);
            // Optimization: Only update DOM if content actually changed to avoid layout thrashing
            if (translation && el.innerHTML !== translation) {
                el.innerHTML = translation;
            }
        }

        if (placeholderKey) {
            const translation = getNestedTranslation(trans, placeholderKey);
            // Optimization: Only update DOM if placeholder actually changed
            if (translation && el.placeholder !== translation) {
                el.placeholder = translation;
            }
        }

        if (ariaLabelKey) {
            const translation = getNestedTranslation(trans, ariaLabelKey);
            if (translation && el.getAttribute('aria-label') !== translation) {
                el.setAttribute('aria-label', translation);
            }
        }
    });

    // Optimization: Hoist frequent lookups outside loops
    const showPasswordLabel = getNestedTranslation(trans, 'common.show_password');
    const hidePasswordLabel = getNestedTranslation(trans, 'common.hide_password');

    // Update password toggle ARIA labels for accessibility after language change
    if (!passwordTogglesCache) {
        passwordTogglesCache = Array.from(document.querySelectorAll('.password-toggle'))
            .map(btn => ({ btn, input: btn.previousElementSibling }));
    }
    passwordTogglesCache.forEach(item => {
        if (item.input) {
            const translation = item.input.type === 'password' ? showPasswordLabel : hidePasswordLabel;
            if (translation) {
                item.btn.setAttribute('aria-label', translation);
            }
        }
    });

    // Update token tooltips and ARIA labels after language change
    if (!tokenCodesCache) {
        tokenCodesCache = document.querySelectorAll('.token-code');
    }
    const insertLabel = getNestedTranslation(trans, 'common.insert') || 'Insert';
    tokenCodesCache.forEach(token => updateTokenButtonTooltip(token, insertLabel));

    // Update font sample ARIA labels after language change
    if (!fontSamplesCache) {
        fontSamplesCache = Array.from(document.querySelectorAll('.font-sample-box'))
            .map(box => ({ box, nameEl: box.querySelector('.font-sample-name') }));
    }
    const selectFontTemplate = getNestedTranslation(trans, 'common.select_font') || 'Select {name} font';
    fontSamplesCache.forEach(item => {
        if (item.nameEl) {
            const fontName = item.nameEl.textContent.trim();
            item.box.setAttribute('aria-label', selectFontTemplate.replace('{name}', fontName));
        }
    });

    const nameElement = el('current-language-name');
    if (nameElement) nameElement.textContent = LANGUAGE_NAMES[effectiveLang] || LANGUAGE_NAMES['en'];

    // Update language selection state in dropdown
    if (!languageOptionsCache) {
        languageOptionsCache = Array.from(document.querySelectorAll('.language-option'))
            .map(option => ({ option, lang: option.getAttribute('data-lang') }));
    }
    languageOptionsCache.forEach(item => {
        const isSelected = item.lang === effectiveLang;
        item.option.classList.toggle('is-active', isSelected);
        item.option.setAttribute('aria-selected', isSelected.toString());
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
    if (!languageToggle || !languageDropdown) {
        return;
    }

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
    settings: false,   // p00, p03, p09, p34-p39 for settings section
    advanced: false,   // p01-p24, p42, p43 for advanced section
    integrations: false // p25-p33 for integrations section
};

// Global navigation state to optimize section switching (avoiding O(N) DOM lookups)
let activeSection = null;
let activeMenuItem = null;
const menuItemsMap = new Map();

function settingsResponseHasParams(data) {
    if (!data || typeof data !== 'object' || data.status === 'error') {
        return false;
    }
    return Object.keys(data).some(k => /^p\d+$/.test(k));
}

function mergeSettingsData(data) {
    if (!data || typeof data !== 'object') {
        return false;
    }
    let merged = false;
    Object.keys(data).forEach(key => {
        if (key === 'status' || key === 'message') {
            return;
        }
        window.settings[key] = data[key];
        merged = true;
    });
    return merged;
}

async function fetchSettingsJson(url, options = {}) {
    const response = await fetch(url);
    let data = null;
    try {
        data = await response.json();
    } catch (parseErr) {
        throw new Error(`Invalid JSON from ${url}`);
    }
    if (!response.ok || (data && data.status === 'error')) {
        const msg = (data && data.message) ? data.message : `HTTP ${response.status}`;
        throw new Error(msg);
    }
    if (!settingsResponseHasParams(data) && options.fallbackUrl) {
        const fallbackResp = await fetch(options.fallbackUrl);
        const fallbackData = await fallbackResp.json();
        if (fallbackResp.ok && settingsResponseHasParams(fallbackData)) {
            return fallbackData;
        }
    }
    return data;
}

// Function to fetch minimal parameters for theme and language
async function fetchThemeParams() {
    if (window.settingsLoaded.theme) {
        return window.settings;
    }

    try {
        let data = await fetchSettingsJson('/api/settings?group=theme', {
            fallbackUrl: '/api/settings'
        });
        if (!settingsResponseHasParams(data)) {
            data = await fetchSettingsJson('/api/settings');
        }
        if (!mergeSettingsData(data)) {
            throw new Error('Theme settings response contained no parameters');
        }
        window.settingsLoaded.theme = true;

        initTheme(data);

        const languageIndex = data.p41 !== undefined ? data.p41 : 0;
        const selectedLang = LANGUAGES[languageIndex] || 'en';
        try {
            await translate(selectedLang);
        } catch (translateErr) {
            console.warn('Translation load failed, using English:', translateErr);
            await translate('en');
        }

        return data;
    } catch (error) {
        console.error('Error loading theme parameters:', error);
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
        return Promise.resolve(window.settings);
    }

    if (window.settingsLoaded[mappedSection]) {
        return Promise.resolve(window.settings);
    }

    const groupedUrl = mappedSection === 'settings'
        ? '/api/settings?group=settings&params=p03,p09'
        : `/api/settings?group=${mappedSection}`;
    return fetchSettingsJson(groupedUrl, { fallbackUrl: '/api/settings' })
        .then(async (data) => {
            if (!settingsResponseHasParams(data)) {
                data = await fetchSettingsJson('/api/settings');
            }
            if (!mergeSettingsData(data)) {
                throw new Error(`No settings parameters in response for ${sectionName}`);
            }
            window.settingsLoaded[mappedSection] = true;
            return data;
        })
        .catch(error => {
            console.error(`Error loading parameters for ${sectionName} section:`, error);
            throw error;
        });
}

document.addEventListener('DOMContentLoaded', function() {
    const initialHash = window.location.hash.substring(1) || 'settings';
    document.querySelectorAll('.page-section').forEach(s => s.style.display = 'none');
    const targetSection = el(initialHash + '-section');
    if (targetSection) {
        targetSection.style.display = 'block';
        showSectionLoader(initialHash);
    }

    // Load theme and settings params together (settings is default page, so preload it) (settings is default page, so preload it)
    // Other sections (advanced, integrations) load on demand when navigated to
    Promise.allSettled([fetchThemeParams(), fetchSectionParams('settings')])
        .then((results) => {
            const settingsResult = results[1];
            if (settingsResult.status === 'rejected') {
                console.error('Settings section failed to load:', settingsResult.reason);
                showStatus(getMessage('error_loading_settings'), 'error');
                return;
            }
            if (results[0].status === 'rejected') {
                console.warn('Theme parameters failed to load:', results[0].reason);
            }

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
    const darkTheme = settings.p40 !== undefined ? settings.p40 : settings.eeprom_dark_theme;
    if (darkTheme !== undefined) {
        const isDarkTheme = !!darkTheme;
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
            p40: !isDarkTheme ? 1 : 0
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
            showSectionLoader('settings');
            fetchSectionParams('settings')
                .then(data => {
                    setupSettingsSection();
                    window.sectionsInitialized.settings = true;
                    hideSectionLoader('settings');
                })
                .catch(error => {
                    console.error('Error loading settings section:', error);
                    hideSectionLoader('settings');
                });
        } else if (hash === 'advanced' && !window.sectionsInitialized.advanced) {
            showSectionLoader('advanced');
            fetchSectionParams('advanced')
                .then(data => {
                    setupAdvancedSection();
                    window.sectionsInitialized.advanced = true;
                    hideSectionLoader('advanced');
                })
                .catch(error => {
                    console.error('Error loading advanced section:', error);
                    hideSectionLoader('advanced');
                });
        } else if (hash === 'integrations' && !window.sectionsInitialized.integrations) {
            showSectionLoader('integrations');
            fetchSectionParams('integrations')
                .then(data => {
                    setupIntegrationsSection();
                    window.sectionsInitialized.integrations = true;
                    hideSectionLoader('integrations');
                })
                .catch(error => {
                    console.error('Error loading integrations section:', error);
                    hideSectionLoader('integrations');
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
            hideSectionLoader('settings');
        } else if (hash === 'advanced' && window.sectionsInitialized.advanced) {
            // Re-initialize with already loaded data
            setupAdvancedSection();
            hideSectionLoader('advanced');
        } else if (hash === 'integrations' && window.sectionsInitialized.integrations) {
            // Re-initialize with already loaded data
            setupIntegrationsSection();
            hideSectionLoader('integrations');
        } else if (hash === 'status' && window.sectionsInitialized.status) {
            // Re-fetch status data when returning to status page
            fetchStatus(true);
            hideSectionLoader('status');
        } else if (hash === 'screen' && window.sectionsInitialized.screen) {
            setupScreenSection();
            refreshScreenLayoutSelect();
            hideSectionLoader('screen');
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


function showSectionLoader(sectionId) {
    const section = el(sectionId + '-section');
    if (!section) return;
    
    section.classList.add('is-loading');
    Array.from(section.children).forEach(child => {
        if (!child.classList.contains('section-loader')) {
            child.style.display = 'none';
        }
    });

    let loader = section.querySelector('.section-loader');
    if (!loader) {
        loader = document.createElement('div');
        loader.className = 'section-loader';
        loader.innerHTML = `
            <div class="spinner"></div>
            <div class="loader-text" data-i18n="common.loading">Loading...</div>
        `;
        section.appendChild(loader);
    }
    
    if (typeof currentLanguage !== 'undefined' && typeof translations !== 'undefined') {
        const trans = translations[currentLanguage] || translations?.en;
        if (trans) {
            const textNode = getNestedTranslation(trans, 'common.loading') || 'Loading...';
            if (textNode) loader.querySelector('.loader-text').textContent = textNode;
        }
    }
    
    loader.style.display = 'flex';
}

function hideSectionLoader(sectionId) {
    const section = el(sectionId + '-section');
    if (!section) return;
    
    let loader = section.querySelector('.section-loader');
    if (loader) loader.style.display = 'none';
    section.classList.remove('is-loading');
    Array.from(section.children).forEach(child => {
        if (!child.classList.contains('section-loader')) {
            child.style.display = '';
            child.style.animation = 'none'; // reset
            void child.offsetWidth; // trigger reflow
            child.style.animation = 'fadeIn 0.4s ease forwards';
        }
    });
}
