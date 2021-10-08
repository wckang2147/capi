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

/*
 * constructor 함수 : main 이전에 실행됨, GCC only
 *
 * eclipse에서 cygwin gdb 디버깅할 경우
 * gdb를 통한 프로그램의 콘솔 출력 버퍼가 정상동작 하지 않고
 * 실행 종료 후 한꺼번에 출력되는 경우가 있음.
 * 이를 피하기 위해 stdout, stderr의 buffering을 끄는 함수.
 * 만일 buffer가 꼭 필요하다면 아래 함수를 주석처리 할 것.
 */
void __attribute__((constructor)) console_setting_for_eclipse_debugging(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

#include <stdint.h>

#define PORT 8080

#define PARSON
#undef PARSON
#ifdef PARSON
#include "parson.h"
#endif

// 타임 스탬프 변수형
typedef uint32_t timestamp_t; //seconds

// 데이트타임 구조체
typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t week;
	uint8_t weekday;
} datetime_t;

// 1일을 초로
#define ONE_DAY                  (1*60*60*24)
// UTC 시작 시간
#define UTC_TIME_WEEKDAY_OFFSET (4) /* 1970,1,1은 목요일이기때문에 */

//날짜                    x, 1월, 2월 ..... 11월, 12월
uint8_t month_days[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//타임 스탬프를 기준으로 요일 얻기
uint8_t timestamp_to_weekday(timestamp_t timestamp_sec) {
	uint8_t result = (timestamp_sec / ONE_DAY + UTC_TIME_WEEKDAY_OFFSET) % 7;
	if (result == 0) {
		result = 7;
	}
	return result;
}

//윤달 확인
int is_leap_year(uint16_t year) {
	if (year % 4 == 0 && ((year % 100) != 0) || ((year % 400) == 0)) {
		return true;
	} else
		return false;
}

//utc 타임 스탬프를 날짜로 변환
void utc_timestamp_to_date(timestamp_t timestamp, datetime_t *datetime) {
	uint8_t month;
	uint32_t days;
	uint16_t days_in_year;
	uint16_t year;
	timestamp_t second_in_day;

// 시/분/초 계산
	second_in_day = timestamp % ONE_DAY;

//초
	datetime->second = second_in_day % 60;

//분
	second_in_day /= 60;
	datetime->minute = second_in_day % 60;

//시
	second_in_day /= 60;
	datetime->hour = second_in_day % 24;

//1970-1-1 0:0:0부터 현재까지 총 일수
	days = timestamp / ONE_DAY;

//days를 계속 차감하면서 해당 년도 계산
	for (year = 1970; year <= 2200; year++) {
		if (is_leap_year(year))
			days_in_year = 366;
		else
			days_in_year = 365;

		if (days >= days_in_year)
			days -= days_in_year;
		else
			break;
	}

//년
	datetime->year = year;

//요일
	datetime->weekday = timestamp_to_weekday(timestamp);

//해당 년도 1월 1일을 기준으로 지금까지의 주(week) 계산
	datetime->week = (days + 11 - datetime->weekday) / 7;

//월 계산하기
	if (is_leap_year(datetime->year)) //윤달의 경우 2월이 29일이다.
		month_days[2] = 29;
	else
		month_days[2] = 28;

//년도와 마찬가지로 일에서 계속 차감해서 찾는다.
	for (month = 1; month <= 12; month++) {
		if (days >= month_days[month])
			days -= month_days[month];
		else
			break;
	}
	datetime->month = month;
	datetime->day = days + 1;
}

int GetTimeT(int year, int month, int day, int hour, int minute, int second) {
	struct tm t = { 0 };
	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;

	return mktime(&t);
}

int convertTimestamp() {

	time_t time_now;
	struct tm *tm;

	time(&time_now);
	tm = localtime(&time_now);

	printf(ctime(&time_now));

	printf(" year : %d \n", tm->tm_year + 1900);
	printf(" month: %d \n", tm->tm_mon + 1);
	printf(" day : %d \n", tm->tm_mday);
	printf(" hour : %d \n", tm->tm_hour);
	printf(" min : %d \n", tm->tm_min);
	printf(" sec : %d \n", tm->tm_sec);
	printf(" wday : %d \n", tm->tm_wday); // 0~6 , day of the week
	printf(" yday : %d \n", tm->tm_yday); // past time from 1, Jan

	return 0;
}

//테스트
int timestamp() {
	timestamp_t unix_timestamp = 1615546840;
	datetime_t datetime;
	utc_timestamp_to_date(unix_timestamp, &datetime);
	printf("unix time : %d\n", unix_timestamp);
	printf("datetime : %d-%d-%d(%d, %d)_%d:%d:%d\n", datetime.year,
			datetime.month, datetime.day, datetime.week, datetime.weekday,
			datetime.hour, datetime.minute, datetime.second);
}

#define DEBUG
//#undef DEBUG
#ifdef DEBUG
#define d_printf(fmt, args...) fprintf(stderr, "[%s:%d:%s()]: " fmt, \
__FILE__, __LINE__, __func__, ##args)
#else
#define d_printf(fmt, args...)
#endif

void removeLineFeed(char *strBuf) {
	char *pos = NULL;
	if ((pos = strchr(strBuf, '\n')) != NULL)
		*pos = '\0';
	if ((pos = strchr(strBuf, '\r')) != NULL)
		*pos = '\0';
}

char* removeEndChar(char *strBuf, char ch) {
	if (strBuf[strlen(strBuf) - 1] == ch)
		strBuf[strlen(strBuf) - 1] = '\0';

	return strBuf;
}

// name#url,url;url,url,url;
typedef struct {
	char urlStr[50];
	int nMaxSession;
	int nCurSession;
} t_url;

typedef struct {
	char indexStr[50];
	int indexInt;

	t_url listURL[10];
	int nCountURL;
} t_contents;

#define MAX_SERVICE 50
t_contents g_readinfo[MAX_SERVICE] = { 0, };
int g_cntInfo = 0;

json_object *g_JsonObject = NULL;

#define PRINT_KEY_VALUE(type,obj) \
	switch(type){ \
		case json_type_null:\
			printf("Type is null\n");\
			break;\
		case json_type_boolean:\
			printf("%d\n",json_object_get_boolean(obj));\
			break;\
		case json_type_double:\
			printf("%f\n",json_object_get_double(obj));\
			break;\
		case json_type_int:\
			printf("%d\n",json_object_get_int(obj));\
			break;\
		case json_type_object:\
			printf("%ld\n",json_object_get_int64(obj));\
			break;\
		case json_type_array:\
			printf("%s\n",json_object_get_string(obj));\
			break;\
		case json_type_string:\
			printf("%s\n",json_object_get_string(obj));\
			break;\
	}

void parseJsonObject1(json_object *pInput) {
	json_object *pJsonObject = NULL;
	int type = 0;

	pJsonObject = json_object_object_get(pInput, "name");
	type = json_object_get_type(pJsonObject);
	printf("[ex1]");
	PRINT_KEY_VALUE(type, pJsonObject);
}

void parseJsonObject2(json_object *pInput) {
	json_object *pJsonObject = NULL;
	int type = 0;

	json_object_object_foreach(pInput, key_name, value_obj)
	{
		printf("[ex2] %s\n", key_name);
		type = json_object_get_type(value_obj);
		PRINT_KEY_VALUE(type, value_obj);
	}
}

void parseJsonObject3(json_object *pInput) {
	json_object *temp = NULL;
	char *name = NULL;
	struct json_object_iterator iter;
	struct json_object_iterator itEnd;
	int type = 0;

	iter = json_object_iter_begin(pInput);
	itEnd = json_object_iter_end(pInput);

	while (!json_object_iter_equal(&iter, &itEnd)) {
		name = json_object_iter_peek_name(&iter);
		temp = json_object_iter_peek_value(&iter);
		type = json_object_get_type(temp);
		printf("[ex3] %s\n", name);
		PRINT_KEY_VALUE(type, temp);
		json_object_iter_next(&iter);
	}
}

void parseJsonArray1(json_object *pInput) {
	json_object *pJsonObject = NULL;
	int type = 0;
	int i = 0;

	pJsonObject = json_object_object_get(pInput, "URL");
	if (json_object_get_type(pJsonObject) != json_type_array)
		return;

	for (i = 0; i < json_object_array_length(pJsonObject); i++) {
		json_object *temp = json_object_array_get_idx(pJsonObject, i);
		type = json_object_get_type(temp);
		printf("[ex4]");
		PRINT_KEY_VALUE(type, temp);
	}
}

void readJSONFile(char *filename) {
	g_JsonObject = json_object_from_file(filename);
	parseJsonObject3(g_JsonObject);
	json_object_put(g_JsonObject);

}

void readFileContents(char *filename) {

	char str[1024] = { 0, };
	char delim1[2] = "#", delim2[2] = ";", delim3[2] = ",";

	char *first;
	char *second;
	char *third;
	char buf1[200], buf2[200], buf3[200];
	FILE *fp = fopen(filename, "r+");

	while (fgets(str, 1000, fp) != NULL) {
		removeLineFeed(str);

		d_printf("Line: %s\n", str);

		for (first = strtok(str, delim1); first != NULL;
				first = strtok(first + strlen(first) + 1, delim1)) {
			strncpy(buf1, first, sizeof(buf1));

			// left 항목 저장
			d_printf("left: %s\n", buf1);
			strcpy((char*) g_readinfo[g_cntInfo].indexStr, first);
			// g_readinfo[g_cntInfo].indexInt = atoi (first); // 숫자 변환 필요 시

			// Right 항목 추가 분류를 위해 다음 토큰 호출
			first = strtok(first + strlen(first) + 1, delim1);
			strncpy(buf1, first, sizeof(buf1));
			d_printf("right str: %s, len = %d\n", buf1, strlen(buf1));

			// Right 항목 추가 분류 시작
			int nCountURL = 0;
			for (second = strtok(buf1, delim2); second != NULL;
					second = strtok(second + strlen(second) + 1, delim2)) {
				strncpy(buf2, second, sizeof(buf2));
				d_printf("\tsecond: %s\n", buf2);

				// 추가 분류 필요 시 반복 호출

				// third의 첫번째가 URL인 경우
				third = strtok(buf2, delim3);
				strcpy((char*) g_readinfo[g_cntInfo].listURL[nCountURL].urlStr,
						third);
				d_printf("\t  third: %s\n", third);
				// third의 두번째가 숫자인 경우
				third = strtok(NULL, delim3);
				d_printf("\t  third: %s\n", third);
				if (third != NULL)
					g_readinfo[g_cntInfo].listURL[nCountURL].nMaxSession = atoi(
							third);
				else
					g_readinfo[g_cntInfo].listURL[nCountURL].nMaxSession = 0;

				nCountURL++;

			}
			g_readinfo[g_cntInfo].nCountURL = nCountURL;
		}
		g_cntInfo++;
	}
	fclose(fp);
}
#ifdef PARSON
void writeJSONFile2(char *filename)
{
    JSON_Value *rootValue;
    JSON_Object *rootObject;

    rootValue = json_value_init_object();             // JSON_Value 생성 및 초기화
    rootObject = json_value_get_object(rootValue);    // JSON_Value에서 JSON_Object를 얻음

    json_object_set_number(rootObject, "Count", g_cntInfo);

	for (int i=0;i<g_cntInfo;i++)
	{
	    JSON_Value *clusterValue[g_readinfo[i].nCluster];
	    JSON_Object *clusterObject[g_readinfo[i].nCluster];

	    for (int j=0;j<g_readinfo[i].nCluster;j++)
		{

		    clusterValue[j] = json_value_init_object();             // JSON_Value 생성 및 초기화
		    clusterObject[j] = json_value_get_object(clusterValue[j]);    // JSON_Value에서 JSON_Object를 얻음

			d_printf ("\tcluster = %s, timeout = %d\n", g_readinfo[i].cluster[j].url , g_readinfo[i].cluster[j].nMaxSession );

		    json_object_set_string (clusterObject[j] , "URL", g_readinfo[i].cluster[j].url);
			json_object_set_number (clusterObject[j] , "Timeout", g_readinfo[i].cluster[j].nMaxSession);

			json_object_dotset_value (rootObject, g_readinfo[i].indexStr, clusterValue[j] );
		}
	}

    json_serialize_to_file_pretty(rootValue, filename);
    json_value_free(rootValue);    // JSON_Value에 할당된 동적 메모리 해제

    /*
    // 객체에 키를 추가하고 문자열 저장
    json_object_set_string(rootObject, "Title", "Inception");
    // 객체에 키를 추가하고 숫자 저장
    json_object_set_number(rootObject, "Year", 2010);
    json_object_set_number(rootObject, "Runtime", 148);
    // 객체에 키를 추가하고 문자열 저장
    json_object_set_string(rootObject, "Genre", "Sci-Fi");
    json_object_set_string(rootObject, "Director", "Christopher Nolan");

    // 객체에 키를 추가하고 배열 생성
    json_object_set_value(rootObject, "Actors", json_value_init_array());
    // 객체에서 배열 포인터를 가져옴
    JSON_Array *actors = json_object_get_array(rootObject, "Actors");
    // 배열에 문자열 요소 추가
    json_array_append_string(actors, "Leonardo DiCaprio");
    json_array_append_string(actors, "Joseph Gordon-Levitt");
    json_array_append_string(actors, "Ellen Page");
    json_array_append_string(actors, "Tom Hardy");
    json_array_append_string(actors, "Ken Watanabe");

    // 객체에 키를 추가하고 숫자 저장
    json_object_set_number(rootObject, "imdbRating", 8.8);
    // 객체에 키를 추가하고 불 값 저장
    json_object_set_boolean(rootObject, "KoreaRelease", true);

    // JSON_Value를 사람이 읽기 쉬운 문자열(pretty)로 만든 뒤 파일에 저장
    json_serialize_to_file_pretty(rootValue, "example.json");

    json_value_free(rootValue);    // JSON_Value에 할당된 동적 메모리 해제
    */

}
#else
void writeJSONFile(char *filename) {

	json_object *rootObject = json_object_new_object();
	json_object_object_add(rootObject, "Count", json_object_new_int(g_cntInfo));

	for (int i = 0; i < g_cntInfo; i++) {
//		json_object_object_add(rootObject, "ClusterCount", json_object_new_int(g_readinfo.nCountURL[i]));

		json_object *jarrayURL[g_readinfo[i].nCountURL];
		json_object *jarraySession[g_readinfo[i].nCountURL];
		json_object *serviceObject[g_readinfo[i].nCountURL];

		jarrayURL[i] = json_object_new_array();
		jarraySession[i] = json_object_new_array();

		serviceObject[i] = json_object_new_object();

		for (int j = 0; j < g_readinfo[i].nCountURL; j++) {

			d_printf("\tcluster = %d, timeout = %s\n", g_readinfo[i].nCountURL,
					g_readinfo[i].listURL[j].urlStr);

			json_object_array_add(jarrayURL[i],
					json_object_new_string(g_readinfo[i].listURL[j].urlStr));
			json_object_array_add(jarraySession[i],
					json_object_new_int(g_readinfo[i].listURL[j].nMaxSession));

		}

		json_object_object_add(serviceObject[i], "ServiceCount",
				json_object_new_int(g_readinfo[i].nCountURL));
		json_object_object_add(serviceObject[i], "URL", jarrayURL[i]);
		json_object_object_add(serviceObject[i], "MaxSession",
				jarraySession[i]);

		json_object_object_add(rootObject, g_readinfo[i].indexStr,
				serviceObject[i]);

	}

	json_object_to_file_ext(filename, rootObject,
	JSON_C_TO_STRING_PRETTY | (1 << 4)); //JSON_C_TO_STRING_NOSLASHESCAPE (1<<4)

	json_object_put(rootObject);

}
void covertJSONtoString(char *str) {

	json_object *rootObject = json_object_new_object();
	json_object_object_add(rootObject, "Count", json_object_new_int(g_cntInfo));

	for (int i = 0; i < g_cntInfo; i++) {
//		json_object_object_add(rootObject, "ClusterCount", json_object_new_int(g_readinfo.nCountURL[i]));

		json_object *jarrayURL[g_readinfo[i].nCountURL];
		json_object *jarraySession[g_readinfo[i].nCountURL];
		json_object *serviceObject[g_readinfo[i].nCountURL];

		jarrayURL[i] = json_object_new_array();
		jarraySession[i] = json_object_new_array();

		serviceObject[i] = json_object_new_object();

		for (int j = 0; j < g_readinfo[i].nCountURL; j++) {

			d_printf("\tcluster = %d, timeout = %s\n", g_readinfo[i].nCountURL,
					g_readinfo[i].listURL[j].urlStr);

			json_object_array_add(jarrayURL[i],
					json_object_new_string(g_readinfo[i].listURL[j].urlStr));
			json_object_array_add(jarraySession[i],
					json_object_new_int(g_readinfo[i].listURL[j].nMaxSession));

		}

		json_object_object_add(serviceObject[i], "ServiceCount",
				json_object_new_int(g_readinfo[i].nCountURL));
		json_object_object_add(serviceObject[i], "URL", jarrayURL[i]);
		json_object_object_add(serviceObject[i], "MaxSession",
				jarraySession[i]);

		json_object_object_add(rootObject, g_readinfo[i].indexStr,
				serviceObject[i]);

	}

//	json_object_to_file_ext(filename, rootObject, JSON_C_TO_STRING_PRETTY|(1<<4) ); //JSON_C_TO_STRING_NOSLASHESCAPE (1<<4)

	strcpy(str, json_object_to_json_string_ext(rootObject,
	JSON_C_TO_STRING_PRETTY | (1 << 4)));
	json_object_put(rootObject);

}
#endif

// ===== start of client code
struct memory {
	char *response;
	size_t size;
};

size_t cb(void *data, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct memory *mem = (struct memory*) userp;

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
	printf("[send GET message]\n");

	curl = curl_easy_init();
	if (curl) {
		sprintf(url, "http://127.0.0.1:8080/helloworld");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 60L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void* )&data);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10); //timeout after 5 minutes (use sigalarm)

		res = curl_easy_perform(curl);
		if (CURLE_OK == res) {
			long status_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

			printf("[status line] Status Code: %ld\n", status_code);
			printf("[body] %s\n", data.response);

		} else {
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
	char post[2000] = { 0, };
	struct curl_slist *list = NULL;

	memset(&data, 0, sizeof(data));

	curl = curl_easy_init();
	if (curl) {
		sprintf(url, "http://127.0.0.1:8080/helloworld");
		sprintf(post, "Hello World!");
		/*
		 covertJSONtoString ((char*) post);
		 d_printf("str = %s\n", post);

		 json_object *jobj = json_tokener_parse (post);
		 d_printf("json = %d\n", json_object_get_int (json_object_object_get (jobj, "Count") ) );
		 */

		/* Header 설정
		 list = curl_slist_append(list, "Content-Type: application/json"); // content-type 정의 내용 list에 저장
		 curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list); // content-type 설정
		 */
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 500L); //
		curl_easy_setopt(curl, CURLOPT_POST, 1L); // POST request
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post); // POST request payload
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(post)); // POST request payload size

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void* )&data); // response

		res = curl_easy_perform(curl);
		if (CURLE_OK == res) {
			long status_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

			printf("[status line] Status Code: %ld\n", status_code);
			printf("[body] %s\n", data.response);
		} else {
			printf("response is not OK: %d\n", res);
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
}
// ============= end of client code =========

struct connection_info {
	char *buffer;
	size_t buffer_size;
};

size_t buffer_write(char *upload_data, size_t upload_data_size,
		struct connection_info *conn_info) {
	char *ptr = realloc(conn_info->buffer,
			conn_info->buffer_size + upload_data_size + 1);
	if (ptr == NULL)
		return 0; /* out of memory! */

	conn_info->buffer = ptr;
	memcpy(&(conn_info->buffer[conn_info->buffer_size]), upload_data,
			upload_data_size);
	conn_info->buffer_size += upload_data_size;
	conn_info->buffer[conn_info->buffer_size] = 0;

	return upload_data_size;
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

	/*
	 //MHD_set_response_options(response, MHD_RF_HTTP_VERSION_1_0_ONLY, MHD_RO_END);
	 MHD_add_response_header(response, "Location", url);
	 MHD_add_response_header(response, "Connection", "close");

	 */
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

int g_flagStopMainDaemon = 1;
/**
 * 접속이 완료되는 경우 호출되는 callback
 */
static void request_completed_callback(void *cls,
		struct MHD_Connection *connection, void **con_cls,
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

// 쓰레드 함수
void* t_function(void *data) {
	pid_t pid;            // process id
	pthread_t tid;        // thread id

	pid = getpid();
	tid = pthread_self();

	char *thread_name = (char*) data;
	int i = 0;

	while (i < 30)   // 0,1,2 까지만 loop 돌립니다.
	{
		// 넘겨받은 쓰레드 이름과
		// 현재 process id 와 thread id 를 함께 출력
		d_printf("[%s] pid:%u, tid:%x --- %d\n", thread_name,
				(unsigned int )pid, (unsigned int )tid, i);
		i++;
		request_get_helloworld();

		sleep(1);  // 1초간 대기
	}
}

void createThread() {
	pthread_t p_thread[2];
	int threadId;
	int status;
	char p1[] = "name_th1";
	char p2[] = "name_th2";
	char p3[] = "name_th3";

	threadId = pthread_create(&p_thread[0], NULL, t_function, (void*) p1); //2
	threadId = pthread_create(&p_thread[1], NULL, t_function, (void*) p2); //2
	threadId = pthread_create(&p_thread[2], NULL, t_function, (void*) p3); //2

}
int cnt = 0;

/**
 * 접속을 처리하는 callback
 */
static enum MHD_Result access_handler_callback(void *cls,
		struct MHD_Connection *connection, const char *url, const char *method,
		const char *version, const char *upload_data, size_t *upload_data_size,
		void **con_cls) {

	printf("[start line] Method: %s\n", method);
	printf("[start line] Request URI: %s\n", url);

	const char *host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND,
			"Host");
	printf("[header] Host: %s\n", host);

// POST 방식 처리 로직
	if (strcmp(method, "POST") == 0) {
		struct connection_info *con_info = (struct connection_info*) *con_cls;
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
			buffer_write((char*) upload_data, *upload_data_size, con_info);
			*upload_data_size = 0;
			return MHD_YES;
		} else {
			// POST 방식 body에 대한 처리 수행
			printf("[POST Received body] %s\n", con_info->buffer);
		}
	}
// GET 방식 처리 로직
	if (strcmp(method, "GET") == 0) {
		cnt = 0;
		pthread_t tid;        // thread id
		tid = pthread_self();
		d_printf("tid:%x --- %d\n", (unsigned int )tid);
		while (1) {
			cnt++;
			printf("[GET Received body] 없음 %d\n", cnt);
			sleep(1);
			if (cnt > 5)
				break;
		}

	}
	printf("\n");

	return response_result(connection, "Hello World!");
}

void* threadDaemon(void *data) {
	char *nameThread = (char*) data;
	pid_t pid;
	pthread_t tid;
	pid = getpid();
	tid = pthread_self();

// HTTP 데몬을 시작한다
	struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
	PORT,
	NULL,
	NULL, &access_handler_callback,
	NULL, MHD_OPTION_NOTIFY_COMPLETED, request_completed_callback,
	NULL, MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
//MHD_OPTION_THREAD_POOL_SIZE, (unsigned int) 10,
			MHD_OPTION_CONNECTION_LIMIT, (unsigned int) 2,

			MHD_OPTION_END);
	if (daemon == NULL) {
		fprintf(stderr, "MHD_start_daemon() error\n");
		return EXIT_FAILURE;
	}

// g_flagStopMainDaemon가 0이 되기 전까지는 종료하지 않는다.
	while (g_flagStopMainDaemon) {
		d_printf("%s\n", nameThread);
		sleep(1);
	}

	d_printf("Stop %s Daemon\n", nameThread);
// HTTP 데몬을 종료한다
	MHD_stop_daemon(daemon);

}

#include <io.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

int findFileAtDir(char *dirpath, char *name, char **find_path) {
	DIR *dir_info = NULL;
	struct dirent *dir_entry = NULL;
	struct stat buf;
	int dir_mode_err;

	char filepath[100];
	dir_info = opendir(dirpath);
	if (NULL == dir_info) {
		printf("Directory Open Error\n");
		return -1;
	}
	while ((dir_entry = readdir(dir_info)) != NULL) {
		//printf("%s\n", dir_entry->d_name);
		if (strncmp(dir_entry->d_name, "..", 2) == 0
				|| strncmp(dir_entry->d_name, ".", 1) == 0) {
			continue;
		}
		sprintf(filepath, "%s%s", dirpath, dir_entry->d_name);

		dir_mode_err = lstat(filepath, &buf);

		if (dir_mode_err == -1) {
			puts("Dir mode error");
			break;
		}
		if (S_ISDIR(buf.st_mode)) {
			strcat(filepath, "/");
			if (findFileAtDir(filepath, name, find_path) == 1) {
				closedir(dir_info);
				return 1;
			}
		} else {
			if (S_ISREG(buf.st_mode)) {
				if (strcmp(dir_entry->d_name, name) == 0) {
					//printf("Found path = %s, dirpath = %s, name = %s\n", *find_path, dirpath, dir_entry->d_name);
					strcat(*find_path, dirpath);
					strcat(*find_path, "/");
					strcat(*find_path, dir_entry->d_name);

					return 1;
				}
				//printf("%s\n", dir_entry->d_name);
			}
		}
	}
	closedir(dir_info);
	return 0;
}

int callSO() {

	void *pHandle = dlopen("./library.so", RTLD_NOW); // RTLD_RAZY
	void (*pFunc)(void);
	pFunc = dlsym(pHandle, "Func1");
	(*pFunc)();
	dlclose(pHandle);
	return 0;
}
int compareInt(const void *a, const void *b) // 오름차순 비교 함수 구현
{
	int num1 = *(int*) a;
	int num2 = *(int*) b;
	if (num1 < num2) // a가 b보다 작을 때는 -1
		return -1;

	if (num1 > num2) // a가 b보다 클 때는 1

		return 1;

	return 0; // a와 b가 같을 때는 0 반환

}

int compareStr(const void *a, const void *b)

{

	char const **aa = (char const**) a;

	char const **bb = (char const**) b;

	return strcmp(*aa, *bb);

}

int compareCh(const void *a, const void *b)

{

	char const *aa = (char const*) a;

	char const *bb = (char const*) b;

	return (*aa - *bb);

}

typedef struct {

	int index;

	char name[20];

} t_str;

int compareStruct(const void *a, const void *b)

{

	t_str *A = (t_str*) a;

	t_str *B = (t_str*) b;

// 이름 순

//return ( strcmp(A->name, B->name) );

// 인덱스 낮은 순

//return ( A->index - B->index );

// 인덱스 높은 순

	return (B->index - A->index);

}

void sortSample()

{

	setvbuf(stdout, NULL, _IONBF, 0);

	setvbuf(stderr, NULL, _IONBF, 0);

	int numArr[10] = { 1000, 400, 50, 55, 23, 17, 10, 330, 106, 900 };

	qsort(numArr, sizeof(numArr) / sizeof(int), sizeof(int), compareInt);

	for (int i = 0; i < 10; i++)

	{

		printf("%d ", numArr[i]);

	}

	printf("\n");

	char ch[10] = "kdriau93e";

	qsort(ch, strlen(ch) / sizeof(char), sizeof(char), compareCh);

	for (int i = 0; i < strlen(ch) / sizeof(char); i++)

	{

		printf("%c", ch[i]);

	}

	printf("\n");

	char *s2[] = { "test", "char", "sort", "qsort", "123" };

	qsort(s2, sizeof(s2) / sizeof(char*), sizeof(char*), compareStr);

	for (int i = 0; i < sizeof(s2) / sizeof(char*); i++)

	{

		printf("%s ", s2[i]);

	}

	printf("\n");

	t_str s3[4] = { { 90, "kang" }, { 20, "kim" }, { 35, "ahn" },
			{ 100, "jung" } };

	qsort(s3, 4, sizeof(t_str), compareStruct);

	for (int i = 0; i < sizeof(s3) / sizeof(t_str); i++)

	{

		printf("[%d] %s ", s3[i].index, s3[i].name);

	}

	printf("\n");

	printf("\n");

}

char* readFullFile(char *filename, int *readSize) // 파일을 읽어서 내용을 반환하는 함수
{
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return NULL;
	int size;
	char *buffer;
// 파일 크기 구하기
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
// 파일 크기 + NULL 공간만큼 메모리를 할당하고 0으로 초기화
	buffer = malloc(size + 1);
	memset(buffer, 0, size + 1);
// 파일 내용 읽기
	if (fread(buffer, size, 1, fp) < 1) {
		*readSize = 0;
		free(buffer);
		fclose(fp);
		return NULL;
	}
// 파일 크기를 넘겨줌
	*readSize = size;

	fclose(fp); // 파일 포인터 닫기

	return buffer;
}

int main(int argc, char *argv[]) {

	int status;
	char nameHTTPDaemon[] = "HTTPDaemonThread";
	pthread_t pthreadHTTPDaemon;
	int thr_id;

	/*
	 int num;
	 scanf("%d", &num);
	 */

	readFileContents("./ReadSample.txt");
	writeJSONFile("./Result.txt");
	readJSONFile("./ReadSampleJson.txt");

	thr_id = pthread_create(&pthreadHTTPDaemon, NULL, threadDaemon,
			(void*) nameHTTPDaemon);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	request_post_helloworld();

	sleep(3);
	g_flagStopMainDaemon = 1;
	createThread();
	pthread_join(pthreadHTTPDaemon, (void**) &status);

	return EXIT_SUCCESS;
}
