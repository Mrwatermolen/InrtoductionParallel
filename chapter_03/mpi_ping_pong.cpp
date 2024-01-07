#include <ctime>
#include <iostream>
#include <mpi.h>

void pingPong(int my_rank, int size) {
  int ping = 0;
  int pong = 1;
  int tag = 0;
  int count = 0;
  int max_count = 10;
  if (my_rank == 0) {
    std::cout << "Input n:\n";
    std::cin >> max_count;
    MPI_Send(&max_count, 1, MPI_INT, pong, tag, MPI_COMM_WORLD);
    auto c_start = clock();
    auto mpi_start = MPI_Wtime();
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < max_count; ++i) {
      MPI_Send(&count, 1, MPI_INT, pong, tag, MPI_COMM_WORLD);
      MPI_Recv(&count, 1, MPI_INT, pong, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    auto c_end = clock();
    auto mpi_end = MPI_Wtime();
    auto end = std::chrono::steady_clock::now();
    std::cout << "ping: " << count << "\n";
    std::cout << "CPU time: " << 1000 * (c_end - c_start) / CLOCKS_PER_SEC << " ms\n";
    std::cout << "MPI time: " << 1000 * (mpi_end - mpi_start) << " ms\n";
    std::cout << "Chrono time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
  } else {
    MPI_Recv(&max_count, 1, MPI_INT, ping, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (int i = 0; i < max_count; ++i) {
      MPI_Recv(&count, 1, MPI_INT, ping, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      ++count;
      MPI_Send(&count, 1, MPI_INT, ping, tag, MPI_COMM_WORLD);
    }
  }

}

int main(int argc, char **argv) {
  int my_rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (size != 2) {
    std::cerr << "This program requires exactly 2 MPI ranks, but you are "
              << "attempting to use " << size << " ranks!\n";
    MPI_Abort(MPI_COMM_WORLD, MPI_ERR_SIZE);
  }

  pingPong(my_rank, size);

  MPI_Finalize();
}
