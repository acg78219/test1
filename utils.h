#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <curl/curl.h>
#include <curl/easy.h>
#include <stdlib.h>
#include <string.h>
#include <string>

// 自定义数据结构接受 post 返回的数据
struct postData {
    char* memory;
    size_t size;
    postData() {
        memory = (char*)malloc(1);
        size = 0;
    }
    ~postData() {
        free(memory);
        memory = NULL;
        size = 0;
    }

    CURLcode res;
    long code;
};

//size_t WriteMemoryCallback(void* ptr, size_t size, size_t nmenb, void* data) {
//    size_t realsize = size * nmenb;
//    struct postData* mem = (struct postData*)data;
//
//    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
//    if (mem->memory) {
//        memcpy(&(mem->memory[mem->size]), ptr, realsize);
//        mem->size += realsize;
//        mem->memory[mem->size] = 0;
//    }
//    return realsize;
//}
#endif
