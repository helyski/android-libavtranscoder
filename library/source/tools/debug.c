#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "debug.h"
#include <dirent.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>





void print_logcat(int log_priority, const char *format,...) {

        va_list args;
        va_start(args, format);

        char log_string[1024];
        vsnprintf(log_string, sizeof(log_string)-1, format, args);
        strcat(log_string,"\n");


        __android_log_print(log_priority, LOG_TAG, "%s", log_string);

        va_end(args);
}



