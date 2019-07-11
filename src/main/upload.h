/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/
#ifndef UPLOAD_H
#define UPLOAD_H 1

#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gapper_fragment {
    STAILQ_ENTRY(gapper_fragment) nodes;
    char* payload;
    size_t length;
} gapper_fragment_t;

typedef STAILQ_HEAD(gapper_fragment_head_s, gapper_fragment) gapper_fragment_head_t;

extern void upload_file(const char* type, const uint8_t* data, size_t length, const char* suffix);
extern void upload_file_scattered(const char* type, gapper_fragment_head_t* head, const char* suffix);

#ifdef __cplusplus
}
#endif

#endif
