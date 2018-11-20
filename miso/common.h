/*
 * aux.h
 *
 *  Created on: May 5, 2018
 *      Author: Congwei Dang
 */

#ifndef MISONLP_COMMON_H_
#define MISONLP_COMMON_H_

#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


void *m_malloc(size_t size);
void m_free(void *p);

size_t m_strcpy(char *to, size_t to_size, const char *from, size_t from_len);
size_t m_strcat(char *to, size_t to_len, size_t to_size, const char *from, size_t from_len);


#ifdef __cplusplus
} // extern "C" {
#endif


#endif /* MISONLP_COMMON_H_ */
