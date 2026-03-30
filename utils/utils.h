#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

extern int DEBUG;
extern int DEBUG_OP;
extern clock_t _dbg_t;

#define DBG_OP_MULT 1
#define DBG_OP_POW  2
#define DBG_OP_FACT 3

#define DBG_ARRAY_TRUNC_LEN 5

#define DBG_HEADER(msg) do { if (DEBUG >= 1 && DEBUG_OP == DBG_OP_MULT) printf("\n\033[1;36m══════ %s ══════\033[0m\n", msg); } while(0)
#define DBG_VAL(label, fmt, ...) do { if (DEBUG >= 1 && DEBUG_OP == DBG_OP_MULT) printf("  \033[33m%-22s\033[0m " fmt "\n", label, __VA_ARGS__); } while(0)
#define DBG_SEP() do { if (DEBUG >= 1 && DEBUG_OP == DBG_OP_MULT) printf("\033[90m  ────────────────────────────────\033[0m\n"); } while(0)
#define DBG_ARRAY(fn, name, arr, len) do { if (DEBUG >= 2 && DEBUG_OP == DBG_OP_MULT) { printf("  \033[33m%-22s\033[0m ", name); fn(name, arr, len); } } while(0)
#define DBG_TIME_START() do { if (DEBUG >= 1 && DEBUG_OP == DBG_OP_MULT) _dbg_t = clock(); } while(0)
#define DBG_TIME_END(label) do { if (DEBUG >= 1 && DEBUG_OP == DBG_OP_MULT) { double _s = (double)(clock() - _dbg_t) / CLOCKS_PER_SEC; printf("  \033[32m%-22s\033[0m %.6f s\n", label, _s); } } while(0)

#define DBG_FACT_HEADER(msg) do { if (DEBUG >= 1 && DEBUG_OP == DBG_OP_FACT) printf("\n\033[1;35m✦ ━━━━━━━━━━ %s ━━━━━━━━━━ ✦\033[0m\n", msg); } while(0)
#define DBG_POW_HEADER(msg) do { if (DEBUG >= 1 && DEBUG_OP == DBG_OP_POW) printf("\n\033[1;34m⚡ ━━━━━━━━━━ %s ━━━━━━━━━━ ⚡\033[0m\n", msg); } while(0)

typedef struct arr {
    int* list;
    int length;
} intArray;

typedef struct arr2 {
    char* list;
    int length;
} charArray;

typedef struct arr3 {
    charArray** list;
    int length;
} strArray;

charArray* create_long_num(int len, char digit);
void freeintArray(intArray* arr);
void freecharArray(charArray* arr);
void freestrArray(strArray* arr);
charArray* getStr(const char * str);
void print_truncated_str(const char* prefix, charArray* str, int max_display);

int bitrev(int num, int bits_num);
void swap_int(int* a, int* b);
void swap_longlong(unsigned long long* a, unsigned long long* b);
unsigned long long next2pow(unsigned long long deg); 
long long modinv(int base, unsigned long long mod);  
unsigned long long modinv_fermat(unsigned long long base, unsigned long long mod);
unsigned long long modpow(unsigned long long base, unsigned long long power, unsigned long long mod);

void print_int_array(char* name, int* array, int len);
void print_longlong_array(char* name, unsigned long long* array, int len);

#endif
