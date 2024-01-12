#include <mpi.h>

#include <chrono>
#include <vector>

#include "helper.h"
#include "homework.h"

int main(int argc, char** argv) {
  int my_rank = 0;
  int size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (my_rank == 0) {
    std::cout << mpiConfigureToString(size) << "\n";
  }

  using TaskType = bin::BinTask<double, std::vector<double>, std::size_t,
                                std::vector<std::size_t>>;

  TaskType total_task;
  TaskType serial_task;
  if (my_rank == 0) {
    total_task = TaskType::createFromInput();
    serial_task = total_task;
  }

  auto&& mpi_time =
      measureTime(bin::mpiImp, my_rank, size, std::ref(total_task), true);

  if (my_rank == 0) {
    std::cout << "MPI Result: ==============================\n";
    std::cout << "MPI Elapsed Time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(mpi_time)
                     .count()
              << " ms\n";
    std::cout << ("MPI histogram: \n" + total_task.info().toString() + "\n" +
                  total_task.headDataToString() + "\n" +
                  total_task.resToString() + "\n");
  }

  if (my_rank == 0) {
    auto f = [&serial_task]() {
      bin::serialImp(serial_task.data(), serial_task.info().n(),
                     serial_task.binCount(), serial_task.binMaxes(),
                     serial_task.info().binN(), serial_task.info().binMin());
    };
    auto&& serial_time = measureTime(f);
    std::cout << "Serial Result: ==============================\n";
    std::cout << "Serial Elapsed Time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     serial_time)
                     .count()
              << " ms\n";
    std::cout << ("Serial histogram: \n" + serial_task.info().toString() +
                  "\n" + serial_task.headDataToString() + "\n" +
                  serial_task.resToString() + "\n");

    // compare result
    std::cout << "Compare Result: ==============================\n";
    bool is_same =
        bin::sameResult(serial_task.binCount(), total_task.binCount(),
                        total_task.info().binN());
    std::cout << "Is same result: " << is_same << "\n";
    auto s = speedUp(std::chrono::duration<double>(serial_time),
                     std::chrono::duration<double>(mpi_time));
    std::cout << "Speed up: " << s << "\n";
    auto e = efficiency(s, size);
    std::cout << "Efficiency: " << e << "\n";
  }

  MPI_Finalize();
  return 0;
}

namespace bin {

void mpiImp(int my_rank, int size,
            BinTask<double, std::vector<double>, std::size_t,
                    std::vector<std::size_t>>& total_task,
            bool printDataCopyTime) {
  using TaskType = bin::BinTask<double, std::vector<double>, std::size_t,
                                std::vector<std::size_t>>;

  std::chrono::high_resolution_clock::time_point start;
  std::chrono::high_resolution_clock::time_point end;

  TaskType local_task;
  TaskType::BinTaskInfo local_info;

  if (my_rank == 0) {
    local_task = total_task;
    local_info = local_task.info();
  }

  if (my_rank == 0 && printDataCopyTime) {
    start = std::chrono::high_resolution_clock::now();
  }

  MPI_Bcast(&local_info, sizeof(local_info), MPI_CHAR, 0, MPI_COMM_WORLD);
  local_task.info() = local_info;
  local_task.binMaxes() = TaskType::binMaxesFromInfo(local_info);
  local_task.binCount() = std::vector<std::size_t>(local_info.binN());
  local_task.resetCount();

  std::vector<int> send_counts(size);
  for (int i = 0; i < size; ++i) {
    std::size_t start = 0;
    std::size_t end = 0;
    distributeTask(i, size, local_info.n(), &start, &end);
    send_counts[i] = (end - start) * sizeof(double);
  }
  local_task.info().n() = send_counts[my_rank] / sizeof(double);
  local_task.data() = std::vector<double>(local_task.info().n());
  std::vector<int> send_displs(size);
  send_displs[0] = 0;
  for (int i = 1; i < size; ++i) {
    send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
  }
  MPI_Scatterv(total_task.data().data(), send_counts.data(), send_displs.data(),
               MPI_CHAR, local_task.data().data(), send_counts[my_rank],
               MPI_CHAR, 0, MPI_COMM_WORLD);

  if (my_rank == 0 && printDataCopyTime) {
    end = std::chrono::high_resolution_clock::now();
    std::stringstream ss;
    ss << "MPI Data Copy Time: "
       << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
              .count()
       << " ms\n";
    std::cout << ss.str();
  }

  bin::serialImp(local_task.data(), local_task.info().n(),
                 local_task.binCount(), local_task.binMaxes(),

                 local_task.info().binN(), local_task.info().binMin());

  //  int count = total_task.info().binN(); error: count must be the same for
  // all? TODO: is right?
  int count = local_task.info().binN();

  MPI_Reduce(local_task.binCount().data(), total_task.binCount().data(), count,
             MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
}

}  // namespace bin
