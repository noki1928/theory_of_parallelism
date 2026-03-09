#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>


double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}


void matrix_vector_product(double *a, double *b, double *c, size_t m, size_t n)
{
    for (int i = 0; i < m; i++)
    {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }
}


double run_serial(size_t n, size_t m)
{
    double *a, *b, *c;
    a = (double*)malloc(sizeof(*a) * m * n);
    b = (double*)malloc(sizeof(*b) * n);
    c = (double*)malloc(sizeof(*c) * m);

    if (a == NULL || b == NULL || c == NULL)
    {
        free(a);
        free(b);
        free(c);
        printf("Error allocate memory!\n");
        exit(1);
    }

    

    for (size_t i = 0; i < m; i++)
    {
        for (size_t j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }

    for (size_t j = 0; j < n; j++)
        b[j] = j;


    double t = cpuSecond();

    matrix_vector_product(a, b, c, m, n);
    t = cpuSecond() - t;

    printf("Elapsed time (serial): %.6f sec.\n", t);
    free(a);
    free(b);
    free(c);

    return t;
}


//=============================================================================


void matrix_vector_product_omp(double *a, double *b, double *c, size_t m, size_t n)
{
    #pragma omp parallel for
    for (int i = 0; i < m; i++)
    {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }
    
}


double run_parallel(size_t n, size_t m)
{
    double *a, *b, *c;

    a = (double*)malloc(sizeof(*a) * m * n);
    b = (double*)malloc(sizeof(*b) * n);
    c = (double*)malloc(sizeof(*c) * m);

    if (a == NULL || b == NULL || c == NULL)
    {
        free(a);
        free(b);
        free(c);
        printf("Error allocate memory!\n");
        exit(1);
    }


    #pragma omp parallel 
        {

            #pragma omp for
            for (size_t i = 0; i < m; i++)
            {
                for (size_t j = 0; j < n; j++)
                    a[i * n + j] = i + j;
            }

            #pragma omp for
            for (size_t j = 0; j < n; j++)
                b[j] = j;

        }
        
    double t = cpuSecond();

    matrix_vector_product_omp(a, b, c, m, n);

    t = cpuSecond() - t;

    printf("Elapsed time (parallel): %.6f sec.\n", t);
    free(a);free(b);free(c);

    return t;
}
    


int main()
{
    
    size_t M = 20'000;
    size_t N = 20'000;

    int threads[7] = {2, 4, 6, 8, 16, 20, 40};

    double T = run_serial(M, N);

    for(int i = 0; i < 7; i++) {;
        omp_set_num_threads(threads[i]);

        printf("Threads count: %d\n", threads[i]);
        double T_p = run_parallel(M, N);
        double S_p = T / T_p;
        printf("Sp:%lf\n\n", S_p);

    }
    return 0;
}
