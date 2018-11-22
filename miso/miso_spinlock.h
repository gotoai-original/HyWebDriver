/*
 * spinlock.h
 *
 *  Created on: May 22, 2018
 *      Author: Congwei Dang
 */

#ifndef MISONLP_INCLUDE_SPINLOCK_H_
#define MISONLP_INCLUDE_SPINLOCK_H_

#include <stdio.h>
#include "miso_types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef volatile mINT32 mSPINLOCK_TYPE;
typedef mSPINLOCK_TYPE * mSPINLOCK_ARRAY;


mSPINLOCK_ARRAY spinlock_array_create(size_t size);
void spinlock_array_destroy(mSPINLOCK_ARRAY sa);


void spinlock_array_lock(mSPINLOCK_ARRAY sa, mINT32 idx);
void spinlock_array_unlock(mSPINLOCK_ARRAY sa, mINT32 idx);


#ifdef __cplusplus
} //extern "C" {
#endif


#endif /* MISONLP_INCLUDE_SPINLOCK_H_ */
