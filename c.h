#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#define nelem(x) (sizeof(x) / sizeof((x)[0]))

typedef signed char        schar;
typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef unsigned long long uvlong;
typedef signed long long   vlong;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uintptr_t uintptr;
typedef intptr_t  intptr;

typedef ulong size_t;

typedef enum { false = 0, true = 1 } bool;

typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_copy(d,s)  __builtin_va_copy(d,s)

/*****************************************************************************/

extern char* ARGV0;

static inline char* _getopt_(int* p_argc, char*** p_argv) {
    if (!(*p_argv)[0][1] && !(*p_argv)[1]) {
        return (char*)0;
    } else if ((*p_argv)[0][1]) {
        return &(*p_argv)[0][1];
    } else {
        *p_argv = *p_argv + 1;
        *p_argc = *p_argc - 1;
        return (*p_argv)[0];
    }
}

#define OPTBEGIN                                                              \
    for (                                                                     \
        ARGV0 = *argv, argc--, argv++;                                        \
        argv[0] && argv[0][1] && argv[0][0] == '-';                           \
        argc--, argv++                                                        \
    ) {                                                                       \
        int brk_; char argc_ , **argv_, *optarg_;                             \
        if (argv[0][1] == '-' && !argv[0][2]) {                               \
            argv++, argc--; break;                                            \
        }                                                                     \
        for (brk_=0, argv[0]++, argv_=argv; argv[0][0] && !brk_; argv[0]++) { \
            if (argv_ != argv) break;                                         \
            argc_ = argv[0][0];                                               \
            switch (argc_)

#define OPTEND }}

#define OPTC() (argc_)

#define OPTARG() \
    (optarg_ = _getopt_(&argc,&argv), brk_ = (optarg_!=0), optarg_)

#define EOPTARG(code) \
    (optarg_ = _getopt_(&argc,&argv), \
    (!optarg_ ? ((code), abort(), (char*)0) : (brk_ = 1, optarg_)))

#define OPTNUM                                        \
    case '0': case '1': case '2': case '3': case '4': \
    case '5': case '6': case '7': case '8': case '9':

#define OPTLONG case '-'

/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* rdline(FILE* input) {
    size_t size  = 8;
    size_t index = 0;
    char*  str   = (char*)malloc(size);
    memset(str, 0, 8);
    if (feof(input)) {
        free(str);
        return NULL;
    }
    while (true) {
        char ch = fgetc(input);
        if (ch == EOF) break;
        str[index++] = ch;
        str[index]   = '\0';
        if (index+1 >= size) {
            size = size << 1;
            str  = realloc(str, size);
        }
        if (ch == '\n') break;
    }
    return str;
}
