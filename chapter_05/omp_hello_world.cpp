#ifdef _OPENMP
#include <omp.h>
#endif

#include <iostream>

int main() {
#ifndef _OPENMP
  std::cout << "OpenMP is not supported here -- sorry.\n";
#endif

#pragma omp parallel
  {
#if _OPENMP
    int my_rank = omp_get_thread_num();
    int thread_count = omp_get_num_threads();
#else
    int my_rank = 0;
    int thread_count = 1;
#endif
    std::string s = "Hello from thread " + std::to_string(my_rank) + " of " +
                    std::to_string(thread_count) + "\n";
    std::cout << s;
  }

  return 0;
}
