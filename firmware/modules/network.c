#include <stdint.h>

#include "modules.h"
#include "../../config.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/http_client.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"

#ifndef HTTP_INFO
#define HTTP_INFO printf
#endif

#ifndef HTTP_INFOC
#define HTTP_INFOC putchar
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
	altcp_recv_fn recv_fn;
	httpc_result_fn result_fn;
	void * callback_arg;
	uint16_t port;
	httpc_connection_t settings;
	int complete;
	httpc_result_t result;
} HTTP_REQUEST_T;

int wifi_connect (char * ssid, char * password) 
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

// Print body to stdout
static err_t http_client_receive_print_fn(__unused void * arg, __unused struct altcp_pcb * conn, struct pbuf * p, err_t err) 
{
	HTTP_INFO("\ncontent err %d\n", err);
    
	u16_t offset = 0;
    
	while (offset < p->tot_len) 
	{
        char c = (char)pbuf_get_at(p, offset++);
        HTTP_INFOC(c);
    }
    
	return ERR_OK;
}

static err_t internal_header_fn(httpc_state_t * connection, void * arg, struct pbuf * hdr, u16_t hdr_len, u32_t content_len) {
    assert(arg);
    HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
    if (req->headers_fn) {
        return req->headers_fn(connection, req->callback_arg, hdr, hdr_len, content_len);
    }
    return ERR_OK;
}

static err_t internal_recv_fn(void * arg, struct altcp_pcb * conn, struct pbuf * p, err_t err) 
{
    assert(arg);
    HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
    if (req->recv_fn) 
	{
        return req->recv_fn(req->callback_arg, conn, p, err);
    }
    return ERR_OK;
}

static void internal_result_fn(void * arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) 
{
    assert(arg);
    HTTP_REQUEST_T *req = (HTTP_REQUEST_T*)arg;
    HTTP_DEBUG("result %d len %u server_response %u err %d\n", httpc_result, rx_content_len, srv_res, err);
    req->complete = true;
    req->result = httpc_result;
    if (req->result_fn) 
	{
        req->result_fn(req->callback_arg, httpc_result, rx_content_len, srv_res, err);
    }
}

static int http_client_request_async(async_context_t * context, HTTP_REQUEST_T * req) 
{
		#define DEFAULT_PORT 4567
		uint16_t port = DEFAULT_PORT;
		
		if (HTTP_SERVER_PORT >= 1024 && HTTP_SERVER_PORT <= 49151)
		{
			port = HTTP_SERVER_PORT;
		}

		req->complete = false;
		req->settings.headers_done_fn = req->headers_fn ? internal_header_fn : NULL;
		req->settings.result_fn = internal_result_fn;
		async_context_acquire_lock_blocking(context);
		
		err_t ret = httpc_get_file_dns(req->hostname, req->port ? req->port : port, req->url, &req->settings, internal_recv_fn, req, NULL);
		printf("httpc_get_file_dns ret: %d\n", ret);
		async_context_release_lock(context);
		
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
	HTTP_REQUEST_T req = {0};
	req.hostname = host;
	req.url = url_request;
	req.headers_fn = http_client_header_print_fn;
	req.recv_fn = http_client_receive_print_fn;
	int result = http_client_request_sync(cyw43_arch_async_context(), &req);
	result += http_client_request_sync(cyw43_arch_async_context(), &req); // repeat

	return 0;
}