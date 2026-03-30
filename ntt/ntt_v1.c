#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include "../utils/utils.h"

#define PRIME 850403524609
#define PRIM_ROOT_G 13
#define N 2147483648

unsigned long long* get_coffs_NTT(charArray* num, int n);
unsigned long long* nthRoots_NTT(int n, unsigned long long g, unsigned long long p);
void nthRoots_NTT_inv(unsigned long long* result_buffer, int n, unsigned long long g_inv, unsigned long long p);
void eval_NTT(unsigned long long* Xs, unsigned long long* coffs, int n, unsigned long long p);
void pointwise_mult_NTT(unsigned long long* result_buffer, unsigned long long* eval1, unsigned long long* eval2, int n, unsigned long long p);
void eval_INTT(unsigned long long* Xs_inv, unsigned long long* NTT_result, int n, unsigned long long p);
charArray* get_final_result_NTT(unsigned long long* INTT_result, int n, int len1, int len2);
charArray* mult_NTT(charArray* num1, charArray* num2);
charArray* mypow_NTT(charArray* num, int power);
intArray* sieve_primes(int n);
intArray* get_primes_exps(intArray* primes, int n);
strArray* get_powered_primes(intArray* primes, intArray* primesExps);
charArray* mulpowprimes(strArray* powered_primes, int start, int end);
charArray* factorial(int n);

int main(int argc, char *argv[]) {
	const char *s1 = NULL, *s2 = NULL, *pow_base = NULL;
	int fact_n = 0, pow_exp = 0;
	DEBUG_OP = DBG_OP_MULT;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-d1") == 0) { DEBUG = 1; continue; }
		if (strcmp(argv[i], "-d2") == 0) { DEBUG = 2; continue; }
		if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) { fact_n = atoi(argv[++i]); DEBUG_OP = DBG_OP_FACT; continue; }
		if (strcmp(argv[i], "-p") == 0 && i + 2 < argc) { pow_base = argv[++i]; pow_exp = atoi(argv[++i]); DEBUG_OP = DBG_OP_POW; continue; }
		if (!s1) s1 = argv[i]; else s2 = argv[i];
	}
	if (fact_n > 0) {
		clock_t start = clock();
		charArray* n_fact = factorial(fact_n);
		clock_t end = clock();
		double secs = (double)(end - start)/CLOCKS_PER_SEC;
		printf("%d! = %s\n", fact_n, n_fact->list);
		printf("It took time = %.3lf seconds to calculate %d!\n", secs, fact_n);
		freecharArray(n_fact);
	} else if (pow_base) {
		charArray* num = getStr(pow_base);
		charArray* pow_result = mypow_NTT(num, pow_exp);
		printf("%s ^ %d = %s\n", num->list, pow_exp, pow_result->list);
		freecharArray(num); freecharArray(pow_result);
	} else {
		if (!s1) { s1 = "5474564654667674"; s2 = "4454353453"; }
		if (!s2) { printf("Usage: %s <num1> <num2> [-d] | -f <N> | -p <base> <exp> [-d]\n", argv[0]); return 1; }
		charArray* num1 = getStr(s1);
		charArray* num2 = getStr(s2);
		charArray* product = mult_NTT(num1, num2);
		printf("%s * %s = %s\n", num1->list, num2->list, product->list);
		freecharArray(num1); freecharArray(num2); freecharArray(product);
	}
}

unsigned long long* get_coffs_NTT(charArray* num, int n) {
	int len = num->length;
	unsigned long long* result = (unsigned long long*)malloc(n * sizeof(unsigned long long));
	int index = 0;
	for (int i = len - 1; i >= 0; i--) {
		result[index] = num->list[i] - '0';
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

	if (DEBUG >= 2) print_longlong_array("coffs_scrambled", coffs, n);

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

	if (DEBUG >= 2) print_longlong_array("NTT_result_scrambled", NTT_result, n);

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

charArray* get_final_result_NTT(unsigned long long* INTT_result, int n, int len1, int len2) {
	int len = len1 + len2;
	char* result = (char*)malloc((len + 1) * sizeof(char));
	char* result_small = (char*)malloc((len) * sizeof(char));
	result[len] = '\0';
	result_small[len - 1] = '\0';
	unsigned long long carry = 0;
	for (int i = 0; i < n && i < (len - 1); i++) {
		unsigned long long sum = INTT_result[i] + carry;
		int digit = sum % 10;
		carry = sum / 10;
		result[len - i - 1] = digit + '0';
		result_small[len - i - 2] = digit + '0';
	}
	charArray* resultArr = (charArray*)malloc(sizeof(charArray));
	if (carry > 0) { //returning result
		free(result_small);
		result[0] = carry + '0';
		resultArr->list = result;
		resultArr->length = len;
	}
	else { //returning result_small
		free(result);
		resultArr->list = result_small;
		resultArr->length = len - 1;
	}
	return resultArr;
}

charArray* mult_NTT(charArray* num1, charArray* num2) {
	if ((num1->length == 1 && num1->list[0] == '0') || (num2->length == 1 && num2->list[0] == '0')) {
		return getStr("0");
	}
	int len1 = num1->length;
	int len2 = num2->length;
	int deg = len1 + len2 - 1;
	unsigned long long n = next2pow(deg);
	unsigned long long p = PRIME;
	unsigned long long g = modpow(PRIM_ROOT_G, (p-1)/n, p);

	DBG_HEADER("NTT MULTIPLICATION");
	DBG_VAL("num1 length", "%d digits", len1);
	DBG_VAL("num2 length", "%d digits", len2);
	DBG_VAL("polynomial degree", "%d", deg);
	DBG_VAL("padded size (n)", "%llu", n);
	DBG_VAL("prime (p)", "%llu", p);
	DBG_VAL("primitive root (g)", "%llu", g);
	DBG_SEP();

	DBG_TIME_START();
	unsigned long long* coffs1 = get_coffs_NTT(num1, n);
	unsigned long long* coffs2 = get_coffs_NTT(num2, n);
	DBG_TIME_END("coefficients");

	if (DEBUG >= 2) print_longlong_array("coffs1", coffs1, n);
	if (DEBUG >= 2) print_longlong_array("coffs2", coffs2, n);

	unsigned long long* Xs = nthRoots_NTT(n, g, p);

	if (DEBUG >= 2) print_longlong_array("Xs", Xs, n);

	DBG_TIME_START();
	eval_NTT(Xs, coffs1, n, p);
	eval_NTT(Xs, coffs2, n, p);
	DBG_TIME_END("forward NTT");
	unsigned long long* eval1 = coffs1;
	unsigned long long* eval2 = coffs2;

	if (DEBUG >= 2) print_longlong_array("eval1", eval1, n);
	if (DEBUG >= 2) print_longlong_array("eval2", eval2, n);

	DBG_TIME_START();
	pointwise_mult_NTT(eval1, eval1, eval2, n, p);
	DBG_TIME_END("pointwise multiply");
	unsigned long long* mult_result = eval1;
	free(eval2);

	if (DEBUG >= 2) print_longlong_array("mult_result", mult_result, n);

	unsigned long long g_inv = modinv_fermat(g, p);
	DBG_VAL("g_inv", "%llu", g_inv);

	nthRoots_NTT_inv(Xs, n, g_inv, p);
	unsigned long long* Xs_inv = Xs;

	if (DEBUG >= 2) print_longlong_array("Xs_inv", Xs_inv, n);

	DBG_TIME_START();
	eval_INTT(Xs_inv, mult_result, n, p);
	unsigned long long* INTT_eval = mult_result;
	free(Xs_inv);
	DBG_TIME_END("inverse NTT");

	if (DEBUG >= 2) print_longlong_array("unscaled_INTT_eval", INTT_eval, n);

	unsigned long long n_inv = modinv_fermat(n, p);
	DBG_VAL("n_inv", "%llu", n_inv);
	for (int i = 0; i< n; i++) {
		__uint128_t product = (__uint128_t) INTT_eval[i] * n_inv;
		INTT_eval[i] = (product) % p;
	}

	if (DEBUG >= 2) print_longlong_array("scaled_INTT_eval", INTT_eval, n);

	charArray* final_result = get_final_result_NTT(INTT_eval, n, len1, len2);
	free(INTT_eval);

	DBG_SEP();
	DBG_VAL("result length", "%d digits", final_result->length);

	return final_result;
}

charArray* mypow_NTT(charArray* num, int power) {
	DBG_POW_HEADER("EXPONENTIATION NTT");
	charArray* result = getStr("1");
	charArray* Num = getStr(num->list);
	while (power > 0) {
		if ((power & 1)) { // odd
			charArray* temp_result = mult_NTT(result, Num);
			freecharArray(result);
			result = temp_result;
			power--;
		}
		else { // even
			charArray* temp_Num = mult_NTT(Num, Num);
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

strArray* get_powered_primes(intArray* primes, intArray* primesExps) {
	int len = primes->length;
	charArray** powedPrimes = (charArray**)malloc(len*sizeof(charArray*));
	if (!powedPrimes) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	charArray* prime = (charArray*)malloc(sizeof(charArray));
	prime->list = (char*)malloc((primesExps->list[0] + 1) * sizeof(char));
	for (int i = 0; i < len; i++) {
		sprintf(prime->list, "%d", primes->list[i]);
		int prime_length = ((int)log10(primes->list[i])) + 1;
		prime->length = prime_length;

		charArray* primepow = mypow_NTT(prime, primesExps->list[i]);
		powedPrimes[i] = primepow;
	}
	freecharArray(prime);
	strArray* poweredPrimesArr = (strArray*)malloc(sizeof(strArray));
	if (!poweredPrimesArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	poweredPrimesArr->list = powedPrimes;
	poweredPrimesArr->length = len;
	return poweredPrimesArr;
}

charArray* mulpowprimes(strArray* powered_primes, int start, int end) {
	int len = end - start;
	if (len == 1) {
		charArray* num = getStr(powered_primes->list[start]->list);
		return num;
	}
	else if (len == 2) {
		charArray* num1 = powered_primes->list[start];
		charArray* num2 = powered_primes->list[start + 1];
		charArray* result = mult_NTT(num1, num2);
		return result;
	}
	int mid = start + (end - start) / 2;
	charArray* left = mulpowprimes(powered_primes, start, mid);
	charArray* right = mulpowprimes(powered_primes, mid, end);
	charArray* result = mult_NTT(left, right);
	freecharArray(left);
	freecharArray(right);
	return result;
}

charArray* factorial(int n) {
	intArray* primes = sieve_primes(n);

	if (DEBUG >= 2) print_int_array("primes", primes->list, primes->length);

	intArray* primes_exps = get_primes_exps(primes, n);

	if (DEBUG >= 2) print_int_array("primes' powers", primes_exps->list, primes_exps->length);

	strArray* powered_primes = get_powered_primes(primes, primes_exps);

	if (DEBUG >= 1 && DEBUG_OP == DBG_OP_FACT) {
		DBG_FACT_HEADER("PRIME POWER RESULTS");
		for (int i = 0; i < powered_primes->length; i++) {
			char prefix[64];
			sprintf(prefix, "%d ^ %d", primes->list[i], primes_exps->list[i]);
			print_truncated_str(prefix, powered_primes->list[i], 30);
		}
		DBG_FACT_HEADER("DIVIDE AND CONQUER TREE");
	}

	freeintArray(primes);
	freeintArray(primes_exps);

	charArray* factorial_result = mulpowprimes(powered_primes, 0, powered_primes->length);

	freestrArray(powered_primes);

	return factorial_result;
}

