//
// Created by Tank on 2022/10/24.
//

#ifndef ANDROID_LIBTRANSCODE_FILETOOL_H
#define ANDROID_LIBTRANSCODE_FILETOOL_H

#include "string.h"
#include "stdio.h"
#include "debug.h"
#include <sys/stat.h>

FILE * open_file_c(const char * full_path);
const char* get_legal_file_name(const char * path);

#endif //ANDROID_LIBTRANSCODE_FILETOOL_H
