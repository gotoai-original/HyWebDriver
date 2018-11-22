/*
 * timestamp.c
 *
 *  Created on: May 10, 2018
 *      Author: Congwei Dang
 */




#include <stdio.h>
#include <time.h>
#include "miso_types.h"

#ifdef __cplusplus
extern "C" {
#endif


mINT64 current_timestamp();

/**
 * Return current time stamp (UTC) in format "YYYY-MM-DD hh:mm:ss"
 * @param buf
 * @param buf_size
 */
void current_timestamp_stdstr(char *buf, int buf_size, mBOOL local) {
	time_t t;
	struct tm ti;

	if(buf_size < 20) {
		return;
	}

	time(&t);

	if(local == mFALSE) {
		ti = *gmtime(&t);
	} else {
		ti = *localtime(&t);
	}

	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
			1900 + ti.tm_year,
			1 + ti.tm_mon,
			ti.tm_mday,
			ti.tm_hour,
			ti.tm_min,
			ti.tm_sec);

}

/**
 * Return current time stamp (UTC) in format "YYYYMMDDhhmmss"
 * @param buf
 * @param buf_size
 */
void current_timestamp_str14(char *buf, int buf_size, mBOOL local) {
	time_t t;
	struct tm ti;

	if(buf_size < 20) {
		return;
	}

	time(&t);

	if(local == mFALSE) {
		ti = *gmtime(&t);
	} else {
		ti = *localtime(&t);
	}

	sprintf(buf, "%04d%02d%02d%02d%02d%02d",
			1900 + ti.tm_year,
			1 + ti.tm_mon,
			ti.tm_mday,
			ti.tm_hour,
			ti.tm_min,
			ti.tm_sec);

}

#ifdef __linux__
#include <sys/time.h>

mINT64 current_timestamp() {
	mINT64 ms;
	struct timeval tv;
    gettimeofday(&tv, NULL);

    ms = tv.tv_sec * 1000;		// seconds
    ms += tv.tv_usec / 1000; 	// microseconds

    return ms;
}


mINT64 current_timestamp_us() {
	mINT64 us;
	struct timeval tv;
    gettimeofday(&tv, NULL);

    us = tv.tv_sec * 1000 * 1000;		// seconds
    us += tv.tv_usec; 	// microseconds

    return us;
}


mINT32 timezone_sec() {
	mINT32 tz_sec = -timezone;
	return tz_sec;
}

#endif


#ifdef __cplusplus
} // extern "C" {
#endif

