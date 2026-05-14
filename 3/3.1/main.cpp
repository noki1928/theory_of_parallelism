#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using Matrix = vector<double>;
using Vec = vector<double>;
using hrc = chrono::steady_clock;

void parallel_init(Matrix& A, Vec& x, int N, int num_threads) {
    vector<jthread> matrix_threads;
    matrix_threads.reserve(static_cast<size_t>(num_threads));

    int rows_per = N / num_threads;
    int rem = N % num_threads;
    int start = 0;

    for(int t = 0; t < num_threads; t++) {
        int end = start + rows_per + (t < rem ? 1 : 0);

        matrix_threads.emplace_back([&A, N, start, end]() {
            for (int i = start; i < end; i++) {
                for (int j = 0; j < N; j++) {
                    A[i * N + j] = 1.0;
                }
            }
        });
        start = end;
    }

    vector<jthread> vector_threads;
    vector_threads.reserve(static_cast<size_t>(num_threads));

    int elems_per = N / num_threads;
    int elems_rem = N % num_threads;
    int x_start = 0;

    for(int t = 0; t < num_threads; t++) {
        int x_end = x_start + elems_per + (t < elems_rem ? 1 : 0);

        vector_threads.emplace_back([&x, x_start, x_end]() {
            for(int i = x_start; i < x_end; i++) {
                x[i] = 1.0;
            }
        });
        x_start = x_end;
    }
}

void parallel_matvec(const Matrix& A, const Vec& x, Vec& y, int N, int num_threads) {
    vector<jthread> threads;
    threads.reserve(static_cast<size_t>(num_threads));

    int rows_per = N / num_threads;
    int rem = N % num_threads;
    int start = 0;

    for(int t = 0; t < num_threads; t++) {
        int end = start + rows_per + (t < rem ? 1 : 0);

        threads.emplace_back([&A, &x, &y, N, start, end]() {
            for(int i = start; i < end; i++) {
                double sum = 0.0;

                for(int j = 0; j < N; j++) {
                    sum += A[i * N + j] * x[j];
                }
                y[i] = sum;
            }
        });
        start = end;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <M=N> <num_threads>\n";
        return 1;
    }
    int M = stoi(argv[1]);
    int N = M;
    int p = stoi(argv[2]);

    Matrix A(M * N);
    Vec x(N), y(M);

    auto t0 = hrc::now();

    if (p == 1) {
        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++)
                A[i * N + j] = 1.0;
        for (auto& v : x) v = 1.0;
        
        t0 = hrc::now();

        for (int i = 0; i < M; ++i) {
            double sum = 0.0;
            for (int j = 0; j < N; ++j) 
                sum += A[i * N + j] * x[j];
            y[i] = sum;
        }
    } else {
        parallel_init(A, x, N, 40);
        t0 = hrc::now();
        parallel_matvec(A, x, y, N, p);
    }

    auto t1 = hrc::now();
    double time = chrono::duration<double>(t1 - t0).count();

    cout << fixed << setprecision(4);
    cout << "M=N=" << M << ", p=" << p << ", Time=" << time << " s\n";
    return 0;
}
