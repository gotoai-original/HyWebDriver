/*
 * aux.c
 *
 *  Created on: May 5, 2018
 *      Author: Congwei Dang
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "miso_types.h"
#include "timestamp.h"
#include "spinlock.h"
#include "common.h"


#ifdef __cplusplus
extern "C" {
#endif


size_t g_alloc_seq_no = 0;
size_t g_free_seq_no = 0;

size_t g_acc_allocated = 0;
size_t g_acc_freed = 0;

char g_output_file[1024];

void init_aux();
void *m_malloc(size_t size);
void m_free(void *p);

mBOOL g_output_flag = mFALSE;

mSPINLOCK_TYPE g_malloc_spinlock = 0;


void init_aux() {

	sprintf(g_output_file, "./temp/alloc_free_records_%.3f.log",
			(mFLOAT64) current_timestamp());

	if(g_output_flag) {
		printf("Set allog-free record output file [%s].\n", g_output_file);
	}
}


void cleanup_aux() {
	printf("alloc_count [%lu] times, free_count [%lu] times.\nacc_alloc [%lu] bytes, acc_free [%lu] bytes\n",
			g_alloc_seq_no, g_free_seq_no,
			g_acc_allocated, g_acc_freed);
}


void output_record(const char *buf, size_t size) {
	FILE *fp = fopen(g_output_file, "a");
	fwrite(buf, size, 1, fp);
	fclose(fp);
}

void *m_malloc(size_t size) {
	void *p;
	spinlock_array_lock(&g_malloc_spinlock, 0);

	p = malloc(size+sizeof(size_t));
	*((size_t*)((mINT8*)p)) = size;

	g_acc_allocated += size;

	//if(g_alloc_seq_no == 16) {
	//	int aaa = 1;
	//	int bbb = aaa;
	//}

	if(g_output_flag) {
		// type, seq_no, timestamp, pointer, size
		char buf[1024];
		sprintf(buf, "%s\t%lu\t%.3f\t%p\t%lu\n",
				"ALLOC",
				g_alloc_seq_no,
				(mFLOAT64) current_timestamp(),
				p,
				size);
		output_record(buf, strlen(buf));
	}

	++g_alloc_seq_no;

	spinlock_array_unlock(&g_malloc_spinlock, 0);

	return (((mINT8*)p) +sizeof(size_t));
}

void m_free(void *p) {
	void *p2;
	spinlock_array_lock(&g_malloc_spinlock, 0);
	p2 = ((mINT8*)p) - sizeof(size_t);
	size_t size = *((size_t*)p2);

	g_acc_freed += size;

	free(p2);

	if(g_output_flag) {
		// type, seq_no, timestamp, pointer, size
		char buf[1024];
		sprintf(buf, "%s\t%lu\t%.3f\t%p\t%lu\n",
				"FREE",
				g_free_seq_no,
				(mFLOAT64) current_timestamp(),
				p,
				(size_t)0);
		output_record(buf, strlen(buf));
	}

	++g_free_seq_no;

	spinlock_array_unlock(&g_malloc_spinlock, 0);
}


size_t m_strcpy(char *to, size_t to_size, const char *from, size_t from_len) {
	size_t copy_len = mMIN(to_size-1, from_len);
	if(copy_len <= 0) return 0;

	memcpy(to, from, copy_len);
	to[copy_len] = 0;

	return copy_len;
}


size_t m_strcat(char *to, size_t to_size, size_t to_len, const char *from, size_t from_len) {
	size_t copy_len = mMIN(to_size - to_len - 1, from_len);
	if(copy_len <= 0) return 0;

	memcpy(to+to_len, from, copy_len);
	to[copy_len] = 0;

	return copy_len;
}


#ifdef __cplusplus
} // extern "C" {
#endif


