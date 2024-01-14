# Introduction Parallel

## Just Note

build in `Release`

run platform:

1. Mac OS: MacBook Pro (13-inch, M1, 2020) 16GB
2. PC:
    - CPU: AMD Ryzen 5 5600X 6-Core @ 12x 3.7GH
    - RAM: 2x 8GiB DIMM DDR4 Synchronous Unbuffered (Unregistered) 3600 MHz (0.3 ns)
    - OS: Ubuntu 22.04 and Windows 11


### 1. Histogram Bin Test

Input Case:

197824571 0 20 7 6 9

Description:

Problem size: 197,824,571

the range of data: 0 ~ 20

the number of bins: 7

the range of dat in bins: 6 ~ 9

Command:

```shell
mpiexec --use-hwthread-cpus  ./build/bin/analysis_mpi_histogram_bin # MPI
./build/bin/analysis_omp_histogram_bin # OpenMP
./build/bin/analysis_thread_histogram_bin # C++ Standard Thread
```

1. Mac OS:

    | (thread = 8) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |--------------|--------------|---------|----------|------------|
    | MPI          | 417 ms       | 1428 ms | 3.42005  | 0.427506   |
    | OpenMP       | 174 ms       | 1377 ms | 7.8753   | 0.984412   |
    | C++ Standard | 233 ms       | 1366 ms | 5.86062  | 0.732578   |

Note: MPI Data copy time(master scatter task data to worker): 210 ms, so if we don't include data copy time, the elapsed time of MPI is 207 ms.

### 2. Trap Integral Test

Input Case:

269845164 -10 10

Description:

Problem size: 269,845,164

the range of data: -10 ~ 10

$$
\begin{aligned}
F(x) = \sin(x) \cos(x) e^{-x} \\
F'(x) = -e^{-x} (0.5 \sin(2x) + \cos(2x)) \\
Result = \int_{-10}^{10} F'(x) dx = F(10) - F(-10)
\end{aligned}
$$

Command:

```shell
mpiexec --use-hwthread-cpus  ./build/bin/analysis_mpi_trap_integral # MPI
./build/bin/analysis_omp_trap_integral # OpenMP
./build/bin/analysis_thread_trap_integral # C++ Standard Thread
```

1. Mac OS
    | (thread = 8) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |--------------|--------------|---------|----------|------------|
    | MPI          | 354 ms       | 4705 ms | 13.2848  | 1.6606     |
    | OpenMP       | 275 ms       | 2020 ms | 7.32903  | 0.916129   |
    | C++ Standard | 371 ms       | 2002 ms | 5.39414  | 0.674268   |

    I guess there is a problem with the MPI version, the serial elapsed time is too long, so that the speed up and efficiency is too high.
2. Ubuntu 22.04 (need to retest)
    | (thread = 8) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |--------------|--------------|---------|----------|------------|
    | MPI          | 695 ms       | 4605 ms | 6.62067  | 0.827584   |
    | OpenMP       | 636 ms       | 4341 ms | 6.82323  | 0.852904   |
    | C++ Standard | 631 ms       | 4389 ms | 6.94838  | 0.868547   |

### 3. Carlo Pi

Input Case:

3458800240 3.3

Description:

the number of points: 3,458,800,240

the radius of circle: 3.3

Command:

```shell
mpiexec --use-hwthread-cpus  ./build/bin/analysis_mpi_carlo_pi # MPI
./build/bin/analysis_omp_carlo_pi # OpenMP
./build/bin/analysis_thread_carlo_pi # C++ Standard Thread
```

1. Mac OS
    | (thread = 8) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |--------------|--------------|----------|----------|------------|
    | MPI          | 9409 ms      | 65811 ms | 6.99389  | 0.874236   |
    | OpenMP       | 7829 ms      | 62956 ms | 8.04126  | 1.00516    |
    | C++ Standard | 26643 ms     | 62543 ms | 2.34745  | 0.293432   |
