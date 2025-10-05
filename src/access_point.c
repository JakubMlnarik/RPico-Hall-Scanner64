#include "access_point.h"

SETTINGS *p_settings;

// Calibration state
static bool calibration_active = false;

char html_page[HTML_RESULT_SIZE];

void update_html_page() {
    // Create a simple, clean HTML interface
    memset(html_page, '\0', HTML_RESULT_SIZE);
    
    snprintf(html_page, HTML_RESULT_SIZE,
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>%s Settings</title>\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }\n"
        "        .container { max-width: 600px; margin: 0 auto; padding: 30px; background: white; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n"
        "        h1 { color: #333; margin-bottom: 20px; }\n"
        "        .setting { margin-bottom: 20px; }\n"
        "        label { display: block; margin-bottom: 5px; font-weight: bold; color: #555; }\n"
        "        input, select { width: 100%%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; font-size: 14px; }\n"
        "        .current { color: #666; font-size: 12px; margin-top: 3px; }\n"
        "        .buttons { margin-top: 30px; text-align: center; }\n"
        "        button, .reset-btn { padding: 10px 20px; margin: 0 10px; border: none; border-radius: 4px; cursor: pointer; text-decoration: none; display: inline-block; }\n"
        "        button { background: #007cba; color: white; }\n"
        "        .reset-btn { background: #666; color: white; }\n"
        "        .info { text-align: center; margin-top: 30px; padding-top: 20px; border-top: 1px solid #eee; color: #888; font-size: 12px; }\n"
        "        .calibration-section { margin-top: 30px; padding: 20px; background: #f8f9fa; border-radius: 8px; border: 1px solid #e9ecef; }\n"
        "        .calibration-btn { background: #28a745; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; margin: 5px; }\n"
        "        .calibration-btn:disabled { background: #6c757d; cursor: not-allowed; }\n"
        "        .modal { display: none; position: fixed; z-index: 1000; left: 0; top: 0; width: 100%%; height: 100%%; background-color: rgba(0,0,0,0.5); }\n"
        "        .modal-content { background-color: #fefefe; margin: 15%% auto; padding: 20px; border-radius: 8px; width: 400px; text-align: center; }\n"
        "        .modal.show { display: block; }\n"
        "        .done-btn { background: #dc3545; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; margin: 10px; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <div class=\"container\">\n"
        "        <h1>%s Configuration</h1>\n"
        "        <p>Firmware: %s</p>\n"
        "        <form method=\"POST\" action=\"/settings\">\n"
        "            <div class=\"setting\">\n"
        "                <label>MIDI Channel (1-16):</label>\n"
        "                <input type=\"number\" name=\"m_ch\" min=\"1\" max=\"16\" value=\"%d\">\n"
        "                <div class=\"current\">Current: %d</div>\n"
        "            </div>\n"
        "            <div class=\"setting\">\n"
        "                <label>Base MIDI Note (0-127):</label>\n"
        "                <input type=\"number\" name=\"m_base\" min=\"0\" max=\"127\" value=\"%d\">\n"
        "                <div class=\"current\">Current: %d</div>\n"
        "            </div>\n"
        "            <div class=\"setting\">\n"
        "                <label>Fast MIDI Mode:</label>\n"
        "                <select name=\"fast_midi\">\n"
        "                    <option value=\"0\"%s>Standard MIDI (31.25 kbps)</option>\n"
        "                    <option value=\"1\"%s>High Speed (Non-standard)</option>\n"
        "                </select>\n"
        "                <div class=\"current\">Current: %s</div>\n"
        "            </div>\n"
        "            <div class=\"setting\">\n"
        "               <label>Keys trigger point calibration:</label>\n"
        "               <button type=\"button\" class=\"calibration-btn\" onclick=\"startCalibration()\">Start Calibration</button>"
        "            </div>\n"
        "            <div class=\"buttons\">\n"
        "                <button type=\"submit\">Save Settings</button>\n"
        "                <a href=\"/settings?default=1\" class=\"reset-btn\" onclick=\"return confirm('Reset all settings to defaults?')\">Reset to Defaults</a>\n"
        "            </div>\n"
        "        </form>\n"
        "        <div class=\"info\">\n"
        "            <p><strong>Device IP:</strong> 192.168.4.1 | <strong>WiFi:</strong> %s</p>\n"
        "            <p>© 2025 %s Project</p>\n"
        "        </div>\n"
        "    </div>\n"
        "    <!-- Calibration Modal -->\n"
        "    <div id=\"calibrationModal\" class=\"modal %s\">\n"
        "        <div class=\"modal-content\">\n"
        "            <h3>Key Calibration in Progress</h3>\n"
        "            <p><strong>TEXT-TODO</strong></p>\n"
        "            <p>Calibration Status: %s</p>\n"
        "            <button class=\"done-btn\" onclick=\"finishCalibration()\">Done</button>\n"
        "        </div>\n"
        "    </div>\n"
        "    <script>\n"
        "        function startCalibration() {\n"
        "            fetch('/settings', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: 'calibrate=start' })\n"
        "                .then(() => { document.getElementById('calibrationModal').classList.add('show'); });\n"
        "        }\n"
        "        function finishCalibration() {\n"
        "            fetch('/settings', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: 'calibrate=done' })\n"
        "                .then(() => { location.reload(); });\n"
        "        }\n"
        "    </script>\n"
        "</body>\n"
        "</html>",
        DEV_NAME, DEV_NAME, FW_VERSION,
        p_settings ? p_settings->m_ch + 1 : 1,
        p_settings ? p_settings->m_ch + 1 : 1,
        p_settings ? p_settings->m_base : 36,
        p_settings ? p_settings->m_base : 36,
        (p_settings && p_settings->fast_midi == 0) ? " selected" : "",
        (p_settings && p_settings->fast_midi == 1) ? " selected" : "",
        (p_settings && p_settings->fast_midi == 1) ? "High Speed" : "Standard",
        calibration_active ? "disabled" : "",
        DEV_NAME, DEV_NAME,
        calibration_active ? "show" : "",
        calibration_active ? "Active" : "Idle");
}

static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

// Helper function to parse URL-encoded parameter
static int parse_param_int(const char *params, const char *name, int *value) {
    char search_pattern[32];
    snprintf(search_pattern, sizeof(search_pattern), "%s=%%d", name);
    return sscanf(params, search_pattern, value);
}

// Helper function to find parameter in URL-encoded string
static char* find_param(const char *params, const char *name) {
    if (!params || !name) return NULL;
    
    char search[64];
    snprintf(search, sizeof(search), "%s=", name);
    char *found = strstr(params, search);
    if (!found) return NULL;
    
    return found + strlen(search);
}

// Helper function to extract parameter value
static int extract_param_value(const char *params, const char *name, char *value, size_t value_size) {
    char *param_start = find_param(params, name);
    if (!param_start) return 0;
    
    char *param_end = strchr(param_start, '&');
    size_t len = param_end ? (size_t)(param_end - param_start) : strlen(param_start);
    
    if (len >= value_size) len = value_size - 1;
    strncpy(value, param_start, len);
    value[len] = '\0';
    
    return 1;
}

static int process_settings_form(const char *params) {
    if (!params || !p_settings) {
        printf("ERROR: process_settings_form called with NULL params or p_settings\n");
        return 0;
    }
    
    bool settings_changed = false;
    char value_str[16];
    int value;
    
    printf("Processing form data (%d bytes): %s\n", (int)strlen(params), params);
    
    // Parse MIDI Channel (1-16, stored as 0-15)
    if (extract_param_value(params, "m_ch", value_str, sizeof(value_str))) {
        value = atoi(value_str);
        if (value >= 1 && value <= 16) {
            p_settings->m_ch = (uint8_t)(value - 1);
            settings_changed = true;
            printf("Updated MIDI channel to: %d\n", value);
        }
    }
    
    // Parse Base MIDI Note (0-127)
    if (extract_param_value(params, "m_base", value_str, sizeof(value_str))) {
        value = atoi(value_str);
        if (value >= 0 && value <= 127) {
            p_settings->m_base = (uint8_t)value;
            settings_changed = true;
            printf("Updated base MIDI note to: %d\n", value);
        }
    }
    
    // Parse Fast MIDI (0-1)
    if (extract_param_value(params, "fast_midi", value_str, sizeof(value_str))) {
        value = atoi(value_str);
        if (value >= 0 && value <= 1) {
            p_settings->fast_midi = (uint8_t)value;
            settings_changed = true;
            printf("Updated fast MIDI to: %d\n", value);
        }
    }
    
    // Handle calibration commands
    if (extract_param_value(params, "calibrate", value_str, sizeof(value_str))) {
        if (strcmp(value_str, "start") == 0) {
            calibration_active = true;
            calibration_init();
            printf("Calibration started\n");
        } else if (strcmp(value_str, "done") == 0) {
            calibration_active = false;
            calibration_calculate_and_save(p_settings);
            printf("Calibration finished\n");
        }
    }
    


    
    // Save settings if any changes were made
    if (settings_changed) {
        settings_save(p_settings);
        printf("Settings saved to flash\n");
        return 1;
    }
    
    return 0;
}

static int test_server_content(const char *request, const char *params, char *result, size_t max_result_len) {
    int len = 0;
    
    // Handle settings page
    if (strncmp(request, SET_URL_SEGMENT, sizeof(SET_URL_SEGMENT) - 1) == 0) {
        
        // Handle default settings reset
        if (params && strstr(params, "default=1") != NULL) {
            printf("Resetting to default settings\n");
            p_settings->fast_midi = SETTINGS_FAST_MIDI_DEF;
            p_settings->m_ch = SETTINGS_M_CH_DEF;
            p_settings->m_base = SETTINGS_M_BASE_DEF;
            settings_save(p_settings);
        }
        // Handle form submission with settings
        else if (params && strlen(params) > 0) {
            process_settings_form(params);
        }
        
        // Generate the settings page
        update_html_page();
        len = strlen(html_page);
        if (len < max_result_len) {
            strcpy(result, html_page);
        } else {
            len = 0; // Buffer too small
        }
    }
    // Handle root page - redirect to settings
    else if (strcmp(request, "/") == 0) {
        len = snprintf(result, max_result_len,
            "<!DOCTYPE html>\n"
            "<html><head><meta http-equiv=\"refresh\" content=\"0;url=/settings\"></head>\n"
            "<body><p>Redirecting to <a href=\"/settings\">settings</a>...</p></body></html>\n");
    }
    // Handle any other request - show simple info page
    else {
        len = snprintf(result, max_result_len,
            "<!DOCTYPE html>\n"
            "<html><head><title>%s</title></head>\n"
            "<body style=\"font-family: Arial, sans-serif; text-align: center; padding: 50px;\">\n"
            "<h1>%s</h1>\n"
            "<p>Firmware: %s</p>\n"
            "<p><a href=\"/settings\" style=\"color: #3498db; text-decoration: none; font-size: 1.2em;\">→ Configure Settings</a></p>\n"
            "</body></html>\n",
            DEV_NAME, DEV_NAME, FW_VERSION);
    }
    
    return len;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!p) {
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);
    
    if (p->tot_len > 0) {
        // Calculate how much data we can still accept
        size_t available_space = sizeof(con_state->headers) - 1 - con_state->received_len;
        size_t copy_len = p->tot_len > available_space ? available_space : p->tot_len;
        
        // Append new data to existing buffer
        pbuf_copy_partial(p, con_state->headers + con_state->received_len, copy_len, 0);
        con_state->received_len += copy_len;
        con_state->headers[con_state->received_len] = '\0';
        
        printf("HTTP Request chunk received (%d bytes, total: %d bytes, buffer capacity: %zu bytes)\n", 
               (int)p->tot_len, con_state->received_len, sizeof(con_state->headers));
        
        if (p->tot_len > available_space) {
            printf("WARNING: Request chunk was truncated! Chunk size: %d, available space: %zu\n", 
                   (int)p->tot_len, available_space);
        }
        
        // Check if we have a complete HTTP request
        bool request_complete = false;
        if (strncmp(con_state->headers, "GET ", 4) == 0) {
            // GET requests are complete when we have the full header
            if (strstr(con_state->headers, "\r\n\r\n") || strstr(con_state->headers, "\n\n")) {
                request_complete = true;
            }
        } else if (strncmp(con_state->headers, "POST ", 5) == 0) {
            // POST requests need header + body
            char *content_length_str = strstr(con_state->headers, "Content-Length: ");
            if (content_length_str) {
                int content_length = atoi(content_length_str + 16);
                char *body_start = strstr(con_state->headers, "\r\n\r\n");
                if (!body_start) body_start = strstr(con_state->headers, "\n\n");
                if (body_start) {
                    body_start += (body_start[1] == '\n') ? 2 : 4; // Skip separator
                    int body_length = con_state->received_len - (body_start - con_state->headers);
                    if (body_length >= content_length) {
                        request_complete = true;
                    }
                    printf("POST progress: expected body length: %d, received: %d\n", content_length, body_length);
                }
            } else {
                // No Content-Length header, assume complete if we have separator
                if (strstr(con_state->headers, "\r\n\r\n") || strstr(con_state->headers, "\n\n")) {
                    request_complete = true;
                }
            }
        }
        
        if (!request_complete) {
            printf("Request incomplete, waiting for more data...\n");
            tcp_recved(pcb, p->tot_len);
            pbuf_free(p);
            return ERR_OK;
        }
        
        printf("Complete HTTP Request received:\n%s\n", con_state->headers);
        
        char *request_line = con_state->headers;
        char request_path_buf[64];  // Buffer for extracted path
        char *request_path = NULL;
        char *params = NULL;
        
        // Handle GET request
        if (strncmp("GET ", request_line, 4) == 0) {
            char *path_start = request_line + 4; // Skip "GET "
            char *space = strchr(path_start, ' ');
            if (space) {
                size_t path_len = space - path_start;
                if (path_len < sizeof(request_path_buf)) {
                    strncpy(request_path_buf, path_start, path_len);
                    request_path_buf[path_len] = '\0';
                    request_path = request_path_buf;
                    
                    // Extract query parameters from our copy
                    char *query = strchr(request_path, '?');
                    if (query) {
                        *query = '\0';
                        params = query + 1;
                    }
                }
            }
            
            printf("GET request: path='%s', params='%s'\n", request_path ? request_path : "unknown", params ? params : "none");
        }
        // Handle POST request
        else if (strncmp("POST ", request_line, 5) == 0) {
            char *path_start = request_line + 5; // Skip "POST "
            char *space = strchr(path_start, ' ');
            if (space) {
                size_t path_len = space - path_start;
                if (path_len < sizeof(request_path_buf)) {
                    strncpy(request_path_buf, path_start, path_len);
                    request_path_buf[path_len] = '\0';
                    request_path = request_path_buf;
                }
            }
            
            printf("POST request to path: '%s'\n", request_path);
            printf("Full request buffer size: %d bytes\n", con_state->received_len);
            
            // Find the body (try multiple separator patterns)
            char *body = strstr(con_state->headers, "\r\n\r\n");
            if (body) {
                body += 4; // Skip the "\r\n\r\n"
                params = body;
                printf("Found POST body after \\r\\n\\r\\n: '%s'\n", params);
            } else {
                // Try with just \n\n (some clients might use this)
                body = strstr(con_state->headers, "\n\n");
                if (body) {
                    body += 2;
                    params = body;
                    printf("Found POST body after \\n\\n: '%s'\n", params);
                } else {
                    // Try mixed line endings \r\n\n
                    body = strstr(con_state->headers, "\r\n\n");
                    if (body) {
                        body += 3;
                        params = body;
                        printf("Found POST body after \\r\\n\\n: '%s'\n", params);
                    } else {
                        printf("ERROR: Could not find POST body separator in request\n");
                        printf("Request size: %d, buffer capacity: %zu\n", con_state->received_len, sizeof(con_state->headers));
                        printf("Request data (first 200 chars): '%.200s'\n", con_state->headers);
                        params = NULL; // No body found
                    }
                }
            }
            
            printf("POST request: path='%s', body='%s'\n", request_path, params ? params : "none");
        }
        
        // Generate content based on the request
        if (request_path) {
            con_state->result_len = test_server_content(request_path, params, con_state->result, sizeof(con_state->result));
        } else {
            // Invalid request
            con_state->result_len = snprintf(con_state->result, sizeof(con_state->result),
                "<!DOCTYPE html><html><head><title>Error</title></head>"
                "<body><h1>400 Bad Request</h1><p>Invalid HTTP request</p></body></html>");
        }
        
        // Check if we had enough buffer space
        if (con_state->result_len <= 0 || con_state->result_len >= sizeof(con_state->result)) {
            printf("Error: Response too large or generation failed\n");
            return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
        }
        
        // Generate HTTP response headers
        con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "Cache-Control: no-cache\r\n"
            "\r\n", 
            con_state->result_len);
        
        if (con_state->header_len >= sizeof(con_state->headers)) {
            printf("Error: Response headers too large\n");
            return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
        }
        
        // Send the headers to the client
        con_state->sent_len = 0;
        err_t tcp_err = tcp_write(pcb, con_state->headers, con_state->header_len, TCP_WRITE_FLAG_COPY);
        if (tcp_err != ERR_OK) {
            printf("Error sending headers: %d\n", tcp_err);
            return tcp_close_client_connection(con_state, pcb, tcp_err);
        }
        
        // Send the body to the client
        tcp_err = tcp_write(pcb, con_state->result, con_state->result_len, TCP_WRITE_FLAG_COPY);
        if (tcp_err != ERR_OK) {
            printf("Error sending body: %d\n", tcp_err);
            return tcp_close_client_connection(con_state, pcb, tcp_err);
        }
        
        printf("HTTP Response sent: %d header bytes + %d body bytes\n", 
               con_state->header_len, con_state->result_len);
        
        tcp_recved(pcb, p->tot_len);
    }
    
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    return tcp_close_client_connection(con_state, pcb, ERR_OK); // Just disconnect clent?
}

static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        return ERR_VAL;
    }

    // Create the state for the connection
    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        return ERR_MEM;
    }
    con_state->pcb = client_pcb; // for checking
    con_state->gw = &state->gw;
    con_state->received_len = 0; // Initialize received data counter

    // setup connection to client
    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static bool tcp_server_open(void *arg, const char *ap_name) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}

// -------------- external ------------
bool is_calibration_active(void) {
    return calibration_active;
}

int wifi_ap_proc(SETTINGS *set) {
    // Turn ON onboard LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    printf("=== Starting WiFi Access Point ===\n");
    p_settings = set;

    // Allocate server state
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        printf("ERROR: Failed to allocate server state\n");
        cyw43_arch_deinit();
        return -1;
    }
    printf("✓ Server state allocated\n");

    // Configure access point settings
    const char *ssid = DEV_NAME;
    const char *password = NULL; // No password - open network
    
    printf("Configuring Access Point...\n");
    printf("  SSID: %s\n", ssid);
    printf("  Password: None (Open Network)\n");
    printf("  Security: Open (No encryption)\n");

    // Enable access point mode
    cyw43_arch_enable_ap_mode(ssid, password, CYW43_AUTH_OPEN);
    printf("✓ Access Point enabled\n");

    // Configure IP addresses
    ip4_addr_t gateway_ip, netmask;
    IP4_ADDR(ip_2_ip4(&gateway_ip), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&netmask), 255, 255, 255, 0);
    state->gw = gateway_ip;
    
    printf("✓ Network configuration:\n");
    printf("  Gateway IP: %s\n", ip4addr_ntoa(ip_2_ip4(&gateway_ip)));
    printf("  Netmask: %s\n", ip4addr_ntoa(ip_2_ip4(&netmask)));

    // Start DHCP server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &gateway_ip, &netmask);
    printf("✓ DHCP server started\n");

    // Start DNS server  
    dns_server_t dns_server;
    dns_server_init(&dns_server, &gateway_ip);
    printf("✓ DNS server started\n");

    // Start HTTP server
    if (!tcp_server_open(state, ssid)) {
        printf("ERROR: Failed to start HTTP server\n");
        dhcp_server_deinit(&dhcp_server);
        dns_server_deinit(&dns_server);
        free(state);
        cyw43_arch_deinit();
        return -1;
    }
    printf("✓ HTTP server started on port %d\n", TCP_PORT);
    
    printf("\n=== Access Point Ready ===\n");
    printf("Connect to open WiFi network '%s' (no password required)\n", ssid);
    printf("Then open http://192.168.4.1 or http://%s.local in your browser\n", ssid);
    printf("===============================\n\n");

    // Main server loop
    // One iteration takes typically 100 ms
    while(1) {
        // Poll for WiFi and lwIP work
        cyw43_arch_poll();
        
        // Handle calibration loop
        if (calibration_active) {
            calibration_update_keys_limits(to_ms_since_boot(get_absolute_time()));
        }
        
        // Wait for work or timeout (1 second)
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
    }

    // Cleanup (never reached in normal operation)
    dhcp_server_deinit(&dhcp_server);
    dns_server_deinit(&dns_server);
    free(state);
    cyw43_arch_deinit();
    return 0;
}
