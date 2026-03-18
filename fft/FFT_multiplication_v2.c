#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <complex.h>
#include <time.h>

int DEBUG = 0;

#define DBG_HEADER(msg) do { if (DEBUG) printf("\n\033[1;36m══════ %s ══════\033[0m\n", msg); } while(0)
#define DBG_VAL(label, fmt, ...) do { if (DEBUG) printf("  \033[33m%-22s\033[0m " fmt "\n", label, __VA_ARGS__); } while(0)
#define DBG_SEP() do { if (DEBUG) printf("\033[90m  ────────────────────────────────\033[0m\n"); } while(0)
clock_t _dbg_t;
#define DBG_TIME_START() _dbg_t = clock()
#define DBG_TIME_END(label) do { if (DEBUG) { double _s = (double)(clock() - _dbg_t) / CLOCKS_PER_SEC; printf("  \033[32m%-22s\033[0m %.6f s\n", label, _s); } } while(0)

typedef struct arr3 {
	char* list;
	int length;
} charArray;

void freecharArray(charArray* arr);
int bitrev(int num, int bits_num);
void swap_complex(double complex* a, double complex* b);
void eval_FFT_iterative(double complex* Xs, double complex* coffs, int n);
double complex* nthRoots(int n);
int next2pow(int deg);
double complex* get_coffs(charArray* num, int n);
void pointwise_mult(double complex* result_buffer, double complex* eval1, double complex* eval2, int n);
void eval_IFFT_iterative(double complex* Xs, double complex* cmplx_result, int n);
charArray* get_final_result(double complex* IFFT_result, int n, int len1, int len2);
charArray* mult_FFT(charArray* num1, charArray* num2);
charArray* mypow(charArray* num, int power);
charArray* getStr(const char * str);
void printCmplx(double complex num);
void print_cmplx_array(char* name, double complex* array, int len);

int main(int argc, char *argv[]) {
	const char *s1 = NULL, *s2 = NULL, *pow_base = NULL;
	int pow_exp = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0) { DEBUG = 1; continue; }
		if (strcmp(argv[i], "-p") == 0 && i + 2 < argc) { pow_base = argv[++i]; pow_exp = atoi(argv[++i]); continue; }
		if (!s1) s1 = argv[i]; else s2 = argv[i];
	}
	if (pow_base) {
		charArray* num = getStr(pow_base);
		charArray* pow_result = mypow(num, pow_exp);
		printf("%s ^ %d = %s\n", num->list, pow_exp, pow_result->list);
		freecharArray(num); freecharArray(pow_result);
	} else {
		if (!s1) { s1 = "434534454354354353454355435345534"; s2 = "999999999999999999999999999999999"; }
		if (!s2) { printf("Usage: %s <num1> <num2> [-d] | -p <base> <exp> [-d]\n", argv[0]); return 1; }
		charArray* num1 = getStr(s1);
		charArray* num2 = getStr(s2);
		charArray* result = mult_FFT(num1, num2);
		printf("%s * %s = %s\n", num1->list, num2->list, result->list);
		freecharArray(num1); freecharArray(num2); freecharArray(result);
	}
}

void freecharArray(charArray* arr) {
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

void swap_complex(double complex* a, double complex* b) {
	double complex temp = *a;
	*a = *b;
	*b = temp;
}

void eval_FFT_iterative(double complex* Xs, double complex* coffs, int n) {
	int bits_num = (int)log2(n);
	for (int i = 0; i < n; i++) {
		int i_rev = bitrev(i, bits_num);
		if (i < i_rev) {
			swap_complex(coffs + i, coffs + i_rev);
		}
	}

	if (DEBUG) print_cmplx_array("coffs_scrambled", coffs, n);

	for (int size = 2; size <= n; size *= 2) {
		for (int i = 0; i < n; i+= size) {
			for (int j = i; j < i + size/2; j++) {
				double complex even = coffs[j];
				double complex twiddle_factor = Xs[(j - i) * (n / size)] * coffs[j + size/2];
				coffs[j] = even + twiddle_factor;
				coffs[j + size/2] = even - twiddle_factor;
			}
		}
	}
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

void eval_IFFT_iterative(double complex* Xs, double complex* cmplx_result, int n) {
	int bits_num = (int)log2(n);
	for (int i = 0; i < n; i++) {
		int i_rev = bitrev(i, bits_num);
		if (i < i_rev) {
			swap_complex(cmplx_result + i, cmplx_result + i_rev);
		}
	}

	if (DEBUG) print_cmplx_array("cmplx_result_scrambled", cmplx_result, n);

	for (int size = 2; size <= n; size *= 2) {
		for (int i = 0; i < n; i+= size) {
			for (int j = i; j < i + size/2; j++) {
				double complex even = cmplx_result[j];
				double complex twiddle_factor = conj(Xs[(j - i) * (n / size)]) * cmplx_result[j + size/2];
				cmplx_result[j] = even + twiddle_factor;
				cmplx_result[j + size/2] = even - twiddle_factor;
			}
		}
	}
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

	DBG_HEADER("FFT MULTIPLICATION (iterative)");
	DBG_VAL("num1 length", "%d digits", len1);
	DBG_VAL("num2 length", "%d digits", len2);
	DBG_VAL("polynomial degree", "%d", deg);
	DBG_VAL("padded size (n)", "%llu", n);
	DBG_SEP();

	DBG_TIME_START();
	double complex* coffs1 = get_coffs(num1, n);
	double complex* coffs2 = get_coffs(num2, n);
	DBG_TIME_END("coefficients");

	if (DEBUG) print_cmplx_array("coffs1", coffs1, n);
	if (DEBUG) print_cmplx_array("coffs2", coffs2, n);

	double complex* Xs = nthRoots(n);

	if (DEBUG) print_cmplx_array("nth roots", Xs, n);

	DBG_TIME_START();
	eval_FFT_iterative(Xs, coffs1, n);
	eval_FFT_iterative(Xs, coffs2, n);
	DBG_TIME_END("forward FFT");
	double complex* eval1 = coffs1;
	double complex* eval2 = coffs2;

	if (DEBUG) print_cmplx_array("eval1", eval1, n);
	if (DEBUG) print_cmplx_array("eval2", eval2, n);

	DBG_TIME_START();
	pointwise_mult(eval1, eval1, eval2, n);
	DBG_TIME_END("pointwise multiply");
	free(eval2);
	double complex* mult_result = eval1;
	
	if (DEBUG) print_cmplx_array("mult_result", mult_result, n);

	DBG_TIME_START();
	eval_IFFT_iterative(Xs, mult_result, n);
	DBG_TIME_END("inverse FFT");
	double complex* IFFT_eval = mult_result;

	if (DEBUG) print_cmplx_array("unscaled_IFFT_eval", IFFT_eval, n);

	free(Xs);

	for (int i = 0; i < n; i++) {
		IFFT_eval[i] /= n;
	}

	if (DEBUG) print_cmplx_array("scaled_IFFT_eval", IFFT_eval, n);

	charArray* final_result = get_final_result(IFFT_eval, n, len1, len2);
	free(IFFT_eval);

	DBG_SEP();
	DBG_VAL("result length", "%d digits", final_result->length);

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