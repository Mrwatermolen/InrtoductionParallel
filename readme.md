# Introduction Parallel

## Just Note

>Be care: The data here is obtained through testing under different time periods and varying system loads, and is not representative.

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

The command is different in Windows.

```cmd
mpiexec.exe -n 12 .\build\bin\analysis_mpi_histogram_bin.exe
.\build\bin\analysis_omp_histogram_bin.exe
.\build\bin\analysis_thread_histogram_bin.exe
```

- Mac OS:

    | (thread = 8) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |--------------|--------------|---------|----------|------------|
    | MPI          | 417 ms       | 1428 ms | 3.42005  | 0.427506   |
    | OpenMP       | 229 ms       | 1373 ms | 5.97169  | 0.746461   |
    | C++ Standard | 233 ms       | 1366 ms | 5.86062  | 0.732578   |

Note: MPI Data copy time(master scatter task data to worker): 210 ms, so if we don't include data copy time, the elapsed time of MPI is 207 ms.

- Ubuntu

    | (thread = 12) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |-------------- |--------------|----------|----------|------------|
    | MPI           | 436 ms       | 1148 ms  | 2.63265  | 0.219388   |
    | OpenMP        | 119 ms       | 1020 ms  | 8.855637 | 0.713031   |
    | C++ Standard  | 106 ms       | 1023 ms  | 9.61855  | 0.801546   |

- Windows

    | (thread = 12) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |-------------- |--------------|----------|----------|------------|
    | MPI           | 494 ms       | 1053 ms  | 2.13188  | 0.177656   |
    | OpenMP        | 95 ms        | 1080 ms  | 11.2659  | 0.938825   |
    | C++ Standard  | 101 ms       | 1068 ms  | 10.531   | 0.877585   |

    Note: MPI Data copy time: 369 ms

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

- Mac OS

    | (thread = 8) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |--------------|--------------|---------|----------|------------|
    | MPI          | 351 ms       | 2106 ms | 5.99394  | 0.749243   |
    | OpenMP       | 391 ms       | 1997 ms | 5.09651  | 0.637063   |
    | C++ Standard | 371 ms       | 2002 ms | 5.39414  | 0.674268   |


- Ubuntu 22.04 (need to retest)

    | (thread = 12) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |---------------|--------------|---------|----------|------------|
    | MPI           | 729 ms       | 4674 ms | 6.40524  | 0.53377    |
    | OpenMP        | 624 ms       | 5436 ms | 7.26813  | 1.00452    |
    | C++ Standard  | 632 ms       | 4610 ms | 7.28916  | 0.60743    |

- Windows

    | (thread = 12) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |---------------|--------------|---------|----------|------------|
    | MPI           | 1288 ms      | 9319 ms | 7.23432  | 0.60286    |
    | OpenMP        | 1247 ms      | 9283 ms | 7.44451  | 0.620376   |
    | C++ Standard  | 1249 ms      | 9283 ms | 7.43059  | 0.619216   |

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

- Mac OS
    | (thread = 8) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |--------------|--------------|----------|----------|------------|
    | MPI          | 9409 ms      | 65811 ms | 6.99389  | 0.874236   |
    | OpenMP       | 5998 ms      | 35391 ms | 5.90013  | 0.737516   |
    | C++ Standard | 6265 ms      | 35454 ms | 5.6584   | 0.707301   |

- Ubuntu

    | (thread = 12) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |-------------- |--------------|----------|----------|------------|
    | MPI           | 8150 ms      | 64845 ms | 7.95603  | 0.663003   |
    | OpenMP        | 7323 ms      | 61583 ms | 8.40882  | 0.700735   |
    | C++ Standard  | 7326 ms      | 61593 ms | 8.40645  | 0.700537   |

- Windows

    | (thread = 12) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |-------------- |--------------|----------|----------|------------|
    | MPI           | 6466 ms      | 53806 ms | 8.32043  | 0.693369   |
    | OpenMP        | 34850 ms     | 53980 ms | 1.54889  | 0.129074   |
    | C++ Standard  | 35379 ms     | 54335 ms | 1.5358   | 0.127983   |

Holy xxxx, the C++ Standard Thread version is so slow, I don't know why. Update: also in OpenMP.

### Issue

1. All the implementation of OpenMP is completely wrong. (Fixed. Reason: the order of compline and linker.)

2. The implementation of C++ Standard Thread and OpenMP is so slow in the Carlo Pi test, I don't know why. (I guess the reason is the random number generator.) Fixed: [Ref](https://stackoverflow.com/questions/21237905/how-do-i-generate-thread-safe-uniform-random-numbers)

3. The test of mpi trap integral in Mac OS represent the poor performance of the serial version. (Fixed. I don't know why but I fixed it. TODO: figure out why.)

## TODO

1. Implement：multiplication between matrix and vector.
2. Implement: Gaussian Elimination.
3. CUDA version.
