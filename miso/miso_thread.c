/*
 * thread.c
 *
 *  Created on: May 10, 2018
 *      Author: Congwei Dang
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "miso_types.h"
#include "miso_common.h"
#include "miso_thread.h"


#ifdef __linux__
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


mINT32 get_current_process_id() {
	mINT32 pid = (mINT32) getpid();
	return pid;
}


/* Wait types and functions */

void msleep(mINT32 ms) {
	usleep(ms * 1000);
}

/* Thread types and functions */

typedef struct st_THREAD_INFO {
	pthread_t pthread;
	mINT32 thread_idx;
	char name[MAX_THREAD_NAME];
	mBOOL quit_flag;
	mBOOL exit_flag;
	THREAD_PROC thread_proc;
	void *param;

} THREAD_INFO;



static void *wrapper_thread_proc(void *param) {
	THREAD_INFO *thread_info = (THREAD_INFO *)param;

	// call target proc
	thread_info->thread_proc((mTHREAD) thread_info, thread_info->param);

	thread_info->exit_flag = mTRUE;

	return NULL;
}


mTHREAD thread_create(THREAD_PROC thread_proc, const char *name, mINT32 idx, void *param) {
	mINT32 res;

	THREAD_INFO *thread_info = m_malloc(sizeof(THREAD_INFO));

	m_strcpy(thread_info->name, MAX_THREAD_NAME, name, strlen(name));
	thread_info->thread_idx = idx;
	thread_info->quit_flag = mFALSE;
	thread_info->exit_flag = mFALSE;
	thread_info->thread_proc = thread_proc;
	thread_info->param = param;

	res = pthread_create(&(thread_info->pthread),
						NULL,
						wrapper_thread_proc,
						(void *)thread_info);

	if(res != 0) {
		m_free(thread_info);
		return NULL;
	}

	return (mTHREAD) thread_info;

}


void thread_destroy(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *) th;
	pthread_detach(thread_info->pthread);
	m_free(thread_info);
}


void thread_require_quit(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *) th;
	thread_info->quit_flag = mTRUE;

	while(thread_info->exit_flag == mFALSE) {
		msleep(10);
	}

	pthread_join(thread_info->pthread, NULL);

	return;
}


mBOOL thread_quit_required(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *) th;
	return (thread_info->quit_flag != mFALSE);
}


mINT32 thread_get_idx(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *) th;
	return thread_info->thread_idx;
}


const char *thread_get_name(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *) th;
		return thread_info->name;
}


/* Lock types and functions */

typedef struct st_LOCK_INFO {
	pthread_mutex_t mutex;
} LOCK_INFO;


mLOCK lock_create() {
	LOCK_INFO *lock_info = (LOCK_INFO *)m_malloc(sizeof(LOCK_INFO));
	pthread_mutex_init(&lock_info->mutex, NULL);

	return (mLOCK) lock_info;
}


void lock_acquire(mLOCK lock) {
	LOCK_INFO *lock_info = (LOCK_INFO *)lock;
	pthread_mutex_lock(&lock_info->mutex);
}


void lock_release(mLOCK lock) {
	LOCK_INFO *lock_info = (LOCK_INFO *)lock;
	pthread_mutex_unlock(&lock_info->mutex);
}


void lock_destroy(mLOCK lock) {
	LOCK_INFO *lock_info = (LOCK_INFO *)lock;
	pthread_mutex_destroy(&lock_info->mutex);
	m_free(lock_info);
}



/* Semaphore types and functions */

typedef struct st_SHMITT_SEMAPHORE_INFO {
	mBOOL enabled;
	mINT32 count;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

} SHMITT_SEMAPHORE_INFO;


mSEMAPHORE m_semaphore_create(mINT32 init_count) {
	SHMITT_SEMAPHORE_INFO *sema_info = (SHMITT_SEMAPHORE_INFO *)m_malloc(sizeof(SHMITT_SEMAPHORE_INFO));
	sema_info->enabled = mTRUE;
	sema_info->count = init_count;
	pthread_mutex_init(&sema_info->mutex, NULL);
	pthread_cond_init(&sema_info->cond, NULL);

	return (mSEMAPHORE) sema_info;
}


mBOOL m_semaphore_wait(mSEMAPHORE semaphore) {
	SHMITT_SEMAPHORE_INFO *sema_info = (SHMITT_SEMAPHORE_INFO *)semaphore;

	pthread_mutex_lock(&(sema_info->mutex));

	while (sema_info->count == 0) {
		pthread_cond_wait(&(sema_info->cond), &(sema_info->mutex));
		if(sema_info->enabled == mFALSE) {
			pthread_mutex_unlock(&(sema_info->mutex));
			return mFALSE;
		}
	}

	-- sema_info->count;

	pthread_mutex_unlock(&(sema_info->mutex));
	return mTRUE;
}


void m_semaphore_increase(mSEMAPHORE semaphore, mINT32 increase) {
	SHMITT_SEMAPHORE_INFO *sema_info = (SHMITT_SEMAPHORE_INFO *)semaphore;

	pthread_mutex_lock(&(sema_info->mutex));
	sema_info->count += increase;
	pthread_cond_broadcast(&(sema_info->cond));
	pthread_mutex_unlock(&(sema_info->mutex));

}


void m_semaphore_disable(mSEMAPHORE semaphore) {
	SHMITT_SEMAPHORE_INFO *sema_info = (SHMITT_SEMAPHORE_INFO *)semaphore;
	pthread_mutex_lock(&(sema_info->mutex));
	sema_info->enabled = mFALSE;
	pthread_cond_broadcast(&(sema_info->cond));
	pthread_mutex_unlock(&(sema_info->mutex));
}


mBOOL m_semaphore_is_enabled(mSEMAPHORE semaphore) {
	SHMITT_SEMAPHORE_INFO *sema_info = (SHMITT_SEMAPHORE_INFO *)semaphore;
	return sema_info->enabled;
}


void m_semaphore_destroy(mSEMAPHORE semaphore) {
	SHMITT_SEMAPHORE_INFO *sema_info = (SHMITT_SEMAPHORE_INFO *)semaphore;
	pthread_cond_destroy(&(sema_info->cond));
	pthread_mutex_destroy(&sema_info->mutex);
	m_free(sema_info);
}


/* Traffic light class */
typedef struct st_TRAFFIC_LIGHT_INFO {
	pthread_mutex_t 	mutex;
	pthread_cond_t 		cond;
	mINT32				state;
} TRAFFIC_LIGHT_INFO;



mTRAFFIC_LIGHT m_traffic_light_create(mINT32 init_state) {
	TRAFFIC_LIGHT_INFO *tl_info = (TRAFFIC_LIGHT_INFO *)m_malloc(sizeof(TRAFFIC_LIGHT_INFO));
	pthread_mutex_init(&tl_info->mutex, NULL);
	pthread_cond_init(&tl_info->cond, NULL);
	tl_info->state = 0;

	return (mTRAFFIC_LIGHT) tl_info;
}


void m_traffic_light_destroy(mTRAFFIC_LIGHT tl) {
	TRAFFIC_LIGHT_INFO *tl_info = (TRAFFIC_LIGHT_INFO *)tl;
	pthread_cond_destroy(&(tl_info->cond));
	pthread_mutex_destroy(&tl_info->mutex);
	m_free(tl_info);
}


mINT32 m_traffic_light_get_state(mTRAFFIC_LIGHT tl, mINT32 state) {
	mINT32 st = 0;
	TRAFFIC_LIGHT_INFO *tl_info = (TRAFFIC_LIGHT_INFO *)tl;

	pthread_mutex_lock(&tl_info->mutex);
	st = tl_info->state;
	pthread_mutex_unlock(&tl_info->mutex);

	return st;
}


mBOOL m_traffic_light_wait_state_change(mTRAFFIC_LIGHT tl, mINT32 wait_ms) {
	TRAFFIC_LIGHT_INFO *tl_info = (TRAFFIC_LIGHT_INFO *)tl;
	struct timespec ts;
	mINT32 res;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += wait_ms / 1000;
	ts.tv_nsec += (wait_ms % 1000) * 1000;

	pthread_mutex_lock(&tl_info->mutex);
	res = pthread_cond_timedwait(&tl_info->cond, &tl_info->mutex, &ts);
	pthread_mutex_unlock(&tl_info->mutex);

	if(res == 0) {
		return mTRUE;
	} else {
		return mFALSE;
	}
}


mBOOL m_traffic_light_set_state(mTRAFFIC_LIGHT tl, mINT32 new_state) {
	return mFALSE;
}


#ifdef __cplusplus
} //extern "C" {
#endif


#endif


#ifdef _WIN32
#include <windows.h>

/* Wait types and functions */

void msleep(mINT32 ms) {
	Sleep(ms);
}

/* Thread types and functions*/

typedef struct st_THREAD_INFO {
	HANDLE  thread;
	mINT32 thread_idx;
	char name[MAX_THREAD_NAME];
	mBOOL quit_flag;
	mBOOL exit_flag;
	THREAD_PROC thread_proc;
	void *param;

} THREAD_INFO;


static DWORD WINAPI wrapper_thread_proc(void *param) {
	THREAD_INFO *thread_info = (THREAD_INFO *)param;

	// call target proc
	thread_info->thread_proc((mTHREAD)thread_info, thread_info->param);

	thread_info->exit_flag = mTRUE;

	return 0;
}


mTHREAD thread_create(THREAD_PROC thread_proc, const char *name, mINT32 idx, void *param) {
	DWORD dwID;

	THREAD_INFO *thread_info = m_malloc(sizeof(THREAD_INFO));

	m_strcpy(thread_info->name, MAX_THREAD_NAME, name, strlen(name));
	thread_info->thread_idx = idx;
	thread_info->quit_flag = mFALSE;
	thread_info->exit_flag = mFALSE;
	thread_info->thread_proc = thread_proc;
	thread_info->param = param;

	thread_info->thread = CreateThread(NULL,
							0,
							wrapper_thread_proc,
							(void *)thread_info,
							0,
							&dwID);

	if (thread_info->thread == NULL) {
		m_free(thread_info);
		return NULL;
	}

	return (mTHREAD)thread_info;

}


void thread_destroy(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *)th;
	CloseHandle(thread_info->thread);
	m_free(thread_info);
}


void thread_require_quit(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *)th;
	thread_info->quit_flag = mTRUE;

	while (thread_info->exit_flag == mFALSE) {
		msleep(10);
	}

	WaitForSingleObject(thread_info->thread, INFINITE);

	return;
}


mBOOL thread_quit_required(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *)th;
	return (thread_info->quit_flag != mFALSE);
}


mINT32 thread_get_idx(mTHREAD th) {
	THREAD_INFO *thread_info = (THREAD_INFO *)th;
	return thread_info->thread_idx;
}


/* Lock types and functions */

typedef struct st_LOCK_INFO {
	//HANDLE  mutex;
	CRITICAL_SECTION critical_section;
} LOCK_INFO;


mLOCK lock_create() {
	LOCK_INFO *lock_info = (LOCK_INFO *)m_malloc(sizeof(LOCK_INFO));

	//lock_info->mutex = CreateMutex(
	//	NULL,              // default security attributes
	//	FALSE,             // initially not owned
	//	NULL);             // unnamed mutex

	InitializeCriticalSection(&(lock_info->critical_section));

	return (mLOCK)lock_info;
}


void lock_acquire(mLOCK lock) {
	LOCK_INFO *lock_info = (LOCK_INFO *)lock;

	//WaitForSingleObject(
	//	lock_info->mutex,    // handle to mutex
	//	INFINITE);		// no time-out interval

	EnterCriticalSection(&(lock_info->critical_section));

}


void lock_release(mLOCK lock) {
	LOCK_INFO *lock_info = (LOCK_INFO *)lock;
	//BOOL res = ReleaseMutex(lock_info->mutex);
	//
	//if (res == FALSE) {
	//	printf("ReleaseMutex() returne FALSE!\n");
	//}

	LeaveCriticalSection(&(lock_info->critical_section));
}


void lock_destroy(mLOCK lock) {
	LOCK_INFO *lock_info = (LOCK_INFO *)lock;
	
	//CloseHandle(lock_info->mutex);
	DeleteCriticalSection(&(lock_info->critical_section));
	m_free(lock_info);
}



#endif
