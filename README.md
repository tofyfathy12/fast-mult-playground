# 🚀 Fast Multiplication Playground

Welcome to the **Fast Multiplication Playground**! This repository is a dedicated space for exploring, implementing, and optimizing high-performance multiplication algorithms in C. It includes classical divide-and-conquer approaches as well as advanced transform-based methods, complete with multithreading support for large-scale computations (multithreading appraoch is not fully implemented yet).

## 🎯 Algorithms Included

This playground features several approaches to fast polynomial and large-integer multiplication:

- **Fast Fourier Transform (FFT):** $O(N \log N)$ multiplication using complex numbers representing roots of unity.
- **Number Theoretic Transform (NTT):** $O(N \log N)$ multiplication over a finite field (modulo prime arithmetic), avoiding floating-point precision issues inherent to FFT.
- **Karatsuba Algorithm:** A $O(N^{\log_2 3}) \approx O(N^{1.585})$ divide-and-conquer approach that is faster than the traditional quadratic $O(N^2)$ method for moderately large numbers.

## 📂 Project Structure

The repository is neatly organized into modular components:

```text
fast-mult-playground/
├── fft/            # Fast Fourier Transform implementations
├── ntt/            # Number Theoretic Transform implementations (v1 to v5)
├── karatsuba/      # Karatsuba multiplication implementations
├── threading/      # Multithreading utilities and Thread Pool implementations
├── utils/          # Shared utilities (e.g., Queue data structures)
├── scripts/        # Python helper scripts (Prime generators, etc.)
└── data/           # Output data and configuration text files
```

## 🛠️ Getting Started

### Prerequisites

To compile and run the source code, you will need:
- A C compiler (e.g., GCC or Clang)
- [pthread](https://en.wikipedia.org/wiki/POSIX_Threads) library for multithreading (typically built-in on Linux/macOS, available via MinGW-w64 on Windows)

### Compilation

You can compile individual implementations using your preferred C compiler. For example, to compile the latest NTT version (which utilizes pthreads):

```bash
gcc -o ntt_v5 ntt/NTT_multiplication_v5.c -lpthread -lm
```

> **Note:** The `-lm` flag links the math library, and `-lpthread` links the POSIX threads library.

### Execution

Once compiled, simply run the executable:

```bash
./ntt_v5
```

## 🧠 Why NTT over FFT?

While both algorithms achieve $O(N \log N)$ time complexity, the Number Theoretic Transformoperates purely on integers modulo a specific prime number. This eliminates the floating-point inaccuracies associated with sine/cosine computations in FFT, ensuring **exact** answers for large integer multiplication.

## 🤝 Contributing

This is a dynamic playground! Feel free to fork the repository, experiment with the algorithms, apply further micro-optimizations, or add new multiplication strategies like the Schönhage–Strassen or Toom-Cook algorithms.

## 📜 License

This project is open-source and available for educational and recreational programming. Explore, learn, and compute faster!
