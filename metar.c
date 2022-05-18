/*
 * Copyright (c) 2022 James Anderson <thesemicolons@protonmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * struct stat
 * mkdir()
 * stat()
 */
#include <sys/stat.h>

#include <sys/types.h>

#include <limits.h>

/*
 * const DT_*
 * struct DIR
 * struct dirent
 * closedir()
 * opendir()
 * readdir()
 */
#include <dirent.h>

/*
 * NOTE: This must come before kcgi.h.
 *
 * va_list
 */
#include <stdarg.h>

/*
 * Functions for CGI.
 */
#include <kcgi.h>

/*
 * const std*
 * FILE
 */
#include <stdio.h>

/*
 * exit()
 */
#include <stdlib.h>

/*
 * strncmp()
 * strsep()
 */
#include <string.h>

/*
 * time()
 */
#include <time.h>

/*
 * unlink()
 */
#include <unistd.h>

#include <curl/curl.h>

struct infoStruct {
	char	 basePath[PATH_MAX];
	char	 baseUrl[PATH_MAX];
	char	 path[PATH_MAX];
	char	 url[PATH_MAX];
	int	 fileItemCount;
	char	 fileList[255][255];
	int	 downloadItemCount;
	char	 downloadList[255][255];
};

void		 checkTime(struct infoStruct*);
void		 downloadMetar(struct infoStruct*);
void		 emptyDir(struct infoStruct*);
void		 parseMetar(struct infoStruct*);
void		 writeOutputHead(char*);
void		 writeOutputFoot(char*);
static size_t	 writeData(void *, size_t, size_t, FILE *);

int
main(int argc, char *argv[])
{
	struct infoStruct info;

	info.fileItemCount = 5;
	info.downloadItemCount = 0;

	(void) strcpy(info.basePath, "/tmp/weather/");
	(void) strcpy(info.baseUrl, "https://aviationweather.gov/adds/dataserver_current/current/");

	(void) strcpy(info.fileList[0], "aircraftreports.cache.csv");
	(void) strcpy(info.fileList[1], "airsigmets.cache.csv");
	(void) strcpy(info.fileList[2], "metars.cache.csv");
	(void) strcpy(info.fileList[3], "pireps.cache.csv");
	(void) strcpy(info.fileList[4], "tafs.cache.csv");

	checkTime(&info);

	if (info.downloadItemCount > 0)
	{
		emptyDir(&info);
		downloadMetar(&info);
	}

	parseMetar(&info);

	return 0;
}

void
checkTime(struct infoStruct *info)
{
	struct stat	 fileInfo;
	int		 i = 0;

	for (i = 0; i < info->fileItemCount; i++) {
		(void) strncpy(info->path, "", 2);
		(void) strncat(info->path, info->basePath, strlen(info->basePath));
		(void) strncat(info->path, info->fileList[i], strlen(info->fileList[i]));

		(void) stat(info->path, &fileInfo);

		if (((unsigned long)time(NULL) - fileInfo.st_mtime) >= 300) {
			(void) strcpy(info->downloadList[info->downloadItemCount], info->fileList[i]);
			info->downloadItemCount++;
		}
	}

	return;
}

void
downloadMetar(struct infoStruct *info)
{
	CURL	*curl;
	FILE	*fp = NULL;
	int	 i = 0;

	for (i = 0; i < info->downloadItemCount; i++) {
		(void) strncpy(info->url, "", 2);
		(void) strncat(info->url, info->baseUrl, strlen(info->baseUrl));
		(void) strncat(info->url, info->downloadList[i], strlen(info->downloadList[i]));

		(void) strncpy(info->path, "", 2);
		(void) strncat(info->path, info->basePath, strlen(info->basePath));
		(void) strncat(info->path, info->downloadList[i], strlen(info->downloadList[i]));

		curl = curl_easy_init();

		if (curl) {
			CURLcode res = CURLE_OK;

			curl_easy_setopt(curl, CURLOPT_URL, info->url);
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				(void) fprintf(stderr, "%s\n", "Download failed.");

				exit(1);
			} else {
				fp = fopen(info->path, "w");

				curl_easy_setopt(curl, CURLOPT_URL, info->url);
				curl_easy_setopt(curl, CURLOPT_NOBODY, 0);
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
				curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

				res = curl_easy_perform(curl);
			}

			curl_easy_cleanup(curl);

			if (fp != NULL) {
				fclose(fp);
			}
		}
	}

	return;
}

void
emptyDir(struct infoStruct *info)
{
	DIR		*directory;
	struct dirent	*ent = NULL;
	int		 i = 0;
	int		 ret = 0;

	for (i = 0; i < info->downloadItemCount; i++) {
		if ((directory = opendir(info->basePath)) != NULL) {
			while ((ent = readdir(directory)) != NULL) {
				if (strncmp(ent->d_name, info->downloadList[i], sizeof(ent->d_name)) == 0) {
					(void) strncpy(info->path, "", 1);
					(void) strncat(info->path, info->basePath, 15);
					(void) strncat(info->path, info->downloadList[i], strlen(info->downloadList[i]));
					(void) unlink(info->path);
				}
			}
		} else {
			ret = mkdir(info->basePath, 0700);

			if (ret != 0)
			{
				(void) fprintf(stderr, "Error accessing or creating directory: %s\n", info->basePath);

				exit(1);
			}
		}
	}

	return;
}

void
parseMetar(struct infoStruct *info)
{
	FILE	*fp;
	char	*line = NULL;
	size_t	 len = 0;
	int	 total = 0;
	ssize_t	 read;
	int	 i = 0;
	char	*token;

	for (i = 0; i < info->fileItemCount; i++) {
		if (strncmp(info->fileList[i], "metars", 6) == 0) {
			(void) strncpy(info->path, "", 2);
			(void) strncat(info->path, info->basePath, strlen(info->basePath));
			(void) strncat(info->path, "metars.cache.csv", strlen("metars.cache.csv"));

			fp = fopen(info->path, "r");

			if (fp == NULL) {
				return;
			}

			while ((read = getline(&line, &len, fp)) != -1)
			{
				if(total < 6) {
					total++;

					continue;
				}

				if (total == 6) {
					while ((token = strsep(&line, ",")) != NULL) {
						(void) fprintf(stdout, "%s\n", token);
					}
				}

				total++;
			}
		}
	}

	return;
}

void
writeOutputHead(char *station)
{
	return;
}

void
writeOutputFoot(char *station)
{
	return;
}

static size_t
writeData(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);

	return written;
}
