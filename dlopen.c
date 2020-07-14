#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

//#pragma comment(lib, "NAME.lib")


int main(int argc, char **argv)
{
    char buf[1024], buf2[1024];
    char *error;
    char * (*ptr_strcat)(char *, size_t, const char *) = NULL;

    void *handle = dlopen ("libmyutil.so", RTLD_LAZY);

    if(handle == NULL)
    {
        printf("%s\n", dlerror());
        return 1;
    }

    ptr_strcat = dlsym(handle, "util_strcat");
    if((error = dlerror()) != NULL)
    {
        dlclose(handle);

        return 1;
    }

    strcpy(buf, "1234567890");
    strcpy(buf2, "ABCD");
    ptr_strcat(buf, sizeof(buf), buf2);

    printf("%s\n", buf);
    dlclose(handle);

    return 0;
}


