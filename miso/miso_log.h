/*
 * log.h
 *
 *  Created on: Jun 14, 2018
 *      Author: Congwei Dang
 */

#ifndef MISONLP_INCLUDE_LOG_H_
#define MISONLP_INCLUDE_LOG_H_

#include "miso_types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	LOG_NONE 		= 0,
	LOG_FATAL 		= 1,
	LOG_ERROR		= 2,
	LOG_WARNING		= 3,
	LOG_INFO		= 4,
	LOG_FINE		= 5,
	LOG_FINER		= 6,
	LOG_FINEST		= 7,
	LOG_ALL			= 99,
} mLOG_LEVEL;


void m_set_log_level(mLOG_LEVEL level);

void m_set_log_file(const char *log_file);

void m_set_log_stdout(mBOOL flag);

void m_log_msg(mLOG_LEVEL level, const char *format, ...);


#ifdef __cplusplus
} //extern "C" {
#endif


#endif /* MISONLP_INCLUDE_LOG_H_ */
