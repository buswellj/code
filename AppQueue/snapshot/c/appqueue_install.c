/*
 * AppQueue
 * Copyright (c) 2009 - 2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.5
 *
 * LICENSE: GPL v2
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 * To contact Carbon Mountain LLC please visit http://www.carbonmountain.com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <string.h>
#include "kattach_types.h"
#include "appqueue.h"

static size_t appqueue_write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}

int appqueue_fetch(u64 appq_tstamp, u32 oindex, const char *payloadfile)
{
	CURL *curl_handle;
	CURLcode res;
	char headerfilename[32];
	FILE *headerfile;
	FILE *payload;
	char finalfn[128];
	char aqilink[128];
	int rc = 0;
	double http_clen, clen_mb;

	sprintf(headerfilename,"%sapp-head.out",APPQUEUE_PATH_HVIMAGE);
	appqueue_options.option[oindex].valid = 0;

	printf("Downloading %s app module:\n\n",appqueue_options.option[oindex].app_mod);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 60000);
	curl_easy_setopt(curl_handle, CURLOPT_URL, appqueue_options.option[oindex].url);
  	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
  	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, appqueue_write_data);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1);

	headerfile = fopen(headerfilename,"w");
	if (headerfile == NULL) {
		curl_easy_cleanup(curl_handle);
		return RC_FAIL;
	}

	payload = fopen(payloadfile,"w");
	if (payload == NULL) {
		curl_easy_cleanup(curl_handle);
		return RC_FAIL;
	}

	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, headerfile);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, payload);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, APPQUEUE_UA);
	res = curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &http_clen);

	fclose(headerfile);
	fclose(payload);

	printf("\nTransaction Report: %s\n",curl_easy_strerror(res));

	curl_easy_cleanup(curl_handle);

	if (!strncmp(appqueue_options.option[oindex].app_mod,"vkaos",strlen(appqueue_options.option[oindex].app_mod))) {
		sprintf(finalfn,"%s%s-%llu",APPQUEUE_PATH_VKAOS,APPQUEUE_VKAOS_KERNEL,appq_tstamp);
		sprintf(aqilink,"%s%s",APPQUEUE_PATH_VKAOS,APPQUEUE_VKAOS_KERNEL);
	} else {
                sprintf(finalfn,"%s%s%s/",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod);
                rc = mkdir(finalfn, APPQUEUE_PERM_GROUP);
                rc = chmod(finalfn, APPQUEUE_PERM_GROUP);
                sprintf(finalfn,"%s%s%s/images",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod);
                rc = mkdir(finalfn, APPQUEUE_PERM_GROUP);
                rc = chmod(finalfn, APPQUEUE_PERM_GROUP);
                sprintf(finalfn,"%s%s%s/cli",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod);
                rc = mkdir(finalfn, APPQUEUE_PERM_GROUP);
                rc = chmod(finalfn, APPQUEUE_PERM_GROUP);
                sprintf(finalfn,"%s%s%s/mgr",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod);
                rc = mkdir(finalfn, APPQUEUE_PERM_GROUP);
                rc = chmod(finalfn, APPQUEUE_PERM_GROUP);
                sprintf(finalfn,"%s%s%s/images/%s-%llu.aqi",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod,
				appqueue_options.option[oindex].app_mod,appq_tstamp);
/*              sprintf(finalfn,"%s%s-%llu.aqi",APPQUEUE_PATH_IMAGES,appqueue_options.option[oindex].app_mod,appq_tstamp); */
                sprintf(aqilink,"%s%s%s/images/%s.aqi",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod,appqueue_options.option[oindex].app_mod);
	}

	if (http_clen < 0) {
		printf("Unknown app module : %s\n",appqueue_options.option[oindex].app_mod);
		printf("Visit http://www.appqueue.info for available modules.\n\n");
		rc = remove(payloadfile);
		return RC_FAIL;
	} else if (http_clen < APPQUEUE_MAX_LEN_CONTENT) {
		printf("\nReceived %g bytes : Download failed module %s not found or invalid.\n\n",http_clen,appqueue_options.option[oindex].app_mod);
		rc = remove(payloadfile);
		return RC_FAIL;
	}

	if (http_clen >= APPQUEUE_DIV_MB) {
		clen_mb = (http_clen / APPQUEUE_DIV_MB);
		printf("\nReceived %g MB :: download complete - linking %s to %s\n\n",clen_mb,finalfn,aqilink);
	} else {
		clen_mb = http_clen;
		printf("\nReceived %g bytes :: download complete - linking %s to %s\n\n",clen_mb,finalfn,aqilink);
	}

	rename(payloadfile, finalfn);
	rc = unlink(aqilink);
	rc = symlink(finalfn,aqilink);
	appqueue_options.option[oindex].valid = 1;
	appqueue_options.option[oindex].size = (u32) clen_mb;
	return RC_OK;
}

int appqueue_fetchcli(u64 appq_tstamp, u32 oindex, const char *payloadfile)
{
	CURL *curl_handle;
	CURLcode res;
	char headerfilename[32];
	FILE *headerfile;
	FILE *payload;
	char finalfn[128];
	char aqilink[128];
	int rc = 0;
	double http_clen, clen_mb;

	sprintf(headerfilename,"%sapp-head-cli.out",APPQUEUE_PATH_HVIMAGE);
	appqueue_options.option[oindex].valid = 0;

	printf("Downloading %s CLI module:\n\n",appqueue_options.option[oindex].app_mod);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 60000);
	curl_easy_setopt(curl_handle, CURLOPT_URL, appqueue_options.option[oindex].cliurl);
  	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
  	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, appqueue_write_data);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1);

	headerfile = fopen(headerfilename,"w");
	if (headerfile == NULL) {
		curl_easy_cleanup(curl_handle);
		return RC_FAIL;
	}

	payload = fopen(payloadfile,"w");
	if (payload == NULL) {
		curl_easy_cleanup(curl_handle);
		return RC_FAIL;
	}

	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, headerfile);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, payload);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, APPQUEUE_UA);
	res = curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &http_clen);

	fclose(headerfile);
	fclose(payload);

	printf("\nTransaction Report: %s\n",curl_easy_strerror(res));

	curl_easy_cleanup(curl_handle);

        sprintf(finalfn,"%s%s%s/cli/climod-%llu.aq",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod,appq_tstamp);
        sprintf(aqilink,"%s%s%s/cli/climod.aq",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod);

	if (http_clen < 0) {
		printf("Unknown app module : %s\n",appqueue_options.option[oindex].app_mod);
		printf("Visit http://www.appqueue.info for available modules.\n\n");
		rc = remove(payloadfile);
		return RC_FAIL;
	} else if (http_clen < APPQUEUE_MAX_LEN_CONTENT) {
		printf("\nReceived %g bytes : Download failed module %s not found or invalid.\n\n",http_clen,appqueue_options.option[oindex].app_mod);
		rc = remove(payloadfile);
		return RC_FAIL;
	}

	if (http_clen >= APPQUEUE_DIV_MB) {
		clen_mb = (http_clen / APPQUEUE_DIV_MB);
		printf("\nReceived %g MB :: download complete - linking %s to %s\n\n",clen_mb,finalfn,aqilink);
	} else {
		clen_mb = http_clen;
		printf("\nReceived %g bytes :: download complete - linking %s to %s\n\n",clen_mb,finalfn,aqilink);
	}

	rename(payloadfile, finalfn);
	rc = unlink(aqilink);
	rc = symlink(finalfn,aqilink);
	appqueue_options.option[oindex].valid = 1;
        sprintf(finalfn,"%s%s%s/cli/climod.aq",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod);
        rc = chmod(finalfn, APPQUEUE_PERM_GROUP);
	return RC_OK;
}

int appqueue_fetchmgr(u64 appq_tstamp, u32 oindex, const char *payloadfile)
{
	CURL *curl_handle;
	CURLcode res;
	char headerfilename[32];
	FILE *headerfile;
	FILE *payload;
	char finalfn[128];
	char aqilink[128];
	int rc = 0;
	double http_clen, clen_mb;

	sprintf(headerfilename,"%sapp-head-cli.out",APPQUEUE_PATH_HVIMAGE);
	appqueue_options.option[oindex].valid = 0;

	printf("Downloading %s CLI module:\n\n",appqueue_options.option[oindex].app_mod);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 60000);
	curl_easy_setopt(curl_handle, CURLOPT_URL, appqueue_options.option[oindex].mgrurl);
  	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
  	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, appqueue_write_data);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1);

	headerfile = fopen(headerfilename,"w");
	if (headerfile == NULL) {
		curl_easy_cleanup(curl_handle);
		return RC_FAIL;
	}

	payload = fopen(payloadfile,"w");
	if (payload == NULL) {
		curl_easy_cleanup(curl_handle);
		return RC_FAIL;
	}

	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, headerfile);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, payload);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, APPQUEUE_UA);
	res = curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &http_clen);

	fclose(headerfile);
	fclose(payload);

	printf("\nTransaction Report: %s\n",curl_easy_strerror(res));

	curl_easy_cleanup(curl_handle);

        sprintf(finalfn,"%s%s%s/mgr/appmgr-%llu.aq",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod,appq_tstamp);
        sprintf(aqilink,"%s%s%s/mgr/appmgr.aq",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod);

	if (http_clen < 0) {
		printf("Unknown app module : %s\n",appqueue_options.option[oindex].app_mod);
		printf("Visit http://www.appqueue.info for available modules.\n\n");
		rc = remove(payloadfile);
		return RC_FAIL;
	} else if (http_clen < APPQUEUE_MAX_LEN_CONTENT) {
		printf("\nReceived %g bytes : Download failed module %s not found or invalid.\n\n",http_clen,appqueue_options.option[oindex].app_mod);
		rc = remove(payloadfile);
		return RC_FAIL;
	}

	if (http_clen >= APPQUEUE_DIV_MB) {
		clen_mb = (http_clen / APPQUEUE_DIV_MB);
		printf("\nReceived %g MB :: download complete - linking %s to %s\n\n",clen_mb,finalfn,aqilink);
	} else {
		clen_mb = http_clen;
		printf("\nReceived %g bytes :: download complete - linking %s to %s\n\n",clen_mb,finalfn,aqilink);
	}

	rename(payloadfile, finalfn);
	rc = unlink(aqilink);
	rc = symlink(finalfn,aqilink);
	appqueue_options.option[oindex].valid = 1;
        sprintf(finalfn,"%s%s%s/mgr/appmgr.aq",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod);
        rc = chmod(finalfn, APPQUEUE_PERM_GROUP);
	return RC_OK;
}


void appqueue_install(void)
{
	char aqtmpf[128];
	struct timeval dlstime;
	time_t dltstamp;
	u64 appq_tstamp;
	u32 oindex = 0;
	int i = 0, rc = 0, v = 0;


	printf("\n");

	for (i = 0; i < appqueue_options.index; i++) {
		v = 0;
		gettimeofday(&dlstime,NULL);
		dltstamp = dlstime.tv_sec;
		appq_tstamp = (u64) dltstamp;
		if (!strncmp(appqueue_options.option[i].app_mod,"vkaos",strlen(appqueue_options.option[i].app_mod))) {
			sprintf(aqtmpf,"%stmp-%llu",APPQUEUE_PATH_VKAOS,appq_tstamp);
			v = 1;
		} else {
			sprintf(aqtmpf,"%stmp-%llu",APPQUEUE_PATH_IMAGES,appq_tstamp);
		}
		rc = appqueue_fetch(appq_tstamp,oindex,aqtmpf);
		if (rc == RC_FAIL) {
			appqueue_options.option[i].valid = 0;
		} else {
			appqueue_options.option[i].valid = 1;
		}
		if ((v == 0) && (appqueue_options.option[i].valid == 1)) {
			sprintf(aqtmpf,"%sclitmp-%llu",APPQUEUE_PATH_IMAGES,appq_tstamp);
			rc = appqueue_fetchcli(appq_tstamp, oindex,aqtmpf);
			sprintf(aqtmpf,"%smgrtmp-%llu",APPQUEUE_PATH_IMAGES,appq_tstamp);
			rc = appqueue_fetchmgr(appq_tstamp, oindex,aqtmpf);
		}

		oindex++;
	}
}

void appqueue_uninstall(void)
{
	char aqrmf[128];
	u32 oindex = 0;
	int i, rc;

	printf("\n");

	for (i = 0; i < appqueue_options.index; i++) {
		/* FIXME: clean up the module directories */
		printf("Removing app module - %s \t --",appqueue_options.option[oindex].app_mod);
                sprintf(aqrmf,"%s%s%s/images/%s.aqi",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,appqueue_options.option[oindex].app_mod,
				appqueue_options.option[oindex].app_mod);

		sprintf(aqrmf,"%s%s.aqi",APPQUEUE_PATH_IMAGES,appqueue_options.option[oindex].app_mod);
		rc = remove(aqrmf);
		if (rc) {
			printf("--> not installed!\n");
		} else {
			printf("--> removed!\n");
		}
		oindex++;
	}
	printf("\n\n");
}

u8 appqueue_install_gethv(char *url, u8 slot)
{
	CURL *curl_handle;
	char headerfilename[128];
	FILE *headerfile;
	FILE *payload;
	char payloadfile[128];
	int rc = 0;
	double http_clen, clen_mb;

	sprintf(headerfilename,"%shv-head.out",APPQUEUE_PATH_HVIMAGE);
	sprintf(payloadfile,"%shvImage.00%u",APPQUEUE_PATH_HVIMAGE,slot);

	printf("\nDownloading Hypervisor Image from %s into slot %u: \n\n",url,slot);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 60000);
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
  	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, appqueue_write_data);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1);

	headerfile = fopen(headerfilename,"w");
	if (headerfile == NULL) {
		curl_easy_cleanup(curl_handle);
		return RC_FAIL;
	}

	payload = fopen(payloadfile,"w");
	if (payload == NULL) {
		curl_easy_cleanup(curl_handle);
		return RC_FAIL;
	}

	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, headerfile);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, payload);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, APPQUEUE_UA);
	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &http_clen);

	fclose(headerfile);
	fclose(payload);

	curl_easy_cleanup(curl_handle);

	if (http_clen < APPQUEUE_MAX_LEN_CONTENT) {
		printf("\nReceived %g bytes : Download failed Hypervisor Image %s not found or invalid.\n\n",http_clen,url);
		rc = remove(payloadfile);
		return RC_FAIL;
	}

	if (http_clen >= APPQUEUE_DIV_MB) {
		clen_mb = (http_clen / APPQUEUE_DIV_MB);
		printf("\nReceived %g MB :: download complete\n\n",clen_mb);
	} else {
		clen_mb = http_clen;
		printf("\nReceived %g bytes :: download complete\n\n",clen_mb);
	}

	return RC_OK;
}

void appqueue_get_applist(void)
{
	CURL *curl_handle;
	CURLcode res;
	char headerfilename[32];
	FILE *headerfile;
	FILE *payload;
	char *listurl = "http://www.carbonmountain.com/opensource/appqueue/list.e";
	char payloadfile[64];
	int rc = 0;
	double http_clen, clen_mb;

	sprintf(headerfilename,"%sapp-list.out",APPQUEUE_PATH_HVIMAGE);
	sprintf(payloadfile,"%s/list.e",APPQUEUE_PATH_IMAGES);
	printf("Downloading Available App Module List:\n\n");	

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 60000);
	curl_easy_setopt(curl_handle, CURLOPT_URL, listurl);
  	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
  	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, appqueue_write_data);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1);

	headerfile = fopen(headerfilename,"w");
	if (headerfile == NULL) {
		curl_easy_cleanup(curl_handle);
		return;
	}

	payload = fopen(payloadfile,"w");
	if (payload == NULL) {
		curl_easy_cleanup(curl_handle);
		return;
	}

	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, headerfile);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, payload);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, APPQUEUE_UA);
	res = curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &http_clen);

	fclose(headerfile);
	fclose(payload);

	printf("\nTransaction Report: %s\n",curl_easy_strerror(res));

	curl_easy_cleanup(curl_handle);

	if (http_clen < 0) {
		printf("Unable to download community list\n");
		printf("Visit http://www.carbonmountain.com for assistance.\n\n");
		rc = remove(payloadfile);
		return;
#if 0
	} else if (http_clen < APPQUEUE_MAX_LEN_CONTENT) {
		printf("\nReceived %g bytes : Download failed not found or invalid.\n\n",http_clen);
		rc = remove(payloadfile);
		return;
#endif /* 0 */
	}

	if (http_clen >= APPQUEUE_DIV_MB) {
		clen_mb = (http_clen / APPQUEUE_DIV_MB);
		printf("\nReceived %g MB :: download complete\n\n",clen_mb);
	} else {
		clen_mb = http_clen;
		printf("\nReceived %g bytes :: download complete\n\n",clen_mb);
	}

	return;
}
