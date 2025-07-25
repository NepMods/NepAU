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

uintptr_t getBase(const char *library);

#endif //MACROS_H
