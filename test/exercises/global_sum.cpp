#include "exercises/global_sum.h"

#include <mpi.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <semaphore>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "helper.h"

void showBroadcast() {
  auto n = getInoutOneDimensionProblemSize();
  std::vector<std::unique_ptr<std::binary_semaphore>> request_sems(n);
  std::vector<std::unique_ptr<std::binary_semaphore>> respond_sems(n);

  std::for_each(request_sems.begin(), request_sems.end(), [](auto&& s) {
    s = std::make_unique<std::binary_semaphore>(0);
  });

  std::for_each(respond_sems.begin(), respond_sems.end(), [](auto&& s) {
    s = std::make_unique<std::binary_semaphore>(0);
  });

  struct S {
    int _my_rank;
    std::vector<std::unique_ptr<std::binary_semaphore>>& _request;
    std::unique_ptr<std::binary_semaphore>& _respond;

    void operator()(int p) {
      _request[p]->release();
      std::string s = "Process: " + std::to_string(_my_rank) + " ---> " +
                      std::to_string(p) + " Waiting for response.\n";
      std::cout << s;
      _respond->acquire();
      s = "Process: " + std::to_string(_my_rank) + " <OK> " +
          std::to_string(p) + "\n";
      std::cout << s;
    }
  };

  struct R {
    int _my_rank;
    std::vector<std::unique_ptr<std::binary_semaphore>>& _respond;
    std::unique_ptr<std::binary_semaphore>& _request;

    void operator()(int p) {
      _request->acquire();
      std::string s = "Process: " + std::to_string(_my_rank) + " <--- " +
                      std::to_string(p) + " Sending response.\n";
      std::cout << s;
      _respond[p]->release();
      s = "Process: " + std::to_string(_my_rank) + " ===> " +
          std::to_string(p) + "\n";
      std::cout << s;
    }
  };

  std::vector<std::thread> threads(n);

  for (int i = 0; i < n; ++i) {
    threads[i] = std::thread(my_msg::broadcast<S, R>, i, n,
                             S{i, request_sems, respond_sems[i]},
                             R{i, respond_sems, request_sems[i]}, 0);
  }

  for (int i = 0; i < n; ++i) {
    threads[i].join();
  }
}

void testMpiTreeSum() {
  int my_rank;
  int size;

  MPI_Init(nullptr, nullptr);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::size_t n = 0;

  if (my_rank == 0) {
    std::cout << mpiConfigureToString(size) << "\n";
    n = getInoutOneDimensionProblemSize();
  }

  auto pc = PerformanceCompare{
      size, [&]() { return exercises::global_sum::mpiImp(my_rank, size, n); },
      [&]() { return exercises::global_sum::serialImp(0, n); }};

  auto mpi_res = pc.executeParallel();

  if (my_rank == 0) {
    auto serial_res = pc.executeSerial();
    std::cout << "mpi result:    " << mpi_res << "\n";
    std::cout << "serial result: " << serial_res << "\n";
    std::cout << "diff:          " << mpi_res - serial_res << "\n";

    std::cout << "PerformanceCompare:\n";
    std::cout << pc.toString() << "\n";

    assert(std::abs(mpi_res - serial_res) < 1e-6);
  }

  MPI_Finalize();
}

void testMpiPlatSum() {
  int my_rank;
  int size;

  MPI_Init(nullptr, nullptr);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::size_t n = 0;

  if (my_rank == 0) {
    std::cout << mpiConfigureToString(size) << "\n";
    n = getInoutOneDimensionProblemSize();
  }

  auto pc = PerformanceCompare{
      size,
      [&]() {
        return exercises::global_sum::mpiAllReduceImp(my_rank, size, n);
      },
      [&]() { return exercises::global_sum::serialImp(0, n); }};

  auto mpi_res = pc.executeParallel();
  std::stringstream ss;
  ss << "Process: " << my_rank << " mpi_res: " << mpi_res << "\n";
  std::cout << ss.str();

  if (my_rank == 0) {
    auto serial_res = pc.executeSerial();

    std::cout << "serial result: " << serial_res << "\n";
    std::cout << "diff:          " << mpi_res - serial_res << "\n";

    std::cout << "PerformanceCompare:\n";
    std::cout << pc.toString() << "\n";

    assert(std::abs(mpi_res - serial_res) < 1e-6);
  }

  MPI_Finalize();
}

void testThreadTreeSum(int argc, char** argv) {
  auto num_threads = getArgNumThread(argc, argv);

  std::cout << threadConfigureToString(num_threads) << "\n";

  auto n = getInoutOneDimensionProblemSize();

  exercises::global_sum::TestCaseSerialArray::instance(n);

  auto thread_res = exercises::global_sum::threadImp(num_threads, 0, n);
  auto serial_res = exercises::global_sum::serialImp(0, n);

  std::cout << "thread result: " << thread_res << "\n";
  std::cout << "serial result: " << serial_res << "\n";
  std::cout << "diff:          " << thread_res - serial_res << "\n";

  assert(std::abs(thread_res - serial_res) < 1e-6);
}

void testOmpTreeSum(int argc, char** argv) {
  auto num_threads = getArgNumThread(argc, argv);

  std::cout << ompConfigureToString(num_threads) << "\n";

  auto n = getInoutOneDimensionProblemSize();

  exercises::global_sum::TestCaseSerialArray::instance(n);

  auto omp_res = exercises::global_sum::ompImp(num_threads, 0, n);
  auto serial_res = exercises::global_sum::serialImp(0, n);

  std::cout << "omp result:    " << omp_res << "\n";
  std::cout << "serial result: " << serial_res << "\n";
  std::cout << "diff:          " << omp_res - serial_res << "\n";

  assert(std::abs(omp_res - serial_res) < 1e-6);
}

int main(int argc, char** argv) {
  // testMpiTreeSum(); // Pass
  // testMpiPlatSum(); // Pass
  // testThreadTreeSum(argc, argv); // Pass
  // testOmpTreeSum(argc, argv); // Pass
  return 0;
}
