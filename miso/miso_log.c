/*
 * log.c
 *
 *  Created on: Jun 14, 2018
 *      Author: Congwei Dang
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "miso_types.h"
#include "miso_log.h"
#include "timestamp.h"
#include "spinlock.h"

#ifdef __cplusplus
extern "C" {
#endif


struct st_mLogInfo {
	mLOG_LEVEL log_level;
	mBOOL file_flag;
	mBOOL stdout_flag;
	char log_file [1024];
};


static struct st_mLogInfo s_log_info;
static mSPINLOCK_TYPE s_log_spinlock = 0;
static char s_msg_buf [4096];

void m_set_log_level(mLOG_LEVEL level) {
	spinlock_array_lock(&s_log_spinlock, 0);

	s_log_info.log_level = level;

	spinlock_array_unlock(&s_log_spinlock, 0);
}


void m_set_log_file(const char *log_file) {
	if(log_file == NULL) {
		spinlock_array_lock(&s_log_spinlock, 0);

		s_log_info.file_flag = mFALSE;

		spinlock_array_unlock(&s_log_spinlock, 0);
	} else {
		mINT32 len = strlen(log_file);
		len = mMIN(len, sizeof(s_log_info.log_file)-1);

		spinlock_array_lock(&s_log_spinlock, 0);

		s_log_info.file_flag = mTRUE;
		memcpy(s_log_info.log_file, log_file, len);
		s_log_info.log_file[len] = 0;

		spinlock_array_unlock(&s_log_spinlock, 0);
	}
}


void m_set_log_stdout(mBOOL flag) {
	spinlock_array_lock(&s_log_spinlock, 0);

	s_log_info.stdout_flag = flag;

	spinlock_array_unlock(&s_log_spinlock, 0);
}


const char *m_get_level_str(mLOG_LEVEL level) {
	if(level == LOG_NONE) {
		return "NONE";
	} else if(level == LOG_FATAL) {
		return "FATAL";
	} else if(level == LOG_ERROR) {
		return "ERROR";
	} else if(level == LOG_WARNING) {
		return "WARNING";
	} else if(level == LOG_INFO) {
		return "INFO";
	} else if(level == LOG_FINE) {
		return "FINE";
	} else if(level == LOG_FINER) {
		return "FINER";
	} else if(level == LOG_FINEST) {
		return "FINEST";
	} else if(level == LOG_ALL) {
		return "ALL";
	} else {
		return "UNKNOWN";
	}
}

void m_log_msg(mLOG_LEVEL level, const char *format, ...) {
	mINT32 len;
	va_list valist;
	FILE *fp;
	const char *level_str;
	char time_buf[20];

	if(level > s_log_info.log_level) {
		return;
	}

	current_timestamp_stdstr(time_buf, sizeof(time_buf), mTRUE);

	level_str = m_get_level_str(level);

	va_start(valist, format);

	spinlock_array_lock(&s_log_spinlock, 0);

	len = vsnprintf(s_msg_buf, sizeof(s_msg_buf), format, valist);
	va_end (valist);

	if(s_log_info.file_flag != mFALSE) {
		fp = fopen(s_log_info.log_file, "a");
		if(fp != NULL) {

			fprintf(fp, "[%s] <%s> %s\n",
					time_buf, level_str, s_msg_buf);

			fclose(fp);
		}
	}

	if(s_log_info.stdout_flag != mFALSE) {
		printf("[%s] <%s> %s\n",
				time_buf, level_str, s_msg_buf);
	}

	spinlock_array_unlock(&s_log_spinlock, 0);

}

#ifdef __cplusplus
} // extern "C" {
#endif


