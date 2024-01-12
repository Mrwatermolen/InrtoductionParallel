# Introduction Parallel

## Just Note

### 1. Histogram Bin Test

Input Case:

197824571
0
20
7
6
9

``` text
n: 197824571
min: 0
max: 20
bin_n: 7
bin_min: 6
bin_max: 9
```

build in `Release` mode

MPI Result:

command: `mpirun -n 8 ./build/bin/mpi_histogram_bin`

- MPI Data Copy Time: 205 ms
- MPI Elapsed Time: 571 ms
- Serial Elapsed Time: 1489 ms
- Speed up: 2.60696 (include data copy)
- Efficiency: 0.32587

OMP Result:

command: `./build/bin/omp_histogram_bin 8`

- Omp Elapsed Time: 469 ms
- Serial Elapsed Time: 1412 ms
- Speed up: 3.00986
- Efficiency: 0.376232

C++ Thread Result:

command: `./build/bin/thread_histogram_bin 8`

- Thread Elapsed Time: 222 ms
- Serial Elapsed Time: 1409 ms
- Speed up: 6.32266
- Efficiency: 0.790332
