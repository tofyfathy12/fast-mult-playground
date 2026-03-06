#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

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
charArray* mulpowprimes(strArray* powered_primes, int start, int end);
strArray* get_powered_primes(intArray* primes, intArray* primesExps);
intArray* get_primes_exps(intArray* primes, int n);
intArray* sieve_primes(int n);
charArray* power(charArray* num, int exp);
charArray* karatsuba_mult(charArray* num1, int start1, int end1, charArray* num2, int start2, int end2);
charArray* advanced_subtract(charArray* num1, int start1, int end1, charArray* num2, int start2, int end2);
charArray* advanced_add(charArray* num1, int start1, int end1, charArray* num2, int start2, int end2);
charArray* int_to_string(int num);
char* substring(const char* str, int start, int end);
charArray* pad(charArray *num, int req_len);
charArray* append_zeros(charArray* num, int zeros_num);
charArray* getStr(const char * str);
void freeintArray(intArray* arr);
void freecharArray(charArray* arr);
void freestrArray(strArray* arr);
void print_int_array(char* name, int* array, int len);

int main(void) {
    charArray *num1 = getStr("12345344444444444444444444444444444444444444444444444444446356565656546456789");
	charArray *num2 = getStr("17644444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444315981");
	int len1 = num1->length;
	int len2 = num2->length;

	charArray *added = advanced_add(num1, 0, len1, num2, 0, len2);
	printf("%s + %s = %s\n", num1->list, num2->list, added->list);
	char* substr1 = substring(num1->list, 1, 3);
	char* substr2 = substring(num2->list, 0, 2);
	char* substr3 = substring(num2->list, 2, 4);
	char* substr4 = substring(num1->list, 2, 4);
	charArray *subted1 = advanced_subtract(num1, 1, 3, num2, 0, 2); // 345 - 17 = 328
	printf("%s - %s = %s\n", substr1, substr2, subted1->list);

	charArray *subted2 = advanced_subtract(num2, 2, 4, num1, 2, 4); // 76 - 67 = 9
	printf("%s - %s = %s\n", substr3, substr4, subted2->list);

	if (len2 > len1) {
		num1 = pad(num1, len2);
		len1 = len2;
	}
	else if (len1 > len2) {
		num2 = pad(num2, len1);
		len2 = len1;
	}
	printf("num1 after padding : %s\n", num1->list);
	printf("num2 after padding : %s\n", num2->list);

	charArray* product = karatsuba_mult(num1, 0, len1, num2, 0, len2);
	printf("%s * %s = %s\n", num1->list, num2->list, product->list);

	charArray* num = getStr("22");
	charArray* power_res = power(num, 5);
	printf("%s ^ %d = %s\n", num->list, 5, power_res->list);

    freecharArray(num1);
    freecharArray(num2);
    freecharArray(added);
    free(substr1);
    free(substr2);
    free(substr3);
    free(substr4);
    freecharArray(subted1);
    freecharArray(subted2);
    freecharArray(product);
    freecharArray(num);
    freecharArray(power_res);
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

charArray* mulpowprimes(strArray* powered_primes, int start, int end) {
	int len = end - start;
	if (len == 1) {
		charArray* num = getStr(powered_primes->list[start]->list);
		// printf("multiplying: %s by 1\n", num->list);
		return num;
	}
	else if (len == 2) {
		charArray* num1 = powered_primes->list[start];
		charArray* num2 = powered_primes->list[start + 1];
		// printf("multiplying: %s by %s\n", num1->list, num2->list);
		if (num1->length < num2->length) {
			num1 = pad(num1, num2->length);
			powered_primes->list[start] = num1;
		}
		else if (num2->length < num1->length) {
			num2 = pad(num2, num1->length);
			powered_primes->list[start + 1] = num2;
		}
		charArray* result = karatsuba_mult(num1, 0, num1->length, num2, 0, num2->length);
		return result;
	}
	int mid = start + (end - start) / 2;
	charArray* left = mulpowprimes(powered_primes, start, mid);
	charArray* right = mulpowprimes(powered_primes, mid, end);
	if (left->length < right->length) {
		left = pad(left, right->length);
	}
	else if (right->length < left->length) {
		right = pad(right, left->length);
	}
	charArray* result = karatsuba_mult(left, 0, left->length, right, 0, right->length);
	freecharArray(left);
	freecharArray(right);
	return result;
}

strArray* get_powered_primes(intArray* primes, intArray* primesExps) {
	int len = primes->length;
	charArray** powedPrimes = (charArray**)malloc(len*sizeof(charArray*));
	if (!powedPrimes) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	for (int i = 0; i < len; i++) {
		charArray* num = int_to_string(primes->list[i]);
		charArray* primepow = power(num, primesExps->list[i]);
		powedPrimes[i] = primepow;
		freecharArray(num);
	}
	strArray* poweredPrimesArr = (strArray*)malloc(sizeof(strArray));
	if (!poweredPrimesArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	poweredPrimesArr->list = powedPrimes;
	poweredPrimesArr->length = len;
	return poweredPrimesArr;
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

charArray* power(charArray* num, int exp) {
	charArray* result = getStr("1");
	charArray* NUM = getStr(num->list);
	while (exp > 0) {
		if (exp % 2 == 0) {
			// char* tempNum = (char*)malloc((2*NUM->length + 3)*sizeof(char));/
			// if (!tempNum) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			// char* scratch_buffer = (char*)malloc((10*NUM->length + 256)*sizeof(char));
			// if (!scratch_buffer) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			// karatsuba_mult_fast(tempNum, scratch_buffer, NUM->list, NUM->list, NUM->length);
			// free(scratch_buffer);
			charArray* temp_num = karatsuba_mult(NUM, 0, NUM->length, NUM, 0, NUM->length);
			freecharArray(NUM);
            NUM = temp_num;
			// NUM = (charArray*)malloc(sizeof(charArray));
			// if (!NUM) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			// NUM->list = tempNum;
			// NUM->length = strlen(tempNum);
			exp /= 2;
		}
		else {
			if (result->length < NUM->length) {
				result = pad(result, NUM->length);
			}
			else if (NUM->length < result->length) {
				NUM = pad(NUM, result->length);
			}
			// char* tempResult = (char*)malloc((2 * NUM->length + 3)*sizeof(char));
			// if (!tempResult) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			// char* scratch_buffer = (char*)malloc((10 * NUM->length + 256) * sizeof(char));
			// if (!scratch_buffer) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			// karatsuba_mult_fast(tempResult, scratch_buffer, NUM->list, result->list, NUM->length);
			// free(scratch_buffer);
			charArray* tempResult = karatsuba_mult(NUM, 0, NUM->length, result, 0, result->length);
			freecharArray(result);
            result = tempResult;
			// result = (charArray*)malloc(sizeof(charArray));
			// if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			// result->list = tempResult;
			// result->length = strlen(tempResult);
			exp -= 1;
		}
	}
	freecharArray(NUM);
	return result;
}

charArray* karatsuba_mult(charArray* num1, int start1, int end1, charArray* num2, int start2, int end2) {

	int len1 = end1 - start1;
	int len2 = end2 - start2;
	if (len1 > len2) {
		num2 = pad(num2, len1);
		len2 = len1;
	}
	else if (len2 > len1) {
		num1 = pad(num1, len2);
		len1 = len2;
	}

	//char* substr1 = substring(num1, start1, end1);
//	char* substr2 = substring(num2, start2, end2);
//	printf("multiplying: %s by %s\n", substr1, substr2); //debugging
//	printf("len1 = %d\n", len1); //debugging
//	printf("len2 = %d\n", len2); //debugging
//	free(substr1);
//	free(substr2);

	if (len1 <= 9 && len2 <= 9) {
		long long Num1 = 0, Num2 = 0;
		for (int i = start1; i < end1; i++) {
			Num1 = (Num1 * 10) + (num1->list[i] - '0');
		}
		for (int i = start2; i < end2; i++) {
			Num2 = (Num2 * 10) + (num2->list[i] - '0');
		}
		long long resultNum = Num1 * Num2;
		charArray* resultArr = (charArray*)malloc(sizeof(charArray));
		if (!resultArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
		
		resultArr->length = 1;
		if (resultNum > 0) {
			resultArr->length = (int)(log10(resultNum)) + 1;
		}
		resultArr->list = (char*)malloc((resultArr->length + 1)*sizeof(char));
		if (!(resultArr->list)) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
		sprintf(resultArr->list, "%lld", resultNum);
		return resultArr;
	}
	int mid1 = start1 + len1 / 2;
	int mid2 = start2 + len2 / 2;
	charArray* z0 = karatsuba_mult(num1, start1, mid1, num2, start2, mid2);
	int z0Len = z0->length;
	charArray* z1 = karatsuba_mult(num1, mid1, end1, num2, mid2, end2);
	int z1Len = z1->length;
	charArray* added1 = advanced_add(num1, start1, mid1, num1, mid1, end1);
	charArray* added2 = advanced_add(num2, start2, mid2, num2, mid2, end2);
	int added1_len = added1->length;
	int added2_len = added2->length;
	if (added1_len < added2_len) {
		added1 = pad(added1, added2_len);
		added1_len = added2_len;
	}
	else if (added2_len < added1_len) {
		added2 = pad(added2, added1_len);
		added2_len = added1_len;
	}
	charArray* z2 = karatsuba_mult(added1, 0, added1_len, added2, 0, added2_len);
	int z2Len = z2->length;
	charArray* resultMidTemp = advanced_subtract(z2, 0, z2Len, z1, 0, z1Len);
	int resultmidtempLen = resultMidTemp->length;
	charArray *resultMid = advanced_subtract(resultMidTemp, 0, resultmidtempLen, z0, 0, z0Len);
	int resultMidLen = resultMid->length;
	int zeros_num = (end1 - mid1) + (end2 - mid2);
	z0 = append_zeros(z0, zeros_num);
	z0Len += zeros_num;
	resultMid = append_zeros(resultMid, zeros_num/2);
	resultMidLen += zeros_num/2;
	charArray* resultTemp = advanced_add(z0, 0, z0Len, resultMid, 0, resultMidLen);
	int resultTempLen = resultTemp->length;
	charArray* result = advanced_add(resultTemp, 0, resultTempLen, z1, 0, z1Len);
	freecharArray(z0);
	freecharArray(z1);
	freecharArray(z2);
	freecharArray(added1);
	freecharArray(added2);
	freecharArray(resultMidTemp);
	freecharArray(resultMid);
	freecharArray(resultTemp);
	return result;
}

charArray *advanced_subtract(charArray* num1, int start1, int end1, charArray* num2, int start2, int end2) {
	int len1 = end1 - start1;
	int len2 = end2 - start2;
	int big = (len1 > len2) ? len1 : len2;
	char *result = (char *)malloc((big + 1) * sizeof(char));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result[big] = '\0';
	char *resultNeg = (char *)malloc((big + 2) * sizeof(char));
	if (!resultNeg) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	resultNeg[big + 1] = '\0';
	int digit1 = num1->list[end1 - 1] - '0';
	int digit2 = num2->list[end2 - 1] - '0';
	int dif = digit1 - digit2;
	int borrow = 0;
	if (dif < 0) {
		borrow = 1;
		dif += 10;
	}
	result[big - 1] = dif + '0';
	resultNeg[big] = (dif == 0) ? dif + '0' : (10 - dif) + '0';
	if (len1 == len2) {
		for (int i = big - 2; i >= 0; i--) {
			digit1 = num1->list[start1 + i] - '0';
			digit2 = num2->list[start2 + i] - '0';
			dif = digit1 - digit2 - borrow;
			if (dif < 0) {
				dif += 10;
				borrow = 1;
			}
			else {
				borrow = 0;
			}
			result[i] = dif + '0';
			if (i == big - 2 && resultNeg[big] == '0')
				resultNeg[i + 1] = (10 - dif) + '0';
			else
				resultNeg[i + 1] = (9 - dif) + '0';
		}
		if (borrow == 1) {
			free(result);
			int zero_index = 1;
			while (zero_index < big && resultNeg[zero_index] == '0') {
				zero_index++;
			}
			for (int i = zero_index; i < big + 2; i++) {
				resultNeg[i - zero_index + 1] = resultNeg[i];
			}
			resultNeg = (char*)realloc(resultNeg, (big - zero_index + 2)*sizeof(char));
			resultNeg[0] = '-';
			charArray* resultNegArr = (charArray*)malloc(sizeof(charArray));
			if (!resultNegArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			
			resultNegArr->list = resultNeg;
			resultNegArr->length = big - zero_index + 1;
			return resultNegArr;
		}
		else {
			free(resultNeg);
			int zero_index = 0;
			while (zero_index < big - 1 && result[zero_index] == '0') {
				zero_index++;
			}
			for (int i = zero_index; i < big + 1; i++) {
				result[i - zero_index] = result[i];
			}
			result = (char*)realloc(result, (big - zero_index + 1)*sizeof(char));
			charArray* resultArr = (charArray*)malloc(sizeof(charArray));
			if (!resultArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			resultArr->list = result;
			resultArr->length = big - zero_index;
			return resultArr;
		}
	}
	else {
		int dif = abs(len1 - len2);
		if (len1 > len2) {
			free(resultNeg);
			int loop1_i = len2 - 2;
			while (loop1_i >= 0) {
				int digit1 = num1->list[start1 + loop1_i + dif] - '0';
				int digit2 = num2->list[start2 + loop1_i] - '0';
				int diff = digit1 - digit2 - borrow;
				if (diff < 0) {
					diff += 10;
					borrow = 1;
				}
				else {
					borrow = 0;
				}
				result[loop1_i + dif] = diff + '0';
				loop1_i--;
			}
			int loop2_i = dif - 1;
			while (loop2_i >= 0) {
				int digit1 = num1->list[start1 + loop2_i] - '0';
				int diff = digit1 - borrow;
				if (diff < 0) {
					diff += 10;
					borrow = 1;
				}
				else {
					borrow = 0;
				}
				result[loop2_i] = diff + '0';
				loop2_i--;
			}
			int zero_index = 0;
			while (zero_index < big - 1 && result[zero_index] == '0') {
				zero_index++;
			}
			for (int i = zero_index; i < big + 1; i++) {
				result[i - zero_index] = result[i];
			}
			result = (char*)realloc(result, (big - zero_index + 1)*sizeof(char));
			charArray* resultArr = (charArray*)malloc(sizeof(charArray));
			if (!resultArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			resultArr->list = result;
			resultArr->length = big - zero_index;
			return resultArr;
		}
		else {
			free(result);
			int loop1_i = len1 - 2;
			while (loop1_i >= 0) {
				int digit1 = num1->list[start1 + loop1_i] - '0';
				int digit2 = num2->list[start2 + loop1_i + dif] - '0';
				int diff = digit1 - digit2 - borrow;
				if (diff < 0) {
					diff += 10;
					borrow = 1;
				}
				else {
					borrow = 0;
				}
				if (loop1_i == len1 - 2 && resultNeg[big] == '0') {
					resultNeg[loop1_i + dif + 1] = (10 - diff) + '0';
				}
				else
					resultNeg[loop1_i + dif + 1] = (9 - diff) + '0';
				loop1_i--;
			}
			int loop2_i = dif - 1;
			while (loop2_i >= 0) {
				int digit2 = num2->list[start2 + loop2_i] - '0';
				int diff = 0 - digit2 - borrow + 10;
				borrow = 1;
				resultNeg[loop2_i + 1] = (9 - diff) + '0';
				loop2_i--;
			}
			int zero_index = 1;
			while (zero_index < big && resultNeg[zero_index] == '0') {
				zero_index++;
			}
			for (int i = zero_index; i < big + 2; i++) {
				resultNeg[i - zero_index + 1] = resultNeg[i];
			}
			resultNeg = (char*)realloc(resultNeg, (big - zero_index + 2)*sizeof(char));
			resultNeg[0] = '-';
			charArray* resultNegArr = (charArray*)malloc(sizeof(charArray));
			if (!resultNegArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
			resultNegArr->list = resultNeg;
			resultNegArr->length = big - zero_index + 1;
			return resultNegArr;
		}
	}
}

charArray* advanced_add(charArray* num1, int start1, int end1, charArray* num2, int start2, int end2) {
	int len1 = end1 - start1;
	int len2 = end2 - start2;
	int big = (len1 > len2) ? len1 : len2;
	char *result = (char *)malloc((big + 2) * sizeof(char));
	if (!result) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	result[big + 1] = '\0';
	int carry = 0;
	if (len1 == len2) {
		int len = len1;
		for (int i = 1; i <= len; i++) {
			int digit1 = num1->list[end1 - i] - '0';
			int digit2 = num2->list[end2 - i] - '0';
			int sum = digit1 + digit2 + carry;
			result[len + 1 - i] = (sum % 10) + '0';
			carry = sum / 10;
		}
	}
	else {
		int dif = abs(len1 - len2);
		if (len2 > len1) {
			int loop1_i = len1 - 1;
			while (loop1_i >= 0) {
				int digit1 = num1->list[start1 + loop1_i] - '0';
				int digit2 = num2->list[start2 + loop1_i + dif] - '0';
				int sum = digit1 + digit2 + carry;
				result[loop1_i + dif + 1] = (sum % 10) + '0';
				carry = sum / 10;
				loop1_i--;
			}
			int loop2_i = dif - 1;
			while (loop2_i >= 0) {
				int digit2 = num2->list[start2 + loop2_i] - '0';
				int sum = digit2 + carry;
				result[loop2_i + 1] = (sum % 10) + '0';
				carry = sum / 10;
				loop2_i--;
			}
		}
		else {
			int loop1_i = len2 - 1;
			while (loop1_i >= 0) {
				int digit1 = num1->list[start1 + loop1_i + dif] - '0';
				int digit2 = num2->list[start2 + loop1_i] - '0';
				int sum = digit1 + digit2 + carry;
				result[loop1_i + dif + 1] = (sum % 10) + '0';
				carry = sum / 10;
				loop1_i--;
			}
			int loop2_i = dif - 1;
			while (loop2_i >= 0) {
				int digit1 = num1->list[start1 + loop2_i] - '0';
				int sum = digit1 + carry;
				result[loop2_i + 1] = (sum % 10) + '0';
				carry = sum / 10;
				loop2_i--;
			}
		}
	}
	result[0] = carry + '0';
	int zero_index = 0;
	while (zero_index < big && result[zero_index] == '0') {
		zero_index++;
	}
	for (int i = zero_index; i < big + 2; i++) {
		result[i - zero_index] = result[i];
	}
	result = (char*)realloc(result, (big + 2 - zero_index)*sizeof(char));
	charArray* resultArr = (charArray*)malloc(sizeof(charArray));
	if (!resultArr) {printf("Memory allocation failed at line: %d\n", __LINE__); exit(EXIT_FAILURE);}
	resultArr->list = result;
	resultArr->length = big - zero_index + 1;
	return resultArr;
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

void print_int_array(char* name, int* array, int len) {
	printf("%s = [", name);
	for (int i = 0; i < len; i++) {
		printf("%d", array[i]);
		if (i < len - 1)
			printf(", ");
	}
	printf("]\n");
}