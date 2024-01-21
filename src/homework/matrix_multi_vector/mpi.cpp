#include "mpi.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <sstream>
#include <vector>

#include "helper.h"
#include "homework/matrix_multi_vector.h"

namespace homework::mmv {

static constexpr bool DEBUG = false;

void init(int my_rank, int size, MyMatrixImp& a, MyMatrixImp& x,
          std::vector<MyMatrixImp::BlockInfo>& vector_block_info,
          std::vector<MyMatrixImp::BlockInfo>& blocks_info,
          std::size_t& num_blocks, const MyMatrixShape& m_shape,
          const MyMatrixShape& v_shape, const MyMatrixShape& b_size) {
  std::stringstream ss;
  std::string prefix = "Process " + std::to_string(my_rank) + ": ";

  if (my_rank == 0) {
    ss << "First time and init\n";

    x = MyMatrixImp(v_shape);
    a = MyMatrixImp(m_shape);

    vector_block_info =
        MyMatrixImp::splitByBlock(MyMatrixShape{1, b_size.second}, x);

    ss << "Vector Block Info: \n";
    for (auto&& i : vector_block_info) {
      ss << i.toString() << "\n";
    }

    blocks_info = MyMatrixImp::splitByBlock(b_size, a);
    ss << "Matrix Block Info: \n";
    for (auto&& i : blocks_info) {
      ss << i.toString() << "\n";
    }
    if constexpr (DEBUG) {
      std::cout << ss.str();
    }

    ss.str("");
    ss.clear();
    num_blocks = blocks_info.size();
    MPI_Bcast(&num_blocks, sizeof(std::size_t), MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(vector_block_info.data(),
              sizeof(MyMatrixImp::BlockInfo) * vector_block_info.size(),
              MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(blocks_info.data(),
              sizeof(MyMatrixImp::BlockInfo) * blocks_info.size(), MPI_CHAR, 0,
              MPI_COMM_WORLD);
  } else {
    // TODO(franzero): try to use mpi data type
    MPI_Bcast(&num_blocks, sizeof(std::size_t), MPI_CHAR, 0, MPI_COMM_WORLD);
    vector_block_info.resize(num_blocks);
    blocks_info.resize(num_blocks);
    MPI_Bcast(vector_block_info.data(),
              sizeof(MyMatrixImp::BlockInfo) * vector_block_info.size(),
              MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(blocks_info.data(),
              sizeof(MyMatrixImp::BlockInfo) * blocks_info.size(), MPI_CHAR, 0,
              MPI_COMM_WORLD);
    // print
    ss << prefix;
    ss << "Broadcast Block Info: \n";
    ss << "Vector Block Info: \n";
    for (auto&& i : vector_block_info) {
      ss << i.toString() << "\n";
    }
    ss << "Matrix Block Info: \n";
    for (auto&& i : blocks_info) {
      ss << i.toString() << "\n";
    }
    if constexpr (DEBUG) {
      std::cout << ss.str();
    }
    ss.str("");
    ss.clear();
  }
}

// Only support to merge block splitted by row
auto mergeBlock(const MyMatrixShape& block_size,
                std::vector<MyMatrixImp::BlockInfo>::const_iterator begin,
                std::vector<MyMatrixImp::BlockInfo>::const_iterator end) {
  std::size_t disp = begin->displacements().front();
  std::size_t count = 0;
  std::for_each(begin, end, [&count](const auto& b) {
    count = std::accumulate(b.counts().begin(), b.counts().end(),
                            static_cast<std::size_t>(0));
  });
  return MyMatrixImp::BlockInfo{std::vector<size_t>{disp},
                                std::vector<size_t>{count}};
}

void deliverTask(int my_rank, int size, MyMatrixImp& a, MyMatrixImp& x,
                 const MyMatrixShape& b_size,
                 const std::vector<MyMatrixImp::BlockInfo>& vector_block_info,
                 const std::vector<MyMatrixImp::BlockInfo>& blocks_info) {
  std::stringstream ss;
  std::string prefix = "Process " + std::to_string(my_rank) + ": ";

  if (b_size.second != x.column()) {
    throw std::runtime_error("Only support split by row\n");
  }

  if (my_rank == 0) {
    if constexpr (DEBUG) {
      ss << "Create Random Matrix"
         << "\n";
      a.randomData(0, 10.0);
      ss << "Matrix: " << a.toString() << "\n";
      ss << "Scatter Matrix to other processes\n";
      std::cout << ss.str();
      ss.str("");
      ss.clear();
    }

    for (std::size_t j = 0; j < size; ++j) {
      auto data_type_size = sizeof(DataTypeImp);
      ss << "To " << j << " data:";
      std::size_t l;
      std::size_t r;
      distributeTask(j, size, blocks_info.size(), &l, &r);
      ss << "[" << l << ", " << r << ")\n";
      for (std::size_t k = l; k < r; ++k) {
        ss << "Block " << k << "\n";
        auto&& block = blocks_info[k];
        auto rows = blocks_info[k].displacements();
        auto rows_size = blocks_info[k].counts();
        auto data = a.data().data();
        for (int ii = 0; ii < rows.size(); ++ii) {
          auto nn = rows_size[ii] / data_type_size;
          for (int jj = 0; jj < nn; ++jj) {
            ss << data[rows[ii] + jj] << " ";
          }
          ss << "\n";
          std::cout << ss.str();
          ss.str("");
          ss.clear();
        }
      }
    }
  } else {
    std::size_t l;
    std::size_t r;
    distributeTask(my_rank, size, blocks_info.size(), &l, &r);
    auto b_l = blocks_info.begin();
    std::advance(b_l, l);
    auto b_r = blocks_info.begin();
    std::advance(b_r, r);
    auto my_block = mergeBlock(b_size, b_l, b_r);
    MyMatrixImp local_x(1, b_size.second);
    auto row_size = (r - l) * b_size.first;
    MyMatrixImp local_a(row_size, b_size.second);

  }

  
}

MyMatrixImp mpiImp(int my_rank, int size, const TaskTypeImp& task,
                   bool printDataCopyTime) {
  auto n = task.n();
  const auto& m_shape = task.matrixShape();
  const auto& v_shape = task.vectorShape();
  const auto& b_size = task.blockSize();

  MyMatrixImp a;
  MyMatrixImp x;

  std::size_t num_blocks;
  std::vector<MyMatrixImp::BlockInfo> vector_block_info;
  std::vector<MyMatrixImp::BlockInfo> blocks_info;

  MyMatrixImp local_a;
  MyMatrixImp local_x;

  if (my_rank == 0) {
    if (b_size.second != m_shape.second) {
      throw std::runtime_error("Only support split by row\n");
    }
  }

  std::stringstream ss;
  std::string prefix = "Process " + std::to_string(my_rank) + ": ";
  std::cout << prefix + "Start\n";
  // generate random matrix and scatter it to other processes
  for (decltype(n) i = 0; i < n; ++i) {
    ss << "Task " << i << "\n";
    std::cout << ss.str();
    ss.str("");
    ss.clear();

    // first do init
    if (i == 0) {
      init(my_rank, size, a, x, vector_block_info, blocks_info, num_blocks,
           m_shape, v_shape, b_size);
    }

    if constexpr (DEBUG) {
      ss << "Vector: " << x.toString() << "\n";
      std::cout << ss.str();
      ss.str("");
      ss.clear();
    }

    deliverTask(my_rank, size, a, x, b_size, vector_block_info, blocks_info);

    // MPI_Scatterv(a.data().data(), nullptr, nullptr, MPI_CHAR,
    //              local_a.data().data(), 0, MPI_CHAR, 0, MPI_COMM_WORLD);
  }

  if (my_rank == 0) {
    // std::stringstream ss;
  }

  return {};
}

}  // namespace homework::mmv
