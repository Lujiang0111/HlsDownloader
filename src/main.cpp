#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include "PlatformFile.h"
#include "http.h"

constexpr auto MAX_URL_SIZE = 4096;

struct HlsContext
{
	std::string url;
	std::vector<HlsContext *> vecVariant;
	std::deque<std::string> deqSegment;
	char buf[MAX_URL_SIZE];
};

static bool bAppStart = true;
static void SigIntHandler(int sig_num)
{
	signal(SIGINT, SigIntHandler);
	bAppStart = false;
}

static void ClearHlsContext(HlsContext *h)
{
	for (auto x = h->vecVariant.begin(); x != h->vecVariant.end(); ++x)
	{
		ClearHlsContext(*x);
	}
	delete h;
}

static bool CharIsSpace(const char c)
{
	return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

static bool StringIsStartWith(const std::string str, const std::string pre)
{
	for (size_t i = 0; pre[i]; ++i)
	{
		if (pre[i] != str[i])
		{
			return false;
		}
	}
	return true;
}

static void DownloadHls(HlsContext *h)
{
	std::string filePath, dirPath;
	GetRelativePath(h->url, filePath, dirPath);
	PFMakeDirectory(dirPath.c_str());
	HttpSyncDownload(h->url, filePath, false);

	FILE *fp = fopen(filePath.c_str(), "r");
	if (!fp)
	{
		std::cout << "Could not open file " << filePath << std::endl;
		return;
	}

	bool isVariant = false;
	int variantCnt = 0;
	bool isSegment = false;
	int segmentCnt = 0;
	while ((bAppStart) && (!feof(fp)))
	{
		fgets(h->buf, MAX_URL_SIZE, fp);
		std::string line = h->buf;
		if (CharIsSpace(line[line.length() - 1]))
		{
			line.pop_back();
		}

		if (StringIsStartWith(line, "#EXT-X-STREAM-INF:"))
		{
			isVariant = true;
		}
		else if (StringIsStartWith(line, "#EXTINF:"))
		{
			isSegment = true;
		}
		else if (StringIsStartWith(line, "#EXT-X-ENDLIST"))
		{
			std::cout << "Hls Stream meet end!";
		}
		else if (StringIsStartWith(line, "#"))
		{

		}
		else
		{
			if (isVariant)
			{
				++variantCnt;
				isVariant = false;

				if (h->vecVariant.size() < variantCnt)
				{
					HlsContext *node = new HlsContext();
					h->vecVariant.push_back(node);
				}

				auto node = h->vecVariant[variantCnt - 1];
				ff_make_absolute_url(h->buf, MAX_URL_SIZE, h->url.c_str(), line.c_str());
				node->url = h->buf;
				DownloadHls(node);
			}
			else if (isSegment)
			{
				++segmentCnt;
				isSegment = false;

				ff_make_absolute_url(h->buf, MAX_URL_SIZE, h->url.c_str(), line.c_str());

				std::string segUrl = h->buf;
				GetRelativePath(segUrl, filePath, dirPath);

				if (1 == segmentCnt)
				{
					while ((h->deqSegment.size() > 0) && (h->deqSegment.front() != filePath))
					{
						h->deqSegment.pop_front();
					}
				}

				bool newSegment = true;
				for (size_t i = 0; i < h->deqSegment.size(); ++i)
				{
					if (h->deqSegment[i] == filePath)
					{
						newSegment = false;
						break;
					}
				}

				if (newSegment)
				{
					h->deqSegment.emplace_back(filePath);
					PFMakeDirectory(dirPath.c_str());
					HttpSyncDownload(segUrl, filePath, true);
				}

			}
		}
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT, SigIntHandler);

	HttpInit();

	std::string sMasterUrl;
	if (argc >= 2)
	{
		sMasterUrl = argv[1];
	}
	else
	{
		std::cout << "Input hls url:";
		std::cin >> sMasterUrl;
	}

	HlsContext *h = new HlsContext();
	ff_make_absolute_url(h->buf, MAX_URL_SIZE, NULL, sMasterUrl.c_str());
	h->url = h->buf;
	std::string filePath, dirPath;
	GetRelativePath(h->url, filePath, dirPath);
	PFRemoveFile(dirPath.c_str());

	std::chrono::time_point<std::chrono::steady_clock> nowTime = std::chrono::steady_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> nextTime = nowTime;
	while (bAppStart)
	{
		nowTime = std::chrono::steady_clock::now();
		if (nextTime > nowTime)
		{
			std::this_thread::sleep_for(nextTime - nowTime);
		}
		else
		{
			nextTime = nowTime;
		}

		DownloadHls(h);

		nextTime += std::chrono::milliseconds(1000);
	}

	ClearHlsContext(h);

	HttpCleanup();
	return 0;
}