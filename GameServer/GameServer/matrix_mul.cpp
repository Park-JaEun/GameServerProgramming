/**
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

 /**
  * Matrix multiplication: C = A * B.
  * Host code.
  *
  * This sample implements matrix multiplication which makes use of shared memory
  * to ensure data reuse, the matrix multiplication is done using tiling
  * approach. It has been written for clarity of exposition to illustrate various
  * CUDA programming principles, not with the goal of providing the most
  * performant generic kernel for matrix multiplication. See also: V. Volkov and
  * J. Demmel, "Benchmarking GPUs to tune dense linear algebra," in Proc. 2008
  * ACM/IEEE Conf. on Supercomputing (SC '08), Piscataway, NJ: IEEE Press, 2008,
  * pp. Art. 31:1-11.
  */
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <chrono>

using namespace std::chrono;

struct dim3 { 
    unsigned x; unsigned y; unsigned z;
    dim3(unsigned a, unsigned b, unsigned c) :x(a), y(b), z(c) {}
};

void ConstantInit(float* data, unsigned size, float val) {
    for (unsigned i = 0; i < size; ++i) {
        data[i] = val;
    }
}

void cudaMallocHost(void** h_A, int mem_size_A)
{
    *h_A = reinterpret_cast<void *>(new char[mem_size_A]);
}

/**
 * Run a simple test of matrix multiplication using CPU
 */
int MatrixMultiply_CPU(int argc, char** argv, int block_size, const dim3& dimsA,
    const dim3& dimsB) {
    // Allocate host memory for matrices A and B
    unsigned int size_A = dimsA.x * dimsA.y;
    unsigned int mem_size_A = sizeof(float) * size_A;
    float* h_A;
    cudaMallocHost((void**)&h_A, mem_size_A);
    unsigned int size_B = dimsB.x * dimsB.y;
    unsigned int mem_size_B = sizeof(float) * size_B;
    float* h_B;
    cudaMallocHost((void**)&h_B, mem_size_B);

    // Initialize host memory
    const float valB = 0.01f;
    ConstantInit(h_A, size_A, 1.0f);
    ConstantInit(h_B, size_B, valB);

    // Allocate host matrix C
    dim3 dimsC(dimsB.x, dimsA.y, 1);
    unsigned int mem_size_C = dimsC.x * dimsC.y * sizeof(float);
    float* h_C;
    cudaMallocHost((void**)&h_C, mem_size_C);

    if (h_C == NULL) {
        fprintf(stderr, "Failed to allocate host matrix C!\n");
        exit(EXIT_FAILURE);
    }

    // Create and start timer
    printf("Computing result using CPU ...\n");
    for (unsigned int y = 0; y < dimsA.y; ++y)
        for (unsigned int x = 0; x < dimsB.x; x++) {
            h_C[y * dimsB.x + x] = 0.0f;
            for (unsigned int i = 0; i < dimsA.x; ++i)
                h_C[y * dimsB.x + x] += h_A[i + y * dimsA.x] * h_B[i * dimsB.x + x];
        }

    // Execute the kernel
    int nIter = 3;

    auto start_t = system_clock::now();

    for (int j = 0; j < nIter; j++) {
        for (unsigned int y = 0; y < dimsA.y; ++y)
            for (unsigned int x = 0; x < dimsB.x; x++) {
                h_C[y * dimsB.x + x] = 0.0f;
                for (unsigned int i = 0; i < dimsA.x; ++i)
                    h_C[y * dimsB.x + x] += h_A[i + y * dimsA.x] * h_B[i * dimsB.x + x];
            }
    }

    auto end_t = system_clock::now();
    auto msecTotal = duration_cast<milliseconds>(end_t - start_t).count();

    printf("done\n");

    // Compute and print the performance
    float msecPerMatrixMul = (float)msecTotal / nIter;
    double flopsPerMatrixMul = 2.0 * static_cast<double>(dimsA.x) *
        static_cast<double>(dimsA.y) *
        static_cast<double>(dimsB.x);
    double gigaFlops =
        (flopsPerMatrixMul * 1.0e-9f) / (msecPerMatrixMul / 1000.0f);
    printf(
        "Performance= %.2f GFlop/s, Time= %.3f msec, Size= %.0f Ops\n",
        gigaFlops, msecPerMatrixMul, flopsPerMatrixMul);

    printf("Checking computed result for correctness: ");
    bool correct = true;

    // test relative error by the formula
    //     |<x, y>_cpu - <x,y>_gpu|/<|x|, |y|>  < eps
    double eps = 1.e-6;  // machine zero

    for (int i = 0; i < static_cast<int>(dimsC.x * dimsC.y); i++) {
        double abs_err = fabs(h_C[i] - (dimsA.x * valB));
        double dot_length = dimsA.x;
        double abs_val = fabs(h_C[i]);
        double rel_err = abs_err / abs_val / dot_length;

        if (rel_err > eps) {
            printf("Error! Matrix[%05d]=%.8f, ref=%.8f error term is > %E\n", i,
                h_C[i], dimsA.x * valB, eps);
            correct = false;
        }
    }

    printf("%s\n", correct ? "Result = PASS" : "Result = FAIL");

    printf(
        "\nNOTE: The Samples are not meant for performance"
        "measurements. Results may vary when CPU Boost is enabled.\n");

    if (correct) return EXIT_SUCCESS;
    else return EXIT_FAILURE;
}

int MatrixMultiply_CPU2(int argc, char** argv, int block_size, const dim3& dimsA,
    const dim3& dimsB) {
    // Allocate host memory for matrices A and B
    unsigned int size_A = dimsA.x * dimsA.y;
    unsigned int mem_size_A = sizeof(float) * size_A;
    float* h_A;
    cudaMallocHost((void**)&h_A, mem_size_A);
    unsigned int size_B = dimsB.x * dimsB.y;
    unsigned int mem_size_B = sizeof(float) * size_B;
    float* h_B;
    cudaMallocHost((void**)&h_B, mem_size_B);

    // Initialize host memory
    const float valB = 0.01f;
    ConstantInit(h_A, size_A, 1.0f);
    ConstantInit(h_B, size_B, valB);

    // Allocate host matrix C
    dim3 dimsC(dimsB.x, dimsA.y, 1);
    unsigned int mem_size_C = dimsC.x * dimsC.y * sizeof(float);
    float* h_C;
    cudaMallocHost((void**)&h_C, mem_size_C);

    if (h_C == NULL) {
        fprintf(stderr, "Failed to allocate host matrix C!\n");
        exit(EXIT_FAILURE);
    }

    // Create and start timer
    printf("Computing result using CPU, cache friendly ...\n");
    for (unsigned int y = 0; y < dimsA.y; ++y)
        for (unsigned int x = 0; x < dimsB.x; x++) {
            h_C[y * dimsB.x + x] = 0.0f;
            for (unsigned int i = 0; i < dimsA.x; ++i)
                h_C[y * dimsB.x + x] += h_A[i + y * dimsA.x] * h_B[i * dimsB.x + x];
        }

    // Execute the kernel
    int nIter = 3;

    auto start_t = system_clock::now();

    for (int j = 0; j < nIter; j++) {
        memset(h_C, 0, mem_size_C);
        for (unsigned int y = 0; y < dimsA.y; ++y)
            for (unsigned int x = 0; x < dimsA.x; x++)
                for (unsigned int i = 0; i < dimsB.x; ++i)
                    h_C[y * dimsB.x + i] += h_A[x + y * dimsA.x] * h_B[x * dimsA.x + i];
    }

    auto end_t = system_clock::now();
    auto msecTotal = duration_cast<milliseconds>(end_t - start_t).count();

    printf("done\n");

    // Compute and print the performance
    float msecPerMatrixMul = (float)msecTotal / nIter;
    double flopsPerMatrixMul = 2.0 * static_cast<double>(dimsA.x) *
        static_cast<double>(dimsA.y) *
        static_cast<double>(dimsB.x);
    double gigaFlops =
        (flopsPerMatrixMul * 1.0e-9f) / (msecPerMatrixMul / 1000.0f);
    printf(
        "Performance= %.2f GFlop/s, Time= %.3f msec, Size= %.0f Ops\n",
        gigaFlops, msecPerMatrixMul, flopsPerMatrixMul);

    printf("Checking computed result for correctness: ");
    bool correct = true;

    // test relative error by the formula
    //     |<x, y>_cpu - <x,y>_gpu|/<|x|, |y|>  < eps
    double eps = 1.e-6;  // machine zero

    for (int i = 0; i < static_cast<int>(dimsC.x * dimsC.y); i++) {
        double abs_err = fabs(h_C[i] - (dimsA.x * valB));
        double dot_length = dimsA.x;
        double abs_val = fabs(h_C[i]);
        double rel_err = abs_err / abs_val / dot_length;

        if (rel_err > eps) {
            printf("Error! Matrix[%05d]=%.8f, ref=%.8f error term is > %E\n", i,
                h_C[i], dimsA.x * valB, eps);
            correct = false;
        }
    }

    printf("%s\n", correct ? "Result = PASS" : "Result = FAIL");

    printf(
        "\nNOTE: The Samples are not meant for performance"
        "measurements. Results may vary when CPU Boost is enabled.\n");

    if (correct) return EXIT_SUCCESS;
    else return EXIT_FAILURE;
}


int main(int argc, char** argv)
{

    printf("[Matrix Multiply] - Starting...\n");
    int block_size = 32;
    dim3 dimsA(15 * 2 * block_size, 15 * 2 * block_size, 1);
    dim3 dimsB(15 * 4 * block_size, 15 * 2 * block_size, 1);

    if (dimsA.x != dimsB.y) {
        printf("Error: outer matrix dimensions must be equal. (%d != %d)\n", dimsA.x, dimsB.y);
        exit(EXIT_FAILURE);
    }
    printf("MatrixA(%d,%d), MatrixB(%d,%d)\n", dimsA.x, dimsA.y, dimsB.x, dimsB.y);
    int matrix_result = MatrixMultiply_CPU(argc, argv, block_size, dimsA, dimsB);
    matrix_result = MatrixMultiply_CPU2(argc, argv, block_size, dimsA, dimsB);

    exit(matrix_result);
}
