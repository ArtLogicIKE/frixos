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

