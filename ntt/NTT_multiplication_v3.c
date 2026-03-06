#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#define PRIME 18446744069414584321ULL
#define PRIM_ROOT_G 7
#define N 8388608

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
int bitrev(int num, int bits_num);
void swap_longlong(unsigned long long* a, unsigned long long* b);
unsigned long long next2pow(unsigned long long deg);
long long modinv(int base, unsigned long long mod);
unsigned long long modinv_fermat(unsigned long long base, unsigned long long mod);
unsigned long long modpow(unsigned long long base, unsigned long long power, unsigned long long mod);
unsigned long long* get_coffs_NTT_base(charArray* num, int n, int chunk_length);
unsigned long long* nthRoots_NTT(int n, unsigned long long g, unsigned long long p);
void nthRoots_NTT_inv(unsigned long long* result_buffer, int n, unsigned long long g_inv, unsigned long long p);
void eval_NTT(unsigned long long* Xs, unsigned long long* coffs, int n, unsigned long long p);
void pointwise_mult_NTT(unsigned long long* result_buffer, unsigned long long* eval1, unsigned long long* eval2, int n, unsigned long long p);
void eval_INTT(unsigned long long* Xs_inv, unsigned long long* NTT_result, int n, unsigned long long p);
double get_mantissa(charArray* num);
charArray* get_final_result_NTT_base(unsigned long long* INTT_result, int n, int result_length, int chunk_length);
charArray* mult_NTT_base(charArray* num1, charArray* num2, int chunk_length);
charArray* mypow_NTT_base(charArray* num, int power, int chunk_length);
intArray* sieve_primes(int n);
intArray* get_primes_exps(intArray* primes, int n);
strArray* get_powered_primes_base(intArray* primes, intArray* primesExps, int chunk_length);
charArray* mulpowprimes_base(strArray* powered_primes, int start, int end, int chunk_length);
charArray* factorial_base(int n, int chunk_length);
charArray* getStr(const char * str);
void print_int_array(char* name, int* array, int len);
void print_longlong_array(char* name, unsigned long long* array, int len);

int main(int argc, char *argv[]) {
    int chunk_length = 5;

    charArray* num1 = getStr("32423084730840000");
    charArray* num2 = getStr("98794984989159419919495213");
    charArray* product = mult_NTT_base(num1, num2, chunk_length);
    printf("%s * %s = %s\n", num1->list, num2->list, product->list);

    int power = 487;
    charArray* pow_result = mypow_NTT_base(num2, power, chunk_length);
    printf("%s ^ %d = %s\n", num2->list, power, pow_result->list);

    int n = 100000;
	clock_t start = clock();
    charArray* n_fact = factorial_base(n, chunk_length);
	clock_t end = clock();
	double secs = (double) (end - start) / CLOCKS_PER_SEC;
    printf("%d! = %s\n", n, n_fact->list);
	printf("It took time = %.3lf seconds to calculate %d!\n", secs, n);

    freecharArray(num1);
    freecharArray(num2);
    freecharArray(product);
    freecharArray(pow_result);
    freecharArray(n_fact);
}

charArray* create_long_num(int len, char digit) {
    char* num = (char*)malloc(len + 1);
	if (!num) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
    if (num == NULL) exit(1);
    memset(num, digit, len);
    num[len] = '\0';
	charArray* numArray = (charArray*)malloc(sizeof(charArray));
	numArray->list = num;
	numArray->length = len;
    return numArray;
}

void freeintArray(intArray* arr) {
	free(arr->list);
	free(arr);
}

void freecharArray(charArray* arr) {
	free(arr->list);
	free(arr);
}

void freestrArray(strArray* arr) {
	for (int i = 0; i < arr->length; i++) {
		freecharArray(arr->list[i]);
	}
	free(arr->list);
	free(arr);
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

void swap_longlong(unsigned long long* a, unsigned long long* b) {
	unsigned long long temp = *a;
	*a = *b;
	*b = temp;
}

unsigned long long next2pow(unsigned long long deg) {
	int power = (int)ceil(log2(deg));
	unsigned long long result = (unsigned long long)pow(2, power);
	return result;
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
		if (small == 0)
			break;

		long long s = s_big - q * s_small;
		long long t = t_big - q * t_small;
		s_big = s_small; t_big = t_small;
		s_small = s; t_small = t;
	}
	if (s_small * mod + t_small * base != 1) {
		return 0; //indicates that no modular inverse exists
	}
	return (t_small + mod) % mod;
}

unsigned long long modinv_fermat(unsigned long long base, unsigned long long mod) {
	return modpow(base, mod - 2, mod);
}

unsigned long long modpow(unsigned long long base, unsigned long long power, unsigned long long mod) {
	base %= mod;
	unsigned long long result = 1;
	while (power > 0) {
		if (power % 2 == 1) {
			__uint128_t product = (__uint128_t)result * base;
			result = product % mod;
		}
		__uint128_t square = (__uint128_t)base * base;	
		base = square % mod;
		power /= 2;
	}
	return result;
}

unsigned long long* get_coffs_NTT_base(charArray* num, int n, int chunk_length) {
	unsigned long long* result = (unsigned long long*)malloc(n * sizeof(unsigned long long));
	int len = num->length;
	int index = 0;
	int i = len - 1;
	while (i >= 0) {
		unsigned long long count = 0;
		unsigned long long temp_num = 0;
		while (count < chunk_length && i >= 0) {
			int digit = num->list[i] - '0';
			temp_num += (unsigned long long)(pow(10, count) + 0.5) * digit;
			i--;
			count++;
		}
		result[index] = temp_num;
		index++;
	}
	while (index < n) {
		result[index] = 0;
		index++;
	}

	return result;
}

unsigned long long* nthRoots_NTT(int n, unsigned long long g, unsigned long long p) {
	unsigned long long* result = (unsigned long long*)malloc(n * sizeof(unsigned long long));
	for (int i = 0; i < n; i++) {
		result[i] = modpow(g, i, p);
	}
	return result;
}

void nthRoots_NTT_inv(unsigned long long* result_buffer, int n, unsigned long long g_inv, unsigned long long p) {
	for (int i = 0; i < n; i++) {
		result_buffer[i] = modpow(g_inv, i, p);
	}
}

void eval_NTT(unsigned long long* Xs, unsigned long long* coffs, int n, unsigned long long p) {
	int bits_num = (int)log2(n);
	for (int i = 0; i < n; i++) {
		int i_rev = bitrev(i, bits_num);
		if (i < i_rev) {
			swap_longlong(coffs + i, coffs + i_rev);
		}
	}

//	print_longlong_array("coffs_scrambled", coffs, n); //debugging

	for (int size = 2; size <= n; size *= 2) {
		for (int i = 0; i < n; i+= size) {
			for (int j = i; j < i + size/2; j++) {
				unsigned long long even = coffs[j];
				__uint128_t product = (__uint128_t) Xs[(j - i) * (n / size)] * (__uint128_t)coffs[j + size/2];
				unsigned long long twiddle_factor = product % p;
				if (even >= p - twiddle_factor) {
					coffs[j] = even + twiddle_factor - p;
				}
				else {
					coffs[j] = 	even + twiddle_factor;
				}
				if (even >= twiddle_factor) {
					coffs[j + size/2] = (even - twiddle_factor) % p;
				}
				else {
					__uint128_t temp = (__uint128_t) even + p - twiddle_factor;
					coffs[j + size/2] = temp % p;
				}
			}
		}
	}
}

void pointwise_mult_NTT(unsigned long long* result_buffer, unsigned long long* eval1, unsigned long long* eval2, int n, unsigned long long p) {
	for (int i = 0; i < n; i++) {
		__uint128_t product = (__uint128_t)eval1[i] * (__uint128_t)eval2[i];
		result_buffer[i] = product % p;
	}
}

void eval_INTT(unsigned long long* Xs_inv, unsigned long long* NTT_result, int n, unsigned long long p) {
	int bits_num = (int)log2(n);
	for (int i = 0; i < n; i++) {
		int i_rev = bitrev(i, bits_num);
		if (i < i_rev) {
			swap_longlong(NTT_result + i, NTT_result + i_rev);
		}
	}

//	print_longlong_array("NTT_result_scrambled", NTT_result, n); //debugging

	for (int size = 2; size <= n; size *= 2) {
		for (int i = 0; i < n; i+= size) {
			for (int j = i; j < i + size/2; j++) {
				unsigned long long even = NTT_result[j];
				__uint128_t product = (__uint128_t)Xs_inv[(j - i) * (n / size)] * (__uint128_t)NTT_result[j + size/2];
				unsigned long long twiddle_factor = product % p;
				if (even >= p - twiddle_factor) {
					NTT_result[j] = even + twiddle_factor - p;
				}
				else {
					NTT_result[j] = even + twiddle_factor;
				}
				if (even >= twiddle_factor) {
					NTT_result[j + size/2] = (even - twiddle_factor) % p;
				}
				else {
					__uint128_t temp = (__uint128_t) even + p - twiddle_factor;
					NTT_result[j + size/2] = temp % p;
				}
			}
		}
	}
}

double get_mantissa(charArray* num) {
	int len = num->length;
	double mantissa = 0;
	for (int i = 0; i < len && i < 15; i++) {
		mantissa += pow(10, -i) * (num->list[i] - '0');
	}
	return mantissa;
}

charArray* get_final_result_NTT_base(unsigned long long* INTT_result, int n, int result_length, int chunk_length) {
	char* result = (char*)malloc((result_length + 1) * sizeof(char));
	result[result_length] = '\0';
	int index = result_length - 1;
	unsigned long long carry = 0;
	unsigned long long digits_length = chunk_length;
	unsigned long long decimal_power_base = (unsigned long long)(pow(10, chunk_length) + 0.5);
	for (int i = 0; i < n; i++) {
		unsigned long long sum = INTT_result[i] + carry;
		unsigned long long digit = sum % decimal_power_base;
		carry = sum / decimal_power_base;
//		printf("%dth %lld digits = %lld\n", i, digits_length, digit); //debugging
		int count = 0;
		while ((digit > 0 || (count < digits_length && (i < n - 1 || carry > 0))) && index >= 0) {
			result[index] = digit % 10 + '0';
			index--;
			digit /= 10;
			count++;
		}
	}
//	printf("carry = %llu\n", carry);
	while (index >= 0) {
		result[index] = carry % 10 + '0';
		index--;
		carry /= 10;
	}
	charArray* resultArr = (charArray*)malloc(sizeof(charArray));
	resultArr->list = result;
	resultArr->length = result_length;
	return resultArr;
}

charArray* mult_NTT_base(charArray* num1, charArray* num2, int chunk_length) {

	// unsigned long long base = (unsigned long long)(pow(10, chunk_length) + 0.5);

	int len1 = num1->length, len2 = num2->length;

	double mantissa1 = get_mantissa(num1);
	double mantissa2 = get_mantissa(num2);

//	printf("mantissa1 = %lf, mantissa2 = %lf\n", mantissa1, mantissa2);

	int estimated_result_length = (int)(log10(mantissa1) + (len1 - 1) + log10(mantissa2) + (len2 - 1)) + 1;

//	printf("result estimated length = %d\n", estimated_result_length);

	int chunks_len1 = (len1 % chunk_length == 0) ? len1 / chunk_length : len1 / chunk_length + 1;
	int chunks_len2 = (len2 % chunk_length == 0) ? len2 / chunk_length : len2 / chunk_length + 1;

//	printf("length of coffs1 = %d\n", chunks_len1);
//	printf("length of coffs2 = %d\n", chunks_len2);

	int deg = chunks_len1 + chunks_len2 - 1;
	int n = next2pow(deg);
//	printf("n = %d\n", n);
	unsigned long long p = PRIME;
	unsigned long long g = modpow(PRIM_ROOT_G, (p-1)/n, p);
//	printf("p = %llu\ng = %llu\n", p, g);

	unsigned long long* coffs1 = get_coffs_NTT_base(num1, n, chunk_length);
	unsigned long long* coffs2 = get_coffs_NTT_base(num2, n, chunk_length);

//	print_longlong_array("coffs1", coffs1, n);
//	print_longlong_array("coffs2", coffs2, n);

	unsigned long long* Xs = nthRoots_NTT(n, g, p);

//	print_longlong_array("Xs", Xs, n);

	eval_NTT(Xs, coffs1, n, p);
	eval_NTT(Xs, coffs2, n, p);
	unsigned long long* eval1 = coffs1;
	unsigned long long* eval2 = coffs2;

//	print_longlong_array("eval1", eval1, n); //debugging
//	print_longlong_array("eval2", eval2, n); //debugging

	pointwise_mult_NTT(eval1, eval1, eval2, n, p);
	free(eval2);
	unsigned long long* mult_result = eval1;

//	print_longlong_array("mult_result", mult_result, n);

	unsigned long long g_inv = modinv_fermat(g, p);

//	printf("g_inv = %llu\n", g_inv);

	nthRoots_NTT_inv(Xs, n, g_inv, p);
	unsigned long long* Xs_inv = Xs;

//	print_longlong_array("Xs_inv", Xs_inv, n);

	eval_INTT(Xs_inv, mult_result, n, p);
	free(Xs_inv);
	unsigned long long* INTT_eval = mult_result;

//	print_longlong_array("unscaled_INTT_eval", INTT_eval, n);

	unsigned long long n_inv = modinv_fermat(n, p);

//	printf("n_inv = %llu\n", n_inv);

	for (int i = 0; i< n; i++) {
		__uint128_t product = (__uint128_t) INTT_eval[i] * n_inv;
		INTT_eval[i] = product % p;
	}

//	print_longlong_array("scaled_INTT_eval", INTT_eval, n);

	charArray* final_result = get_final_result_NTT_base(INTT_eval, n, estimated_result_length, chunk_length);
	free(INTT_eval);

	return final_result;
}

charArray* mypow_NTT_base(charArray* num, int power, int chunk_length) {
	charArray* result = getStr("1");
	charArray* Num = getStr(num->list);
	while (power > 0) {
		if ((power & 1)) { // odd
			charArray* temp_result = mult_NTT_base(result, Num, chunk_length);
			freecharArray(result);
			result = temp_result;
			power--;
		}
		else { // even
			charArray* temp_Num = mult_NTT_base(Num, Num, chunk_length);
			freecharArray(Num);
			Num = temp_Num;
			power /= 2;
		}
	}
	freecharArray(Num);
	return result;
}

intArray* sieve_primes(int n) {
	bool isprime[n + 1];
	for (int i = 0; i < n + 1; i++) {
		isprime[i] = true;
	}
	int size = (int)(n / log(n));
	int* primes = (int*)malloc(size*sizeof(int));
	if (!primes) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	if (primes == NULL) {
		printf("Error : not enough space for primes\n");
		exit(EXIT_FAILURE);
	}
	int index = 0;
	for (int num = 2; num <= n; num++) {
		if (isprime[num]) {
			if (index < size) {
				primes[index] = num;
				index++;
			}
			else {
				size += 5;
				primes = (int*)realloc(primes, size*sizeof(int));
				if (primes == NULL) {
			printf("Error : not enough space for primes\n");
			exit(EXIT_FAILURE);
		}
				primes[index] = num;
				index++;
			}
			unsigned long long multiple = (unsigned long long) num * (unsigned long long) num;
			if (multiple <= n) {
				for (int mult = (int)multiple; mult <= n; mult += num) {
					isprime[mult] = false;
				}
			}
		}
	}
	intArray* result = (intArray*)malloc(sizeof(intArray));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result->list = primes;
	result->length = index;
	return result;
}

intArray* get_primes_exps(intArray* primes, int n) {
	int* exps = (int*)malloc(primes->length*sizeof(int));
	if (!exps) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	for (int i = 0; i < primes->length; i++) {
		int prime_count = n / primes->list[i];
		int prime_counter = 0;
		int trialExp = 1;
		while (prime_count > 0) {
			prime_counter += prime_count;
			trialExp++;
			prime_count = n / (int)(pow(primes->list[i], trialExp));
		}
		exps[i] = prime_counter;
	}
	intArray* result = (intArray*)malloc(sizeof(intArray));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result->list = exps;
	result->length = primes->length;
	return result;
}

strArray* get_powered_primes_base(intArray* primes, intArray* primesExps, int chunk_length) {
	int len = primes->length;
	charArray** powedPrimes = (charArray**)malloc(len*sizeof(charArray*));
	if (!powedPrimes) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	charArray* prime = (charArray*)malloc(sizeof(charArray));
	prime->list = (char*)malloc((primesExps->list[0] + 1) * sizeof(char));
	for (int i = 0; i < len; i++) {
		sprintf(prime->list, "%d", primes->list[i]);
		int prime_length = ((int)log10(primes->list[i])) + 1;
		prime->length = prime_length;

		charArray* primepow = mypow_NTT_base(prime, primesExps->list[i], chunk_length);
		powedPrimes[i] = primepow;
	}
	freecharArray(prime);
	strArray* poweredPrimesArr = (strArray*)malloc(sizeof(strArray));
	if (!poweredPrimesArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	poweredPrimesArr->list = powedPrimes;
	poweredPrimesArr->length = len;
	return poweredPrimesArr;
}

charArray* mulpowprimes_base(strArray* powered_primes, int start, int end, int chunk_length) {
	int len = end - start;
	if (len == 1) {
		charArray* num = getStr(powered_primes->list[start]->list);
		return num;
	}
	else if (len == 2) {
		charArray* num1 = powered_primes->list[start];
		charArray* num2 = powered_primes->list[start + 1];
		charArray* result = mult_NTT_base(num1, num2, chunk_length);
		return result;
	}
	int mid = start + (end - start) / 2;
	charArray* left = mulpowprimes_base(powered_primes, start, mid, chunk_length);
	charArray* right = mulpowprimes_base(powered_primes, mid, end, chunk_length);
	charArray* result = mult_NTT_base(left, right, chunk_length);
	freecharArray(left);
	freecharArray(right);
	return result;
}

charArray* factorial_base(int n, int chunk_length) {
	intArray* primes = sieve_primes(n);

	// print_int_array("primes", primes->list, primes->length); //debugging

	intArray* primes_exps = get_primes_exps(primes, n);

	// print_int_array("primes' powers", primes_exps->list, primes_exps->length); //debugging

	strArray* powered_primes = get_powered_primes_base(primes, primes_exps, chunk_length);

	freeintArray(primes);
	freeintArray(primes_exps);
	// for (int i = 0; i < powered_primes->length; i++) { //debugging
	// printf("%d ^ %d = %s\n", primes->list[i], primes_exps->list[i], powered_primes->list[i]->list); //debugging
	// } //debugging

	charArray* factorial_result = mulpowprimes_base(powered_primes, 0, powered_primes->length, chunk_length);

	freestrArray(powered_primes);

	return factorial_result;
}

charArray* getStr(const char * str) {
	int len = strlen(str);
	char *result = (char*)malloc((len + 1)*sizeof(char));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result[len] = '\0';
	for (int i = 0; i < len; i++) {
		result[i] = str[i];
	}
	charArray* resultArr = (charArray*)malloc(sizeof(charArray));
	if (!resultArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	resultArr->list = result;
	resultArr->length = len;
	return resultArr;
}

void print_int_array(char* name, int* array, int len) {
	printf("%s = [", name);
	for (int i = 0; i < len; i++) {
		printf("%d", array[i]);
		if (i < len - 1)
			printf(", ");
	}
	printf("]\n");
}

void print_longlong_array(char* name, unsigned long long* array, int len) {
	printf("%s = [", name);
	for (int i = 0; i < len; i++) {
		printf("%llu", array[i]);
		if (i < len - 1)
			printf(", ");
	}
	printf("]\n");
}