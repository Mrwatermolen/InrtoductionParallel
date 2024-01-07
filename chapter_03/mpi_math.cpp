#include <chrono>
#include <iostream>
#include <mpi.h>
#include <random>

auto randomVector(double *v, int n) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(1.0, 2.0);
  for (int i = 0; i < n; ++i) {
    v[i] = dis(gen);
  }

  return v;
}

void printVector(double *v, int n) {
  for (int i = 0; i < n; ++i) {
    std::cout << v[i] << " ";
  }
}

void serialVecAdd(double *a, double *b, double *c, int n) {
  for (int i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

void doAdd(double *vec_a, double *vec_b, double *vec_c, int block_size) {
  double *local_a = new double[block_size];
  double *local_b = new double[block_size];
  double *local_c = new double[block_size];

  MPI_Scatter(vec_a, block_size, MPI_DOUBLE, local_a, block_size, MPI_DOUBLE, 0,
              MPI_COMM_WORLD);
  MPI_Scatter(vec_b, block_size, MPI_DOUBLE, local_b, block_size, MPI_DOUBLE, 0,
              MPI_COMM_WORLD);

  serialVecAdd(local_a, local_b, local_c, block_size);

  MPI_Gather(local_c, block_size, MPI_DOUBLE, vec_c, block_size, MPI_DOUBLE, 0,
             MPI_COMM_WORLD);

  delete[] local_a;
  delete[] local_b;
  delete[] local_c;
}

void mpiVecAdd(int my_rank, int size) {
  int n;
  double *vec_a = nullptr;
  double *vec_b = nullptr;
  double *res = nullptr;

  int block_size = 0;

  if (my_rank == 0) {
    std::cout << "input vector dimension: \n";
    std::cin >> n;
    if ((n % size) != 0) {
      std::cerr << "n % size != 0\n";
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    block_size = static_cast<int>(n / size);
    vec_a = new double[n];
    vec_b = new double[n];
    res = new double[n];
    std::cout << "block size: " << block_size << "\n";
    randomVector(vec_a, n);
    randomVector(vec_b, n);
    std::cout << "vec_a: ";
    printVector(vec_a, n);
    std::cout << "\n";
    std::cout << "vec_b: ";
    printVector(vec_b, n);
    std::cout << "\n";
  }

  MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  double *vec_c = nullptr;
  if (my_rank == 0) {
    vec_c = new double[n];
    serialVecAdd(vec_a, vec_b, vec_c, n);

    std::cout << "Serial Add res: ";
    printVector(vec_c, n);
    std::cout << "\n";
    std::cout << "MPI Add res:    ";
    printVector(res, n);
    std::cout << "\n";

    delete[] vec_a;
    delete[] vec_b;
    delete[] vec_c;
    delete[] res;
  }
}

void serialMatrixVec(double *A, double *x, double *y, int m, int n) {
  for (int i = 0; i < m; ++i) {
    y[i] = 0;
    for (int j = 0; j < n; ++j) {
      y[i] += A[i * n + j] * x[j];
    }
  }
}

double doMulti(double *A, double *local_x, double *y, int block_size, int n) {
  double *local_A = new double[block_size * n];
  auto s_time = std::chrono::high_resolution_clock::now();
  // MPI_Scatter(A, block_size * n, MPI_DOUBLE, local_A, block_size * n,
  //             MPI_DOUBLE, 0, MPI_COMM_WORLD);
  auto e_time = std::chrono::high_resolution_clock::now();
  auto scatter_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(e_time - s_time)
          .count();

  double *x = new double[n];
  MPI_Allgather(local_x, block_size, MPI_DOUBLE, x, block_size, MPI_DOUBLE,
                MPI_COMM_WORLD);

  double *local_y = new double[n];
  serialMatrixVec(local_A, x, local_y, block_size, n);

  MPI_Gather(local_y, block_size, MPI_DOUBLE, y, block_size, MPI_DOUBLE, 0,
             MPI_COMM_WORLD);

  delete[] local_A;
  delete[] x;

  return scatter_duration;
}

void mpiMatrixVec(int my_rank, int size) {
  int n = 0;
  double *A = nullptr;
  int block_size = 0;

  if (my_rank == 0) {
    std::cout << "input dimension\n";
    std::cin >> n;

    if ((n % size) != 0) {
      std::cerr << "n % size != 0\n";
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    block_size = static_cast<int>(n / size);
    A = new double[n * n];
    for (int i = 0; i < n; ++i) {
      randomVector(A + i * n, n);
    }
    if (n < 20) {
      std::cout << "A: \n";
      for (int i = 0; i < n; ++i) {
        printVector(A + i * n, n);
        std::cout << "\n";
      }
    }
  }
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

  MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  double *res = nullptr;
  if (my_rank == 0) {
    res = new double[n];
  }

  double *local_x = new double[block_size];
  randomVector(local_x, block_size);

  start = std::chrono::high_resolution_clock::now();
  auto t = doMulti(A, local_x, res, block_size, n);
  end = std::chrono::high_resolution_clock::now();
  auto mpi_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  if (my_rank == 0) {
    std::cout << "Scatter time = " << t << " ms\n";
  }

  // validate
  double *x = nullptr;
  if (my_rank == 0) {
    x = new double[n];
  }
  MPI_Gather(local_x, block_size, MPI_DOUBLE, x, block_size, MPI_DOUBLE, 0,
             MPI_COMM_WORLD);

  if (my_rank == 0) {
    double *y = nullptr;
    y = new double[n];
    start = std::chrono::high_resolution_clock::now();
    serialMatrixVec(A, x, y, n, n);
    end = std::chrono::high_resolution_clock::now();
    auto serial_duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    if (n < 50) {
      std::cout << "x: ";
      for (int i = 0; i < n; ++i) {
        std::cout << x[i] << " ";
      }
      std::cout << "\n";
      std::cout << "Serial res: ";
      for (int i = 0; i < n; ++i) {
        std::cout << y[i] << " ";
      }
      std::cout << "\n";
      std::cout << "MPI res:    ";
      for (int i = 0; i < n; ++i) {
        std::cout << res[i] << " ";
      }
      std::cout << "\n";
    }

    std::cout << "MPI duration: " << mpi_duration << " ms\n";
    std::cout << "Serial duration: " << serial_duration << " ms\n";

    auto s = static_cast<double>(serial_duration) / mpi_duration;
    std::cout << "Speedup: " << s << "\n";
    auto eff = s / size;
    std::cout << "Efficiency: " << eff << "\n";

    mpi_duration -= t;
    s = static_cast<double>(serial_duration) / mpi_duration;
    std::cout << "Speedup without scatter: " << s << "\n";
    eff = s / size;
    std::cout << "Efficiency without scatter: " << eff << "\n";

    delete[] x;
    delete[] y;
    delete[] res;
  }

  delete[] local_x;
};

int main(int argc, char *argv[]) {
  int my_rank, size;

  std::cout.precision(3);
  std::cout.setf(std::ios::fixed);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (my_rank == 0) {
    std::cout << "MPI Configuration: " << size << " processes\n";
  }

  if (my_rank == 0) {
    std::cout << "do vector add\n";
  }
  mpiVecAdd(my_rank, size);

  MPI_Barrier(MPI_COMM_WORLD);
  if (my_rank == 0) {
    std::cout << "do matrix vector multiplication\n";
  }
  mpiMatrixVec(my_rank, size);

  MPI_Finalize();
}
