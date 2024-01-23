#include <gtest/gtest.h>

#include <memory>
#include <random>
#include <thread>

#include "exercises/thread_safe_structure/thread_safe_lookup_table.h"

using exercises::t_s_s::ThreadSafeLookupTable;

TEST(ThreadSafeLookupTable, Basic) {
  auto test = std::make_unique<ThreadSafeLookupTable<int, std::string>>(2);

  // insert several key/value pairs
  test->insert(1, "a");
  test->insert(2, "b");
  test->insert(3, "c");
  test->insert(4, "d");
  test->insert(5, "e");
  test->insert(6, "f");
  test->insert(7, "g");
  test->insert(8, "h");
  test->insert(9, "i");
  EXPECT_EQ(2, test->localDepth(0));
  EXPECT_EQ(3, test->localDepth(1));
  EXPECT_EQ(2, test->localDepth(2));
  EXPECT_EQ(2, test->localDepth(3));

  // find test
  std::string result;
  test->find(9, result);
  EXPECT_EQ("i", result);
  test->find(8, result);
  EXPECT_EQ("h", result);
  test->find(2, result);
  EXPECT_EQ("b", result);
  EXPECT_EQ(0, test->find(10, result));

  // delete test
  EXPECT_EQ(1, test->remove(8));
  EXPECT_EQ(1, test->remove(4));
  EXPECT_EQ(1, test->remove(1));
  EXPECT_EQ(0, test->remove(20));
}

TEST(ThreadSafeLookupTable, Basic2) {
  // set leaf size as 2
  auto test = std::make_unique<ThreadSafeLookupTable<int, std::string>>(2);

  // insert several key/value pairs
  test->insert(1, "a");
  test->insert(2, "b");
  test->insert(3, "c");
  test->insert(4, "d");
  test->insert(5, "e");
  test->insert(6, "f");
  test->insert(7, "g");
  test->insert(8, "h");
  test->insert(9, "i");
  EXPECT_EQ(2, test->localDepth(0));
  EXPECT_EQ(3, test->localDepth(1));
  EXPECT_EQ(2, test->localDepth(2));
  EXPECT_EQ(2, test->localDepth(3));

  // find test
  std::string result;
  test->find(9, result);
  EXPECT_EQ("i", result);
  test->find(8, result);
  EXPECT_EQ("h", result);
  test->find(2, result);
  EXPECT_EQ("b", result);
  EXPECT_EQ(0, test->find(10, result));

  // delete test
  EXPECT_EQ(1, test->remove(8));
  EXPECT_EQ(1, test->remove(4));
  EXPECT_EQ(1, test->remove(1));
  EXPECT_EQ(0, test->remove(20));

  test->insert(1, "a");
  test->insert(2, "b");
  test->insert(3, "c");
  test->insert(4, "d");
  test->insert(5, "e");
  test->insert(6, "f");
  test->insert(7, "g");
  test->insert(8, "h");
  test->insert(9, "i");

  test->find(9, result);
  EXPECT_EQ("i", result);
  test->find(8, result);
  EXPECT_EQ("h", result);
  test->find(2, result);
  EXPECT_EQ("b", result);
  EXPECT_EQ(0, test->find(10, result));
}

// first split increase global depth from 0 to 3
TEST(ThreadSafeLookupTableTest, BasicDepthTest) {
  // set leaf size as 2
  auto test = std::make_unique<ThreadSafeLookupTable<int, std::string>>(2);

  // insert several key/value pairs
  test->insert(6, "a");   // b'0110
  test->insert(10, "b");  // b'1010
  test->insert(14, "c");  // b'1110

  EXPECT_EQ(3, test->globalDepth());

  EXPECT_EQ(3, test->localDepth(2));
  EXPECT_EQ(3, test->localDepth(6));

  EXPECT_EQ(2, test->localDepth(0));
  EXPECT_EQ(1, test->localDepth(1));
  EXPECT_EQ(1, test->localDepth(3));
  EXPECT_EQ(2, test->localDepth(4));
  EXPECT_EQ(1, test->localDepth(5));
  EXPECT_EQ(1, test->localDepth(7));

  // four buckets in use
  EXPECT_EQ(4, test->numBuckets());

  // insert more key/value pairs
  test->insert(1, "d");
  test->insert(3, "e");
  test->insert(5, "f");

  EXPECT_EQ(5, test->numBuckets());

  EXPECT_EQ(2, test->localDepth(1));
  EXPECT_EQ(2, test->localDepth(3));
  EXPECT_EQ(2, test->localDepth(5));
}
#define TEST_NUM 1000
#define BUCKET_SIZE 64
TEST(ThreadSafeLookupTableTest, BasicRandomTest) {
  auto test = std::make_unique<ThreadSafeLookupTable<int, int>>(64);
  // insert
  int seed = time(nullptr);
  std::cerr << "seed: " << seed << '\n';
  std::default_random_engine engine(seed);
  std::uniform_int_distribution<int> distribution(0, TEST_NUM);
  std::map<int, int> comparator;

  for (int i = 0; i < TEST_NUM; ++i) {
    auto item = distribution(engine);
    comparator[item] = item;
    // printf("%d,",item);
    test->insert(item, item);
    // std::cerr << std::dec << item << std::hex << "( 0x" << item << " )" <<
    // std::endl;
  }
  // printf("\n");

  // compare result
  int value = 0;
  for (auto i : comparator) {
    test->find(i.first, value);
    // printf("%d,%d\n",,i.first);
    EXPECT_EQ(i.first, value);
    // delete
    EXPECT_EQ(1, test->remove(value));
    // find again will fail
    value = 0;
    EXPECT_EQ(0, test->find(i.first, value));
  }
}

TEST(ThreadSafeLookupTableTest, LargeRandominsertTest) {
  // set leaf size as 2
  auto *test = new ThreadSafeLookupTable<int, int>(10);

  int seed = 0;

  for (size_t i = 0; i < 100000; i++) {
    srand(time(nullptr) + i);
    if ((random() % 3) != 0) {
      test->insert(seed, seed);
      seed++;
    } else {
      if (seed > 0) {
        int value;
        int x = random() % seed;
        EXPECT_EQ(true, test->find(x, value));
        EXPECT_EQ(x, value);
      }
    }
  }
}

TEST(ThreadSafeLookupTableTest, RandominsertAndDeleteTest) {
  // set leaf size as 2
  auto *test = new ThreadSafeLookupTable<int, int>(10);

  for (int i = 0; i < 1000; i++) {
    test->insert(i, i);
  }

  for (int i = 0; i < 1000; i++) {
    srand(time(nullptr) + i);
    if (rand() % 2 == 0) {
      test->remove(i);
      int value;
      EXPECT_NE(test->find(i, value), true);
    } else {
      test->insert(i, i + 2);
      int value;
      EXPECT_EQ(test->find(i, value), true);
      EXPECT_EQ(value, i + 2);
    }
  }
}

TEST(ThreadSafeLookupTableTest, ConcurrentInsertTest) {
  const int num_runs = 50;
  const int num_threads = 3;
  // Run concurrent test multiple times to guarantee correctness.
  for (int run = 0; run < num_runs; run++) {
    std::shared_ptr<ThreadSafeLookupTable<int, int>> test{
        new ThreadSafeLookupTable<int, int>(2)};
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (int tid = 0; tid < num_threads; tid++) {
      threads.emplace_back([tid, &test]() { test->insert(tid, tid); });
    }
    for (int i = 0; i < num_threads; i++) {
      threads[i].join();
    }
    EXPECT_EQ(test->globalDepth(), 1);
    for (int i = 0; i < num_threads; i++) {
      int val;
      EXPECT_TRUE(test->find(i, val));
      EXPECT_EQ(val, i);
    }
  }
}

TEST(ThreadSafeLookupTableTest, ConcurrentRemoveTest) {
  const int num_threads = 5;
  const int num_runs = 50;
  for (int run = 0; run < num_runs; run++) {
    std::shared_ptr<ThreadSafeLookupTable<int, int>> test{
        new ThreadSafeLookupTable<int, int>(2)};
    std::vector<std::thread> threads;
    std::vector<int> values{0, 10, 16, 32, 64};
    for (int value : values) {
      test->insert(value, value);
    }

    EXPECT_EQ(test->globalDepth(), 6);
    threads.reserve(num_threads);
    for (int tid = 0; tid < num_threads; tid++) {
      threads.emplace_back([tid, &test, &values]() {
        test->remove(values[tid]);
        test->insert(tid + 4, tid + 4);
      });
    }
    for (int i = 0; i < num_threads; i++) {
      threads[i].join();
    }
    EXPECT_EQ(test->globalDepth(), 6);
    int val;
    EXPECT_EQ(0, test->find(0, val));
    EXPECT_EQ(1, test->find(8, val));
    EXPECT_EQ(0, test->find(16, val));
    EXPECT_EQ(0, test->find(3, val));
    EXPECT_EQ(1, test->find(4, val));
  }
}
