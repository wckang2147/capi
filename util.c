#include <stdio.h>
#include <stdlib.h>
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

/*
 *
 * constructor 함수 : main 이전에 실행됨, GCC only
 *
 * eclipse에서 cygwin gdb 디버깅할 경우 gdb를 통한 프로그램의 콘솔 출력 버퍼가 정상동작 하지 않고 실행 종료 후 한꺼번에 출력됨.
 * 이를 피하기 위해 stdout, stderr의 buffering을 끄는 함수.
 * 만일 buffer가 꼭 필요하다면 아래 함수를 주석처리 할 것.
 */
void __attribute__((constructor)) console_setting_for_eclipse_debugging( void ){
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

#define ARRAY_SIZE(x) ( sizeof((x))/sizeof((x)[0]) ) // 배열의 길이
#define ABS(x)        ( ((x)<0)?-(x):(x) ) // 절대값
#define SQUARE(x)     ( (x)*(x) ) // 제곱근
#define UPCASE(c)     ( ((c)>='a'&&(c)<='z')?(c)-('a'-'A'):(c) ) // 소문자 -> 대문자
#define LOWCASE(c)    ( ((c)>='A'&&(c)<='z')?(c)+('a'-'A'):(c) ) // 대문자 -> 소문자
#define MAX(x,y)      ( (x)>(y)?(x):(y) ) // 최대 값
#define MIN(x,y)      ( (x)<(y)?(x):(y) ) // 최소 값

int addSet(char set[][20], char *buf, int size)
{
    int i;
    printf("buf = %s\n", buf);
    for (i=0; i<size;i++)
    {
       if ( strcmp(buf, set[i] ) == 0 )
        {
            return size;
        }
    }

    strcpy (set[i] , buf);
    printf("set[%d] = %s, buf = %s\n", i, set[i],  buf);
    return size+1;
}




//input string을 delim으로 구분하여 array에 저장
void removeLineFeed(char *strBuf)
{
	char *pos = NULL;
	if ((pos = strchr(strBuf, '\n')) != NULL)
		*pos = '\0';
	if ((pos = strchr(strBuf, '\r')) != NULL)
		*pos = '\0';
}

typedef struct {
	char name[1024];
	char value[1024];
} t_input;

t_input condition[1024] = { 0, };
int index_cond = 0;
void parseInput(char *input, char *delim)
{
	char *buf ;

	buf = strtok((char*)input, delim);
	strcpy( condition[index_cond].name , buf);
	buf = strtok(NULL, delim);
	strcpy( condition[index_cond].value , buf);

	index_cond++;

}

void parseInput2(char *buf)
{
	while ( fgets(buf, 1024, fp) != NULL )
	{
		removeLineFeed(buf);
		d_printf("Read Line\n");

		sscanf(buf, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]", log[i].date , log[i].time, log[i].name, log[i].category, log[i].comments);
		d_printf("%s %s %s %s Comments = %s\n", log[i].date , log[i].time, log[i].name, log[i].category, log[i].comments);

		i++;
	}

}


// thread

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

static pthread_t p_thread;
static int thr_id;
static int thr_exit = TRUE;
pthread_mutex_t mutex_var;

static int test = 0;
void *t_function(void *data )
{
	int i=0;
	 pthread_mutex_lock(&mutex_var);

	while (thr_exit != TRUE || i < 10)
	{
		// Thread logic
		printf("Thread1 = %d test = %d\n", i++, test);
		test = 1;
		sleep (1); // Context Switching
	}
	pthread_mutex_unlock(&mutex_var);

	pthread_exit((void*) 0);
}
void start_thread()
{
	thr_exit = FALSE;
	thr_id = pthread_create(&p_thread, NULL, t_function, NULL);
}

void end_thread()
{
	thr_exit = TRUE;
	pthread_join (p_thread, (void **) NULL);

}
void main() {

	printf("Hello\n");
	pthread_mutex_init(&mutex_var, NULL);

	start_thread();

	// Main Logic
	for (int i=0;i<10;i++)
	{
		printf("Main = %d test = %d\n", i, test);
		test=0;
		sleep(1); // Context Switching

	}

	end_thread();
}

#include <dlfcn.h>



int main()
{

	void* pHandle = dlopen("./test.so", RTLD_NOW); // RTLD_RAZY
	void (*pFunc)(void);
	pFunc = dlsym(pHandle, "Func1");

	(*pFunc)();

	dlclose(pHandle);

	return 0;

}



#include <stdio.h>

#define MAXLINE 256

int main()
{
    FILE *fp;
    int state;

    char buff[MAXLINE];
    fp = popen("ls -al", "r");
    if (fp == NULL)
    {
        perror("erro : ");
        exit(0);
    }

    while(fgets(buff, MAXLINE, fp) != NULL)
    {
        printf("%s", buff);
    }

    state = pclose(fp);
    printf("state is %d\n", state);
}

//https://www.joinc.co.kr/w/man/2/popen



#include &ltstdio.h>

#define MAXSTRS 5

int main(void)
{
        int  cntr;
        FILE *pipe_fp;
        char *strings[MAXSTRS] = { "echo", "bravo", "alpha",
                                  "charlie", "delta"};

        /*popen() 호출을 사용하여 단방향 파이프를 만든다*/
        if (( pipe_fp = popen("sort", "w")) == NULL)
        {
                perror("popen");
                exit(1);
        }

        /*반복 처리*/
        for(cntr=0; cntr<MAXSTRS; cntr++) {
                fputs(strings[cntr], pipe_fp);
                fputc('\n', pipe_fp);
        }

        /*파이프를 닫는다*/
        pclose(pipe_fp);

        return(0);
}


#define DEBUG
//#undef DEBUG
#ifdef DEBUG
#define d_printf(fmt, args...) fprintf(stderr, "[%s:%d:%s()]: " fmt, \
__FILE__, __LINE__, __func__, ##args)
#else
#define d_printf(fmt, args...)
#endif

#include <stdio.h>

int main(void)
{
        FILE *pipein_fp, *pipeout_fp;
        char readbuf[80];

        /*popen() 호출을 사용하여 단방향 파이프를 만든다*/
        if (( pipein_fp = popen("ls", "r")) == NULL)
        {
                perror("popen");
                exit(1);
        }

        /*popen() 호출을 사용하여 단방향 파이프를 만든다*/
        if (( pipeout_fp = popen("sort", "w")) == NULL)
        {
                perror("popen");
                exit(1);
        }

        /*반복 처리*/
        while(fgets(readbuf, 80, pipein_fp))
                fputs(readbuf, pipeout_fp);

        /*파이프를 닫는다*/
        pclose(pipein_fp);
        pclose(pipeout_fp);

        return(0);
}
