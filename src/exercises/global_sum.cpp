#include "exercises/global_sum.h"

#include <mpi.h>
#include <omp.h>

// #include <random>
#include <memory>
#include <semaphore>
#include <sstream>
#include <thread>
#include <vector>

#include "helper.h"

namespace exercises::global_sum {

double computeNextValueWithRandomSleep(std::size_t x) {
  // static std::random_device rd;
  // static std::mt19937 gen(rd());
  // static std::uniform_real_distribution<> dis(0.0, 1.0);
  // return dis(gen);

  static int count = 0;
  std::this_thread::sleep_for(std::chrono::milliseconds(10 * count));
  ++count;
  return TestCaseSerialArray::instance().data()[x];
}

double serialImp(std::size_t l, std::size_t r) {
  double sum = 0;

  for (int i = l; i < r; ++i) {
    sum += computeNextValueWithRandomSleep(i);
  }
  return sum;
}

double mpiImp(int my_rank, int size, std::size_t n) {
  double local_sum = 0;

  std::size_t local_l;
  std::size_t local_r;

  // MPI_Bcast(&n, sizeof(n), MPI_CHAR, 0, MPI_COMM_WORLD);
  struct BS {
    void operator()(int dst) {
      MPI_Send(_n, sizeof(_n), MPI_CHAR, dst, 0, MPI_COMM_WORLD);
    }
    int _my_rank;
    std::size_t *_n;
  };

  struct BR {
    void operator()(int src) {
      MPI_Recv(_n, sizeof(_n), MPI_CHAR, src, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    }

    int _my_rank;
    std::size_t *_n;
  };
  my_msg::broadcast(my_rank, size, BS{my_rank, &n}, BR{my_rank, &n});
  TestCaseSerialArray::instance(
      n);  // Don't delete this line. Initialize the test array.
  distributeTask(my_rank, size, n, &local_l, &local_r);

  local_sum += serialImp(local_l, local_r);
  std::stringstream ss;
  ss << "Process: " << my_rank;
  ss << " Total N: " << n;
  ss << " Task: "
     << "[" << local_l << ", " << local_r << ")";
  ss << " Local sum: " << local_sum << "\n";
  std::cout << ss.str();

  // auto total_sum = local_sum;
  // MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
  // MPI_COMM_WORLD); local_sum = total_sum;

  struct S {
    void operator()(int dst) {
      MPI_Send(_sum, sizeof(double), MPI_CHAR, dst, 0, MPI_COMM_WORLD);
    }

    double *_sum;
  };

  struct R {
    void operator()(int src) {
      double recv_sum = 0;
      MPI_Recv(&recv_sum, sizeof(double), MPI_CHAR, src, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      *_sum += recv_sum;
    }

    double *_sum;
  };

  S s_f{&local_sum};
  R r_f{&local_sum};
  my_msg::reduce(my_rank, size, s_f, r_f);

  return local_sum;
}

double mpiAllReduceImp(int my_rank, int size, std::size_t n) {
  double local_sum = 0;

  std::size_t local_l;
  std::size_t local_r;

  // MPI_Bcast(&n, sizeof(n), MPI_CHAR, 0, MPI_COMM_WORLD);
  struct BS {
    void operator()(int dst) {
      MPI_Send(_n, sizeof(_n), MPI_CHAR, dst, 0, MPI_COMM_WORLD);
    }
    int _my_rank;
    std::size_t *_n;
  };

  struct BR {
    void operator()(int src) {
      MPI_Recv(_n, sizeof(_n), MPI_CHAR, src, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    }

    int _my_rank;
    std::size_t *_n;
  };
  my_msg::broadcast(my_rank, size, BS{my_rank, &n}, BR{my_rank, &n});
  TestCaseSerialArray::instance(
      n);  // Don't delete this line. Initialize the test array.
  distributeTask(my_rank, size, n, &local_l, &local_r);

  local_sum += serialImp(local_l, local_r);
  std::stringstream ss;
  ss << "Process: " << my_rank;
  ss << " Total N: " << n;
  ss << " Task: "
     << "[" << local_l << ", " << local_r << ")";
  ss << " Local sum: " << local_sum << "\n";
  std::cout << ss.str();

  // auto total_sum = local_sum;
  // MPI_Allreduce(&local_sum, &total_sum, sizeof(double), MPI_CHAR, MPI_SUM,
  //               MPI_COMM_WORLD);
  // local_sum = total_sum;

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

  my_msg::reduceAll(my_rank, size, SR{&local_sum, 0}, RS{&local_sum, 0},
                    S{&local_sum, 0}, R{&local_sum, 0});

  return local_sum;
}

double ompImp(int num_threads, std::size_t l, std::size_t r) {
  double total_sum = 0;
  auto n = r - l;

#pragma omp parallel num_threads(num_threads)
  {
    double local_sum = 0;
    std::size_t local_l;
    std::size_t local_r;
    distributeTask(omp_get_thread_num(), num_threads, n, &local_l, &local_r);
    local_sum = serialImp(local_l, local_r);

#pragma omp critical
    { total_sum += local_sum; }
  }

  return total_sum;
}

double threadImp(int num_threads, std::size_t l, std::size_t r) {
  std::vector<std::thread> threads(num_threads - 1);
  std::vector<double> local_sum(num_threads);

  auto n = r - l;
  auto f = [&local_sum, &num_threads, n](int my_rank) {
    local_sum[my_rank] = 0;
    std::size_t local_l;
    std::size_t local_r;
    distributeTask(my_rank, num_threads, n, &local_l, &local_r);
    local_sum[my_rank] = serialImp(local_l, local_r);
  };

  for (int i = 1; i < num_threads; ++i) {
    threads[i] = std::thread(f, i);
  }
  f(0);

  for (int i = 1; i < num_threads; ++i) {
    threads[i].join();
  }

  // print all local sum
  for (int i = 0; i < num_threads; ++i) {
    std::stringstream ss;
    ss << "Thread: " << i << " Local sum: " << local_sum[i] << "\n";
    std::cout << ss.str();
  }

  std::vector<std::unique_ptr<std::binary_semaphore>> sems;
  sems.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    sems.emplace_back(std::make_unique<std::binary_semaphore>(0));
  }

  struct S {
    std::unique_ptr<std::binary_semaphore> &_request;

    void operator()(int dst) { _request->release(); }
  };

  struct R {
    int _my_rank;
    std::vector<double> &_local_sum;
    std::vector<std::unique_ptr<std::binary_semaphore>> &_sems;

    void operator()(int src) {
      _sems[src]->acquire();
      _local_sum[_my_rank] += _local_sum[src];
    }
  };

  for (int i = 1; i < num_threads; ++i) {
    threads[i] = std::thread(my_msg::reduce<S, R>, i, num_threads, S{sems[i]},
                             R{i, local_sum, sems}, 0);
  }

  my_msg::reduce(0, num_threads, S{sems[0]}, R{0, local_sum, sems}, 0);

  for (int i = 1; i < num_threads; ++i) {
    threads[i].join();
  }

  return local_sum[0];
}

}  // namespace exercises::global_sum
