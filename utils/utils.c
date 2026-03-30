#include "utils.h"

int DEBUG = 0;
int DEBUG_OP = 0;
clock_t _dbg_t = 0;

charArray* create_long_num(int len, char digit) {
    char* num = (char*)malloc(len + 1);
    if (!num) { printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE); }
    memset(num, digit, len);
    num[len] = '\0';
    charArray* numArray = (charArray*)malloc(sizeof(charArray));
    numArray->list = num;
    numArray->length = len;
    return numArray;
}

void freeintArray(intArray* arr) {
    if(arr) {
        if(arr->list) free(arr->list);
        free(arr);
    }
}

void freecharArray(charArray* arr) {
    if(arr) {
        if(arr->list) free(arr->list);
        free(arr);
    }
}

void freestrArray(strArray* arr) {
    if(arr) {
        if(arr->list) {
            for (int i = 0; i < arr->length; i++) {
                freecharArray(arr->list[i]);
            }
            free(arr->list);
        }
        free(arr);
    }
}

int bitrev(int num, int bits_num) {
    int result = 0;
    for (int i = 0; i < bits_num; i++) {
        if ((num & (1 << i))) {
            result |= (1 << (bits_num - 1 - i));
        }
    }
    return result;
}

void swap_int(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void swap_longlong(unsigned long long* a, unsigned long long* b) {
    unsigned long long temp = *a;
    *a = *b;
    *b = temp;
}

unsigned long long next2pow(unsigned long long deg) {
    int power = (int)ceil(log2((double)deg));
    return (unsigned long long)pow(2.0, power);
}

long long modinv(int base, unsigned long long mod) {
    unsigned long long big = mod;
    long long small = (long long)base;

    long long s_big = 1, t_big = 0;
    long long s_small = 0, t_small = 1;
    while (small > 0) {
        long long q = (long long)(big / small);
        long long temp_small = big % small;
        big = small;
        small = temp_small;
        if (small == 0) break;

        long long s = s_big - q * s_small;
        long long t = t_big - q * t_small;
        s_big = s_small; t_big = t_small;
        s_small = s; t_small = t;
    }
    if (s_small * mod + t_small * base != 1) return 0;
    return (t_small + mod) % mod;
}

unsigned long long modinv_fermat(unsigned long long base, unsigned long long mod) {
    return modpow(base, mod - 2, mod);
}

unsigned long long modpow(unsigned long long base, unsigned long long power, unsigned long long mod) {
    base %= mod;
    unsigned long long result = 1;

#if defined(__GNUC__) || defined(__clang__)
    while (power > 0) {
        if (power % 2 == 1) {
            __uint128_t product = (__uint128_t)result * base;
            result = product % mod;
        }
        __uint128_t square = (__uint128_t)base * base;
        base = square % mod;
        power /= 2;
    }
#else
    while (power > 0) {
        if (power % 2 == 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        power /= 2;
    }
#endif
    return result;
}

charArray* getStr(const char * str) {
    int len = (int)strlen(str);
    char *result = (char*)malloc((len + 1) * sizeof(char));
    if (!result) { printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE); }
    result[len] = '\0';
    for (int i = 0; i < len; i++) {
        result[i] = str[i];
    }
    charArray* resultArr = (charArray*)malloc(sizeof(charArray));
    if (!resultArr) { printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE); }
    resultArr->list = result;
    resultArr->length = len;
    return resultArr;
}

void print_int_array(char* name, int* array, int len) {
    int t = DBG_ARRAY_TRUNC_LEN;
    printf("%s = [", name);
    if (len <= 2 * t) {
        for (int i = 0; i < len; i++) {
            printf("%d", array[i]);
            if (i < len - 1) printf(", ");
        }
    } else {
        for (int i = 0; i < t; i++) printf("%d, ", array[i]);
        printf("... (%d more) ... ", len - 2 * t);
        for (int i = len - t; i < len; i++) {
            printf("%d", array[i]);
            if (i < len - 1) printf(", ");
        }
    }
    printf("]\n");
}

void print_longlong_array(char* name, unsigned long long* array, int len) {
    int t = DBG_ARRAY_TRUNC_LEN;
    printf("%s = [", name);
    if (len <= 2 * t) {
        for (int i = 0; i < len; i++) {
            printf("%llu", array[i]);
            if (i < len - 1) printf(", ");
        }
    } else {
        for (int i = 0; i < t; i++) printf("%llu, ", array[i]);
        printf("... (%d more) ... ", len - 2 * t);
        for (int i = len - t; i < len; i++) {
            printf("%llu", array[i]);
            if (i < len - 1) printf(", ");
        }
    }
    printf("]\n");
}

void print_truncated_str(const char* prefix, charArray* str, int max_display) {
    if (!str || !str->list) return;
    int len = str->length;
    if (len <= max_display) {
        printf("  \033[36m➤\033[0m \033[90m%s:\033[0m \033[33m%s\033[0m\n", prefix, str->list);
    } else {
        int half = max_display / 2;
        printf("  \033[36m➤\033[0m \033[90m%s:\033[0m \033[33m", prefix);
        for (int i = 0; i < half; i++) printf("%c", str->list[i]);
        printf("\033[1;30m...[+%d digits]...\033[0m\033[33m", len - 2 * half);
        for (int i = len - half; i < len; i++) printf("%c", str->list[i]);
        printf("\033[0m \033[35m(len=%d)\033[0m\n", len);
    }
}
