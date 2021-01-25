#ifndef HTTP_H_
#define HTTP_H_

#include <string>

void HttpInit();

void HttpCleanup();

bool HttpSyncDownload(const std::string &url, const std::string &filePath, bool needPrint);

// fork from ffmpeg 4.2
void ff_make_absolute_url(char *buf, int size, const char *base, const char *rel);

void GetRelativePath(const std::string &url, std::string &filePath, std::string &dirPath);

#endif // !HTTP_H_