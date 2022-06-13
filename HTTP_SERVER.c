#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include <glib.h>
#include <microhttpd.h>
#include <json-c/json.h>
#include <curl/curl.h>

#define PORT 8080

/*
 * constructor 함수 : main 이전에 실행됨, GCC only
 *
 * eclipse에서 cygwin gdb 디버깅할 경우
 * gdb를 통한 프로그램의 콘솔 출력 버퍼가 정상동작 하지 않고
 * 실행 종료 후 한꺼번에 출력되는 경우가 있음.
 * 이를 피하기 위해 stdout, stderr의 buffering을 끄는 함수.
 * 만일 buffer가 꼭 필요하다면 아래 함수를 주석처리 할 것.
 */
void __attribute__((constructor)) console_setting_for_eclipse_debugging( void ){
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

struct connection_info {
    char *buffer;
    size_t buffer_size;
};

size_t buffer_write(char *upload_data, size_t upload_data_size, struct connection_info *conn_info) {
    char *ptr = realloc(conn_info->buffer, conn_info->buffer_size + upload_data_size + 1);
    if (ptr == NULL)
        return 0; /* out of memory! */

    conn_info->buffer = ptr;
    memcpy(&(conn_info->buffer[conn_info->buffer_size]), upload_data, upload_data_size);
    conn_info->buffer_size += upload_data_size;
    conn_info->buffer[conn_info->buffer_size] = 0;

    return upload_data_size;
}

static int send_error(struct MHD_Connection *connection, int error)
{
	struct MHD_Response *response = NULL;
	// cannot automate since cannot translate automagically between error number and MHD's status codes -- and cannot rely on MHD_HTTP_ values to provide an upper bound for an array
	const char *page_400 = "<html><head><title>Error 400</title></head><body><h1>Error 400 - Bad Request</h1></body></html>";
	const char *page_403 = "<html><head><title>Error 403</title></head><body><h1>Error 403 - Forbidden</h1></body></html>";
	const char *page_404 = "<html><head><title>Error 404</title></head><body><h1>Error 404 - Not Found</h1></body></html>";
	const char *page_500 = "<html><head><title>Error 500</title></head><body><h1>Error 500 - Internal Server Error. Oh no!</body></html>";
	const char *page_501 = "<html><head><title>Error 501</title></head><body><h1>Error 501 - Not Implemented</h1></body></html>";
	const char *page_503 = "<html><head><title>Error 503</title></head><body><h1>Error 503 - Internal Server Error</h1></body></html>";

	const char *mimetype = lookup_mimetype("foo.html");

	int ret = MHD_NO;

	switch (error) {
	case 400:
		response = MHD_create_response_from_buffer(strlen(page_400), (char *)page_400, MHD_RESPMEM_PERSISTENT);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
		break;

	case 403:
		response = MHD_create_response_from_buffer(strlen(page_403), (char *)page_403, MHD_RESPMEM_PERSISTENT);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_FORBIDDEN, response);
		break;

	case 404:
		response = MHD_create_response_from_buffer(strlen(page_404), (char *)page_404, MHD_RESPMEM_PERSISTENT);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
		break;

	case 500:
		response = MHD_create_response_from_buffer(strlen(page_500), (char *)page_500, MHD_RESPMEM_PERSISTENT);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
		break;

	case 501:
		response = MHD_create_response_from_buffer(strlen(page_501), (char *)page_501, MHD_RESPMEM_PERSISTENT);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_NOT_IMPLEMENTED, response);
		break;
	case 503:
		response = MHD_create_response_from_buffer(strlen(page_503), (char *)page_503, MHD_RESPMEM_PERSISTENT);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
		break;
	}

	if (response)
		MHD_destroy_response(response);
	return ret;
}

/**
 * 200(OK)로 응답 수행
 */
static enum MHD_Result response_result(struct MHD_Connection *connection,
        const char *page) {
    enum MHD_Result ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer(strlen(page), (void*) page,
            MHD_RESPMEM_MUST_COPY);
    if (!response)
        return MHD_NO;
	//MHD_add_response_header (response, "Content-Type", mimetype.c_str());
    //	MHD_add_response_header(response, "Content-type", "application/json");
    //MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_ENCODING, "text/plain");

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

/**
 * 입력된 Status Code로 응답 수행
 */
static enum MHD_Result response_error(struct MHD_Connection *connection,
        int status_code) {
    enum MHD_Result ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_MUST_COPY);
    if (!response)
        return MHD_NO;

    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}

/**
 * 접속이 완료되는 경우 호출되는 callback
 */
static void request_completed_callback(void *cls,
        struct MHD_Connection *connection,
        void **con_cls,
        enum MHD_RequestTerminationCode toe) {

    struct connection_info *con_info = *con_cls;
    if (con_info) {
        if (con_info->buffer) {
            free(con_info->buffer);
        }
        free(con_info);
    }
    *con_cls = NULL;
}
struct memory {
    char *response;
    size_t size;
};
size_t cb(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *) userp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if (ptr == NULL)
        return 0; /* out of memory! */

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

void request_get_helloworld2(void) {
    CURL *curl;
    CURLcode res;
    struct memory data;
    char url[100] = { 0, };

    memset(&data, 0, sizeof(data));
    printf("Pass Helloworld\n");

    curl = curl_easy_init();
    printf("Pass Helloworld\n");
    if (curl) {
        sprintf(url, "http://127.0.0.1:8081/helloworld");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 500L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);

        res = curl_easy_perform(curl);
        if (CURLE_OK == res) {
            long status_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

            printf("[status line] Status Code: %ld\n", status_code);
            printf("[body] %s\n", data.response);
            printf("Received Helloworld\n");

        } else {
            printf("response is not OK: %d\n", res);
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

int g_nSession[10]={0};
int g_nCntSession=0;
/**
 * 접속을 처리하는 callback
 */
static enum MHD_Result access_handler_callback(void *cls,
        struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *version,
        const char *upload_data,
        size_t *upload_data_size,
        void **con_cls) {

    printf("[start line] Method: %s\n", method);
    printf("[start line] Request URI: %s\n", url);

    const char *host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Host");
    printf("[header] Host: %s\n", host);

    // URL/page?value=??? get ??? value
    const char *value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value");
    printf("[Parameter] %s\n", value);

    g_nCntSession++;
    // POST 방식 처리 로직
    if (strcmp(method, "POST") == 0) {


        struct connection_info *con_info = (struct connection_info *)*con_cls;
        // 최초 접속의 경우
        if (con_info == NULL) {
            con_info = calloc(1, sizeof(struct connection_info));
            if (con_info == NULL) {
                return MHD_NO;
            }
            *con_cls = con_info;
            return MHD_YES;
        }

        if (*upload_data_size != 0) {
            buffer_write((char *)upload_data, *upload_data_size, con_info);
            *upload_data_size = 0;
            return MHD_YES;
        } else {
            // POST 방식 body에 대한 처리 수행
            printf("[body] %s\n", con_info->buffer);

            // 클라이언트가 Http body로 보낸 데이터를 json_object로 만듬
            //json_object *post_object = json_tokener_parser(con_info->buffer);
            //json_object *tmp;
            //json_object *obj_foo = json_object_object_get_ex(post_object, "foo", &tmp);

            //d_printf("Name: %s\n", json_object_get_string(tmp));


            printf("test1\n");
            //curl_global_init(CURL_GLOBAL_DEFAULT);
            printf("test2\n");
            request_get_helloworld2();
            printf("test3\n");

            return response_result(connection, "Hello World2!");

        }
    }
    // GET 방식 처리 로직
    if (strcmp(method, "GET") == 0) {
        printf("[body] 없음\n");
        sleep(1000);
    }
    printf("\n");

    return response_result(connection, "Hello World!");
}

int main(int argc, char *argv[]) {

	curl_global_init(CURL_GLOBAL_DEFAULT);

	int port;
	port = atoi(argv[1]);
    // HTTP 데몬을 시작한다
    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
    		port,
        NULL,
        NULL,
        &access_handler_callback,
        NULL,
        MHD_OPTION_NOTIFY_COMPLETED,
        request_completed_callback,
        NULL,
		MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
		//MHD_OPTION_THREAD_POOL_SIZE, (unsigned int) 10,
					MHD_OPTION_CONNECTION_LIMIT, (unsigned int) 2,

        MHD_OPTION_END);
    if (daemon == NULL) {
        fprintf(stderr, "MHD_start_daemon() error\n");
        return EXIT_FAILURE;
    }

    while (true) {
        getc(stdin);
    }

    // HTTP 데몬을 종료한다
    MHD_stop_daemon(daemon);

    return EXIT_SUCCESS;
}
