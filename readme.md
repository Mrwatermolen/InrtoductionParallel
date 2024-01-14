# Introduction Parallel

## Just Note

build in `Release`

run platform:
1. MacBook Pro (13-inch, M1, 2020)
2. PC: 
    - CPU: AMD Ryzen 5 5600X 6-Core @ 12x 3.7GH
    - RAM: 2x 8GiB DIMM DDR4 Synchronous Unbuffered (Unregistered) 3600 MHz (0.3 ns)
    - OS: Ubuntu 22.04

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

#### MacBook

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

#### PC

MPI command: `mpirun --use-hwthread-cpus  ./build/bin/mpi_trap_integral`

C++ Standard Thread command: `./build/bin/thread_trap_integral 12`

OMP command: `./build/bin/omp_trap_integral 12`

MPI Result:

command: `mpirun -n 8 ./build/bin/mpi_trap_integral`

- Serial: Elapsed Time: 4605 ms
- Parallel: Elapsed Time: 695 ms
- Speed up: 6.62067
- Efficiency: 0.551722

OMP Result:

- Serial: Elapsed Time: 4341 ms
- Parallel: Elapsed Time: 636 ms
- Speed up: 6.82323
- Efficiency: 0.568602

C++ Thread Result:

- Serial: Elapsed Time: 4389 ms
- Parallel: Elapsed Time: 631 ms
- Speed up: 6.94838
- Efficiency: 0.579031

