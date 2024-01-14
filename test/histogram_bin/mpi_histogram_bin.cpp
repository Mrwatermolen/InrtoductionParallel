#include "helper.h"
#include "homework/histogram_bin.h"
#include <cstddef>
#include <functional>
#include <ios>
#include <iostream>
#include <limits>
#include <mpi.h>
#include <random>
#include <string>

int main(int argc, char **argv) {
  int my_rank = 0;
  int size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (my_rank == 0) {
    std::cout << mpiConfigureToString(size) << "\n";
  }

  using TaskType = homework::histogram_bin::TaskTypeImp;
  using DataType = homework::histogram_bin::DataTypeImp;
  using DataTypeArray = homework::histogram_bin::DataTypeArrayImp;
  using SizeType = homework::histogram_bin::SizeTypeImp;
  using SizeTypeArray = homework::histogram_bin::SizeTypeArrayImp;
  using ResultType = homework::histogram_bin::ResultTypeImp;

  auto random_data = [](std::size_t n, double min, double max) {
    static std::default_random_engine gen;
    static std::uniform_real_distribution<> dis(min, max);
    std::vector<double> data(n);
    for (std::size_t i = 0; i < n; ++i) {
      data[i] = dis(gen);
    }
    return data;
  };

  TaskType mpi_task;
  TaskType serial_task;

  DataTypeArray data;

  if (my_rank == 0) {
    auto &&input_task = TaskType::createFromInput();
    if (std::numeric_limits<int>::max() <= input_task.n()) {
      std::cerr << "MPI count limit: n must be less than "
                << std::numeric_limits<int>::max() << "\n";
      MPI_Abort(MPI_COMM_WORLD, MPI_ERR_ARG);
      return 1;
    }

    std::cout << "Preparing Data: ==============================\n";
    mpi_task = input_task;
    serial_task = input_task;
    data = random_data(input_task.n(), input_task.min(), input_task.max());
    std::cout
        << ("Task Info: ==============================\n" +
            input_task.toString() + "\n" +
            homework::histogram_bin::headDataToString(data, input_task.n()) +
            "\n" + "Data Memory: " +
            std::to_string(data.size() * sizeof(DataType) / 1024.0 / 1024.0) +
            " MB\n");
  }

  auto pc = PerformanceCompare{
      size, homework::histogram_bin::mpiImp,
      homework::histogram_bin::serialImp<DataType, DataTypeArray, SizeType,
                                         SizeTypeArray>};

  auto mpi_res = ResultType{pc.executeParallel(
      my_rank, size, std::ref(mpi_task), std::ref(data), true)};

  if (my_rank == 0) {
    std::cout << "MPI Result: ==============================\n";
    std::cout << (mpi_res.toString(mpi_task) + "\n");

    auto bin_maxes = TaskType::binMaxesFromInfo(
        mpi_task.binN(), mpi_task.binMin(), mpi_task.binMax());
    auto serial_bin_count = SizeTypeArray(mpi_task.binN(), 0);
    pc.executeSerial(std::ref(data), static_cast<SizeType>(0), serial_task.n(),
                     std::ref(serial_bin_count), std::ref(bin_maxes),
                     serial_task.binN(), serial_task.binMin());
    auto serial_res = ResultType{serial_bin_count};
    std::cout << "Serial Result: ==============================\n";
    std::cout << (serial_res.toString(serial_task) + "\n");

    std::cout << ("Verify Result: ==============================\n");
    std::cout << ("Same: ") << std::boolalpha
              << (ResultType::sameResult(serial_res, mpi_res,
                                         serial_task.binN()))
              << "\n";

    std::cout << "Performance Compare: ==============================\n";
    std::cout << pc.toString() << "\n";
  }

  MPI_Finalize();
  return 0;
}
