#include <mpi.h>

#include <chrono>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <thread>

#include "helper.h"
#include "homework.h"

int main(int argc, char *argv[]) { global_sum::mpiRun(); }

namespace global_sum {

void mpiRun() {
  int my_rank;
  int size;
  std::size_t n = 0;

  MPI_Init(nullptr, nullptr);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (my_rank == 0) {
    n = getInoutOneDimensionProblemSize();
  }
  MPI_Bcast(&n, sizeof(n), MPI_CHAR, 0, MPI_COMM_WORLD);

  std::string s = "process " + std::to_string(my_rank) + " ";
  // local sum
  std::size_t l = 0;
  std::size_t r = 0;
  distributeTask(my_rank, size, n, &l, &r);
  std::cout << (s + taskToString(l, r) + "\n");
  auto local_sum = serialSum(l, r);
  std::cout.precision(5);
  std::cout << std::fixed;
  std::cout << (s + "local sum  : " + std::to_string(local_sum) + "\n");

  // MPI_Barrier(MPI_COMM_WORLD);
  if (my_rank == 0) {
    std::cout << (s + "start converge\n");
  }
  auto tree_sum = local_sum;
  auto plat_sum = local_sum;
  auto tree_time = measureTime(mpiTreeSum, my_rank, size, 0, &tree_sum);
  mpiPlatSum(my_rank, size, 0, &plat_sum);

  if (my_rank == 0) {
    std::cout << (s + "converge done\n");
    auto t_ms = std::chrono::duration_cast<std::chrono::milliseconds>(tree_time)
                    .count();
    std::cout << (s + "tree time : " + std::to_string(t_ms) + " ms\n");
    std::cout << (s + "tree sum  : " + std::to_string(tree_sum) + "\n");
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  MPI_Barrier(MPI_COMM_WORLD);
  std::cout << (s + "plat sum  : " + std::to_string(plat_sum) + "\n");

  // validate
  double res = 0;
  // MPI_Reduce(&local_sum, &res, sizeof(double), MPI_CHAR, MPI_SUM, 0,
  //  MPI_COMM_WORLD); // error for count is used for vector add?
  MPI_Reduce(&local_sum, &res, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (my_rank == 0) {
    std::cout << (s + "validated : " + std::to_string(res) + "\n");
  }

  MPI_Finalize();
}

double computeNextValue() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(0.0, 1.0);
  return dis(gen);
}

double computeNextValueWithRandomSleep() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(0.0, 1.0);
  static int count = 0;
  std::this_thread::sleep_for(std::chrono::milliseconds(10 * count));
  ++count;
  return dis(gen);
}

template <typename T>
std::string taskToString(T l, T r) {
  return static_cast<std::string>("[" + std::to_string(l) + ", " +
                                  std::to_string(r) + ")");
}

void mpiTreeSum(int my_rank, int size, int tag, double *sum) {
  struct S {
    void operator()(int dst) {
      MPI_Send(_sum, sizeof(double), MPI_CHAR, dst, _tag, MPI_COMM_WORLD);
    }

    double *_sum;
    int _tag;
  };

  struct R {
    void operator()(int src) {
      double recv_sum = 0;
      MPI_Recv(&recv_sum, sizeof(double), MPI_CHAR, src, _tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      *_sum += recv_sum;
    }

    double *_sum;
    int _tag;
  };
  treeCommunication(my_rank, size, S{sum, tag}, R{sum, tag});
}

void mpiPlatSum(int my_rank, int size, int tag, double *sum) {
  struct SR {
    void operator()(int partner) {
      double recv_sum = 0;
      MPI_Send(_sum, sizeof(double), MPI_CHAR, partner, _tag, MPI_COMM_WORLD);
      MPI_Recv(&recv_sum, sizeof(double), MPI_CHAR, partner, _tag,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      *_sum += recv_sum;
    }

    double *_sum;
    int _tag;
  };

  struct RS {
    void operator()(int partner) {
      double recv_sum = 0;
      MPI_Recv(&recv_sum, sizeof(double), MPI_CHAR, partner, _tag,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send(_sum, sizeof(double), MPI_CHAR, partner, _tag, MPI_COMM_WORLD);
      *_sum += recv_sum;
    }

    double *_sum;
    int _tag;
  };

  struct S {
    void operator()(int dst) {
      MPI_Send(_sum, sizeof(double), MPI_CHAR, dst, _tag, MPI_COMM_WORLD);
    }

    double *_sum;
    int _tag;
  };

  struct R {
    void operator()(int dst) {
      double recv_sum = 0;
      MPI_Recv(_sum, sizeof(double), MPI_CHAR, dst, _tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    }

    double *_sum;
    int _tag;
  };

  platCommunication(my_rank, size, SR{sum, tag}, RS{sum, tag}, S{sum, tag},
                    R{sum, tag});
}

void mpiTreeSum01(int my_rank, int size, int divisor, int core_different,
                  double *sum, int tag) {
  if ((size << 1) < divisor) {
    return;
  }

  bool send = (my_rank % (divisor)) != 0;

  if (send) {
    int dest = my_rank - core_different;
    MPI_Send(sum, sizeof(double), MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    return;
  }

  int src = my_rank + core_different;
  core_different <<= 1;

  if (size <= src) {
    mpiTreeSum01(my_rank, size, divisor << 1, core_different, sum, tag);
    return;
  }

  double recv_sum;
  MPI_Recv(&recv_sum, sizeof(double), MPI_CHAR, src, tag, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
  *sum += recv_sum;
  mpiTreeSum01(my_rank, size, divisor << 1, core_different, sum, tag);
}

}  // namespace global_sum
