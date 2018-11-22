/*
 * thread.h
 *
 *  Created on: May 11, 2018
 *      Author: Congwei Dang
 */

#ifndef MISO_INCLUDE_THREAD_H_
#define MISO_INCLUDE_THREAD_H_

#include "miso_types.h"

#ifdef __cplusplus
extern "C" {
#endif


mINT32 get_current_process_id();


/* Wait types and functions */
void msleep(mINT32 ms);


/* Thread types and functions */
#define MAX_THREAD_NAME	256

typedef void* mTHREAD;

typedef void (*THREAD_PROC) (mTHREAD th, void *param);

mTHREAD thread_create(THREAD_PROC thread_proc, const char *name, mINT32 idx, void *param);
mBOOL thread_is_alive(mTHREAD th);
const char *thread_get_name(mTHREAD th);
mINT32 thread_get_idx(mTHREAD th);
void thread_require_quit(mTHREAD th);
mBOOL thread_quit_required(mTHREAD th);
void thread_destroy(mTHREAD);


/* Lock types and functions */
typedef void* mLOCK;

mLOCK lock_create();
void lock_destroy(mLOCK lock);
void lock_acquire(mLOCK lock);
void lock_release(mLOCK lock);


/* Semaphore types and functions */
typedef void* mSEMAPHORE;

mSEMAPHORE m_semaphore_create(mINT32 init_state);
mBOOL m_semaphore_wait(mSEMAPHORE semaphore);
void m_semaphore_increase(mSEMAPHORE semaphore, mINT32 increase);
void m_semaphore_destroy(mSEMAPHORE semaphore);
void m_semaphore_disable(mSEMAPHORE semaphore);

/* Traffic light types and functions */
typedef void *mTRAFFIC_LIGHT;


#ifdef __cplusplus
} //extern "C" {
#endif


#endif /* MISO_INCLUDE_THREAD_H_ */
