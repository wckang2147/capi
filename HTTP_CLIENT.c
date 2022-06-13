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

void request_get_helloworld(void) {
    CURL *curl;
    CURLcode res;
    struct memory data;
    char url[100] = { 0, };

    memset(&data, 0, sizeof(data));

    curl = curl_easy_init();
    if (curl) {
        sprintf(url, "http://127.0.0.1:8080/helloworld?value=test");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 10000L); // 1000L - 1sec
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L); // 1000L - 1sec , 10000L - 10sec
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);

        res = curl_easy_perform(curl);
        printf("res = %d \n", res);
        if (CURLE_OK == res) {
            long status_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

            printf("[status line] Status Code: %ld\n", status_code);
            printf("[body] %s\n", data.response);

        }
        else if (CURLE_COULDNT_CONNECT == res)
		{
        	printf("Operation connection timed out.\n");
		}
        else if (CURLE_OPERATION_TIMEDOUT == res)
		{
        	printf("Operation timed out.\n");
		}

        else {
            printf("response is not OK: %d\n", res);
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

void request_post_helloworld(void) {
    CURL *curl;
    CURLcode res;
    struct memory data;
    char url[100] = { 0, };
    char post[100] = { 0, };

    memset(&data, 0, sizeof(data));

    curl = curl_easy_init();
    if (curl) {
        sprintf(url, "http://127.0.0.1:8080/helloworld");
        sprintf(post, "{\"name\":\"KIM\",\"phone\":\"010000000000\"}");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 500L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 500L); // 500L - 0.5sec , 1000L - 1sec , 10000L - 10sec
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // POST request
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post); // POST request payload
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(post)); // POST request payload size

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);

        res = curl_easy_perform(curl);
        if (CURLE_OK == res) {
            long status_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

            printf("[status line] Status Code: %ld\n", status_code);
            printf("[body] %s\n", data.response);
        }
        else if (CURLE_COULDNT_CONNECT == res)
		{
        	printf("Operation connection timed out.\n");
		}
        else if (CURLE_OPERATION_TIMEDOUT == res)
		{
        	printf("Operation timed out.\n");
		}
        else {
            printf("response is not OK: %d\n", res);
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

int main(int argc, char *argv[]){

    curl_global_init(CURL_GLOBAL_DEFAULT);

    request_get_helloworld();
    request_post_helloworld();

    return EXIT_SUCCESS;
}
