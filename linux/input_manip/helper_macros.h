#pragma once

// macros to test bits within an array
#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  ((x)/BITS_PER_LONG)
#define LONG(x) ((x)/BITS_PER_LONG)
#define TEST_BIT(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

// Debug log macro
#ifdef _DEBUG
	#define DLOG(x) x;
#else // Release - noop
	#define DLOG(x) ;
#endif // _DEBUG
