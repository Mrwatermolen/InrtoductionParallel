#include "helper.h"
#include "homework.h"
#include <cstddef>
#include <ios>
#include <iostream>
#include <mpi.h>
#include <random>
#include <cmath>
#include <string>

auto randomDouble() {
  std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(-1.0, 1.0);
  return dis(gen);
}

void carloPi(int my_rank, int size) {
  constexpr double length = 2;
  constexpr double radius = 1;

  long long total_counter = 0;
  long long total_n = 0;
  long long local_n = 0;
  // read input
  if (my_rank == 0) {
    long long n;
    std::cout << "Input n:\n";
    std::cin >> n;

    total_n = n;
    auto remain = n % size;
    auto step = n / size;
    local_n = step;
    if (remain != 0) {
      ++local_n;
      --remain;
    }
    for (int i = 1; i < size; ++i) {
      auto send_n = step;
      if (remain != 0) {
        ++send_n;
        --remain;
      }

      MPI_Send(&send_n, 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD);
    }
  } else {
    MPI_Recv(&local_n, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
  }

  long long local_counter = 0;
  for (int i = 0; i < local_n; ++i) {
    auto x = randomDouble();
    auto y = randomDouble();
    auto dis = x * x + y * y;
    if (dis <= radius) {
      ++local_counter;
    }
  }

  MPI_Reduce(&local_counter, &total_counter, 1, MPI_LONG_LONG, MPI_SUM, 0,
             MPI_COMM_WORLD);

  if (my_rank == 0) {
    auto pi_estimate = 4 * (total_counter) / static_cast<double>(total_n);
    std::cout.precision(10);
    std::cout << std::fixed;
    std::cout << "M_PI: " << M_PI << "\n";
    std::cout << "process " << my_rank << " pi: " << pi_estimate << "\n";
  }
}

int main(int argc, char **argv) {
  int my_rank;
  int size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // carloPi(my_rank, size);

  using TaskType = carlo_pi::CarloPITask;
  using ResultType = carlo_pi::CarloPIResult;

  TaskType input_task;
  TaskType mpi_task;
  TaskType serial_task;

  if (my_rank == 0) {
    std::cout << mpiConfigureToString(size) << "\n";

    input_task = TaskType::createFromInput();
    mpi_task = input_task;
    serial_task = input_task;

    std::cout << "Task Info: ==============================\n";
    std::cout << (input_task.toString() + "\n");
  }

  auto p = PerformanceCompare{size};
  auto mpi_res = ResultType{p.executeParallel(carlo_pi::mpiImp, my_rank, size,
                                              std::ref(mpi_task), false),
                            input_task.n()};
  if (my_rank == 0) {
    auto serial_res =
        ResultType{p.executeSerial(carlo_pi::serialImp, serial_task.n(),
                                   serial_task.radius()),
                   input_task.n()};

    std::cout << ("Result: ==============================\n");
    std::cout << ("MPI Result: " + mpi_res.toString() + "\n");
    std::cout << ("Serial Result: " + serial_res.toString() + "\n");

    auto pi = M_PI;
    auto exact_res = ResultType{pi};
    std::cout << ("Exact Result: " + exact_res.toString() + "\n");

    ResultType::epsilon = 1e-4;
    std::cout << ("Verify MPI Result: ==============================\n");
    std::cout << ("Same(eps: "+ std::to_string(ResultType::epsilon) + "): ") << std::boolalpha
              << (ResultType::sameResult(mpi_res, exact_res)) << "\n";

    std::cout << ("Performance: ==============================\n");
    std::cout << (p.toString() + "\n");
  }

  MPI_Finalize();
}

namespace carlo_pi {

std::size_t mpiImp(int my_rank, int size, const CarloPITask &task,
                   bool printDataCopyTime) {
  using TaskType = CarloPITask;

  TaskType local_task;
  std::size_t local_counter = 0;
  std::size_t total_counter = 0;

  if (my_rank == 0) {
    local_task = task;
  }

  MPI_Bcast(&local_task, sizeof(TaskType), MPI_BYTE, 0, MPI_COMM_WORLD);

  std::size_t l;
  std::size_t r;
  distributeTask(my_rank, size, local_task.n(), &l, &r);
  auto n = r - l;

  local_counter = serialImp(n, local_task.radius());

  MPI_Reduce(&local_counter, &total_counter, sizeof(decltype(local_counter)),
             MPI_CHAR, MPI_SUM, 0, MPI_COMM_WORLD);

  return total_counter;
}

}  // namespace carlo_pi
