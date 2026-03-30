#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <complex.h>
#include <time.h>
#include "../utils/utils.h"

double complex* eval_FFT(double complex* Xs, int org_n, double complex* coffs, int n, int start, int step);
double complex* nthRoots(int n);

double complex* get_coffs(charArray* num, int n);
void pointwise_mult(double complex* result_buffer, double complex* eval1, double complex* eval2, int n);
double complex* eval_IFFT(double complex* Xs, int org_n, double complex* cmplx_result, int n, int start, int step);
charArray* get_final_result(double complex* IFFT_result, int n, int len1, int len2);
charArray* mult_FFT(charArray* num1, charArray* num2);
charArray* mypow(charArray* num, int power);

void printCmplx(double complex num);
void print_cmplx_array(char* name, double complex* array, int len);

int main(int argc, char *argv[]) {
	const char *s1 = NULL, *s2 = NULL, *pow_base = NULL;
	int pow_exp = 0;
	DEBUG_OP = DBG_OP_MULT;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-d1") == 0) { DEBUG = 1; continue; }
		if (strcmp(argv[i], "-d2") == 0) { DEBUG = 2; continue; }
		if (strcmp(argv[i], "-p") == 0 && i + 2 < argc) { pow_base = argv[++i]; pow_exp = atoi(argv[++i]); DEBUG_OP = DBG_OP_POW; continue; }
		if (!s1) s1 = argv[i]; else s2 = argv[i];
	}
	if (pow_base) {
		charArray* num = getStr(pow_base);
		charArray* pow_result = mypow(num, pow_exp);
		printf("%s ^ %d = %s\n", num->list, pow_exp, pow_result->list);
		freecharArray(num); freecharArray(pow_result);
	} else {
		if (!s1) { s1 = "88888"; s2 = "99999"; }
		if (!s2) { printf("Usage: %s <num1> <num2> [-d] | -p <base> <exp> [-d]\n", argv[0]); return 1; }
		charArray* num1 = getStr(s1);
		charArray* num2 = getStr(s2);
		charArray* result = mult_FFT(num1, num2);
		printf("%s * %s = %s\n", num1->list, num2->list, result->list);
		freecharArray(num1); freecharArray(num2); freecharArray(result);
	}
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
	if ((num1->length == 1 && num1->list[0] == '0') || (num2->length == 1 && num2->list[0] == '0')) {
		return getStr("0");
	}
	int len1 = num1->length;
	int len2 = num2->length;
	int deg = len1 + len2 - 1;
	unsigned long long n = next2pow(deg);

	DBG_HEADER("FFT MULTIPLICATION");
	DBG_VAL("num1 length", "%d digits", len1);
	DBG_VAL("num2 length", "%d digits", len2);
	DBG_VAL("polynomial degree", "%d", deg);
	DBG_VAL("padded size (n)", "%llu", n);
	DBG_SEP();

	DBG_TIME_START();
	double complex* coffs1 = get_coffs(num1, n);
	double complex* coffs2 = get_coffs(num2, n);
	DBG_TIME_END("coefficients");

	if (DEBUG >= 2) print_cmplx_array("coffs1", coffs1, n);
	if (DEBUG >= 2) print_cmplx_array("coffs2", coffs2, n);

	double complex* Xs = nthRoots(n);

	if (DEBUG >= 2) print_cmplx_array("nth roots", Xs, n);

	DBG_TIME_START();
	double complex* eval1 = eval_FFT(Xs, n, coffs1, n, 0, 1);
	double complex* eval2 = eval_FFT(Xs, n, coffs2, n, 0, 1);
	DBG_TIME_END("forward FFT");

    free(coffs1); free(coffs2);
	if (DEBUG >= 2) print_cmplx_array("eval1", eval1, n);
	if (DEBUG >= 2) print_cmplx_array("eval2", eval2, n);

	DBG_TIME_START();
	pointwise_mult(eval1, eval1, eval2, n);
	DBG_TIME_END("pointwise multiply");
	free(eval2);
	double complex* mult_result = eval1;

	if (DEBUG >= 2) print_cmplx_array("mult_result", mult_result, n);

	DBG_TIME_START();
    double complex* IFFT_eval = eval_IFFT(Xs, n, mult_result, n, 0, 1);
    free(mult_result); free(Xs);
	DBG_TIME_END("inverse FFT");

	if (DEBUG >= 2) print_cmplx_array("unscaled_IFFT_eval", IFFT_eval, n);

	for (int i = 0; i < n; i++) {
		IFFT_eval[i] /= n;
	}

	if (DEBUG >= 2) print_cmplx_array("scaled_IFFT_eval", IFFT_eval, n);

	charArray* final_result = get_final_result(IFFT_eval, n, len1, len2);
	free(IFFT_eval);

	DBG_SEP();
	DBG_VAL("result length", "%d digits", final_result->length);

	return final_result;
}

charArray* mypow(charArray* num, int power) {
	DBG_POW_HEADER("EXPONENTIATION FFT");
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
	int t = DBG_ARRAY_TRUNC_LEN;
	printf("%s = [", name);
	if (len <= 2 * t) {
		for (int i = 0; i < len; i++) {
			printCmplx(array[i]);
			if (i < len - 1) printf(", ");
		}
	} else {
		for (int i = 0; i < t; i++) { printCmplx(array[i]); printf(", "); }
		printf("... (%d more) ... ", len - 2 * t);
		for (int i = len - t; i < len; i++) {
			printCmplx(array[i]);
			if (i < len - 1) printf(", ");
		}
	}
	printf("]\n");
}