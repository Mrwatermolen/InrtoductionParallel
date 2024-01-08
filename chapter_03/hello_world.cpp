#include <iostream>
#include <mpi.h>
#include <string>

int main(int argc, char *argv[]) {
  int my_rank;
  int size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (my_rank != 0) {
    std::string mes = "Hello, world! from process " + std::to_string(my_rank);
    MPI_Send(mes.c_str(), mes.length() + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
  } else {
    std::string mes;
    std::cout << "MPI Configuration: " << size << " processes\n";
    std::cout << "process 0: Ready to receive messages\n";
    for (int i = 1; i < size; i++) {
      MPI_Status status;
      MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      int count;
      MPI_Get_count(&status, MPI_CHAR, &count);
      char *buf = new char[count];
      MPI_Recv(buf, count, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      mes = buf;
      delete[] buf;
      std::cout << "Message from process " << status.MPI_SOURCE
                << ". Content: " << mes << "\n";
    }
  }

  MPI_Finalize();

  return 0;
}
