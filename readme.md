# Introduction Parallel

## Just Note

build in `Release` mode

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

### 2. Trap Integral Test

Input Case:

269845164 -10 10
```
n: 269845164
min: -10
max: 10
```

MPI Result:

command: `mpirun -n 8 ./build/bin/mpi_trap_integral`

- MPI Task Data Copy Time: 0 ms
- MPI Elapsed Time: 360 ms
- Serial Elapsed Time: 2108 ms
- Speed up: 5.85456
- Efficiency: 0.731819

OMP Result:

C++ Thread Result:

command: `./build/bin/thread_trap_integral 8`

- Thread Elapsed Time: 594 ms
- Serial Elapsed Time: 2025 ms
- Speed up: 3.40777
- Efficiency: 0.425972
