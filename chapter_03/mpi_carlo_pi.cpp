#include <ios>
#include <iostream>
#include <mpi.h>
#include <random>

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

  carloPi(my_rank, size);

  MPI_Finalize();
}
