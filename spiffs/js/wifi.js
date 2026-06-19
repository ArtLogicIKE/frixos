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

