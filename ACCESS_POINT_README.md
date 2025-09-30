# Hall Scanner 64 - Access Point Configuration

## Overview
The enhanced access point module provides a modern web interface for configuring your Hall Scanner device via WiFi.

## Features

### 🎨 Modern Web Interface
- **Responsive Design**: Works on desktop, tablet, and mobile devices
- **Beautiful UI**: Modern gradient design with smooth animations
- **Real-time Updates**: Live preview of slider values
- **Form Validation**: Client-side validation for all inputs

### 🔧 Configuration Options
- **MIDI Channel**: Set the MIDI channel (1-16)
- **Base MIDI Note**: Configure the base MIDI note (0-127)
- **Fast MIDI Mode**: Enable/disable high-speed MIDI mode
- **Sensitivity**: Adjust sensitivity percentage (0-100%)
- **Threshold**: Set detection threshold (0-100%)
- **Coefficient A/B**: Advanced calibration parameters

### 🌐 Network Features
- **WiFi Access Point**: Creates its own WiFi network
- **DHCP Server**: Automatically assigns IP addresses to clients
- **DNS Server**: Resolves domain names locally
- **HTTP Server**: Serves the configuration web interface

## Getting Started

### 1. Flash the Firmware
- Build the project: `make` in the build directory
- Flash `my_project.uf2` to your Pico W device

### 2. Connect to WiFi
- Look for WiFi network: **"Hall Scanner"**
- **No password required** (Open Network)
- Connect your device (phone, laptop, tablet)

### 3. Open Web Interface
- Open your web browser
- Navigate to: **http://192.168.4.1**
- The settings page will load automatically

### 4. Configure Settings
- Adjust settings using the intuitive interface
- Use sliders for percentage values
- Click **"💾 Save Settings"** to store changes
- Click **"🔄 Reset to Defaults"** to restore factory settings

## Network Configuration

| Setting | Value |
|---------|--------|
| SSID | Hall Scanner |
| Password | None (Open Network) |
| Security | Open (No encryption) |
| Gateway IP | 192.168.4.1 |
| DHCP Range | 192.168.4.16 - 192.168.4.23 |
| Subnet Mask | 255.255.255.0 |

## HTTP Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Redirects to settings page |
| `/settings` | GET | Display configuration interface |
| `/settings` | POST | Process form submissions |
| `/settings?default=1` | GET | Reset to default settings |

## Technical Details

### Hardware Requirements
- Raspberry Pi Pico W (WiFi required)
- Minimum 2MB flash memory
- Stable power supply (USB or external)

### Software Components
- **lwIP**: Lightweight TCP/IP stack
- **CYW43 Driver**: WiFi chip driver
- **DHCP Server**: From MicroPython project
- **DNS Server**: From Raspberry Pi Foundation
- **HTTP Server**: Custom implementation

### Performance
- **Concurrent Connections**: Up to 8 clients
- **Response Time**: < 100ms for page loads
- **Memory Usage**: ~32KB RAM for networking
- **Flash Usage**: ~400KB total firmware size

## Troubleshooting

### Can't Find WiFi Network
- Ensure Pico W is powered on
- Check LED is lit (indicates AP is active)
- Try restarting the device
- Check for interference from other 2.4GHz devices

### Can't Connect to WiFi
- No password required (open network)
- Ensure device WiFi is enabled
- Try forgetting and reconnecting to the network
- Check for interference from other 2.4GHz devices

### Web Page Won't Load
- Verify you're connected to "Hall Scanner" network
- Try http://192.168.4.1 (no HTTPS)
- Clear browser cache
- Try a different browser
- Check firewall settings

### Settings Won't Save
- Ensure stable WiFi connection
- Wait for "✓ Settings saved successfully!" message
- Check serial console for error messages
- Try refreshing the page and re-entering values

## Serial Console Output
Connect a serial terminal (115200 baud) to see detailed status information:

```
=== Starting WiFi Access Point ===
✓ CYW43 architecture initialized
✓ Server state allocated
Configuring Access Point...
  SSID: Hall Scanner
  Password: None (Open Network)
  Security: Open (No encryption)
✓ Access Point enabled
✓ Network configuration:
  Gateway IP: 192.168.4.1
  Netmask: 255.255.255.0
✓ DHCP server started
✓ DNS server started
✓ HTTP server started on port 80

=== Access Point Ready ===
Connect to open WiFi network 'Hall Scanner' (no password required)
Then open http://192.168.4.1 or http://Hall Scanner.local in your browser
===============================
```

## Development Notes

### Adding New Settings
1. Add field to `SETTINGS` structure in `settings.h`
2. Add HTML form element in `update_html_page()`
3. Add parsing logic in `process_settings_form()`
4. Add default value handling

### Customizing Appearance
- Modify CSS in `update_html_page()` function
- Colors, fonts, and layouts can be easily changed
- Responsive breakpoints at 768px for mobile

### Security Considerations
- **⚠️ SECURITY WARNING**: Network is now OPEN (no password protection)
- Any device in range can connect and access the configuration interface
- Consider adding password protection for production environments
- Add HTTPS support for sensitive configurations
- Implement authentication for advanced settings
- Rate limiting for API endpoints
- Use in trusted environments only

## License
This project uses components from:
- MicroPython (MIT License) - DHCP Server
- Raspberry Pi Foundation (BSD License) - DNS Server
- Raspberry Pi Pico SDK (BSD License) - Core libraries