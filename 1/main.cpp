#include <iostream>
#include <vector>
#include <cmath>

#ifdef DOUBLE
typedef double R;
#else
typedef float R;
#endif


using namespace std;

int main() {

    unsigned long N = 10000000;

    vector<R> array(N);
    
    for(unsigned long i = 0; i < N; i++) {
        double angle = 2 * M_PI * static_cast<double>(i) / N;
        array[i] = sin(angle);
    }


    double sum = 0;
    for(auto val : array) {
        sum += val;
    }

    cout << sum << endl;
}