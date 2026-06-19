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