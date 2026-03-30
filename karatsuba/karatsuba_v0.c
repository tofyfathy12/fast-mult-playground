#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include "../utils/utils.h"

const size_t MAX_PRACTICAL_DIGITS = 1000000000UL;
const size_t MAX_MEMORY_GB = 8;
const size_t MAX_SCRATCH_BYTES = MAX_MEMORY_GB * 1024UL * 1024UL * 1024UL;

size_t compute_result_space_factorial(int start, int end, int* prefix_digits_sum);
charArray* factorial(int n);
void mulpowprimes_fast(char* result_buffer, char* scratch_buffer, strArray* powered_primes, int start, int end, int* prefix_digits_sum);
strArray* get_powered_primes_fast(intArray* primes, intArray* primesExps, int* prefix_digits_sum);
intArray* get_primes_exps(intArray* primes, int n);
intArray* sieve_primes(int n);
void power_fast(char* result_buffer, char* scratch_space, char* num, int len, int exp);
charArray* mult_karatsuba(charArray* num1, charArray* num2);
void karatsuba_mult_fast(char* result_buffer, char* scratch_buffer, const char* num1, const char* num2, int len);
void advanced_subtract_fast(char* result_buffer, char* resultNeg_buffer, const char* num1, const char* num2, int len1, int len2);
void advanced_add_fast(char* result_buffer, const char* num1, const char* num2, int len1, int len2);
charArray* add(charArray* num1, charArray* num2, int start, int end);

void reverse(charArray* str);
charArray* subtract(charArray* num1, charArray* num2);
void pad_inplace(char* num, int len, int req_len);
charArray* pad(charArray *num, int req_len);
charArray* append_zeros(charArray* num, int zeros_num);
char* substring(const char* str, int start, int end);
charArray* int_to_string(int num);

char* remove_leading_zeros(char* num, int len);
void remlzeros(char* num, int* len);
void print_char_array(char* str, int len);
size_t compute_scratch_space(int n);

int main(int argc, char *argv[])
{
	const char *s1 = NULL, *s2 = NULL;
	int fact_n = 0;
	DEBUG_OP = DBG_OP_MULT;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-d1") == 0) { DEBUG = 1; continue; }
		if (strcmp(argv[i], "-d2") == 0) { DEBUG = 2; continue; }
		if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) { fact_n = atoi(argv[++i]); DEBUG_OP = DBG_OP_FACT; continue; }
		if (!s1) s1 = argv[i]; else s2 = argv[i];
	}
	if (fact_n > 0) {
		clock_t start = clock();
		charArray* n_fact = factorial(fact_n);
		clock_t end = clock();
		double seconds = (double)(end - start) / CLOCKS_PER_SEC;
		printf("%d! = %s\n", fact_n, n_fact->list);
		printf("total time taken = %.3lf seconds\n", seconds);
		freecharArray(n_fact);
	} else {
		if (!s1) { s1 = "12345"; s2 = "67890"; }
		if (!s2) { printf("Usage: %s <num1> <num2> [-d] | -f <N> [-d]\n", argv[0]); return 1; }
		charArray* num1 = getStr(s1);
		charArray* num2 = getStr(s2);

		DBG_HEADER("KARATSUBA MULTIPLICATION (Optimized)");
		DBG_VAL("num1 length", "%d digits", num1->length);
		DBG_VAL("num2 length", "%d digits", num2->length);
		DBG_SEP();

		DBG_TIME_START();
		charArray* product = mult_karatsuba(num1, num2);
		DBG_TIME_END("karatsuba total");

		DBG_SEP();
		DBG_VAL("result length", "%d digits", product->length);

		printf("%s * %s = %s\n", num1->list, num2->list, product->list);
		freecharArray(num1); freecharArray(num2); freecharArray(product);
	}
}

size_t compute_result_space_factorial(int start, int end, int* prefix_digits_sum) {
	int len = end - start;
	if (len == 1) {
		size_t result_length = prefix_digits_sum[end] - prefix_digits_sum[start] + 1;
		return result_length;
	}
	else if (len == 2) {
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

charArray* factorial(int n) {
	intArray* primesArr = sieve_primes(n);

	intArray* primesExps = get_primes_exps(primesArr, n);

	int* prefix_digits_sum = malloc((primesArr->length + 1) * sizeof(int));
	prefix_digits_sum[0] = 0;
	strArray* poweredPrimes = get_powered_primes_fast(primesArr, primesExps, prefix_digits_sum);
	freeintArray(primesArr);
	freeintArray(primesExps);

	size_t factorial_result_size = compute_result_space_factorial(0, poweredPrimes->length, prefix_digits_sum);
	if (DEBUG >= 1 && DEBUG_OP == DBG_OP_FACT) {
		DBG_FACT_HEADER("DIVIDE AND CONQUER TREE");
		printf("  \033[36m➤\033[0m \033[90mestimated result size:\033[0m \033[33m%zu bytes\033[0m\n", factorial_result_size);
	}
	size_t factorial_scratch_size = compute_scratch_space(factorial_result_size);
	if (DEBUG >= 1 && DEBUG_OP == DBG_OP_FACT) printf("  \033[36m➤\033[0m \033[90mestimated scratch space:\033[0m \033[33m%zu bytes\033[0m\n", factorial_scratch_size);
	char* factorial_result_buffer = (char*)malloc(factorial_result_size*sizeof(char));
	char* factorial_scratch_buffer = (char*)malloc(factorial_scratch_size*sizeof(char));

	mulpowprimes_fast(factorial_result_buffer, factorial_scratch_buffer, poweredPrimes, 0, poweredPrimes->length, prefix_digits_sum);

	free(prefix_digits_sum);
	freestrArray(poweredPrimes);
	charArray* result = (charArray*)malloc(sizeof(charArray));
	result->list = factorial_result_buffer;
	result->length = strlen(factorial_result_buffer);
	return result;
}

void mulpowprimes_fast(char* result_buffer, char* scratch_buffer, strArray* powered_primes, int start, int end, int* prefix_digits_sum) {
	// prefix_digits_sum is assumed to be of length = powered_primes->length + 1
	int len = end - start;
	if (len == 1) {
		int result_length = powered_primes->list[start]->length;
		for (int i = 0; i <= result_length; i++) {
			result_buffer[i] = powered_primes->list[start]->list[i];
		}
		if (DEBUG >= 1 && DEBUG_OP == DBG_OP_FACT) {
			print_truncated_str("copying", powered_primes->list[start], 20);
			charArray dummy = {result_buffer, strlen(result_buffer)};
			print_truncated_str("copied result", &dummy, 20);
		}
	}
	else if (len == 2) {
		charArray* num1 = powered_primes->list[start];
		charArray* num2 = powered_primes->list[start + 1];
		int len = (num1->length > num2->length) ? num1->length : num2->length;
		char num1_padded[len + 1]; char num2_padded[len + 1];
		if (num1->length < len) {
			int dif = len - num1->length;
			num1_padded[len] = '\0';
			int i = len - 1;
			while (i >= 0) {
				num1_padded[i] = (i - dif >= 0) ? num1->list[i - dif] : '0';
				i--;
			}
			sprintf(num2_padded, "%s", num2->list);
		}
		else if (num2->length < len) {
			int dif = len - num2->length;
			num2_padded[len] = '\0';
			int i = len - 1;
			while (i >= 0) {
				num2_padded[i] = (i - dif >= 0) ? num2->list[i - dif] : '0';
				i--;
			}
			sprintf(num1_padded, "%s", num1->list);
		}
		else {
			sprintf(num1_padded, "%s", num1->list);
			sprintf(num2_padded, "%s", num2->list);
		}
		if (DEBUG >= 1 && DEBUG_OP == DBG_OP_FACT) {
			print_truncated_str("multiplying (left)", num1, 20);
			print_truncated_str("multiplying (right)", num2, 20);
		}
		karatsuba_mult_fast(result_buffer, scratch_buffer, num1_padded, num2_padded, len);
		if (DEBUG >= 1 && DEBUG_OP == DBG_OP_FACT) {
			charArray dummy = {result_buffer, strlen(result_buffer)};
			print_truncated_str("result of mult", &dummy, 20);
		}
	}
	else {
		int mid = start + (end - start) / 2;
		int left_length = prefix_digits_sum[mid] - prefix_digits_sum[start];
		int right_length = prefix_digits_sum[end] - prefix_digits_sum[mid];
		char* left = result_buffer;
		char* right = left + left_length + 1;

		mulpowprimes_fast(left, scratch_buffer, powered_primes, start, mid, prefix_digits_sum);
		mulpowprimes_fast(right, scratch_buffer, powered_primes, mid, end, prefix_digits_sum);

		left_length = strlen(left);
		right_length = strlen(right);
		if (left_length < right_length) {
			char left_padded[right_length + 1];
			int zeros_num = right_length - left_length;
			for (int i = 0; i < zeros_num; i++) {
				left_padded[i] = '0';
			}
			for (int i = 0; i <= left_length; i++) {
				left_padded[i + zeros_num] = left[i];
			}
			karatsuba_mult_fast(result_buffer, scratch_buffer, left_padded, right, right_length);
		}
		else if (right_length < left_length) {
			char right_padded[left_length + 1];
			int zeros_num = left_length - right_length;
			for (int i = 0; i < zeros_num; i++) {
				right_padded[i] = '0';
			}
			for (int i = 0; i <= right_length; i++) {
				right_padded[i + zeros_num] = right[i];
			}
			karatsuba_mult_fast(result_buffer, scratch_buffer, left, right_padded, left_length);
		}
		else {
			karatsuba_mult_fast(result_buffer, scratch_buffer, left, right, left_length);
		}
	}
}

strArray* get_powered_primes_fast(intArray* primes, intArray* primesExps, int* prefix_digits_sum) {
	int len = primes->length;
	strArray* poweredPrimes = (strArray*)malloc(sizeof(strArray));
	if (!poweredPrimes) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	poweredPrimes->length = len;
	poweredPrimes->list = (charArray**)malloc(len * sizeof(charArray*));
	if (!(poweredPrimes->list)) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	int MAX_POWERED_LEN = primesExps->list[1];
	int scratch_size = compute_scratch_space(MAX_POWERED_LEN) + MAX_POWERED_LEN + 3;
	char* scratch_buffer = (char* )malloc(scratch_size * sizeof(char));
	if (!scratch_buffer) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	for (int i = 0; i < len; i++) {
		int prime_size = ((int)log10(primes->list[i])) + 1;
		int result_size = prime_size * primesExps->list[i] + 3;
		char* powered_prime = (char*)malloc(result_size * sizeof(char));
		if (!powered_prime) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
		char prime[prime_size + 1];
		sprintf(prime, "%d", primes->list[i]);
		power_fast(powered_prime, scratch_buffer, prime, prime_size, primesExps->list[i]);
		poweredPrimes->list[i] = (charArray*)malloc(sizeof(charArray));
		if (!(poweredPrimes->list[i])) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
		poweredPrimes->list[i]->length = strlen(powered_prime);
		poweredPrimes->list[i]->list = (char*)realloc(powered_prime, (poweredPrimes->list[i]->length + 1) * sizeof(char));
		prefix_digits_sum[i + 1] = prefix_digits_sum[i] + poweredPrimes->list[i]->length;
	}
	free(scratch_buffer);
	return poweredPrimes;
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
			long long multiple = (long long) num * (long long) num;
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

void power_fast(char* result_buffer, char* scratch_space, char* num, int len, int exp) {
	sprintf(result_buffer, "%d", 1);
	int result_length = 1;
	char* working_num = scratch_space;
	char* real_scratch_space = working_num + (len * exp + 3);
	sprintf(working_num, "%s", num);
	int working_num_length = len;
	while (exp > 0) {
		if (exp % 2 == 0) {
			karatsuba_mult_fast(working_num, real_scratch_space, working_num, working_num, working_num_length);
			if (working_num[working_num_length*2 - 1] == '\0') {
				working_num_length = working_num_length*2 - 1;
			}
			else {
				working_num_length *= 2;
			}
			exp /= 2;
		}
		else {
			if (result_length < working_num_length) {
				pad_inplace(result_buffer, result_length, working_num_length);
			}
			karatsuba_mult_fast(result_buffer, real_scratch_space, working_num, result_buffer, working_num_length);
			int new_len = result_length + working_num_length - 1;
			if (result_buffer[new_len] == '\0') {
				result_length = new_len;
			}
			else {
				result_length = new_len + 1;
			}
			exp -= 1;
		}
	}
}

charArray* mult_karatsuba(charArray* num1, charArray* num2) {
	int len = (num1->length > num2->length) ? num1->length : num2->length;
	char num1_padded[len + 1];
	char num2_padded[len + 1];
	if (num1->length < len) {
		int dif = len - num1->length;
		num1_padded[len] = '\0';
		int i = len - 1;
		while (i >= 0) {
			num1_padded[i] = (i - dif >= 0) ? num1->list[i - dif] : '0';
			i--;
		}
		sprintf(num2_padded, "%s", num2->list);
	}
	else if (num2->length < len) {
		int dif = len - num2->length;
		num2_padded[len] = '\0';
		int i = len - 1;
		while (i >= 0) {
			num2_padded[i] = (i - dif >= 0) ? num2->list[i - dif] : '0';
			i--;
		}
		sprintf(num1_padded, "%s", num1->list);
	}
	else {
		sprintf(num1_padded, "%s", num1->list);
		sprintf(num2_padded, "%s", num2->list);
	}

	int scratch_size = compute_scratch_space(len);
	char* scratch_space = (char*)malloc(scratch_size*sizeof(char));
	char* result_buffer = (char*)malloc((2*len + 2)*sizeof(char));

	karatsuba_mult_fast(result_buffer, scratch_space, num1_padded, num2_padded, len);

	free(scratch_space);
	charArray* resultArr = (charArray*)malloc(sizeof(charArray));
	resultArr->list = result_buffer;
	resultArr->length = strlen(result_buffer);
	return resultArr;
}

void karatsuba_mult_fast(char* result_buffer,
						char* scratch_buffer,
						const char* num1,
						const char* num2,
						int len) {

	while (len > 1 && num1[0] == '0' && num2[0] == '0') {
		num1++;
		num2++;
		len--;
	}

	if (len <= 9) {
		long long N1 = 0;
		long long N2 = 0;
		for (int i = 0; i < len; i++) {
			N1 = (N1 * 10) + (num1[i] - '0');
			N2 = (N2 * 10) + (num2[i] - '0');
		}
		long long resultNum = N1 * N2;
		sprintf(result_buffer, "%lld", resultNum);
	}
	else {
		int high_len = len / 2;
		int low_len = len - high_len;

		char* z0 = scratch_buffer;
		char* z1 = z0 + (2*high_len + 1);
		char* z2 = z1 + (2*low_len + 1);
		char* added1 = z2 + ((2*low_len + 2) + 1);
		char* added2 = added1 + ((low_len + 1) + 1);

		karatsuba_mult_fast(z0, added1, num1, num2, high_len);
		karatsuba_mult_fast(z1, added1, num1 + high_len, num2 + high_len, low_len);

		advanced_add_fast(added1, num1, num1 + high_len, high_len, low_len);
		advanced_add_fast(added2, num2, num2 + high_len, high_len, low_len);

		int add_len = strlen(added1);
		karatsuba_mult_fast(z2, added2 + ((low_len + 1) + 1), added1, added2, add_len);

		int z2Len = strlen(z2);
		int z1Len = strlen(z1);
		int z0Len = strlen(z0);
		advanced_subtract_fast(z2, NULL, z2, z1, z2Len, z1Len);

		z2Len = strlen(z2);

		advanced_subtract_fast(z2, NULL, z2, z0, z2Len, z0Len);

		for (int i = 0; i < z0Len; i++) {
			result_buffer[i] = z0[i];
		}
		int result_len = z0Len;
		if (result_buffer[0] != '0') {
			for (int i = z0Len; i < z0Len + (2 * low_len); i++) {
				result_buffer[i] = '0';
			}
			result_len += (2 * low_len);
		}
		result_buffer[result_len] = '\0';
		advanced_add_fast(result_buffer, result_buffer, z1, result_len, z1Len);
		result_len = (result_len > z1Len) ? result_len + 1 : z1Len + 1;
		remlzeros(result_buffer, &result_len);

		for (int i = z2Len; i < z2Len + low_len; i++) {
			z2[i] = '0';
		}
		z2Len += low_len;
		z2[z2Len] = '\0';
		advanced_add_fast(result_buffer, result_buffer, z2, result_len, z2Len);
		result_len = (result_len > z2Len) ? result_len + 1 : z2Len + 1;

		remlzeros(result_buffer, &result_len);
	}
}

void advanced_subtract_fast(char* result_buffer, char* resultNeg_buffer, const char* num1, const char* num2, int len1, int len2) {
	int big = (len1 > len2) ? len1 : len2;
	if (resultNeg_buffer != NULL) {
		resultNeg_buffer[0] = '0';
		resultNeg_buffer[big + 1] = '\0';
	}
	result_buffer[big] = '\0';
	int digit1 = num1[len1 - 1] - '0';
	int digit2 = num2[len2 - 1] - '0';
	int dif = digit1 - digit2;
	int borrow = 0;
	if (dif < 0) {
		borrow = 1;
		dif += 10;
	}
	result_buffer[big - 1] = dif + '0';
	if (resultNeg_buffer != NULL) {
		resultNeg_buffer[big] = (dif == 0) ? dif + '0' : (10 - dif) + '0';
	}
	if (len1 == len2) {
		for (int i = big - 2; i >= 0; i--) {
			digit1 = num1[i] - '0';
			digit2 = num2[i] - '0';
			dif = digit1 - digit2 - borrow;
			if (dif < 0) {
				dif += 10;
				borrow = 1;
			}
			else {
				borrow = 0;
			}
			result_buffer[i] = dif + '0';
			if (resultNeg_buffer != NULL) {
				if (i == big - 2 && resultNeg_buffer[big] == '0')
					resultNeg_buffer[i + 1] = (10 - dif) + '0';
				else
					resultNeg_buffer[i + 1] = (9 - dif) + '0';
			}
		}
		if (borrow == 1 && resultNeg_buffer != NULL) {
			resultNeg_buffer[0] = '-';
		}
	}
	else {
		int dif = abs(len1 - len2);
		if (len1 > len2) {
			int loop1_i = len2 - 2;
			while (loop1_i >= 0) {
				int digit1 = num1[loop1_i + dif] - '0';
				int digit2 = num2[loop1_i] - '0';
				int diff = digit1 - digit2 - borrow;
				if (diff < 0) {
					diff += 10;
					borrow = 1;
				}
				else {
					borrow = 0;
				}
				result_buffer[loop1_i + dif] = diff + '0';
				loop1_i--;
			}
			int loop2_i = dif - 1;
			while (loop2_i >= 0) {
				int digit1 = num1[loop2_i] - '0';
				int diff = digit1 - borrow;
				if (diff < 0) {
					diff += 10;
					borrow = 1;
				}
				else {
					borrow = 0;
				}
				result_buffer[loop2_i] = diff + '0';
				loop2_i--;
			}
		}
		else {
			int loop1_i = len1 - 2;
			while (loop1_i >= 0) {
				int digit1 = num1[loop1_i] - '0';
				int digit2 = num2[loop1_i + dif] - '0';
				int diff = digit1 - digit2 - borrow;
				if (diff < 0) {
					diff += 10;
					borrow = 1;
				}
				else {
					borrow = 0;
				}
				if (loop1_i == len1 - 2 && resultNeg_buffer[big] == '0') {
					resultNeg_buffer[loop1_i + dif + 1] = (10 - diff) + '0';
				}
				else
					resultNeg_buffer[loop1_i + dif + 1] = (9 - diff) + '0';
				loop1_i--;
			}
			int loop2_i = dif - 1;
			while (loop2_i >= 0) {
				int digit2 = num2[loop2_i] - '0';
				int diff = 0 - digit2 - borrow + 10;
				borrow = 1;
				resultNeg_buffer[loop2_i + 1] = (9 - diff) + '0';
				loop2_i--;
			}
			resultNeg_buffer[0] = '-';
		}
	}
}

void advanced_add_fast(char* result_buffer, const char* num1, const char* num2, int len1, int len2) {
	int big = (len1 > len2) ? len1 : len2;
	result_buffer[big + 1] = '\0';
	int carry = 0;
	if (len1 == len2) {
		int len = len1;
		for (int i = 1; i <= len; i++) {
			int digit1 = num1[len1 - i] - '0';
			int digit2 = num2[len2 - i] - '0';
			int sum = digit1 + digit2 + carry;
			result_buffer[len - i + 1] = (sum % 10) + '0';
			carry = sum / 10;
		}
	}
	else {
		int dif = abs(len1 - len2);
		if (len2 > len1) {
			int loop1_i = len1 - 1;
			while (loop1_i >= 0) {
				int digit1 = num1[loop1_i] - '0';
				int digit2 = num2[loop1_i + dif] - '0';
				int sum = digit1 + digit2 + carry;
				result_buffer[loop1_i + dif + 1] = (sum % 10) + '0';
				carry = sum / 10;
				loop1_i--;
			}
			int loop2_i = dif - 1;
			while (loop2_i >= 0) {
				int digit2 = num2[loop2_i] - '0';
				int sum = digit2 + carry;
				result_buffer[loop2_i + 1] = (sum % 10) + '0';
				carry = sum / 10;
				loop2_i--;
			}
		}
		else {
			int loop1_i = len2 - 1;
			while (loop1_i >= 0) {
				int digit1 = num1[loop1_i + dif] - '0';
				int digit2 = num2[loop1_i] - '0';
				int sum = digit1 + digit2 + carry;
				result_buffer[loop1_i + dif + 1] = (sum % 10) + '0';
				carry = sum / 10;
				loop1_i--;
			}
			int loop2_i = dif - 1;
			while (loop2_i >= 0) {
				int digit1 = num1[loop2_i] - '0';
				int sum = digit1 + carry;
				result_buffer[loop2_i + 1] = (sum % 10) + '0';
				carry = sum / 10;
				loop2_i--;
			}
		}
	}
	result_buffer[0] = carry + '0';
}

charArray* add(charArray* num1, charArray* num2, int start, int end) {
	int len = end - start;
	char *result = (char *)malloc((len + 2) * sizeof(char));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result[len + 1] = '\0';
	int carry = 0;
	for (int i = end - 1; i >= start; i--) {
		int digit1 = num1->list[i] - '0';
		int digit2 = num2->list[i] - '0';
		int sum = digit1 + digit2 + carry;
		result[i - start + 1] = (sum % 10) + '0';
		carry = sum / 10;
	}
	result[0] = carry + '0';
	int zero_index = 0;
	while (result[zero_index] == '0') {
		zero_index++;
	}
	for (int i = zero_index; i < len + 2; i++) {
		result[i - zero_index] = result[i];
	}
	result = (char*)realloc(result, (len + 2 - zero_index)*sizeof(char));
	charArray* resultArr = (charArray*)malloc(sizeof(charArray));
	if (!resultArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	resultArr->list = result;
	resultArr->length = len - zero_index + 1;
	return resultArr;
}

void reverse(charArray* str) {
	int len = str->length;
	int i = 0, j = len - 1;
	while (j > i) {
		char temp = str->list[j];
		str->list[j] = str->list[i];
		str->list[i] = temp;
		i++;
		j--;
	}
}

charArray* subtract(charArray* num1, charArray* num2) {
	int len = num1->length;
	char *result = (char *)malloc((len + 1) * sizeof(char));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result[len] = '\0';
	char *resultNeg = (char *)malloc((len + 2) * sizeof(char));
	if (!resultNeg) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	resultNeg[len + 1] = '\0';
	int digit1 = num1->list[len - 1] - '0';
	int digit2 = num2->list[len - 1] - '0';
	int dif = digit1 - digit2;
	int borrow = 0;
	if (dif < 0) {
		borrow = 1;
		dif += 10;
	}
	result[len - 1] = dif + '0';
	resultNeg[len] = (10 - dif) + '0';
	for (int i = len - 2; i >= 0; i--) {
		digit1 = num1->list[i] - '0';
		digit2 = num2->list[i] - '0';
		dif = digit1 - digit2 - borrow;
		if (dif < 0) {
			dif += 10;
			borrow = 1;
		}
		else {
			borrow = 0;
		}
		result[i] = dif + '0';
		resultNeg[i + 1] = (9 - dif) + '0';
	}
	charArray* resultArr = (charArray*)malloc(sizeof(charArray));
	if (!resultArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	if (borrow == 1) {
		resultNeg[0] = '-';
		free(result);
		resultArr->list = resultNeg;
		resultArr->length = len + 1;
	}
	else if (borrow == 0) {
		free(resultNeg);
		resultArr->list = result;
		resultArr->length = len;
	}
	return resultArr;
}

void pad_inplace(char* num, int len, int req_len) { // assuming there is enough space for the additional zeros
	int zeros_num = req_len - len;
	for (int i = req_len; i >= zeros_num; i--) { //shfit to right
		num[i] = num[i - zeros_num];
	}
	for (int i = zeros_num - 1; i >= 0; i--) { //add zeros to left
		num[i] = '0';
	}
}

charArray* pad(charArray* num, int req_len) {
	int len = num->length;
	int dif = req_len - len;
	char *result = (char *)realloc(num->list, (req_len + 1) * sizeof(char));
	result[req_len] = '\0';
	for (int i = 0; i < len; i++) {
		result[req_len - 1 - i] = result[len - 1 - i];
	}
	for (int i = 0; i < dif; i++) {
		result[i] = '0';
	}
	num->list = result;
	num->length = req_len;
	return num;
}

charArray* append_zeros(charArray* num, int zeros_num) {
	int len = num->length;
	char *result = (char*)realloc(num->list, (len + zeros_num + 1)*sizeof(char));
	result[len + zeros_num] = '\0';
	for (int i = len; i < (len + zeros_num); i++) {
		result[i] = '0';
	}
	num->list = result;
	num->length = len + zeros_num;
	return num;
}

char* substring(const char* str, int start, int end) {
	int len = end - start;
	char *result = (char*)malloc((len + 1) * sizeof(char));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result[len] = '\0';
	for (int i = start; i < end; i++) {
		result[i - start] = str[i];
	}
	return result;
}

charArray* int_to_string(int num) {
	int length = (int)log10(num) + 1;
	char* result = (char*)malloc((length + 1)*sizeof(char));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result[length] = '\0';
	for (int i = length - 1; i >= 0; i--) {
		result[i] = (num % 10) + '0';
		num /= 10;
	}
	charArray* resultArr = (charArray*)malloc(sizeof(charArray));
	if (!resultArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	resultArr->list = result;
	resultArr->length = length;
	return resultArr;
}

char* remove_leading_zeros(char* num, int len) {
	int zero_index;
	bool negative = false;
	if (num[0] == '-') {
		zero_index = 1;
		negative = true;
	}
	else
		zero_index = 0;
	while (zero_index < len - 1 && num[zero_index] == '0')
		zero_index++;
	int offset = (negative) ? 1 : 0;
	if (!((negative && zero_index == 1) || (!negative && zero_index == 0))) {
		for (int i = zero_index; i <= len; i++) {
			num[i - zero_index + offset] = num[i];
		}
	}
	num = (char*)realloc(num, (len - (zero_index - offset) + 1) * sizeof(char));
	return num;
}

void remlzeros(char* num, int* len) {
	int zero_index = 0;
	while (zero_index < (*len - 1) && num[zero_index] == '0')
		zero_index++;

	int i = zero_index;
	while (num[i] != '\0') {
		num[i - zero_index] = num[i];
		i++;
	}
	num[i - zero_index] = '\0';
	*len -= zero_index;
}

void print_char_array(char* str, int len) {
	for (int i = 0; i < len; i++) {
		if (str[i] == '\0')
			printf("\\0");
		else
			printf("%c", str[i]);
	}
	printf("\n");
}

size_t compute_scratch_space(int n) {
    if (n <= 9) return 32; // Base case: small multiplication
    int high_len = n / 2;
    int low_len = n - high_len;
    size_t z0_size = 2 * high_len + 2;
    size_t z1_size = 2 * low_len + 2;
    size_t z2_size = 2 * (low_len + 1) + 2;
    size_t added1_size = low_len + 2;
    size_t added2_size = low_len + 2;
    size_t local_space = z0_size + z1_size + z2_size + added1_size + added2_size;
    size_t recursive_space = compute_scratch_space(low_len + 1);
    return local_space + recursive_space;
}

