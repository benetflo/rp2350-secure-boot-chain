#include <stdint.h>
#include <stddef.h>

#include "modules.h"
#include "../../config.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/http_client.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

#ifndef HTTP_INFO
#define HTTP_INFO printf
#endif

#ifndef HTTP_INFOC
#define HTTP_INFOC putchar
#endif

#ifndef HTTP_DEBUG
#ifdef NDEBUG
#define HTTP_DEBUG
#else
#define HTTP_DEBUG printf
#endif
#endif

#ifndef HTTP_ERROR
#define HTTP_ERROR printf
#endif

typedef struct
{
	const char * hostname;
	const char * url;
    httpc_headers_done_fn headers_fn;
	void * callback_arg;
	uint16_t port;
	httpc_connection_t settings;
	int complete;
	httpc_result_t result;
} HTTP_REQUEST_T;

static uint32_t flash_write_offset = 0;
static uint8_t  flash_page_buf[FLASH_PAGE_SIZE];
static uint32_t flash_page_buf_len = 0;
static uint32_t total_bytes_recv = 0;
static int ota_skip = 0;

int wifi_connect(char *ssid, char *password)
{
    static int initialized = 0;

    if (!initialized) {
        if (cyw43_arch_init_with_country(CYW43_COUNTRY_SWEDEN)) {
            return 1;
        }
        cyw43_arch_enable_sta_mode();
        initialized = 1;
    }

    for (int i = 0; i < 5; i++) {
        printf("WiFi connect attempt %d\n", i+1);

        if (cyw43_arch_wifi_connect_timeout_ms(
                ssid, password,
                CYW43_AUTH_WPA2_AES_PSK,
                15000) == 0)
        {
            return 0; // success
        }

        sleep_ms(2000);
    }

    // Wifi is dead
    printf("WiFi failed after retries, rebooting...\n");
    sleep_ms(2000);
    watchdog_reboot(0, 0, 0);
    return 1; // never reached
}

// Print headers to stdout
static err_t http_client_header_print_fn(__unused httpc_state_t * connection, __unused void * arg, struct pbuf * hdr, u16_t hdr_len, __unused u32_t content_len) 
{
	HTTP_INFO("\nheaders %u\n", hdr_len);
    
	u16_t offset = 0;
    
	while (offset < hdr->tot_len && offset < hdr_len) 
	{
        char c = (char)pbuf_get_at(hdr, offset++);
        HTTP_INFOC(c);
    }
    
	return ERR_OK;
}

static err_t internal_header_fn(httpc_state_t * connection, void * arg, struct pbuf * hdr, u16_t hdr_len, u32_t content_len) 
{
    char buf[16] = {0};
    pbuf_copy_partial(hdr, buf, 15, 0);
    if (strstr(buf, "304")) // if firmware is the same version as firmware running
    {
        ota_skip = 1;
    }
    
    assert(arg);
    HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
    
	if (req->headers_fn) 
	{
        return req->headers_fn(connection, req->callback_arg, hdr, hdr_len, content_len);
    }
    return ERR_OK;
}

extern int main(void);

static uint32_t get_running_partition(void)
{
    uint32_t addr = (uint32_t)&main;
    return (addr >= FIRMWARE_B) ? 1 : 0;
}

static uint32_t get_inactive_flash_offset(void)
{
    uint32_t running = get_running_partition();
    return (running == 1) ? FIRMWARE_A_FLASH_OFFSET : FIRMWARE_B_FLASH_OFFSET;
}

static uint32_t get_inactive_header_offset(void)
{
    uint32_t running = get_running_partition();
    return (running == 1) ? FIRMWARE_A_HEADER_OFFSET : FIRMWARE_B_HEADER_OFFSET;
}

static uint32_t get_inactive_partition_id(void)
{
    uint32_t running = get_running_partition();
    return running ^ 1; // 0→1, 1→0
}

static void flash_write_chunk(const uint8_t *data, size_t len)
{
    if (ota_skip)
    {
        return;
    }
    
    total_bytes_recv += len;
    printf("recv chunk len=%u total=%u\n", len, total_bytes_recv);
	size_t i = 0;
    while (i < len)
    {
        flash_page_buf[flash_page_buf_len++] = data[i++];

        if (flash_page_buf_len == FLASH_PAGE_SIZE)
        {
            // Radera sektor om vi är i början av en ny sektor
            if (flash_write_offset % FLASH_SECTOR_SIZE == 0)
            {
                uint32_t ints = save_and_disable_interrupts();
                flash_range_erase(get_inactive_flash_offset() + flash_write_offset, FLASH_SECTOR_SIZE);
                restore_interrupts(ints);
            }

            uint32_t ints = save_and_disable_interrupts();
            flash_range_program(get_inactive_flash_offset() + flash_write_offset, flash_page_buf, FLASH_PAGE_SIZE);
            restore_interrupts(ints);

            flash_write_offset += FLASH_PAGE_SIZE;
            flash_page_buf_len = 0;
        }
    }
}

static int handle_payload (struct pbuf * p)
{
	struct pbuf * temp = p; // store in new variable to avoid losing data if LWIP re-uses payload.

	while (temp != NULL)
	{
		uint8_t * data = (uint8_t *)temp->payload;
		size_t length = temp->len;

		flash_write_chunk(data, length);
		temp = temp->next;
	}
	
	return 0;
}

static err_t internal_recv_fn(void * arg, struct altcp_pcb * conn, struct pbuf * p, err_t err) 
{
    assert(arg);
    HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
    
    if (!p)
    {
        return ERR_OK;
    }

    handle_payload(p);
    altcp_recved(conn, p->tot_len);  // bekräfta mottagen data
    pbuf_free(p);
    return ERR_OK;
}

static void internal_result_fn(void * arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) 
{
    assert(arg);
    HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
    HTTP_DEBUG("result %d len %u server_response %u err %d\n", httpc_result, rx_content_len, srv_res, err);
    req->complete = true;

    if (ota_skip)
    {
        //Firmware is up to date, no update needed
        return;
    }

    // Flush remaining bytes i page buffer
    if (flash_page_buf_len > 0)
    {
        memset(flash_page_buf + flash_page_buf_len, 0xFF, FLASH_PAGE_SIZE - flash_page_buf_len);
        
        if (flash_write_offset % FLASH_SECTOR_SIZE == 0)
        {
            uint32_t ints = save_and_disable_interrupts();
            flash_range_erase(get_inactive_flash_offset() + flash_write_offset, FLASH_SECTOR_SIZE);
            restore_interrupts(ints);
        }
        uint32_t ints = save_and_disable_interrupts();
        flash_range_program(get_inactive_flash_offset() + flash_write_offset, flash_page_buf, FLASH_PAGE_SIZE);
        restore_interrupts(ints);
        flash_write_offset += FLASH_PAGE_SIZE;
        flash_page_buf_len = 0;
    }

    // Skriv firmware size till header
    uint32_t fw_size = total_bytes_recv - 64;
    uint8_t size_buf[FLASH_PAGE_SIZE];
    memset(size_buf, 0xFF, FLASH_PAGE_SIZE);
    memcpy(size_buf, &fw_size, 4);
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(get_inactive_header_offset(), size_buf, FLASH_PAGE_SIZE);
    restore_interrupts(ints);


    // Write metadata - activate partition B
    uint8_t meta_buf[FLASH_PAGE_SIZE];
    memset(meta_buf, 0xFF, FLASH_PAGE_SIZE);
    uint32_t active = get_inactive_partition_id();
    uint32_t magic  = 0xDEADBEEF;
    memcpy(meta_buf,     &active, 4);
    memcpy(meta_buf + 4, &magic,  4);
    ints = save_and_disable_interrupts();
    flash_range_erase(METADATA_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(METADATA_FLASH_OFFSET, meta_buf, FLASH_PAGE_SIZE);
    restore_interrupts(ints);

    printf("OTA complete! Rebooting...\n");
    watchdog_reboot(0, 0, 0);
}

// HTTP request
static int http_client_request_async(async_context_t * context, HTTP_REQUEST_T * req) 
{
        #define DEFAULT_PORT 4567

        uint16_t port = DEFAULT_PORT;
		
        // If port is a common user port
		if (HTTP_SERVER_PORT >= 1024 && HTTP_SERVER_PORT <= 49151)
		{
			port = HTTP_SERVER_PORT;
		}

		req->complete = false; // request is in progress
		req->settings.headers_done_fn = req->headers_fn ? internal_header_fn : NULL; // set header callback function. Runs after header has been received but before the body. This is so that you can react to headers, 403 status code for example.
		req->settings.result_fn = internal_result_fn; // set callback function to run after request is done
		async_context_acquire_lock_blocking(context); // lock context so that other threads or interupts does not störa
		
		err_t ret = httpc_get_file_dns(req->hostname, req->port ? req->port : port, req->url, &req->settings, internal_recv_fn, req, NULL); // DNS-lookup, open TCP, send GET, recieve data in internal_recv_fn, call internal_result_fn when everythin is done
		printf("httpc_get_file_dns ret: %d\n", ret);
		async_context_release_lock(context); // unlock context
		
		if (ret != ERR_OK) 
		{
			HTTP_ERROR("http request failed: %d", ret);
		}
		
		return ret;
}

// Make a http request and only return when it has completed. Returns true on success
static int http_client_request_sync(async_context_t * context, HTTP_REQUEST_T * req) 
{
    assert(req);
    int ret = http_client_request_async(context, req);
    
	if (ret != 0) 
	{
        return ret;
    }
    while(!req->complete) 
	{
        async_context_poll(context);
        async_context_wait_for_work_ms(context, 1000);
    }
    
	return req->result;
}

int http_connect (char * host, char * url_request)
{
    // erase full flash area of inactive partition
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(get_inactive_flash_offset(), SLOT_SIZE);
    restore_interrupts(ints);
    
    // reset all variables before new connections
    flash_write_offset = 0;
    flash_page_buf_len = 0;
    total_bytes_recv = 0;
    ota_skip = 0;

    // connect to OTA server
    HTTP_REQUEST_T req = {0};
	req.hostname = host;
	req.url = url_request;
    req.headers_fn = http_client_header_print_fn;
	int result = http_client_request_sync(cyw43_arch_async_context(), &req);


	return result;
}