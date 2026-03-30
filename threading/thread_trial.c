#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

typedef struct arr2 {
	char* list;
	int length;
} charArray;

typedef struct cfsdta {
    unsigned long long* coffs_buffer;
    charArray* num;
    int n;
    int chunk_length;
} coffs_data;

void freecharArray(charArray* arr);
unsigned long long next2pow(unsigned long long deg);
void* get_coffs_NTT_fast_base(void* data);
charArray* getStr(const char * str);
void print_longlong_array(char* name, unsigned long long* array, int len);

int main(int argc, char *argv[]) {
    int chunk_length = 5;
    charArray* num1 = getStr("3284923740740");
    charArray* num2 = getStr("309782340317478");
    int chunks1 = (num1->length % chunk_length == 0) ? num1->length / chunk_length : num1->length / chunk_length + 1;
    int chunks2 = (num2->length % chunk_length == 0) ? num2->length / chunk_length : num2->length / chunk_length + 1;
    
    int n = next2pow(chunks1 + chunks2 - 1);
    int coffs_buffer_size = 4 * n;
    unsigned long long* coffs_buffer = (unsigned long long*)malloc(coffs_buffer_size * sizeof(unsigned long long));
    unsigned long long* coffs1 = coffs_buffer;
    unsigned long long* coffs2 = coffs1 + n;

    coffs_data data1 = {coffs1, num1, n, chunk_length};
    coffs_data data2 = {coffs2, num2, n, chunk_length};

    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, get_coffs_NTT_fast_base, &data1);
    pthread_create(&thread2, NULL, get_coffs_NTT_fast_base, &data2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    print_longlong_array("coffs1", coffs1, n);
    print_longlong_array("coffs2", coffs2, n);

    freecharArray(num1);
    freecharArray(num2);
    free(coffs_buffer);
}

void freecharArray(charArray* arr) {
	free(arr->list);
	free(arr);
}

unsigned long long next2pow(unsigned long long deg) {
	int power = (int)ceil(log2(deg));
	unsigned long long result = (unsigned long long)pow(2, power);
	return result;
}
void* get_coffs_NTT_fast_base(void* data) {
    coffs_data* the_data = (coffs_data*) data;
    unsigned long long* coffs_buffer = the_data->coffs_buffer;
    charArray* num = the_data->num;
    int n = the_data->n;
    int chunk_length = the_data->chunk_length;

	int len = num->length;
	int index = 0;
	int i = len - 1;
	while (i >= 0) {
		unsigned long long count = 0;
		unsigned long long temp_num = 0;
		while (count < chunk_length && i >= 0) {
			unsigned long long digit = (unsigned long long)(num->list[i] - '0');
			temp_num += (unsigned long long)(pow(10, count) + 0.5) * digit;
			i--;
			count++;
		}
		coffs_buffer[index] = temp_num;
		index++;
	}
	while (index < n) {
		coffs_buffer[index] = 0;
		index++;
	}
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

void print_longlong_array(char* name, unsigned long long* array, int len) {
	printf("%s = [", name);
	for (int i = 0; i < len; i++) {
		printf("%llu", array[i]);
		if (i < len - 1)
			printf(", ");
	}
	printf("]\n");
}