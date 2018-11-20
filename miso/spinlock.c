/*
 * spinlock.c
 *
 *  Created on: May 22, 2018
 *      Author: Congwei Dang
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "miso_types.h"
#include "spinlock.h"
#include "common.h"


#ifdef __cplusplus
extern "C" {
#endif


#if defined(__GNUC__)

mSPINLOCK_ARRAY spinlock_array_create(size_t size) {
	mSPINLOCK_ARRAY sa = (mSPINLOCK_ARRAY) m_malloc(sizeof(mSPINLOCK_TYPE) * size);
	memset((void *)sa, 0, sizeof(mSPINLOCK_TYPE) * size);
	return sa;
}


void spinlock_array_destroy(mSPINLOCK_ARRAY sa) {
	m_free((void*)sa);
}


void spinlock_array_lock(mSPINLOCK_ARRAY sa, mINT32 idx) {
	//{ return;}
	mSPINLOCK_TYPE *lock = &(sa[idx]);

    while (__sync_lock_test_and_set(lock, 1)) {
       while (*lock);
    }
}


void spinlock_array_unlock(mSPINLOCK_ARRAY sa, mINT32 idx) {
	//{ return;}
	mSPINLOCK_TYPE *lock = &(sa[idx]);
	__sync_lock_release(lock);
}



#elif defined(_MSC_VER)


#else


#endif

#ifdef __cplusplus
} // extern "C" {
#endif


