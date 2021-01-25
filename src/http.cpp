#include <string.h>
#include <vector>
#include <curl/curl.h>
#include "http.h"

static std::string progressBar[101];
static void ProgressBarInit()
{
	for (int i = 0; i <= 100; ++i)
	{
		progressBar[i] = "[";
		int j = 0;
		for (j = 0; j < i / 2; ++j)
		{
			progressBar[i] += "=";
		}
		for (; j < 50; ++j)
		{
			progressBar[i] += " ";
		}
		progressBar[i] += "][";
		progressBar[i] += std::to_string(i);
		progressBar[i] += "%]";
	}
}

static int HttpSyncDownloadProgressCb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	int rate = (dltotal > 0) ? static_cast<int>(dlnow / dltotal * 100) : 0;
	if (rate < 0)
	{
		rate = 0;
	}
	if (rate > 100)
	{
		rate = 100;
	}

	printf("%s\r", progressBar[rate].c_str());
	return CURLE_OK;
}

void HttpInit()
{
	ProgressBarInit();
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

void HttpCleanup()
{
	curl_global_cleanup();
}

static size_t HttpSyncDownloadCb(char *buffer, size_t size, size_t nitems, void *outstream)
{
	FILE *fp = static_cast<FILE *>(outstream);
	return fwrite(buffer, size, nitems, fp);
}

bool HttpSyncDownload(const std::string &url, const std::string &filePath, bool needPrint)
{
	CURL *curlHdl = curl_easy_init();
	if (!curlHdl)
	{
		return false;
	}

	FILE *fp = fopen(filePath.c_str(), "wb");
	if (!fp)
	{
		curl_easy_cleanup(curlHdl);
		return false;
	}

	curl_easy_setopt(curlHdl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curlHdl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curlHdl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curlHdl, CURLOPT_WRITEFUNCTION, HttpSyncDownloadCb);
	curl_easy_setopt(curlHdl, CURLOPT_WRITEDATA, fp);

	if (needPrint)
	{
		curl_easy_setopt(curlHdl, CURLOPT_PROGRESSFUNCTION, HttpSyncDownloadProgressCb);
		curl_easy_setopt(curlHdl, CURLOPT_PROGRESSDATA, NULL);
		curl_easy_setopt(curlHdl, CURLOPT_NOPROGRESS, 0L);
	}

	if (needPrint)
	{
		printf("Downloading %s to %s\n", url.c_str(), filePath.c_str());
	}

	CURLcode retCode = curl_easy_perform(curlHdl);
	if (needPrint)
	{
		printf("\n");
	}

	fclose(fp);
	return true;
}

// fork from ffmpeg 4.2
static size_t av_strlcpy(char *dst, const char *src, size_t size)
{
	size_t len = 0;
	while (++len < size && *src)
		*dst++ = *src++;
	if (len <= size)
		*dst = 0;
	return len + strlen(src) - 1;
}

// fork from ffmpeg 4.2
static size_t av_strlcat(char *dst, const char *src, size_t size)
{
	size_t len = strlen(dst);
	if (size <= len + 1)
		return len + strlen(src);
	return len + av_strlcpy(dst + len, src, size - len);
}

// fork from ffmpeg 4.2
static int av_strstart(const char *str, const char *pfx, const char **ptr)
{
	while (*pfx && *pfx == *str) {
		pfx++;
		str++;
	}
	if (!*pfx && ptr)
		*ptr = str;
	return !*pfx;
}

// fork from ffmpeg 4.2
void ff_make_absolute_url(char *buf, int size, const char *base, const char *rel)
{
	char *sep, *path_query;
	/* Absolute path, relative to the current server */
	if (base && strstr(base, "://") && rel[0] == '/')
	{
		if (base != buf)
			av_strlcpy(buf, base, size);
		sep = strstr(buf, "://");
		if (sep) {
			/* Take scheme from base url */
			if (rel[1] == '/') {
				sep[1] = '\0';
			}
			else {
				/* Take scheme and host from base url */
				sep += 3;
				sep = strchr(sep, '/');
				if (sep)
					*sep = '\0';
			}
		}
		av_strlcat(buf, rel, size);
		return;
	}

	/* If rel actually is an absolute url, just copy it */
	if (!base || strstr(rel, "://") || rel[0] == '/') {
		av_strlcpy(buf, rel, size);
		return;
	}
	if (base != buf)
		av_strlcpy(buf, base, size);

	/* Strip off any query string from base */
	path_query = strchr(buf, '?');
	if (path_query)
		*path_query = '\0';

	/* Is relative path just a new query part? */
	if (rel[0] == '?') {
		av_strlcat(buf, rel, size);
		return;
	}

	/* Remove the file name from the base url */
	sep = strrchr(buf, '/');
	if (sep)
		sep[1] = '\0';
	else
		buf[0] = '\0';
	while (av_strstart(rel, "../", NULL) && sep) {
		/* Remove the path delimiter at the end */
		sep[0] = '\0';
		sep = strrchr(buf, '/');
		/* If the next directory name to pop off is "..", break here */
		if (!strcmp(sep ? &sep[1] : buf, "..")) {
			/* Readd the slash we just removed */
			av_strlcat(buf, "/", size);
			break;
		}
		/* Cut off the directory name */
		if (sep)
			sep[1] = '\0';
		else
			buf[0] = '\0';
		rel += 3;
	}
	av_strlcat(buf, rel, size);
}

void GetRelativePath(const std::string &url, std::string &filePath, std::string &dirPath)
{
	auto sep = url.find_first_of("://");
	filePath = url;
	do
	{
		if (std::string::npos == sep)
		{
			break;
		}

		sep = url.find_first_of('/', sep + 3);
		if (std::string::npos == sep)
		{
			break;
		}

		filePath = &url.c_str()[sep + 1];
		sep = filePath.find_first_of('?');
		if (std::string::npos != sep)
		{
			filePath.erase(sep);
		}
	} while (0);

	dirPath = filePath;
	sep = dirPath.find_last_of('/');
	if (std::string::npos == sep)
	{
		dirPath = "";
	}
	else
	{
		dirPath.erase(sep);
	}
}