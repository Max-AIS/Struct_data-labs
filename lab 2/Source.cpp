#include <iostream>
#include <complex>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cblas.h>


using namespace std;
using namespace std::chrono;
using Complex = complex<double>;

vector<Complex> generate_random_matrix(int n) {
    vector<Complex> mat(n * n);
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dist(-1.0, 1.0);
    for (int i = 0; i < n * n; ++i) {
        mat[i] = Complex(dist(gen), dist(gen));
    }
    return mat;
}

void naive_multiply(const vector<Complex>& A, const vector<Complex>& B,
    vector<Complex>& C, int n) {
    fill(C.begin(), C.end(), Complex(0.0, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            Complex aik = A[i * n + k];
            for (int j = 0; j < n; ++j) {
                C[i * n + j] += aik * B[k * n + j];
            }
        }
    }
}

void optimized_multiply(const vector<Complex>& A,
    const vector<Complex>& B,
    vector<Complex>& C,
    int n) {

    fill(C.begin(), C.end(), Complex(0.0, 0.0));

    vector<Complex> Bt(n * n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            Bt[j * n + i] = B[i * n + j];
        }
    }

    unsigned int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    vector<thread> threads;

    auto worker = [&](int start_row, int end_row) {
        for (int i = start_row; i < end_row; ++i) {
            const Complex* a_row = &A[i * n];
            Complex* c_row = &C[i * n];

            for (int j = 0; j < n; ++j) {
                Complex sum(0.0, 0.0);
                const Complex* b_col = &Bt[j * n];

                int k = 0;
                for (; k + 3 < n; k += 4) {
                    sum += a_row[k] * b_col[k];
                    sum += a_row[k + 1] * b_col[k + 1];
                    sum += a_row[k + 2] * b_col[k + 2];
                    sum += a_row[k + 3] * b_col[k + 3];
                }
                for (; k < n; ++k) {
                    sum += a_row[k] * b_col[k];
                }
                c_row[j] = sum;
            }
        }
        };

    int rows_per_thread = n / num_threads;
    for (unsigned int t = 0; t < num_threads; ++t) {
        int start = t * rows_per_thread;
        int end = (t == num_threads - 1) ? n : (t + 1) * rows_per_thread;
        threads.emplace_back(worker, start, end);
    }

    for (auto& th : threads) {
        th.join();
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    const int n = 2048;
    const double c = 2.0 * n * n * n;

    cout << "Перемножение матриц " << n << "x" << n << endl;

    auto A = generate_random_matrix(n);
    auto B = generate_random_matrix(n);

    vector<Complex> C_naive(n * n);
    vector<Complex> C_blas(n * n);
    vector<Complex> C_opt(n * n);

    cout << "\n[1] Наивный алгоритм" << endl;
    auto start = high_resolution_clock::now();
    naive_multiply(A, B, C_naive, n);
    auto end = high_resolution_clock::now();
    double time_naive = duration<double>(end - start).count();
    double mflops_naive = c / time_naive * 1e-6;
    cout << "    Время: " << time_naive << " с" << endl;
    cout << "    Производительность: " << mflops_naive << " MFLOPS" << endl;

    cout << "\n[2] cblas_zgemm из OpenBLAS" << endl;
    double alpha = 1.0;
    double beta = 0.0;
    start = high_resolution_clock::now();
    cblas_zgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n, &alpha, A.data(), n, B.data(), n, &beta, C_blas.data(),n);
    end = high_resolution_clock::now();
    double time_blas = duration<double>(end - start).count();
    double mflops_blas = c / time_blas * 1e-6;
    cout << "    Время: " << time_blas << " с" << endl;
    cout << "    Производительность: " << mflops_blas << " MFLOPS" << endl;

    cout << "\n[3] Оптимизированный алгоритм (транспонирование + потоки + размотка)" << endl;
    start = high_resolution_clock::now();
    optimized_multiply(A, B, C_opt, n);
    end = high_resolution_clock::now();
    double time_opt = duration<double>(end - start).count();
    double mflops_opt = c / time_opt * 1e-6;
    cout << "    Время: " << time_opt << " с" << endl;
    cout << "    Производительность: " << mflops_opt << " MFLOPS" << endl << endl;

    double percent = (mflops_opt / mflops_blas) * 100.0;
    cout << "Оптимизированный алгоритм достигает " << percent << "% от BLAS" << endl;

    return 0;
}