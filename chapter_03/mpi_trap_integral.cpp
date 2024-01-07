#include <iostream>
#include <mpi.h>
#include <string>

struct Task {
  double l;
  double r;
  int n;

  std::string toString() {
    return "Task: l: " + std::to_string(l) + " r: " + std::to_string(r) +
           " n: " + std::to_string(n);
  }
};

void serialImp(double l, double r, int n, double (*f)(double));

void simpleImp(int my_rank, int size, Task *task, double (*f)(double));

void reduceImp(int my_rank, int size, Task *task, double (*f)(double));

void allReduceImp(int my_rank, int size, Task *task, double (*f)(double));

double trap(double l, double r, int n, double (*f)(double));

double func(double x) { return x * x; }

int main(int argc, char *argv[]) {
  int my_rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  double a;
  double b;
  int n;
  Task task{0, 0, 0};

  if (my_rank == 0) {
    std::cout << "Enter a, b, and n\n";
    std::cin >> a >> b >> n;
    auto h = (b - a) / n;
    int strip = n / size;
    int temp = strip + n % size;
    task = Task{a, a + h * temp, temp};
    double l = a + h * temp;
    double r = 0;
    for (int i = 1; i < size; ++i) {
      r = l + h * strip;
      auto task = Task{l, r, strip};
      l = r;
      MPI_Send(&task, sizeof(task), MPI_CHAR, i, 0, MPI_COMM_WORLD);
    }

    serialImp(a, b, n, func);
  } else {
    MPI_Recv(&task, sizeof(task), MPI_CHAR, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    std::cout << "process " << my_rank << " received task: " << task.toString()
              << "\n";
  }

  simpleImp(my_rank, size, &task, func);
  reduceImp(my_rank, size, &task, func);
  allReduceImp(my_rank, size, &task, func);

  MPI_Finalize();
}

void serialImp(double l, double r, int n, double (*f)(double)) {
  auto sum = trap(l, r, n, f);
  std::cout << "sum: " << sum << "\n";
}

void simpleImp(int my_rank, int size, Task *task, double (*f)(double)) {
  double local_sum = trap(task->l, task->r, task->n, f);

  if (my_rank == 0) {
    double total_sum = 0;
    total_sum += local_sum;
    for (int i = 1; i < size; ++i) {
      double recv_sum = 0;
      MPI_Recv(&recv_sum, sizeof(recv_sum), MPI_CHAR, i, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      total_sum += recv_sum;
    }

    std::cout << "process " << my_rank << " sum: " << total_sum << "\n";
    return;
  }

  MPI_Send(&local_sum, sizeof(local_sum), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
}

void reduceImp(int my_rank, int size, Task *task, double (*f)(double)) {
  double local_sum = trap(task->l, task->r, task->n, f);

  double total_sum = 0;

  MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (my_rank == 0) {
    std::cout << "process " << my_rank << " sum: " << total_sum << "\n";
  }
}

void allReduceImp(int my_rank, int size, Task *task, double (*f)(double)) {
  double local_sum = trap(task->l, task->r, task->n, f);

  double total_sum = 0;

  MPI_Allreduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  if (my_rank == 1) {
    std::cout << "process " << my_rank << " sum: " << total_sum << "\n";
  }
}

double trap(double l, double r, int n, double (*f)(double)) {
  auto h = (r - l) / n;
  r = l + h * n;
  double sum = 0;
  sum += (f(l) + f(r)) / 2.0;
  for (int i = 1; i < n; ++i) {
    sum += f(l + i * h);
  }
  sum = sum * h;
  return sum;
}
