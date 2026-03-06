#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <complex.h>
#include <time.h>

typedef struct arr3 {
	char* list;
	int length;
} charArray;

void freecharArray(charArray* arr);
double complex* eval_FFT(double complex* Xs, int org_n, double complex* coffs, int n, int start, int step);
double complex* nthRoots(int n);
int next2pow(int deg);
double complex* get_coffs(charArray* num, int n);
void pointwise_mult(double complex* result_buffer, double complex* eval1, double complex* eval2, int n);
double complex* eval_IFFT(double complex* Xs, int org_n, double complex* cmplx_result, int n, int start, int step);
charArray* get_final_result(double complex* IFFT_result, int n, int len1, int len2);
charArray* mult_FFT(charArray* num1, charArray* num2);
charArray* mypow(charArray* num, int power);
charArray* getStr(const char * str);
void printCmplx(double complex num);
void print_cmplx_array(char* name, double complex* array, int len);
void print_int_array(char* name, int* array, int len);

int main(int argc, char *argv[]) {
    charArray* num1 = getStr("88888");
    charArray* num2 = getStr("99999");

    charArray* result = mult_FFT(num1, num2);

    printf("%s * %s = %s\n", num1->list, num2->list, result->list);

	charArray* num = getStr("2");
	int power = 30;
	charArray* pow_result = mypow(num, power);

	printf("%s ^ %d = %s\n", num->list, power, pow_result->list);

    freecharArray(num1);
    freecharArray(num2);
    freecharArray(result);
	freecharArray(num);
	freecharArray(pow_result);
}

void freecharArray(charArray* arr) {
	free(arr->list);
	free(arr);
}

double complex* eval_FFT(double complex* Xs, int org_n, double complex* coffs, int n, int start, int step) {
	if (n == 1) {
		double complex* result = (double complex*)malloc(sizeof(double complex));
		result[0] = coffs[start];
		return result;
	}

	double complex* evenEval = eval_FFT(Xs, org_n, coffs, n/2, start, step * 2);
	double complex* oddEval = eval_FFT(Xs, org_n, coffs, n/2, start + step, step * 2);

	double complex* result = (double complex*)malloc(n*sizeof(double complex));
	for (int i = 0; i < n/2; i++) {
		double complex twiddle_factor = Xs[i * (org_n / n)] * oddEval[i];
		result[i] = evenEval[i] + twiddle_factor;
		result[i + n/2] = evenEval[i] - twiddle_factor;
	}
	free(evenEval);
	free(oddEval);
	return result;
}

double complex* nthRoots(int n) {
	double complex* roots = (double complex*)malloc(n * sizeof(double complex));
	double complex constant = 2 * M_PI * I / n;
	for (int k = 0; k < n; k++) {
		roots[k] = cexp(constant * k);
	}
	return roots;
}

int next2pow(int deg) {
	int power = (int)ceil(log2(deg));
	int result = (int)pow(2, power);
	return result;
}

double complex* get_coffs(charArray* num, int n) {
	int len = num->length;
	double complex* result = (double complex*)malloc(n*sizeof(double complex));
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

void pointwise_mult(double complex* result_buffer, double complex* eval1, double complex* eval2, int n) {
	for (int i = 0; i < n; i++) {
		result_buffer[i] = eval1[i] * eval2[i];
	}
}

double complex* eval_IFFT(double complex* Xs, int org_n, double complex* cmplx_result, int n, int start, int step) {
	if (n == 1) {
		double complex* result = (double complex*)malloc(sizeof(double complex));
		result[0] = cmplx_result[start];
		return result;
	}

	double complex* evenEval = eval_IFFT(Xs, org_n, cmplx_result, n/2, start, step * 2);
	double complex* oddEval = eval_IFFT(Xs, org_n, cmplx_result, n/2, start + step, step * 2);

	double complex* result = (double complex*)malloc(n*sizeof(double complex));
	for (int i = 0; i < n/2; i++) {
		double complex twiddle_factor = conj(Xs[i * (org_n / n)]) * oddEval[i];
		result[i] = (evenEval[i] + twiddle_factor);
		result[i + n/2] = (evenEval[i] - twiddle_factor);
	}
	free(evenEval);
	free(oddEval);
	return result;
}

charArray* get_final_result(double complex* IFFT_result, int n, int len1, int len2) {
	int len = len1 + len2;
	char* result = (char*)malloc((len + 1) * sizeof(char));
	char* result_small = (char*)malloc((len) * sizeof(char));
	result[len] = '\0';
	result_small[len - 1] = '\0';
	int carry = 0;
	for (int i = 0; i < n && i < (len - 1); i++) {
		int sum = (int)(creal(IFFT_result[i]) + 0.5) + carry;
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

charArray* mult_FFT(charArray* num1, charArray* num2) {
	int len1 = num1->length;
	int len2 = num2->length;
	int deg = len1 + len2 - 1;
	unsigned long long n = next2pow(deg);
	// printf("n (next power of 2) = %lld\n", n); //debugging
	double complex* coffs1 = get_coffs(num1, n);
	double complex* coffs2 = get_coffs(num2, n);

	// print_cmplx_array("coffs1", coffs1, n); //debugging
	// print_cmplx_array("coffs2", coffs2, n); //debugging

	double complex* Xs = nthRoots(n);

	// print_cmplx_array("nth roots", Xs, n); //debugging

	double complex* eval1 = eval_FFT(Xs, n, coffs1, n, 0, 1);
	double complex* eval2 = eval_FFT(Xs, n, coffs2, n, 0, 1);

    free(coffs1); free(coffs2);
	// print_cmplx_array("eval1", eval1, n); //debugging
	// print_cmplx_array("eval2", eval2, n); //debugging

	pointwise_mult(eval1, eval1, eval2, n);
	free(eval2);
	double complex* mult_result = eval1;
	
	// print_cmplx_array("mult_result", mult_result, n); //debugging

	// eval_IFFT_iterative(Xs, mult_result, n);
    double complex* IFFT_eval = eval_IFFT(Xs, n, mult_result, n, 0, 1);
    free(mult_result); free(Xs);

	// print_cmplx_array("unscaled_IFFT_eval", IFFT_eval, n); //debugging
	
	for (int i = 0; i < n; i++) {
		IFFT_eval[i] /= n;
	}

//	print_cmplx_array("scaled_IFFT_eval", IFFT_eval, n); //debugging

	charArray* final_result = get_final_result(IFFT_eval, n, len1, len2);
	free(IFFT_eval);

	return final_result;
}

charArray* mypow(charArray* num, int power) {
	charArray* result = getStr("1");
	charArray* Num = getStr(num->list);
	while (power > 0) {
		if ((power & 1)) { // odd
			charArray* temp_result = mult_FFT(result, Num);
			freecharArray(result);
			result = temp_result;
			power--;
		}
		else { // even
			charArray* temp_Num = mult_FFT(Num, Num);
			freecharArray(Num);
			Num = temp_Num;
			power /= 2;
		}
	}
	freecharArray(Num);
	return result;
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

void printCmplx(double complex num) {
	double real = creal(num);
	double imag = cimag(num);
	char neg = (imag > 0) ? '+' : '\0';
	if (real != 0 && imag != 0) {
		printf("%.2f %c %.2fi", real, neg, imag);
	}
	else if (real != 0 && imag == 0) {
		printf("%.2f", real);
	}
	else if (real == 0 && imag != 0) {
		printf("%.2fi", imag);
	}
	else
		printf("0");
}

void print_cmplx_array(char* name, double complex* array, int len) {
	printf("%s = [", name);
	for (int i = 0; i < len; i++) {
		printCmplx(array[i]);
		if (i < len - 1)
			printf(", ");
	}
	printf("]\n");
}