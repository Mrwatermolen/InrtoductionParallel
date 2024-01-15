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
    | OpenMP       | 257 ms       | 1377 ms | 7.8753   | 0.984412   |
    | C++ Standard | 233 ms       | 1366 ms | 5.86062  | 0.732578   |

Note: MPI Data copy time(master scatter task data to worker): 210 ms, so if we don't include data copy time, the elapsed time of MPI is 207 ms.

- Ubuntu

    | (thread = 12) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |-------------- |--------------|----------|----------|------------|
    | MPI           | 436 ms       | 1148 ms  | 2.63265  | 0.219388   |
    | OpenMP        | 89 ms        | 1025 ms  | 11.5091  | 0.95909    |
    | C++ Standard  | 106 ms       | 1023 ms  | 9.61855  | 0.801546   |

- Windows

    | (thread = 12) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |-------------- |--------------|----------|----------|------------|
    | MPI           | 359 ms       | 1313 ms  | 3.65102  | 0.304252   |
    | OpenMP        | 117 ms       | 1319 ms  | 11.1882  | 0.932351   |
    | C++ Standard  | 138 ms       | 1272 ms  | 9.1672   | 0.763934   |

    Note: MPI Data copy time: 247 ms

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
    | MPI          | 354 ms       | 4705 ms | 13.2848  | 1.6606     |
    | OpenMP       | 275 ms       | 2020 ms | 7.32903  | 0.916129   |
    | C++ Standard | 371 ms       | 2002 ms | 5.39414  | 0.674268   |

    I guess there is a problem with the MPI version, the serial elapsed time is too long, so that the speed up and efficiency is too high.

- Ubuntu 22.04 (need to retest)

    | (thread = 12) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |---------------|--------------|---------|----------|------------|
    | MPI           | 729 ms       | 4674 ms | 6.40524  | 0.53377    |
    | OpenMP        | 377 ms       | 4554 ms | 12.0542  | 1.00452    |
    | C++ Standard  | 632 ms       | 4610 ms | 7.28916  | 0.60743    |

- Windows

    | (thread = 12) | Elapsed Time | Serial  | Speed Up | Efficiency |
    |---------------|--------------|---------|----------|------------|
    | MPI           | 565 ms       | 3783 ms | 6.69522  | 0.557935   |
    | OpenMP        | 313 ms       | 3754 ms | 11.9697  | 0.997474   |
    | C++ Standard  | 548 ms       | 3801 ms | 6.92921  | 0.57743    |

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
    | OpenMP       | 7829 ms      | 62956 ms | 8.04126  | 1.00516    |
    | C++ Standard | 26643 ms     | 62543 ms | 2.34745  | 0.293432   |

- Ubuntu

    | (thread = 12) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |-------------- |--------------|----------|----------|------------|
    | MPI           | 8150 ms      | 64845 ms | 7.95603  | 0.663003   |
    | OpenMP        | 3933 ms      | 46431 ms | 11,8049  | 0.983743   |
    | C++ Standard  | 46205 ms     | 50956 ms | 0.906765 | 0.0755638  |

- Windows

    | (thread = 12) | Elapsed Time | Serial   | Speed Up | Efficiency |
    |-------------- |--------------|----------|----------|------------|
    | MPI           | 7485 ms      | 46904 ms | 6.26587  | 0.522156   |
    | OpenMP        | 3834 ms      | 46934 ms | 12.2386  | 1.01989    |
    | C++ Standard  | 114867 ms    | 47626 ms | 0.414618 | 0.0345515  |

Holy xxxx, the C++ Standard Thread version is so slow, I don't know why.

### Issue

1. All the implementation of OpenMP is completely wrong.

2. The implementation of C++ Standard Thread is so slow in the Carlo Pi test, I don't know why.

## TODO

1. Implement：multiplication between matrix and vector.
2. Implement: Gaussian Elimination.
3. CUDA version.
