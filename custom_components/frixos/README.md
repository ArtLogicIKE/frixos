# Frixos Home Assistant Integration

This custom integration allows you to control and monitor your Frixos device directly from Home Assistant, providing access to all settings and sensor data available in the web UI.

## Features

- **Full Settings Control**: Access and modify all device settings through Home Assistant entities
- **Real-time Monitoring**: Monitor device status, sensor readings, and system information
- **Automatic Updates**: Settings and sensor data are automatically refreshed every 60 seconds
- **User-Friendly UI**: All settings are properly categorized as switches, numbers, selects, and text inputs

## Supported Entities

### Sensors (Read-only)
- Light Level (lux)
- Uptime
- Free Heap Memory
- Temperature (from weather data)
- Humidity (from weather data)
- WiFi Connection Status
- Last Weather Update
- Last Time Update

### Switches
- Temperature in Fahrenheit
- 12-Hour Time Format
- Show Scrolling Message
- Show Weather Forecast
- Show Grid
- Mirror Display
- Maintain Full Brightness
- Show Leading Zero
- Auto Firmware Update
- Dark Theme

### Number Inputs
- X Offset (0-160)
- Y Offset (0-160)
- Scroll Delay (30-500 ms)
- Light Sensitivity (0-50 lux)
- Day Threshold (0-500 lux)
- LED Brightness Day (1-100%)
- LED Brightness Night (1-100%)
- PWM Frequency (10-5000 Hz)
- Max Power (1-1023)
- Home Assistant Refresh Interval (1-7200 min)
- Stock Refresh Interval (1-1440 min)
- Dexcom Refresh Interval (1-60 min)

### Select Dropdowns
- Display Rotation (0°, 90°, 180°, 270°)
- Day Font (bold, light, lcd, nixie, etc.)
- Night Font (same options)
- Day Color Filter (None, Red, Green, Blue, B&W)
- Night Color Filter (same options)
- Message Font Size (8pt, 10pt, 12pt)
- Dexcom Region (Disabled, US, Japan, Rest of World)
- Language (English, Deutsch, Français, etc.)

### Text Inputs
- Hostname
- Scrolling Message (supports tokens like [HA:entity:path], [temp], etc.)
- Latitude
- Longitude
- Timezone (POSIX format, e.g., "EET-2EEST,M3.5.0/3,M10.5.0/4")
- Home Assistant URL
- Home Assistant Token (password field)
- Stock API Key (password field)
- Dexcom Username
- Dexcom Password (password field)

## Installation

### Method 1: Manual Installation (Recommended for Testing)

1. **Access your Home Assistant installation**
   - If using Home Assistant OS/Supervised: Use SSH add-on or direct file system access
   - If using Home Assistant Core: Access the installation directory

2. **Navigate to the custom components directory**
   ```bash
   cd /config/custom_components
   ```
   
   Note: If the `custom_components` directory doesn't exist, create it:
   ```bash
   mkdir -p /config/custom_components
   ```

3. **Copy the integration folder**
   ```bash
   # Copy the entire 'frixos' folder to custom_components
   cp -r /path/to/custom_components/frixos /config/custom_components/
   ```
   
   Or if you're cloning from a repository:
   ```bash
   git clone https://github.com/yourusername/frixos-ha-integration.git
   mv frixos-ha-integration/custom_components/frixos /config/custom_components/
   ```

4. **Set proper permissions** (if needed)
   ```bash
   chmod -R 755 /config/custom_components/frixos
   ```

5. **Restart Home Assistant**
   - Go to Settings → System → Hardware → Restart
   - Or restart from the terminal/SSH

### Method 2: Using HACS (Home Assistant Community Store)

**Note**: This requires the integration to be published in the HACS default repository. For now, use manual installation.

If/when published to HACS:

1. Install HACS if you haven't already (see [HACS documentation](https://hacs.xyz/docs/setup/download))
2. Go to HACS → Integrations
3. Click "Explore & Download Repositories"
4. Search for "Frixos"
5. Click "Download"
6. Restart Home Assistant

## Configuration

### Adding the Integration

1. **Open Home Assistant**
   - Go to Settings → Devices & Services
   - Click "Add Integration" button (bottom right)

2. **Search for Frixos**
   - Type "Frixos" in the search box
   - Select "Frixos" from the results

3. **Configure the device**
   - **Host**: Enter the IP address or hostname of your Frixos device (e.g., `frixos.local` or `192.168.1.100`)
   - **Port**: Enter the HTTP port (default: 80)
   - **Name**: Enter a friendly name for this device (default: "Frixos")

4. **Submit**
   - Click "Submit"
   - The integration will validate the connection
   - If successful, you'll see a confirmation message

### Multiple Devices

You can add multiple Frixos devices by repeating the configuration process. Each device will have its own set of entities.

## Usage

### Accessing Entities

Once configured, all entities will appear in Home Assistant:

- **Sensors**: Settings → Devices & Services → Sensors
- **Switches**: Settings → Devices & Services → Switches
- **Numbers**: Settings → Devices & Services → Numbers
- **Selects**: Settings → Devices & Services → Selects
- **Texts**: Settings → Devices & Services → Texts

Or use the search bar to find specific entities by name (e.g., "Frixos Light Level").

### Automations

You can create automations using Frixos entities:

```yaml
# Example: Turn on night mode when it's dark
automation:
  - alias: "Frixos Night Mode"
    trigger:
      - platform: numeric_state
        entity_id: sensor.frixos_light_level
        below: 16
    action:
      - service: number.set_value
        target:
          entity_id: number.frixos_led_brightness_night
        data:
          value: 30
```

### Scrolling Message Tokens

When setting the scrolling message, you can use tokens that the device will replace:

- `[device]` - Device name
- `[greeting]` - Time-based greeting
- `[day]` - Current day of the week
- `[date]` - Current date
- `[mon]` - Current month
- `[temp]` - Current temperature
- `[hum]` - Current humidity
- `[high]` - Today's high temperature
- `[low]` - Today's low temperature
- `[rise]` - Sunrise time
- `[set]` - Sunset time
- `[HA:entity_id:path]` - Fetch from Home Assistant (requires HA integration enabled on device)
- `[$:symbol]` - Stock price (requires stock integration enabled)

### Important Notes

⚠️ **Device Restart**: Some settings trigger a device restart:
- Hostname
- WiFi SSID/Password
- Latitude/Longitude
- Timezone

When changing these settings, the device will restart and become temporarily unavailable.

⚠️ **Password Fields**: Token fields (Home Assistant Token, Stock API Key, Dexcom Password) are stored as plain text in Home Assistant. Keep your configuration secure.

⚠️ **Polling Interval**: The integration polls the device every 60 seconds by default. This can be adjusted in `coordinator.py` if needed.

## Troubleshooting

### Integration Not Showing Up

1. **Check file structure**: Ensure the `frixos` folder is directly in `custom_components/`
2. **Check permissions**: Files should be readable
3. **Check logs**: Look in Settings → System → Logs for errors
4. **Restart**: Make sure you've restarted Home Assistant after installation

### Cannot Connect to Device

1. **Verify network connectivity**: Ensure Home Assistant can reach the Frixos device
   ```bash
   ping frixos.local
   # or
   ping 192.168.1.100
   ```

2. **Check device IP/hostname**: Ensure the hostname or IP address is correct
3. **Check port**: Verify the port (default is 80)
4. **Check device web UI**: Try accessing the device's web interface directly in a browser
5. **Check firewall**: Ensure no firewall is blocking the connection

### Settings Not Updating

1. **Check device logs**: Some settings may require device restart
2. **Check integration logs**: Look for errors in Home Assistant logs
3. **Verify API response**: Check if `/api/settings` endpoint is accessible

### Entity States Show as "Unknown"

1. **Wait for initial update**: The integration needs to fetch data on first load
2. **Check device connectivity**: Ensure the device is reachable
3. **Check coordinator logs**: Look for errors in the data update process

## Development

### File Structure

```
custom_components/frixos/
├── __init__.py          # Main integration setup
├── config_flow.py       # Configuration UI
├── const.py             # Constants and parameter mappings
├── coordinator.py       # Data update coordinator
├── entity.py            # Base entity class
├── manifest.json        # Integration metadata
├── sensor.py            # Sensor entities
├── switch.py            # Switch entities
├── number.py            # Number entities
├── select.py            # Select entities
└── text.py              # Text entities
```

### API Endpoints

The integration uses the following endpoints:

- `GET /api/settings` - Retrieve all device settings
- `POST /api/settings` - Update device settings (JSON payload)
- `GET /api/status` - Retrieve device status and sensor data

### Contributing

To contribute improvements:

1. Fork the repository
2. Make your changes
3. Test thoroughly with your Frixos device
4. Submit a pull request

## Support

For issues or questions:

1. Check this README and troubleshooting section
2. Check device logs in Home Assistant (Settings → System → Logs)
3. Open an issue on the GitHub repository
4. Contact Frixos support at support@buyfrixos.com

## License

This integration is provided as-is for use with Frixos devices.

## Changelog

### Version 1.0.0
- Initial release
- Full settings control via Home Assistant entities
- Real-time sensor monitoring
- Support for all device parameters
