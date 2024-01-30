#ifndef _DEBUG
#define _DEBUG

// compile with "-D DEBUG" to enable

#ifdef DEBUG

#include <stdio.h>

#define DEBUG_PRINT(fmt, args...) fprintf(stdout, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
#else 
#define DEBUG_PRINT(fmt, args...)
#endif

#endif // _DEBUG
