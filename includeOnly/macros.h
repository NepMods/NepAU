//
// Created by arjun on 7/25/25.
//
// Created by arjun on 7/25/25.
//

#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dlfcn.h"
inline void* custom_dlopen(const char* lib, int flag) {
    return dlopen(lib, flag);
}
inline void* custom_dlsym(void* lib, const char *sym) {
    return dlsym(lib, sym);
}


uintptr_t getBase(const char *library) {
    char filename[0xFF] = {0},
            buffer[1024] = {0};
    FILE *fp = NULL;
    uintptr_t address = 0;

    sprintf(filename, "/proc/self/maps");
    fp = fopen(filename, "rt");
    if (fp == NULL) {
        perror("fopen");
        goto done;
    }
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, library)) {
            address = (uintptr_t) strtoul(buffer, NULL, 16);
            goto done;
        }
    }
    done:
    if (fp) {
        fclose(fp);
    }
    return address;
}

#endif //MACROS_H
