/*
 * miso_types.h
 *
 *  Created on: May 2, 2018
 *      Author: Congwei Dang
 */

#ifndef MISO_TYPES_H_
#define MISO_TYPES_H_


#define mERR_PTR ((void*)(-1))
#define mMAX_INT32_SIZE (0x7fffffff)

typedef char mINT8;
typedef unsigned char mUINT8;

typedef short mINT16;
typedef unsigned short mUNIT16;


/* ASSERT sizeof(int) == 4 */
typedef char ASSERT_SIZEOF_INT_EQ_4 [ (sizeof(int) == 4) ? 1 : -1];

typedef int mINT32;
typedef unsigned int mUINT32;

#if defined(__GNUC__)

/* ASSERT sizeof(long) == 8 */
typedef char ASSERT_SIZEOF_LONG_EQ_8 [ (sizeof(long) == 8) ? 1 : -1];

typedef long mINT64;
typedef unsigned long mUINT64;

#elif defined(_MSC_VER)

/* ASSERT sizeof(long long) == 8 */
typedef char ASSERT_SIZEOF_LONG_EQ_8[(sizeof(long long) == 8) ? 1 : -1];

typedef long long mINT64;
typedef unsigned long long mUINT64;

#endif 


typedef mUINT8 mBYTE;

typedef char mBOOL;

/* ASSERT sizeof(float) == 4 */
typedef char ASSERT_SIZEOF_FLOAT_EQ_4 [ (sizeof(float) == 4) ? 1 : -1];

typedef float mFLOAT32;

/* ASSERT sizeof(double) == 8 */
typedef char ASSERT_SIZEOF_DOUBLE_EQ_8 [ (sizeof(double) == 8) ? 1 : -1];

typedef double mFLOAT64;


#define mTRUE (1)
#define mFALSE (0)
#define mINVALID_SIZE (-1)
#define mINVALID_INDEX (-1)


#define mMIN(x, y) ((x) < (y) ? (x) : (y))
#define mMAX(x, y) ((x) > (y) ? (x) : (y))


#endif /* MISO_TYPES_H_ */
