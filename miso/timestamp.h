/*
 * timestamp.h
 *
 *  Created on: May 10, 2018
 *      Author: Congwei Dang
 */

#ifndef MISONLP_INCLUDE_TIMESTAMP_H_
#define MISONLP_INCLUDE_TIMESTAMP_H_

#ifdef __cplusplus
extern "C" {
#endif


void current_timestamp_stdstr(char *buf, int buf_size, mBOOL local);

void current_timestamp_str14(char *buf, int buf_size, mBOOL local);

mINT64 current_timestamp();

mINT64 current_timestamp_us();

mINT32 timezone_sec();

#ifdef __cplusplus
} // extern "C" {
#endif


#endif /* MISONLP_INCLUDE_TIMESTAMP_H_ */
