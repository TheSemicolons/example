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

#include <sys/types.h>

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

struct progressData {
	const char	*host;
	const char	*file;
	const char	*url;
	int		 longest;
} data;

static unsigned int	 longestString(const char **, int);
static unsigned int	 progressBar(void *, curl_off_t, curl_off_t, curl_off_t, curl_off_t);
static size_t		 writeData(void *, size_t, size_t, FILE *);

int
main(int argc, char *argv[])
{
	CURL		*curl;
	FILE		*fp = NULL;
	unsigned int	 i = 0;
	unsigned int	 longest = 0;
	const char	*host = "https://aviationweather.gov/adds/dataserver_current/current/";
	const char	*extension = ".cache.csv";
	const char	*fileList[5];

	fileList[0] = "aircraftreports";
	fileList[1] = "airsigmets";
	fileList[2] = "metars";
	fileList[3] = "pireps";
	fileList[4] = "tafs";

	longest = longestString(fileList, sizeof(fileList) / sizeof(*fileList));

	system("rm -Rf /tmp/weather");
	system("mkdir -m 755 /tmp/weather");

	fprintf(stdout, "%s\n", "Downloading files:");

	for(i = 0; i < (sizeof(fileList) / sizeof(*fileList)); i++) {
		char	 url[255];
		char	 outFile[255];

		snprintf(url, 255,"%s%s%s", host, fileList[i],  extension);
		snprintf(outFile, 255,"%s%s%s", "/tmp/weather/", fileList[i], extension);

		curl = curl_easy_init();

		if (curl) {
			CURLcode res = CURLE_OK;

			data.host = host;
			data.file = fileList[i];
			data.url = url;
			data.longest = longest;

			curl_easy_setopt(curl, CURLOPT_URL, data.url);
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				fprintf(stdout, "\r%-*s", data.longest, "Download failed.");
			} else {
				fp = fopen(outFile, "wb");

				curl_easy_setopt(curl, CURLOPT_URL, data.url);
				curl_easy_setopt(curl, CURLOPT_NOBODY, 0);
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
				curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressBar);
				curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &data);
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

		fprintf(stdout, "%s", "\n");
	}

/*
	fprintf(stdout, "%s", "\n");

	fprintf(stdout, "%s\n", "Unzipping files:");

	for(i = 0; i < (sizeof(fileList) / sizeof(*fileList)); i++) {
		char	 outFile[255];
		char	 gunzipCmd[255];

		snprintf(gunzipCmd, 255,"%s%s%s%s", "gunzip ","/tmp/weather/", fileList[i], extension);

		system(gunzipCmd);
	}
*/

	fprintf(stdout, "\n%s\n", "Done.");

	return 0;
}

static unsigned int
longestString(const char **lines, int arrayLength)
{
	int	i = 0;
	int	longest = 0;

	for (i = 0; i < arrayLength; i++) {
	    if (strlen(lines[i]) > longest) {
	    	longest = strlen(lines[i]);
	    }
	}

	return longest;
}

static unsigned int
progressBar(void *ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded)
{
	struct		 progressData *pData = (struct progressData*)ptr;
	int		 totalDots = 25;
	int		 ii = 0;

	double		 fractionDownloaded = (double)nowDownloaded / (double)totalToDownload;
	double		 dots = round(fractionDownloaded * totalDots);

	if (totalToDownload <= 0.0) {
		return 0;
	}

	fprintf(stdout, "\r%-*s [", pData->longest, pData->file);

	while (ii < dots) {
	    fprintf(stdout, "%s", "#");

	    ii++;
	}

	while (ii < totalDots) {
	    fprintf(stdout, "%s", "-");

	    ii++;
	}

	fprintf(stdout, "] %3d%%", ((int)(fractionDownloaded * 100)));

	fflush(stdout);

	return 0;
}

static size_t
writeData(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);

	return written;
}
