#include <iostream>
#include <mpi.h>
#include <ostream>
#include <random>
#include <thread>

struct Task {
  int l;
  int r;
};

double nextComputeValue() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(0.0, 1.0);
  static int count = 0;
  std::this_thread::sleep_for(std::chrono::milliseconds(10 * count));
  ++count;
  return dis(gen);
}

void range(int n, int size, int *l, int *r) {
  int step = n / size;
  int remain = n % size;
  l[0] = 0;
  r[0] = step;
  if (remain != 0) {
    r[0]++;
    remain--;
  }

  for (int i = 1; i < size; ++i) {
    l[i] = r[i - 1];
    r[i] = l[i] + step;
    if (remain == 0) {
      continue;
    }

    r[i]++;
    remain--;
  }
}

void masterDispatchTask(int n, int size, Task *master_task) {
  int *l = new int[size];
  int *r = new int[size];
  range(n, size, l, r);
  master_task->l = l[0];
  master_task->r = r[0];
  for (int i = 0; i < size; ++i) {
    Task task;
    task.l = l[i];
    task.r = r[i];
    MPI_Send(&task, sizeof(Task), MPI_CHAR, i, 0, MPI_COMM_WORLD);
  }
  delete[] l;
  delete[] r;
}

void workRequestTask(int my_rank, Task *task) {
  MPI_Status status;
  MPI_Recv(task, sizeof(Task), MPI_CHAR, 0, 0, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
  std::cout << "process " << my_rank << " received task: [" << task->l << ", "
            << task->r << ")\n";
}

void converge(int my_rank, int size, int divisor, int core_different,
              double *sum, int tag) {
  if ((size << 1) < divisor) {
    return;
  }

  bool send = my_rank % (divisor);

  if (send) {
    int dest = my_rank - core_different;
    MPI_Send(sum, sizeof(double), MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    return;
  }

  int src = my_rank + core_different;
  core_different <<= 1;

  if (size <= src) {
    converge(my_rank, size, divisor << 1, core_different, sum, tag);
    return;
  }

  double recv_sum;
  MPI_Recv(&recv_sum, sizeof(double), MPI_CHAR, src, tag, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
  *sum += recv_sum;
  converge(my_rank, size, divisor << 1, core_different, sum, tag);
}

void convergeTree(int my_rank, int size, int stage, double *sum, int tag) {
  if (size <= stage) {
    // communication done
    return;
  }

  int partner = my_rank ^ stage;
  if (size <= partner) {
    // partner which is in this stage doesn't exist.
    // go to next stage
    convergeTree(my_rank, size, stage << 1, sum, tag);
    return;
  }

  bool right_node = my_rank & stage;
  if (right_node) {
    // right node send message to left node
    int dst = partner;
    MPI_Send(sum, sizeof(double), MPI_CHAR, dst, tag, MPI_COMM_WORLD);
    // right node exit
    return;
  }

  // left node receive message from right node
  int src = partner;
  double recv_sum = 0;
  MPI_Recv(&recv_sum, sizeof(double), MPI_CHAR, src, tag, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
  *sum += recv_sum;
  // go to next stage
  convergeTree(my_rank, size, stage << 1, sum, tag);
}

void convergePlat(int my_rank, int size, int stage, double *sum, int tag) {
  if (size <= stage) {
    return;
  }

  int partner = my_rank ^ stage;
  if (size <= partner) {
    convergePlat(my_rank, size, stage << 1, sum, tag);
    return;
  }

  bool send_then_recv = my_rank & stage;
  if (send_then_recv) {
    int dst = partner;
    MPI_Send(sum, sizeof(decltype(*sum)), MPI_CHAR, dst, tag, MPI_COMM_WORLD);
    int src = dst;
    MPI_Recv(sum, sizeof(decltype(*sum)), MPI_CHAR, src, tag, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
  } else {
    int src = partner;
    double recv_sum;
    MPI_Recv(&recv_sum, sizeof(decltype(recv_sum)), MPI_CHAR, src, tag,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *sum += recv_sum;
    int dst = src;
    MPI_Send(sum, sizeof(decltype(*sum)), MPI_CHAR, dst, tag, MPI_COMM_WORLD);
  }

  convergePlat(my_rank, size, stage << 1, sum, tag);
}

double doSum(int my_rank, int size, Task *task) {
  double local_sum = 0.0;
  for (int i = task->l; i < task->r; ++i) {
    local_sum += nextComputeValue();
  }

  std::cout.precision(5);
  std::cout << std::fixed;
  std::cout << "process " << my_rank << " local sum: " << local_sum << "\n";
  std::cout << std::flush;
  MPI_Barrier(MPI_COMM_WORLD);
  if (my_rank == 0) {
    std::cout << "process " << my_rank << " start converge\n";
  }

  auto tree_sum_0 = local_sum;
  auto tree_sum_1 = local_sum;
  auto plat_sum = local_sum;
  converge(my_rank, size, 2, 1, &tree_sum_0, 0);
  convergeTree(my_rank, size, 1, &tree_sum_1, 0);
  convergePlat(my_rank, size, 1, &plat_sum, 0);

  MPI_Barrier(MPI_COMM_WORLD);
  if (my_rank == 0) {
    std::cout << "process " << my_rank << " converge done\n";
    std::cout << "process " << my_rank << " tree01 sum: " << tree_sum_0 << "\n";
    std::cout << "process " << my_rank << " tree02 sum: " << tree_sum_1 << "\n";
    std::cout << std::flush;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  MPI_Barrier(MPI_COMM_WORLD);
  std::cout << "process " << my_rank << " plat sum  : " << plat_sum << "\n";

  return local_sum;
}

int main(int argc, char *argv[]) {
  int my_rank, size;
  Task task{0, 0};

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (my_rank == 0) {
    if (argc != 2) {
      std::cout << "Usage: mpirun -n <number of threads> " << argv[0]
                << " <size>\n";
      MPI_Abort(MPI_COMM_WORLD, MPI_ERR_ARG);
    }

    std::cout << "MPI Configuration: " << size << " processes\n";
    int n = atoi(argv[1]);
    masterDispatchTask(n, size, &task);
  } else {
    workRequestTask(my_rank, &task);
  }
  std::cout << std::flush;
  // for print in order
  MPI_Barrier(MPI_COMM_WORLD);

  auto local_sum = doSum(my_rank, size, &task);
  MPI_Barrier(MPI_COMM_WORLD);

  // validate
  double res = 0;
  // MPI_Reduce(&local_sum, &res, sizeof(decltype(res)), MPI_CHAR, MPI_SUM, 0,
  //            MPI_COMM_WORLD); // error for count is used for vector add?
  MPI_Reduce(&local_sum, &res, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (my_rank == 0) {
    std::cout << "process " << my_rank << " validate sum: " << res << "\n";
  }

  MPI_Finalize();
}
