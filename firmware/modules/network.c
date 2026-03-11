#include "modules.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"



int wifi_connect(char * ssid, char * password) 
{

	if (cyw43_arch_init_with_country(CYW43_COUNTRY_SWEDEN)) 
    {
		return 1;
	}

	cyw43_arch_enable_sta_mode();

	if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000)) 
    {
		return 1;
	}
	
    return 0;
}