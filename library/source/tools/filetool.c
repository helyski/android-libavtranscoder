//
// Created by Tank on 2022/10/24.
//

#include "filetool.h"

FILE* open_file_c(const char * full_path){
    if(!full_path || strlen(full_path) <=0)
        return NULL;
    FILE* output;
    int i,str_bf_len = 256;
    char str[str_bf_len];

    // remove file first.
    if(access(full_path,F_OK)){
        remove(full_path);
    }

    // get file name in a path.
    const char* file = get_legal_file_name(full_path);
    if(!file){
        return NULL;
    }

    // check dir , if dir does not exist build it.
    strncpy(str,full_path,str_bf_len);
    uint32_t len = strlen(str);
    for (i = 0; i < len; i++) {
        if (str[i] == '/') {
            str[i] = '\0';
            if (access(str, 0) != 0) {
                mkdir(str, 0777);
            }
            str[i] = '/';
        }
    }

    end:
    output = fopen(full_path,"wb+");
    return output;
}

/**
 * Get
 * @param path
 * @return
 */
const char *get_legal_file_name(const char *path) {
    if (!path || strlen(path) <= 0)
        return NULL;

    LOGW("filetool get_file_name path:%s, len:%d", path, strlen(path));

    int i;
    uint32_t len = strlen(path);
    int last_index_of_dir_div = 0;

    // get last index of "/".
    for (i = 0; i < len; i++) {
        if (path[i] == '/') {
            last_index_of_dir_div = i;
        }
    }

    // get the path from last "/" to end.
    uint32_t last_div_to_end_len = len - 1 - last_index_of_dir_div;
    char last_name[128];
    for (i = 0; i < last_div_to_end_len; i++) {
        last_name[i] = path[last_index_of_dir_div + i + 1];
        if (last_name[i] == '\0') {
            break;
        }
    }
    LOGW("filetool get_file_name last_name:%s", last_name);
    if (strlen(last_name) <= 0) {
        return NULL;
    }

    return last_name;
}