#include "exercises/hello_world.h"

int main(int argc, char **argv) {
  exercises::hello_world::ompPrintHelloWorld();
  return 0;
}

// #include <cstdio>
// #include <cstdlib>
// #include <omp.h>

// int main() {
// #pragma omp parallel
//     {
//         printf("The parallel region is executed by thread %d\n",
//                 omp_get_thread_num());
//         if ( omp_get_thread_num() == 2 ) {
//             printf(" Thread %d does things differently\n", omp_get_thread_num());
//         }
//     }
//     return 0;
// }