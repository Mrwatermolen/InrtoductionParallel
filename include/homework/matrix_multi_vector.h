#ifndef __PROJECT_NAME_MATRIX_MULTI_VECTOR_H__
#define __PROJECT_NAME_MATRIX_MULTI_VECTOR_H__

#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "helper.h"
#include "homework.h"

namespace homework::mmv {

using MyMatrixShape = std::pair<std::size_t, std::size_t>;

template <typename T>
class MyMatrix {
 public:
  struct BlockInfo {
    const auto& displacements() const { return _disp; }

    const auto& counts() const { return _count; }

    auto toString() const {
      std::stringstream ss;
      ss << "Displacements: ";
      for (const auto& disp : _disp) {
        ss << disp << " ";
      }
      ss << "Counts: ";
      for (const auto& count : _count) {
        ss << count << " ";
      }
      return ss.str();
    }

    std::vector<std::size_t> _disp;
    std::vector<std::size_t> _count;
  };

  MyMatrix() = default;

  MyMatrix(std::size_t row, std::size_t column)
      : _shape(row, column), _data(std::vector<T>(n(), {})) {}

  explicit MyMatrix(MyMatrixShape shape)
      : _shape(std::move(shape)), _data(std::vector<T>(n(), {})) {}

  MyMatrix(std::size_t row, std::size_t column, std::vector<T> data) {
    if (row * column != data.size()) {
      throw std::runtime_error("Invalid data size.");
    }

    _shape = std::make_pair(row, column);
    this->data() = std::move(data);
  }

  MyMatrix(MyMatrixShape shape, std::vector<T> data) {
    if (shape.first * shape.second != data.size()) {
      throw std::runtime_error("Invalid data size.");
    }

    _shape = std::move(shape);
    this->data() = std::move(data);
  }

  ~MyMatrix() = default;

  const MyMatrixShape& shape() const { return _shape; }

  MyMatrixShape& shape() { return _shape; }

  auto row() const { return _shape.first; }

  auto column() const { return _shape.second; }

  auto n() const { return row() * column(); }

  const auto& data() const { return _data; }

  auto& data() { return _data; }

  auto randomData(T&& min, T&& max) {
    static std::default_random_engine generator;
    static std::uniform_real_distribution<T> distribution(std::forward<T>(min),
                                                          std::forward<T>(max));
    for (std::size_t i = 0; i < row(); ++i) {
      for (std::size_t j = 0; j < column(); ++j) {
        this->operator()(i, j) = distribution(generator);
      }
    }
  }

  const auto& operator()(std::size_t i, std::size_t j) const {
    return this->operator[](offset(i, j, shape()));
  }

  auto& operator()(std::size_t i, std::size_t j) {
    return this->operator[](offset(i, j, shape()));
  }

  const auto& operator[](std::size_t i) const { return _data[i]; }

  auto& operator[](std::size_t i) { return _data[i]; }

  auto operator*(const MyMatrix<T>& rhs) const {
    if (column() != rhs.row()) {
      throw std::runtime_error("Invalid matrix size.");
    }

    MyMatrix<T> res(row(), rhs.column());
    for (std::size_t i = 0; i < row(); ++i) {
      for (std::size_t j = 0; j < rhs.column(); ++j) {
        for (std::size_t k = 0; k < column(); ++k) {
          res(i, j) += this->operator()(i, k) * rhs(k, j);
        }
      }
    }

    return res;
  }

  auto transpose() const {
    MyMatrix<T> res(column(), row());
    for (std::size_t i = 0; i < row(); ++i) {
      for (std::size_t j = 0; j < column(); ++j) {
        res(j, i) = this->operator()(i, j);
      }
    }

    return res;
  }

  std::string toString() const {
    std::stringstream ss;
    ss << "Shape: " << _shape.first << " x " << _shape.second << "\n";
    ss << "HeadData: \n";

    auto n = _shape.first > 6UL ? 6UL : _shape.first;
    auto m = _shape.second > 6UL ? 6UL : _shape.second;
    for (std::size_t i = 0; i < n; ++i) {
      for (std::size_t j = 0; j < m; ++j) {
        ss << this->operator()(i, j) << " ";
      }
      ss << "\n";
    }

    auto str = ss.str();
    str.pop_back();
    return ss.str();
  }

  static auto createRandomMatrix(const MyMatrixShape& shape, T&& min, T&& max) {
    MyMatrix<T> matrix(shape);
    matrix.randomData(std::forward<T>(min), std::forward<T>(max));
    return matrix;
  }

  static auto offset(std::size_t i, std::size_t j, const MyMatrixShape& shape) {
    return i * shape.second + j;
  }

  static auto createBlockInfo(const MyMatrix<T>& A, const std::size_t start_i,
                              const std::size_t start_j, const std::size_t ni,
                              const std::size_t nj, bool row_major = true) {
    std::vector<std::size_t> displacements(ni);
    std::vector<std::size_t> counts(ni);

    for (std::size_t ii = 0; ii < ni; ++ii) {
      auto disp = offset(start_i + ii, start_j, A.shape());
      auto count = nj;
      displacements[ii] = disp;
      counts[ii] = count * sizeof(T);
    }

    return BlockInfo{displacements, counts};
  }

  static auto splitByRow(std::size_t row_size, const MyMatrix<T>& A) {
    using BlockSize = MyMatrixShape;
    using Block = MyMatrix<T>;
    MyMatrix<Block> blocks;
    BlockSize bs = std::make_pair(row_size, A.column());
    return Block::splitByBlock(bs, A);
  }

  static auto splitByColumn(std::size_t column_size, const MyMatrix<T>& A) {
    using BlockSize = MyMatrixShape;
    using Block = MyMatrix<T>;
    MyMatrix<Block> blocks;
    BlockSize bs = std::make_pair(A.row(), column_size);
    return Block::splitByBlock(bs, A);
  }

  /**
   * @brief split Matrix A by block size
   *
   * @param block_size block size: a pair of row and column
   * @param A
   * @return blocks information: displacements and counts
   */
  static auto splitByBlock(const MyMatrixShape& block_size,
                           const MyMatrix<T>& A) {
    using BlockSize = MyMatrixShape;
    using Block = MyMatrix<T>;
    auto a_m = A.row();
    auto a_n = A.column();
    auto bs_m = block_size.first;
    auto bs_n = block_size.second;
    auto b_m = a_m / bs_m + (a_m % bs_m == 0 ? 0 : 1);
    auto b_n = a_n / bs_n + (a_n % bs_n == 0 ? 0 : 1);
    auto num_blocks = b_m * b_n;

    std::vector<BlockInfo> blocks_info;

    for (std::size_t i = 0; i < b_m; ++i) {
      auto end_i = i * bs_m + bs_m;
      if (end_i > a_m) {
        end_i = a_m;
      }
      for (std::size_t j = 0; j < b_n; ++j) {
        auto disp = offset(i, j, A.shape());
        auto end_j = j * bs_n + bs_n;
        if (end_j > a_n) {
          end_j = a_n;
        }

        auto info = createBlockInfo(A, i * bs_m, j * bs_n, end_i - i * bs_m,
                                    end_j - j * bs_n);
        blocks_info.emplace_back(std::move(info));
      }
    }

    return blocks_info;
  }

 private:
  MyMatrixShape _shape;
  std::vector<T> _data;
};

template <typename T>
class MMVTask : public Task {
 public:
  MMVTask() = default;

  MMVTask(std::size_t n, MyMatrixShape matrix_shape, MyMatrixShape vector_shape,
          MyMatrixShape block_size)
      : Task(n),
        _matrix_shape(std::move(matrix_shape)),
        _vector_shape(std::move(vector_shape)),
        _block_size(std::move(block_size)) {}

  ~MMVTask() override = default;

  auto bytes() const -> std::size_t override {
    // return sizeof(MMVTask<T>) + sizeof(MyMatrix<T>) + sizeof(MyMatrix<T>);
    return sizeof(*this);
  }

  auto toString() const -> std::string override {
    std::stringstream ss;
    ss << "Matrix Multiplicate Vector Task: Time of operation: " << n() << " ";
    ss << " Matrix Shape: " << _matrix_shape.first << " x "
       << _matrix_shape.second << " ";
    ss << " Vector Shape: " << _vector_shape.first << " x "
       << _vector_shape.second << " ";
    return ss.str();
  }

  auto matrixShape() const -> const MyMatrixShape& { return _matrix_shape; }

  auto vectorShape() const -> const MyMatrixShape& { return _vector_shape; }

  auto blockSize() const -> const MyMatrixShape& { return _block_size; }

  static auto createFromInput() {
    std::cout << "Input operation times: \n";
    auto n = getInoutOneDimensionProblemSize();
    std::cout << "Input matrix shape: \n";
    std::size_t a_m;
    std::size_t a_n;
    std::cin >> a_m >> a_n;
    std::size_t b_m = a_n;
    std::size_t b_n = 1;
    std::cout << "Input block size: \n";
    std::size_t bs_m;
    std::size_t bs_n;
    std::cin >> bs_m >> bs_n;
    return MMVTask(n, std::make_pair(a_m, a_n), std::make_pair(b_m, b_n),
                   std::make_pair(bs_m, bs_n));
  }

 private:
  MyMatrixShape _matrix_shape;
  MyMatrixShape _vector_shape;
  MyMatrixShape _block_size;
};

template <typename T>
class MMVResult : public Result<MyMatrix<T>> {
 public:
  MMVResult() = default;

  explicit MMVResult(MyMatrix<T> res) : Result<MyMatrix<T>>(std::move(res)) {}

  MMVResult(const MMVResult& other) = default;

  MMVResult(MMVResult&& other) noexcept = default;

  MMVResult& operator=(const MMVResult& other) = default;

  MMVResult& operator=(MMVResult&& other) noexcept = default;

  ~MMVResult() override = default;

  auto toString() const -> std::string override {
    std::stringstream ss;
    ss << "Matrix Multiplicate Vector Result: ";
    ss << this->res().toString();
    return ss.str();
  }

  inline static T epsilon = 1e-6;
  static auto sameResult(const MMVResult<T>& lhs, const MMVResult<T>& rhs) {
    if (lhs.res().shape() != rhs.res().shape()) {
      return false;
    }

    for (std::size_t i = 0; i < lhs.res().row(); ++i) {
      for (std::size_t j = 0; j < lhs.res().column(); ++j) {
        if (std::abs(lhs.res()(i, j) - rhs.res()(i, j)) > epsilon) {
          return false;
        }
      }
    }

    return true;
  }
};

/*
1. 输入相乘次数 n
2. 输入矩阵和向量维度 m 生成一个 m x m 的矩阵和 m 维向量
3. i = 0 to n-1
    A_[i] = random
    (
    分离任务
    x_[i+1] = A_[i] * x_[i]
    汇总
    ) 并行实现部分
*/

template <typename T>
inline auto serialImp(const MyMatrix<T>& A, const MyMatrix<T>& x) {
  return A * x;
}

using DataTypeImp = double;
using MyMatrixImp = MyMatrix<DataTypeImp>;
using TaskTypeImp = MMVTask<DataTypeImp>;
using ResultTypeImp = MMVResult<DataTypeImp>;

MyMatrixImp mpiImp(int my_rank, int size, const TaskTypeImp& task,
                   bool printDataCopyTime = true);

MyMatrixImp ompImp(int num_threads, const TaskTypeImp& task);

MyMatrixImp threadImp(int num_threads, const TaskTypeImp& task);

}  // namespace homework::mmv

#endif  // __PROJECT_NAME_MATRIX_MULTI_VECTOR_H__
