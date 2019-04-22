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

#ifdef __cplusplus
extern "C" {
#endif

extern void upload_file(const char* type, const uint8_t* data, size_t length, const char* suffix);

#ifdef __cplusplus
}
#endif

#endif
