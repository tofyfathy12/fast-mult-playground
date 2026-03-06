#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <pthread.h>
#include <time.h>
#define PRIME 18446744069414584321ULL
#define PRIM_ROOT_G 7
#define N 8388608

typedef struct arr
{
	int *list;
	int length;
} intArray;

typedef struct arr2
{
	char *list;
	int length;
} charArray;

typedef struct arr3
{
	charArray **list;
	int length;
} strArray;

charArray *create_long_num(int len, char digit);
void freeintArray(intArray *arr);
void freecharArray(charArray *arr);
void freestrArray(strArray *arr);
int bitrev(int num, int bits_num);
void swap_int(int *a, int *b);
void swap_longlong(unsigned long long *a, unsigned long long *b);
unsigned long long next2pow(unsigned long long deg);
long long modinv(int base, unsigned long long mod);
unsigned long long modinv_fermat(unsigned long long base, unsigned long long mod);
unsigned long long modpow(unsigned long long base, unsigned long long power, unsigned long long mod);
void get_coffs_NTT_fast_base(unsigned long long *coffs_buffer, charArray *num, int n, int chunk_length);
void nthRoots_NTT_fast(unsigned long long *result_buffer, int n, unsigned long long g, unsigned long long p);
void nthRoots_NTT_inv(unsigned long long *result_buffer, int n, unsigned long long g_inv, unsigned long long p);
void eval_NTT(unsigned long long *Xs, unsigned long long *coffs, int n, unsigned long long p);
void pointwise_mult_NTT(unsigned long long *result_buffer, unsigned long long *eval1, unsigned long long *eval2, int n, unsigned long long p);
void eval_INTT(unsigned long long *Xs_inv, unsigned long long *NTT_result, int n, unsigned long long p);
double get_mantissa(charArray *num);
void get_final_result_NTT_base_fast(charArray *result_buffer, unsigned long long *INTT_result, int n, int result_length, int chunk_length);
charArray *mult_NTT_fast_base(charArray *num1, charArray *num2, int chunk_length);
void mult_fast_base(charArray *result_buffer, unsigned long long *coffs_buffer, int coffs_buffer_len, charArray *num1, charArray *num2, int pre_n, int chunk_length);
charArray *mypow_NTT_base_fast(charArray *num, int power, int chunk_length);
void power_base_fast(charArray *result_buffer, charArray *scratch_space, unsigned long long *coffs_scratch_space, int coffs_scratch_len, charArray *num, int power, int chunk_length);
intArray *sieve_primes(int n);
intArray *get_primes_exps(intArray *primes, int n);
strArray *get_powered_primes_fast_base(intArray *primes, intArray *primesExps, int *prefix_digits_sum, int chunk_length);
void mulpowprimes_fast_base(charArray *result_buffer, unsigned long long *coffs_buffer, int coffs_buffer_len, strArray *powered_primes, int start, int end, int *prefix_digits_sum, int chunk_length);
size_t compute_result_space_factorial(int start, int end, int *prefix_digits_sum);
charArray *factorial_base_fast(int n, int chunk_length);
charArray *getStr(const char *str);
void print_int_array(char *name, int *array, int len);
void print_longlong_array(char *name, unsigned long long *array, int len);
void remlzeros(charArray *num);

int main(int argc, char *argv[])
{
	unsigned long long chunk_length = 5; // base = 100,000

	charArray *num1 = getStr("923456789329480");
	charArray *num2 = getStr("23894732984703");
	charArray *product = mult_NTT_fast_base(num1, num2, chunk_length);
	printf("%s * %s = %s\n", num1->list, num2->list, product->list);
	freecharArray(product);

	int power = 11;
	charArray *pow_result_base = mypow_NTT_base_fast(num1, power, chunk_length);
	printf("%s ^ %d = %s\n", num1->list, power, pow_result_base->list);
	freecharArray(pow_result_base);
	freecharArray(num1);
	freecharArray(num2);

	int n = 100000;
	clock_t start = clock();
	charArray *n_fact = factorial_base_fast(n, chunk_length);
	clock_t end = clock();
	double secs = (double)(end - start) / CLOCKS_PER_SEC;

	printf("%d! = %s\n", n, n_fact->list);
	printf("It took time = %.3lf seconds to calculate %d!\n", secs, n);
	freecharArray(n_fact);
}

charArray *create_long_num(int len, char digit)
{
	char *num = (char *)malloc(len + 1);
	if (!num)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	if (num == NULL)
		exit(1);
	memset(num, digit, len);
	num[len] = '\0';
	charArray *numArray = (charArray *)malloc(sizeof(charArray));
	numArray->list = num;
	numArray->length = len;
	return numArray;
}

void freeintArray(intArray *arr)
{
	free(arr->list);
	free(arr);
}

void freecharArray(charArray *arr)
{
	free(arr->list);
	free(arr);
}

void freestrArray(strArray *arr)
{
	for (int i = 0; i < arr->length; i++)
	{
		freecharArray(arr->list[i]);
	}
	free(arr->list);
	free(arr);
}

int bitrev(int num, int bits_num)
{
	int result = 0;
	for (int i = 0; i < bits_num; i++)
	{
		if ((num & (1 << i)))
		{
			result |= (1 << (bits_num - 1 - i));
		}
	}
	return result;
}

void swap_int(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void swap_longlong(unsigned long long *a, unsigned long long *b)
{
	unsigned long long temp = *a;
	*a = *b;
	*b = temp;
}

unsigned long long next2pow(unsigned long long deg)
{
	int power = (int)ceil(log2(deg));
	unsigned long long result = (unsigned long long)pow(2, power);
	return result;
}

long long modinv(int base, unsigned long long mod)
{
	unsigned long long big = mod;
	long long small = (long long)base;

	long long s_big = 1, t_big = 0;
	long long s_small = 0, t_small = 1;
	while (small > 0)
	{
		long long q = (long long)(big / small);
		long long temp_small = big % small;
		big = small;
		small = temp_small;
		if (small == 0)
			break;

		long long s = s_big - q * s_small;
		long long t = t_big - q * t_small;
		s_big = s_small;
		t_big = t_small;
		s_small = s;
		t_small = t;
	}
	if (s_small * mod + t_small * base != 1)
	{
		return 0; // indicates that no modular inverse exists
	}
	return (t_small + mod) % mod;
}

unsigned long long modinv_fermat(unsigned long long base, unsigned long long mod)
{
	return modpow(base, mod - 2, mod);
}

unsigned long long modpow(unsigned long long base, unsigned long long power, unsigned long long mod)
{
	base %= mod;
	unsigned long long result = 1;
	while (power > 0)
	{
		if (power % 2 == 1)
		{
			__uint128_t product = (__uint128_t)result * base;
			result = product % mod;
		}
		__uint128_t square = (__uint128_t)base * base;
		base = square % mod;
		power /= 2;
	}
	return result;
}

void get_coffs_NTT_fast_base(unsigned long long *coffs_buffer, charArray *num, int n, int chunk_length)
{
	int len = num->length;
	int index = 0;
	int i = len - 1;
	while (i >= 0)
	{
		unsigned long long count = 0;
		unsigned long long temp_num = 0;
		while (count < chunk_length && i >= 0)
		{
			unsigned long long digit = (unsigned long long)(num->list[i] - '0');
			temp_num += (unsigned long long)(pow(10, count) + 0.5) * digit;
			i--;
			count++;
		}
		coffs_buffer[index] = temp_num;
		index++;
	}
	while (index < n)
	{
		coffs_buffer[index] = 0;
		index++;
	}
}

void nthRoots_NTT_fast(unsigned long long *result_buffer, int n, unsigned long long g, unsigned long long p)
{
	for (int i = 0; i < n; i++)
	{
		result_buffer[i] = modpow(g, i, p);
	}
}

void nthRoots_NTT_inv(unsigned long long *result_buffer, int n, unsigned long long g_inv, unsigned long long p)
{
	for (int i = 0; i < n; i++)
	{
		result_buffer[i] = modpow(g_inv, i, p);
	}
}

void eval_NTT(unsigned long long *Xs, unsigned long long *coffs, int n, unsigned long long p)
{
	int bits_num = (int)log2(n);
	for (int i = 0; i < n; i++)
	{
		int i_rev = bitrev(i, bits_num);
		if (i < i_rev)
		{
			swap_longlong(coffs + i, coffs + i_rev);
		}
	}

	//	print_longlong_array("coffs_scrambled", coffs, n); //debugging

	for (int size = 2; size <= n; size *= 2)
	{
		for (int i = 0; i < n; i += size)
		{
			for (int j = i; j < i + size / 2; j++)
			{
				unsigned long long even = coffs[j];
				__uint128_t product = (__uint128_t)Xs[(j - i) * (n / size)] * (__uint128_t)coffs[j + size / 2];
				unsigned long long twiddle_factor = product % p;
				if (even >= p - twiddle_factor)
				{
					coffs[j] = even + twiddle_factor - p;
				}
				else
				{
					coffs[j] = even + twiddle_factor;
				}
				if (even >= twiddle_factor)
				{
					coffs[j + size / 2] = (even - twiddle_factor) % p;
				}
				else
				{
					__uint128_t temp = (__uint128_t)even + p - twiddle_factor;
					coffs[j + size / 2] = temp % p;
				}
			}
		}
	}
}

void pointwise_mult_NTT(unsigned long long *result_buffer, unsigned long long *eval1, unsigned long long *eval2, int n, unsigned long long p)
{
	for (int i = 0; i < n; i++)
	{
		__uint128_t product = (__uint128_t)eval1[i] * (__uint128_t)eval2[i];
		result_buffer[i] = product % p;
	}
}

void eval_INTT(unsigned long long *Xs_inv, unsigned long long *NTT_result, int n, unsigned long long p)
{
	int bits_num = (int)log2(n);
	for (int i = 0; i < n; i++)
	{
		int i_rev = bitrev(i, bits_num);
		if (i < i_rev)
		{
			swap_longlong(NTT_result + i, NTT_result + i_rev);
		}
	}

	//	print_longlong_array("NTT_result_scrambled", NTT_result, n); //debugging

	for (int size = 2; size <= n; size *= 2)
	{
		for (int i = 0; i < n; i += size)
		{
			for (int j = i; j < i + size / 2; j++)
			{
				unsigned long long even = NTT_result[j];
				__uint128_t product = (__uint128_t)Xs_inv[(j - i) * (n / size)] * (__uint128_t)NTT_result[j + size / 2];
				unsigned long long twiddle_factor = product % p;
				if (even >= p - twiddle_factor)
				{
					NTT_result[j] = even + twiddle_factor - p;
				}
				else
				{
					NTT_result[j] = even + twiddle_factor;
				}
				if (even >= twiddle_factor)
				{
					NTT_result[j + size / 2] = (even - twiddle_factor) % p;
				}
				else
				{
					__uint128_t temp = (__uint128_t)even + p - twiddle_factor;
					NTT_result[j + size / 2] = temp % p;
				}
			}
		}
	}
}

double get_mantissa(charArray *num)
{
	int len = num->length;
	double mantissa = 0;
	for (int i = 0; i < len && i < 15; i++)
	{
		mantissa += pow(10, -i) * (num->list[i] - '0');
	}
	return mantissa;
}

void get_final_result_NTT_base_fast(charArray *result_buffer, unsigned long long *INTT_result, int n, int result_length, int chunk_length)
{
	result_buffer->length = result_length;
	result_buffer->list[result_length] = '\0';
	int index = result_length - 1;
	unsigned long long carry = 0;
	unsigned long long digits_length = chunk_length;
	unsigned long long decimal_power_base = (unsigned long long)(pow(10, chunk_length) + 0.5);
	// print_longlong_array("INTT_result", INTT_result, n); //debugging
	for (int i = 0; i < n; i++)
	{
		unsigned long long sum = INTT_result[i] + carry;
		unsigned long long digit = sum % decimal_power_base;
		carry = sum / decimal_power_base;
		// printf("%dth %lld digits = %lld\n", i, digits_length, digit); //debugging
		int count = 0;
		while ((digit > 0 || (count < (int)digits_length && (i < n - 1 || carry > 0))) && index >= 0)
		{
			result_buffer->list[index] = (char)((digit % 10ULL) + '0');
			index--;
			digit /= 10;
			count++;
		}
		// printf("=============================================\n"); //debugging
	}
	// printf("carry = %llu\n", carry);
	// printf(".\n.\n.\n.\n.\n.\n.\n"); //debugging
	while (index >= 0)
	{
		result_buffer->list[index] = (char)((carry % 10ULL) + '0');
		index--;
		carry /= 10ULL;
	}
	// printf("result before removing leading zeros = %s\n", result_buffer->list); //debugging
	int zero_index = 0;
	while (result_buffer->list[zero_index] == '0')
		zero_index++;
	for (int i = zero_index; i <= result_buffer->length; i++)
	{
		result_buffer->list[i - zero_index] = result_buffer->list[i];
	}
	result_buffer->length -= zero_index;
	// printf("result after removing leading zeros = %s\n", result_buffer->list); //debugging
	// printf("========================================\n"); //debugging
}

charArray *mult_NTT_fast_base(charArray *num1, charArray *num2, int chunk_length)
{

	int len1 = num1->length, len2 = num2->length;

	int chunks_len1 = (len1 % chunk_length == 0) ? len1 / chunk_length : len1 / chunk_length + 1;
	int chunks_len2 = (len2 % chunk_length == 0) ? len2 / chunk_length : len2 / chunk_length + 1;

	//	printf("length of coffs1 = %d\n", chunks_len1);
	//	printf("length of coffs2 = %d\n", chunks_len2);

	int deg = chunks_len1 + chunks_len2 - 1;
	int pre_n = next2pow(deg);
	int coffs_scratch_len = 3 * pre_n;
	unsigned long long *coffs_scratch_space = (unsigned long long *)malloc(sizeof(unsigned long long) * coffs_scratch_len);
	charArray *result = (charArray *)malloc(sizeof(charArray));
	result->list = (char *)malloc((len1 + len2 + 1) * sizeof(char));

	mult_fast_base(result, coffs_scratch_space, coffs_scratch_len, num1, num2, pre_n, chunk_length);
	free(coffs_scratch_space);

	return result;
}

void mult_fast_base(charArray *result_buffer, unsigned long long *coffs_buffer, int coffs_buffer_len, charArray *num1, charArray *num2, int pre_n, int chunk_length)
{
	// coffs_buffer_len is supposed to be 3*pre_n

	int len1 = num1->length, len2 = num2->length;

	double mantissa1 = get_mantissa(num1);
	double mantissa2 = get_mantissa(num2);

	//	printf("mantissa1 = %lf, mantissa2 = %lf\n", mantissa1, mantissa2);

	int estimated_result_length = (int)(log10(mantissa1) + (len1 - 1) + log10(mantissa2) + (len2 - 1)) + 1;

	// printf("result estimated length = %d\n", estimated_result_length); //debugging

	// unsigned long long base = (unsigned long long)(pow(10, chunk_length) + 0.5);

	int n = pre_n;
	unsigned long long p = PRIME;
	unsigned long long g = modpow(PRIM_ROOT_G, (p - 1) / n, p);
	// printf("g = %lld\n", g); //debugging

	// printf("next power of two (n) = %d\n", n); //debugging

	unsigned long long *coffs1 = coffs_buffer;
	unsigned long long *coffs2 = coffs1 + (coffs_buffer_len / 3);
	unsigned long long *Xs = coffs2 + (coffs_buffer_len / 3);

	get_coffs_NTT_fast_base(coffs1, num1, n, chunk_length);
	get_coffs_NTT_fast_base(coffs2, num2, n, chunk_length);

	// print_longlong_array("coffs1", coffs1, n); //debugging
	// print_longlong_array("coffs2", coffs2, n); //debugging

	nthRoots_NTT_fast(Xs, n, g, p);

	// print_longlong_array("Xs", Xs, n); //debugging

	eval_NTT(Xs, coffs1, n, p);
	eval_NTT(Xs, coffs2, n, p);
	unsigned long long *eval1 = coffs1;
	unsigned long long *eval2 = coffs2;

	// print_longlong_array("eval1", eval1, n); //debugging
	// print_longlong_array("eval2", eval2, n); //debugging

	pointwise_mult_NTT(eval1, eval1, eval2, n, p);
	unsigned long long *mult_result = eval1;

	// print_longlong_array("mult_result", mult_result, n); //debugging

	unsigned long long g_inv = modinv_fermat(g, p);

	// printf("g_inv = %lld\n", g_inv); //debugging

	nthRoots_NTT_inv(Xs, n, g_inv, p);
	unsigned long long *Xs_inv = Xs;

	// print_longlong_array("Xs_inv", Xs_inv, n); //debugging

	eval_INTT(Xs_inv, mult_result, n, p);
	unsigned long long *INTT_eval = mult_result;

	// print_longlong_array("unscaled_INTT_eval", INTT_eval, n); //debugging

	unsigned long long n_inv = modinv_fermat(n, p);
	for (int i = 0; i < n; i++)
	{
		__uint128_t product = (__uint128_t)INTT_eval[i] * n_inv;
		INTT_eval[i] = product % p;
	}

	// print_longlong_array("scaled_INTT_eval", INTT_eval, n); //debugging

	get_final_result_NTT_base_fast(result_buffer, INTT_eval, n, estimated_result_length, chunk_length);
}

charArray *mypow_NTT_base_fast(charArray *num, int power, int chunk_length)
{
	int len = num->length;
	int chunks_length = (len % chunk_length == 0) ? len / chunk_length : len / chunk_length + 1;
	int coffs_scratch_len = 3 * next2pow(power * chunks_length);
	unsigned long long *coffs_scratch_space = (unsigned long long *)malloc(coffs_scratch_len * sizeof(unsigned long long));

	charArray *scratch_space = (charArray *)malloc(sizeof(charArray));
	scratch_space->list = (char *)malloc((power * num->length + 1) * sizeof(char));

	charArray *result = (charArray *)malloc(sizeof(charArray));
	result->list = (char *)malloc((power * num->length + 1) * sizeof(char));

	power_base_fast(result, scratch_space, coffs_scratch_space, coffs_scratch_len, num, power, chunk_length);
	freecharArray(scratch_space);
	free(coffs_scratch_space);
	return result;
}

void power_base_fast(charArray *result_buffer, charArray *scratch_space, unsigned long long *coffs_scratch_space, int coffs_scratch_len, charArray *num, int power, int chunk_length)
{
	// coffs_scratch_len is supposed to be equal to 3 * next2pow(power * num->length - (power - 1))
	sprintf(result_buffer->list, "%d", 1);
	result_buffer->length = 1;
	charArray *working_num = scratch_space;
	sprintf(working_num->list, "%s", num->list);
	working_num->length = num->length;
	//	printf("working_num->list = %s, working_num->length = %d\n", working_num->list, working_num->length); //debugging
	while (power > 0)
	{
		if (power % 2 == 0)
		{
			int chunks_length = (working_num->length % chunk_length == 0) ? working_num->length / chunk_length : working_num->length / chunk_length + 1;
			int pre_n = next2pow(2 * chunks_length - 1);
			//			printf("pre_n = %d\n", pre_n); //debugging
			mult_fast_base(working_num, coffs_scratch_space, coffs_scratch_len, working_num, working_num, pre_n, chunk_length);
			power /= 2;
		}
		else
		{
			int chunks_len1 = (result_buffer->length % chunk_length == 0) ? result_buffer->length / chunk_length : result_buffer->length / chunk_length + 1;
			int chunks_len2 = (working_num->length % chunk_length == 0) ? working_num->length / chunk_length : working_num->length / chunk_length + 1;
			int pre_n = next2pow(chunks_len1 + chunks_len2 - 1);
			//			printf("pre_n = %d\n", pre_n); //debugging
			mult_fast_base(result_buffer, coffs_scratch_space, coffs_scratch_len, result_buffer, working_num, pre_n, chunk_length);
			power -= 1;
		}
	}
}

intArray *sieve_primes(int n)
{
	bool isprime[n + 1];
	for (int i = 0; i < n + 1; i++)
	{
		isprime[i] = true;
	}
	int size = (int)(n / log(n));
	int *primes = (int *)malloc(size * sizeof(int));
	if (!primes)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	if (primes == NULL)
	{
		printf("Error : not enough space for primes\n");
		exit(EXIT_FAILURE);
	}
	int index = 0;
	for (int num = 2; num <= n; num++)
	{
		if (isprime[num])
		{
			if (index < size)
			{
				primes[index] = num;
				index++;
			}
			else
			{
				size += 5;
				primes = (int *)realloc(primes, size * sizeof(int));
				if (primes == NULL)
				{
					printf("Error : not enough space for primes\n");
					exit(EXIT_FAILURE);
				}
				primes[index] = num;
				index++;
			}
			unsigned long long multiple = (unsigned long long)num * (unsigned long long)num;
			if (multiple <= n)
			{
				for (int mult = (int)multiple; mult <= n; mult += num)
				{
					isprime[mult] = false;
				}
			}
		}
	}
	intArray *result = (intArray *)malloc(sizeof(intArray));
	if (!result)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	result->list = primes;
	result->length = index;
	return result;
}

intArray *get_primes_exps(intArray *primes, int n)
{
	int *exps = (int *)malloc(primes->length * sizeof(int));
	if (!exps)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < primes->length; i++)
	{
		int prime_count = n / primes->list[i];
		int prime_counter = 0;
		int trialExp = 1;
		while (prime_count > 0)
		{
			prime_counter += prime_count;
			trialExp++;
			prime_count = n / (int)(pow(primes->list[i], trialExp));
		}
		exps[i] = prime_counter;
	}
	intArray *result = (intArray *)malloc(sizeof(intArray));
	if (!result)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	result->list = exps;
	result->length = primes->length;
	return result;
}

strArray *get_powered_primes_fast_base(intArray *primes, intArray *primesExps, int *prefix_digits_sum, int chunk_length)
{
	int len = primes->length;
	strArray *poweredPrimes = (strArray *)malloc(sizeof(strArray));
	if (!poweredPrimes)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	poweredPrimes->length = len;
	poweredPrimes->list = (charArray **)malloc(len * sizeof(charArray *));
	if (!(poweredPrimes->list))
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}

	int MAX_POWERED_LEN = primesExps->list[0];

	int coffs_scratch_len = 3 * next2pow(MAX_POWERED_LEN);
	unsigned long long *coffs_scratch_space = (unsigned long long *)malloc(coffs_scratch_len * sizeof(unsigned long long));

	int max_prime_size = ((int)log10(primes->list[len - 1])) + 1;

	int scratch_size = MAX_POWERED_LEN + 1 + max_prime_size + 1;
	charArray *scratch_buffer = (charArray *)malloc(sizeof(charArray));
	scratch_buffer->list = (char *)malloc(scratch_size * sizeof(char));
	if (!scratch_buffer)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}

	charArray *prime = (charArray *)malloc(sizeof(charArray));
	prime->list = scratch_buffer->list + MAX_POWERED_LEN + 1;

	for (int i = 0; i < len; i++)
	{
		int prime_size = ((int)log10(primes->list[i])) + 1;
		int result_size = prime_size * primesExps->list[i] + 2;

		charArray *powered_prime = (charArray *)malloc(sizeof(charArray));
		powered_prime->list = (char *)malloc(result_size * sizeof(char));
		if (!powered_prime->list)
		{
			printf("Memory allocation failed at line: %d\n", __LINE__);
			exit(EXIT_FAILURE);
		}

		sprintf(prime->list, "%d", primes->list[i]);
		prime->length = prime_size;

		// printf("raising %s to power %d\n", prime->list, primesExps->list[i]); //debugging
		power_base_fast(powered_prime, scratch_buffer, coffs_scratch_space, coffs_scratch_len, prime, primesExps->list[i], chunk_length);
		poweredPrimes->list[i] = powered_prime;
		prefix_digits_sum[i + 1] = prefix_digits_sum[i] + poweredPrimes->list[i]->length;
	}
	free(coffs_scratch_space);
	freecharArray(scratch_buffer);
	free(prime);
	return poweredPrimes;
}

void mulpowprimes_fast_base(charArray *result_buffer, unsigned long long *coffs_buffer, int coffs_buffer_len, strArray *powered_primes, int start, int end, int *prefix_digits_sum, int chunk_length)
{
	// prefix_digits_sum is assumed to be of length = powered_primes->length + 1
	charArray *left = (charArray *)malloc(sizeof(charArray));
	charArray *right = (charArray *)malloc(sizeof(charArray));
	int len = end - start;
	if (len == 1)
	{
		sprintf(result_buffer->list, "%s", powered_primes->list[start]->list);
		result_buffer->length = powered_primes->list[start]->length;

		// printf("copying %s\n", powered_primes->list[start]->list); //debugging
		// printf("copied result: %s\n", result_buffer->list); //debugging
		// printf("=======================================\n"); //debugging
	}
	else if (len == 2)
	{
		charArray *num1 = powered_primes->list[start];
		charArray *num2 = powered_primes->list[start + 1];
		// printf("multiplying: %s by %s\n", num1->list, num2->list); //debugging

		int chunks_len1 = (num1->length % chunk_length == 0) ? num1->length / chunk_length : num1->length / chunk_length + 1;
		int chunks_len2 = (num2->length % chunk_length == 0) ? num2->length / chunk_length : num2->length / chunk_length + 1;
		int pre_n = next2pow(chunks_len1 + chunks_len2 - 1);
		mult_fast_base(result_buffer, coffs_buffer, coffs_buffer_len, num1, num2, pre_n, chunk_length);
		// printf("result of multiplication: %s\n", result_buffer->list); //debugging
		// printf("=======================================\n"); //debugging
	}
	else
	{
		int mid = start + (end - start) / 2;

		int left_length = prefix_digits_sum[mid] - prefix_digits_sum[start];
		int right_length = prefix_digits_sum[end] - prefix_digits_sum[mid];

		left->list = result_buffer->list;
		right->list = left->list + left_length + 1;

		mulpowprimes_fast_base(left, coffs_buffer, coffs_buffer_len, powered_primes, start, mid, prefix_digits_sum, chunk_length);
		mulpowprimes_fast_base(right, coffs_buffer, coffs_buffer_len, powered_primes, mid, end, prefix_digits_sum, chunk_length);

		left_length = left->length;
		right_length = right->length;

		int chunks_len1 = (left_length % chunk_length == 0) ? left_length / chunk_length : left_length / chunk_length + 1;
		int chunks_len2 = (right_length % chunk_length == 0) ? right_length / chunk_length : right_length / chunk_length + 1;
		int pre_n = next2pow(chunks_len1 + chunks_len2 - 1);
		mult_fast_base(result_buffer, coffs_buffer, coffs_buffer_len, left, right, pre_n, chunk_length);
	}
	free(left);
	free(right);
}

size_t compute_result_space_factorial(int start, int end, int *prefix_digits_sum)
{
	int len = end - start;
	if (len == 1)
	{
		size_t result_length = prefix_digits_sum[end] - prefix_digits_sum[start] + 1;
		return result_length;
	}
	else if (len == 2)
	{
		size_t size1 = prefix_digits_sum[start + 1] - prefix_digits_sum[start] + 1;
		size_t size2 = prefix_digits_sum[end] - prefix_digits_sum[start + 1] + 1;
		size_t result_length = size1 + size2 + 1;
		return result_length;
	}
	int mid = start + len / 2;
	size_t left_size = compute_result_space_factorial(start, mid, prefix_digits_sum);
	size_t right_size = compute_result_space_factorial(mid, end, prefix_digits_sum);
	size_t result_size = left_size + right_size + 1;
	return result_size;
}

charArray *factorial_base_fast(int n, int chunk_length)
{
	intArray *primes = sieve_primes(n);

	// print_int_array("primes", primes->list, primes->length); //debugging

	intArray *primes_exps = get_primes_exps(primes, n);

	// print_int_array("primes' powers", primes_exps->list, primes_exps->length); //debugging

	int prefix_digits_sum[primes->length + 1];
	prefix_digits_sum[0] = 0;

	strArray *powered_primes = get_powered_primes_fast_base(primes, primes_exps, prefix_digits_sum, chunk_length);

	// for (int i = 0; i < powered_primes->length; i++) { //debugging
	// printf("%d ^ %d = %s\n", primes->list[i], primes_exps->list[i], powered_primes->list[i]->list); //debugging
	// } //debugging
	// print_int_array("prefix_digits_sum", prefix_digits_sum, primes->length + 1); //debugging

	freeintArray(primes);
	freeintArray(primes_exps);

	int MAX_LENGTH = prefix_digits_sum[powered_primes->length];
	int chunks_length = (MAX_LENGTH % chunk_length == 0) ? MAX_LENGTH / chunk_length : MAX_LENGTH / chunk_length + 1;
	int coffs_buffer_len = 3 * next2pow(chunks_length);
	unsigned long long *coffs_buffer = (unsigned long long *)malloc((coffs_buffer_len) * sizeof(unsigned long long));

	size_t factorial_result_size = compute_result_space_factorial(0, powered_primes->length, prefix_digits_sum);
	charArray *factorial_result = (charArray *)malloc(sizeof(charArray));
	factorial_result->list = (char *)malloc(factorial_result_size * sizeof(char));

	mulpowprimes_fast_base(factorial_result, coffs_buffer, coffs_buffer_len, powered_primes, 0, powered_primes->length, prefix_digits_sum, chunk_length);

	freestrArray(powered_primes);
	free(coffs_buffer);

	return factorial_result;
}

charArray *getStr(const char *str)
{
	int len = strlen(str);
	char *result = (char *)malloc((len + 1) * sizeof(char));
	if (!result)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	result[len] = '\0';
	for (int i = 0; i < len; i++)
	{
		result[i] = str[i];
	}
	charArray *resultArr = (charArray *)malloc(sizeof(charArray));
	if (!resultArr)
	{
		printf("Memory allocation failed at line: %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	resultArr->list = result;
	resultArr->length = len;
	return resultArr;
}

void print_int_array(char *name, int *array, int len)
{
	printf("%s = [", name);
	for (int i = 0; i < len; i++)
	{
		printf("%d", array[i]);
		if (i < len - 1)
			printf(", ");
	}
	printf("]\n");
}

void print_longlong_array(char *name, unsigned long long *array, int len)
{
	printf("%s = [", name);
	for (int i = 0; i < len; i++)
	{
		printf("%llu", array[i]);
		if (i < len - 1)
			printf(", ");
	}
	printf("]\n");
}

void remlzeros(charArray *num)
{

	int zero_index = 0;
	while (zero_index < (num->length - 1) && num->list[zero_index] == '0')
		zero_index++;

	int i = zero_index;
	while (num->list[i] != '\0')
	{
		num[i - zero_index] = num[i];
		i++;
	}
	num->list[i - zero_index] = '\0';
	num->length -= zero_index;
}