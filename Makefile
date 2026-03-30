CC=gcc
CFLAGS=-O3 -Wall -Wno-unused-variable -pthread -I./utils
LDFLAGS=-lm

NTT_SRCS=$(wildcard ntt/*.c)
FFT_SRCS=$(wildcard fft/*.c)
KARATSUBA_SRCS=$(wildcard karatsuba/*.c)
UTILS_SRC=utils/utils.c

NTT_OBJS=$(NTT_SRCS:.c=.o)
FFT_OBJS=$(FFT_SRCS:.c=.o)
KARATSUBA_OBJS=$(KARATSUBA_SRCS:.c=.o)
UTILS_OBJ=$(UTILS_SRC:.c=.o)

NTT_EXES=$(NTT_SRCS:.c=.exe)
FFT_EXES=$(FFT_SRCS:.c=.exe)
KARATSUBA_EXES=$(KARATSUBA_SRCS:.c=.exe)

all: ntt fft karatsuba

ntt: $(NTT_EXES)

fft: $(FFT_EXES)

karatsuba: $(KARATSUBA_EXES)

%.exe: %.c $(UTILS_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(UTILS_OBJ): $(UTILS_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f utils/*.o ntt/*.o fft/*.o karatsuba/*.o ntt/*.exe fft/*.exe karatsuba/*.exe
