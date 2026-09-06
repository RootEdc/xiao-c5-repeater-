#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/lwip_napt.h"
#include "lwip/err.h"

// ================= KONFIGURACJA =================
#define WIFI_STA_SSID "Króliczok_2G"    // <-- WPISZ NAZWĘ DOMOWEGO WI-FI
#define WIFI_STA_PASS "krzysioisisi123"    // <-- WPISZ HASŁO DO DOMOWEGO WI-FI

#define WIFI_AP_SSID  "ESP_DRUKARKA"        // Sieć tworzona dla drukarki
#define WIFI_AP_PASS  "Drukarka123"         // Hasło sieci drukarki (min. 8 znaków)

#define PRINTER_IP    "192.168.4.2"         // Pierwszy przydzielony adres IP
// ================================================

static const char *TAG = "REPEATER";

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Rozlaczono z routerem. Ponawianie polaczenia...");
        esp_wifi_connect();
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Polaczono! IP ESP: " IPSTR, IP2STR(&event->ip_info.ip));

        // Aktywacja translacji NAPT na interfejsie STA
        ip_napt_enable(event->ip_info.ip.addr, 1);

        // Przekierowanie portów na drukarkę (Port Forwarding)
        ip_addr_t printer_ip;
        ipaddr_aton(PRINTER_IP, &printer_ip);

        // Port 80 (Interfejs WWW / OctoPrint / Mainsail / Fluidd)
        ip_portmap_add(IP_PROTO_TCP, event->ip_info.ip.addr, 80, printer_ip.u_addr.ip4.addr, 80);
        // Port 7125 (Klipper / Moonraker API)
        ip_portmap_add(IP_PROTO_TCP, event->ip_info.ip.addr, 7125, printer_ip.u_addr.ip4.addr, 7125);
    }
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .password = WIFI_AP_PASS,
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .channel = 6
        },
    };

    wifi_config_t sta_config = {
        .sta = {
            .ssid = WIFI_STA_SSID,
            .password = WIFI_STA_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}
