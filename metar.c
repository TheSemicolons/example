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
	char	 outputBasePath[PATH_MAX];
	char	 outputPath[PATH_MAX];
	char	 path[PATH_MAX];
	char	 url[PATH_MAX];
	int	 fileItemCount;
	char	 fileList[255][255];
	int	 downloadItemCount;
	char	 downloadList[255][255];
};

struct metarLine {
	char	 raw_text[PATH_MAX];
	char	 station_id[PATH_MAX];
	char	 observation_time[PATH_MAX];
	char	 latitude[PATH_MAX];
	char	 longitude[PATH_MAX];
	char	 temp_c[PATH_MAX];
	char	 dewpoint_c[PATH_MAX];
	char	 wind_dir_degrees[PATH_MAX];
	char	 wind_speed_kt[PATH_MAX];
	char	 wind_gust_kt[PATH_MAX];
	char	 visibility_statute_mi[PATH_MAX];
	char	 altim_in_hg[PATH_MAX];
	char	 sea_level_pressure_mb[PATH_MAX];
	char	 corrected[PATH_MAX];
	char	 is_auto[PATH_MAX];
	char	 auto_station[PATH_MAX];
	char	 maintenance_indicator_on[PATH_MAX];
	char	 no_signal[PATH_MAX];
	char	 lightning_sensor_off[PATH_MAX];
	char	 freezing_rain_sensor_off[PATH_MAX];
	char	 present_weather_sensor_off[PATH_MAX];
	char	 wx_string[PATH_MAX];
	char	 sky_cover_0[PATH_MAX];
	char	 cloud_base_ft_agl_0[PATH_MAX];
	char	 sky_cover_1[PATH_MAX];
	char	 cloud_base_ft_agl_1[PATH_MAX];
	char	 sky_cover_2[PATH_MAX];
	char	 cloud_base_ft_agl_2[PATH_MAX];
	char	 sky_cover_3[PATH_MAX];
	char	 cloud_base_ft_agl_3[PATH_MAX];
	char	 flight_category[PATH_MAX];
	char	 three_hr_pressure_tendency_mb[PATH_MAX];
	char	 maxT_c[PATH_MAX];
	char	 minT_c[PATH_MAX];
	char	 maxT24hr_c[PATH_MAX];
	char	 minT24hr_c[PATH_MAX];
	char	 precip_in[PATH_MAX];
	char	 pcp3hr_in[PATH_MAX];
	char	 pcp6hr_in[PATH_MAX];
	char	 pcp24hr_in[PATH_MAX];
	char	 snow_in[PATH_MAX];
	char	 vert_vis_ft[PATH_MAX];
	char	 metar_type[PATH_MAX];
	char	 elevation_m[PATH_MAX];
};

void		 checkTime(struct infoStruct*);
void		 downloadMetar(struct infoStruct*);
void		 emptyDir(struct infoStruct*);
void		 parseMetar(struct infoStruct*);
void		 writeMetar(struct infoStruct*, char*);
static size_t	 writeData(void *, size_t, size_t, FILE *);

int
main(int argc, char *argv[])
{
	struct infoStruct info;

	info.fileItemCount = 5;
	info.downloadItemCount = 0;

	(void) strcpy(info.basePath, "/tmp/weather/");
	(void) strcpy(info.outputBasePath, "/home/www/htdocs/mimas.dev/weather/");
	(void) strcpy(info.baseUrl, "https://aviationweather.gov/adds/dataserver_current/current/");

	(void) strcpy(info.fileList[0], "aircraftreports.cache.csv");
	(void) strcpy(info.fileList[1], "airsigmets.cache.csv");
	(void) strcpy(info.fileList[2], "metars.cache.csv");
	(void) strcpy(info.fileList[3], "pireps.cache.csv");
	(void) strcpy(info.fileList[4], "tafs.cache.csv");

	checkTime(&info);

	if (info.downloadItemCount > 0)
	{
/*
 * Commented out because there has to be a better way to do this.
 * 
 * There is a possibility that the webpage will request a file in
 * here in the short time between when it is removed and the new
 * file is created. This would be bad.
 */
		emptyDir(&info);
		downloadMetar(&info);
		parseMetar(&info);
	}

	return 0;
}

void
checkTime(struct infoStruct *info)
{
	struct stat	 fileInfo;
	int		 ret = 0;
	int		 i = 0;

	for (i = 0; i < info->fileItemCount; i++) {
		(void) strlcpy(info->path, info->basePath, strlen(info->basePath) + 1);
		(void) strncat(info->path, info->fileList[i], strlen(info->fileList[i]));

		ret = stat(info->path, &fileInfo);

		if (ret == -1) {
			(void) strlcpy(info->downloadList[info->downloadItemCount], info->fileList[i], strlen(info->fileList[i]) + 1);

			info->downloadItemCount++;
		}

		if ((ret == 0) && (((unsigned long)time(NULL) - fileInfo.st_mtime) >= 900)) {
			(void) strlcpy(info->downloadList[info->downloadItemCount], info->fileList[i], strlen(info->fileList[i]) + 1);

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
		(void) strlcpy(info->url, info->baseUrl, strlen(info->baseUrl) + 1);
		(void) strncat(info->url, info->downloadList[i], strlen(info->downloadList[i]));

		(void) strlcpy(info->path, info->basePath, strlen(info->basePath) + 1);
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
				if ((strncmp(ent->d_name, ".", 1) == 0) || (strncmp(ent->d_name, "..", 2) == 0)) {
					continue;
				}

/*
 * Commented out because we don't really need to delete things.
 */
/*
				if (strncmp(ent->d_name, info->downloadList[i], sizeof(ent->d_name)) == 0) {
					(void) strlcpy(info->path, info->basePath, strlen(info->basePath) + 1);
					(void) strncat(info->path, info->downloadList[i], strlen(info->downloadList[i]));
					(void) unlink(info->path);
				}
*/
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
	DIR		*directory;
	FILE		*fp;
	struct dirent	*ent = NULL;
	char		*line = NULL;
	int		 i = 0;
	int		 total = 0;
	int		 y = 0;
	size_t		 len = 0;
	ssize_t		 read;

	if ((directory = opendir(info->outputBasePath)) == NULL) {
		if (mkdir(info->outputBasePath, 0700) != 0) {
			(void) fprintf(stderr, "Output directory does not exist: %s\n", info->outputBasePath);

			exit(1);
		}
	}

	if ((directory = opendir(info->outputBasePath)) != NULL) {
		while ((ent = readdir(directory)) != NULL) {
			if ((strncmp(ent->d_name, ".", 1) == 0) || (strncmp(ent->d_name, "..", 2) == 0)) {
				continue;
			}

			(void) strlcpy(info->path, info->outputBasePath, strlen(info->outputBasePath) + 1);
			(void) strncat(info->path, ent->d_name, strlen(ent->d_name));
			(void) unlink(info->path);
		}
	}

	for (i = 0; i < info->fileItemCount; i++) {
		if (strncmp(info->fileList[i], "metars", 6) == 0) {
			(void) strlcpy(info->path, info->basePath, strlen(info->basePath) + 1);
			(void) strncat(info->path, info->fileList[i], strlen(info->fileList[i]));

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

				if (total >= 6) {
					writeMetar(info, line);
				}

				total++;
			}
		}
	}

	return;
}

void
writeMetar(struct infoStruct *info, char *line)
{
	FILE			*ofp;
	char			*token;
	int			 x = 0;

	struct metarLine *metar = malloc(sizeof(struct metarLine));

	while ((token = strsep(&line, ",")) != NULL) {
		switch (x) {
		case 0:
			(void) strncpy(metar->raw_text, token, strlen(token) + 1);
			break;
		case 1:
			(void) strncpy(metar->station_id, token, strlen(token) + 1);
			break;
		case 2:
			(void) strncpy(metar->observation_time, token, strlen(token) + 1);
			metar->observation_time[10] = ' ';
			break;
		case 3:
			(void) strncpy(metar->latitude, token, strlen(token) + 1);
			break;
		case 4:
			(void) strncpy(metar->longitude, token, strlen(token) + 1);
			break;
		case 5:
			(void) strncpy(metar->temp_c, token, strlen(token) + 1);
			break;
		case 6:
			(void) strncpy(metar->dewpoint_c, token, strlen(token) + 1);
			break;
		case 7:
			(void) strncpy(metar->wind_dir_degrees, token, strlen(token) + 1);
			break;
		case 8:
			(void) strncpy(metar->wind_speed_kt, token, strlen(token) + 1);
			break;
		case 9:
			(void) strncpy(metar->wind_gust_kt, token, strlen(token) + 1);
			break;
		case 10:
			(void) strncpy(metar->visibility_statute_mi, token, strlen(token) + 1);
			break;
		case 11:
			(void) strncpy(metar->altim_in_hg, token, strlen(token) + 1);
			break;
		case 12:
			(void) strncpy(metar->sea_level_pressure_mb, token, strlen(token) + 1);
			break;
		case 13:
			(void) strncpy(metar->corrected, token, strlen(token) + 1);
			break;
		case 14:
			(void) strncpy(metar->is_auto, token, strlen(token) + 1);
			break;
		case 15:
			(void) strncpy(metar->auto_station, token, strlen(token) + 1);
			break;
		case 16:
			(void) strncpy(metar->maintenance_indicator_on, token, strlen(token) + 1);
			break;
		case 17:
			(void) strncpy(metar->no_signal, token, strlen(token) + 1);
			break;
		case 18:
			(void) strncpy(metar->lightning_sensor_off, token, strlen(token) + 1);
			break;
		case 19:
			(void) strncpy(metar->freezing_rain_sensor_off, token, strlen(token) + 1);
			break;
		case 20:
			(void) strncpy(metar->present_weather_sensor_off, token, strlen(token) + 1);
			break;
		case 21:
			(void) strncpy(metar->wx_string, token, strlen(token) + 1);
			break;
		case 22:
			(void) strncpy(metar->sky_cover_0, token, strlen(token) + 1);
			break;
		case 23:
			(void) strncpy(metar->cloud_base_ft_agl_0, token, strlen(token) + 1);
			break;
		case 24:
			(void) strncpy(metar->sky_cover_1, token, strlen(token) + 1);
			break;
		case 25:
			(void) strncpy(metar->cloud_base_ft_agl_1, token, strlen(token) + 1);
			break;
		case 26:
			(void) strncpy(metar->sky_cover_2, token, strlen(token) + 1);
			break;
		case 27:
			(void) strncpy(metar->cloud_base_ft_agl_2, token, strlen(token) + 1);
			break;
		case 28:
			(void) strncpy(metar->sky_cover_3, token, strlen(token) + 1);
			break;
		case 29:
			(void) strncpy(metar->cloud_base_ft_agl_3, token, strlen(token) + 1);
			break;
		case 30:
			(void) strncpy(metar->flight_category, token, strlen(token) + 1);
			break;
		case 31:
			(void) strncpy(metar->three_hr_pressure_tendency_mb, token, strlen(token) + 1);
			break;
		case 32:
			(void) strncpy(metar->maxT_c, token, strlen(token) + 1);
			break;
		case 33:
			(void) strncpy(metar->minT_c, token, strlen(token) + 1);
			break;
		case 34:
			(void) strncpy(metar->maxT24hr_c, token, strlen(token) + 1);
			break;
		case 35:
			(void) strncpy(metar->minT24hr_c, token, strlen(token) + 1);
			break;
		case 36:
			(void) strncpy(metar->precip_in, token, strlen(token) + 1);
			break;
		case 37:
			(void) strncpy(metar->pcp3hr_in, token, strlen(token) + 1);
			break;
		case 38:
			(void) strncpy(metar->pcp6hr_in, token, strlen(token) + 1);
			break;
		case 39:
			(void) strncpy(metar->pcp24hr_in, token, strlen(token) + 1);
			break;
		case 40:
			(void) strncpy(metar->snow_in, token, strlen(token) + 1);
			break;
		case 41:
			(void) strncpy(metar->vert_vis_ft, token, strlen(token) + 1);
			break;
		case 42:
			(void) strncpy(metar->metar_type, token, strlen(token) + 1);
			break;
		case 43:
			if (strlen(token) > 1) {
				token[strlen(token) - 1] = '\0';
			}

			(void) strncpy(metar->elevation_m, token, strlen(token) + 1);
			break;
		default:
			break;
		};

		x++;
	}


	(void) strlcpy(info->outputPath, info->outputBasePath, strlen(info->outputBasePath) + 1);
	(void) strncat(info->outputPath, metar->station_id, strlen(metar->station_id));

	ofp = fopen(info->outputPath, "w");

	if (ofp == NULL) {
		(void) fprintf(stderr, "Could not open %s for writing.\n", metar->station_id);

		return;
	}

	(void) fprintf(ofp, "Station: %s<br />\n", metar->station_id);
	(void) fprintf(ofp, "Type: %s<br />\n", metar->metar_type);

	if (strlen(metar->maintenance_indicator_on) > 0) {
		(void) fprintf(ofp, "Maint: %s<br />\n", metar->maintenance_indicator_on);
	}

	(void) fprintf(ofp, "Time: %s<br />\n", metar->observation_time);
	(void) fprintf(ofp, "Location: %s, %s<br />\n", metar->latitude, metar->longitude);
	(void) fprintf(ofp, "Elevation: %s meters<br />\n", metar->elevation_m);
	(void) fprintf(ofp, "Temp: %s&deg;C<br />\n", metar->temp_c);
	(void) fprintf(ofp, "Dewpoint: %s&deg;C<br />\n", metar->dewpoint_c);
	(void) fprintf(ofp, "Wind: From %s degrees at %s knots<br />\n", metar->wind_dir_degrees, metar->wind_speed_kt);

	if (strlen(metar->wind_gust_kt) > 0) {
		(void) fprintf(ofp, "Gust: %s knots<br />\n", metar->wind_gust_kt);
	}

	(void) fprintf(ofp, "Visibility: %s miles<br />\n", metar->visibility_statute_mi);
	(void) fprintf(ofp, "Altimeter: %s<br />\n", metar->altim_in_hg);

	if (strlen(metar->wx_string) > 0) {
		(void) fprintf(ofp, "Weather: %s<br />\n", metar->wx_string);
	}

	free(metar);

	fclose(ofp);
	return;
}

static size_t
writeData(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);

	return written;
}
