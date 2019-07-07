/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "main.h"
#include <esp_log.h>

#include <curl/curl.h>

struct WriteThis {
  const uint8_t *readptr;
  size_t sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *upload = (struct WriteThis *)userp;
  size_t max = size*nmemb;

  if(max < 1)
    return 0;

  if(upload->sizeleft) {
    size_t copylen = max;
    if(copylen > upload->sizeleft)
      copylen = upload->sizeleft;
    memcpy(ptr, upload->readptr, copylen);
    upload->readptr += copylen;
    upload->sizeleft -= copylen;
    return copylen;
  }

  return 0;                          /* no more data left to deliver */
}

void upload_file(const char* type, const uint8_t* data, size_t length, const char* suffix)
{
    CURLcode res;
    struct WriteThis upload;

    time_t now;
    struct tm timeinfo;
    time(&now);
    gmtime_r(&now, &timeinfo);

    char ftp_dirname[12];
    char ftp_filename[18];
    char ftp_url[96];

    strftime(ftp_dirname, sizeof(ftp_dirname), "%Y%m%d", &timeinfo);
    strftime(ftp_filename, sizeof(ftp_filename), "%Y%m%d%H%M%S", &timeinfo);
    snprintf(ftp_url, sizeof(ftp_url), "%s/%s/%s/%s.%s", CONFIG_FTP_BASE_URL, type, ftp_dirname, ftp_filename, suffix);

    if (strlen(CONFIG_FTP_BASE_URL) == 0)
    {
        ESP_LOGI(TAG, "Simulate FTP upload to ~%s.", ftp_url);
        return;
    }

    upload.readptr = data;
    upload.sizeleft = length;

    /* First set the URL, the target file */
    curl_easy_setopt(s_curl, CURLOPT_URL, ftp_url);

    /* User and password for the FTP login */
    curl_easy_setopt(s_curl, CURLOPT_USERPWD, CONFIG_FTP_USERPASSWORD);

    /* Binary transfer mode is the default */
    /* Now specify we want to UPLOAD data */
    curl_easy_setopt(s_curl, CURLOPT_UPLOAD, 1L);

    /* we want to use our own read function */
    curl_easy_setopt(s_curl, CURLOPT_READFUNCTION, read_callback);

    /* pointer to pass to our read function */
    curl_easy_setopt(s_curl, CURLOPT_READDATA, &upload);

    /* get verbose debug output please */
    curl_easy_setopt(s_curl, CURLOPT_VERBOSE, 1L);

    /* create missing directories */
    curl_easy_setopt(s_curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR);

    /* Set the expected upload size. */
    curl_easy_setopt(s_curl, CURLOPT_INFILESIZE_LARGE,
                        (curl_off_t)upload.sizeleft);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(s_curl);
    /* Check for errors */
    if(res != CURLE_OK)
    {
        ESP_LOGE(TAG, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
}
