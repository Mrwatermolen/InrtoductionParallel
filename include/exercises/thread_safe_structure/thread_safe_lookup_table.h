#ifndef __ConcurrencyAction_THREAD_SAFE_LOOKUP_TABLE_H__
#define __ConcurrencyAction_THREAD_SAFE_LOOKUP_TABLE_H__

#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

namespace exercises::t_s_s {

template <typename Key, typename Value, typename Hasher = std::hash<Key>>
class ThreadSafeLookupTable {
 public:
  explicit ThreadSafeLookupTable(std::size_t bucket_max_size,
                                 std::size_t global_depth = 0)
      : _global_depth(global_depth),
        _bucket_max_size(bucket_max_size),
        _current_number_bucket(1),
        _buckets(BucketArray{std::make_unique<Bucket>(bucket_max_size)}) {}

  std::size_t globalDepth() const;

  std::size_t localDepth(const Key& key) const;

  std::size_t numBuckets() const;

  bool find(const Key& key, Value& value) const;

  void insert(const Key& key, Value value);

  bool remove(const Key& key);

 private:
  class Bucket {
   public:
    using BucketValue = std::pair<Key, Value>;
    using BucketData = std::list<BucketValue>;
    using BucketIterator = typename BucketData::iterator;

    explicit Bucket(std::size_t max_size, std::size_t depth = 0)
        : _max_size(max_size), _depth(depth), _data() {}

    ~Bucket() = default;

    // no lock
    bool isFull() const;

    // no lock
    std::size_t size() const;

    // no lock
    std::size_t depth() const;

    std::shared_mutex& mutex() { return _mutex; }

    BucketData& data() { return _data; }

    bool find(const Key& key, Value& value) const;

    bool insert(const Key& key, Value value);

    bool remove(const Key& key);

    template <typename Predicate>
    std::pair<BucketData, BucketData> split(Predicate pred) {
      auto l{BucketData{}};
      auto r{BucketData{}};

      auto p = std::partition(data().begin(), data().end(), pred);
      l.splice(l.begin(), data(), data().begin(), p);
      r.splice(r.begin(), data(), p, data().end());
      return std::pair<BucketData, BucketData>(std::move(l), std::move(r));
    }

   private:
    mutable std::shared_mutex _mutex;
    std::size_t _max_size, _depth;
    BucketData _data;

    auto findEntry(const Key& key) const;

    auto findEntry(const Key& key);
  };

  using BucketArray = std::vector<std::shared_ptr<Bucket>>;

  mutable std::shared_mutex _mutex;

  std::size_t _global_depth{}, _bucket_max_size{}, _current_number_bucket{};
  BucketArray _buckets;

  std::size_t globalDepthInternal() const;

  std::size_t numBucketsInternal() const;

  std::size_t index(const Key& key) const;

  void redistribute(const Key& key, Value value);
};

template <typename Key, typename Value, typename Hasher>
inline std::size_t ThreadSafeLookupTable<Key, Value, Hasher>::globalDepth()
    const {
  std::shared_lock lock(_mutex);
  return globalDepthInternal();
}

template <typename Key, typename Value, typename Hasher>
inline std::size_t ThreadSafeLookupTable<Key, Value, Hasher>::localDepth(
    const Key& key) const {
  std::shared_lock lock(_mutex);
  return _buckets[index(key)]->depth();
}

template <typename Key, typename Value, typename Hasher>
inline std::size_t ThreadSafeLookupTable<Key, Value, Hasher>::numBuckets()
    const {
  std::shared_lock lock(_mutex);
  return numBucketsInternal();
}

template <typename Key, typename Value, typename Hasher>
inline bool ThreadSafeLookupTable<Key, Value, Hasher>::find(
    const Key& key, Value& value) const {
  std::shared_lock lock(_mutex);
  return _buckets[index(key)]->find(key, value);
}

template <typename Key, typename Value, typename Hasher>
inline void ThreadSafeLookupTable<Key, Value, Hasher>::insert(const Key& key,
                                                              Value value) {
  std::shared_lock lock(_mutex);
  auto bucket_index = this->index(key);
  auto& bucket = _buckets[bucket_index];

  if (bucket->insert(key, value)) {
    return;
  }

  lock.unlock();
  redistribute(key, value);
}

template <typename Key, typename Value, typename Hasher>
inline void ThreadSafeLookupTable<Key, Value, Hasher>::redistribute(
    const Key& key, Value value) {
  std::unique_lock write_table_lock(_mutex);
  while (true) {
    auto bucket_index = index(key);
    auto bucket = _buckets[bucket_index];
    if (bucket->insert(key, value)) {
      return;
    }

    std::unique_lock write_bucket_lock(bucket->mutex());
    const auto current_depth = bucket->depth();
    ++_current_number_bucket;
    if (current_depth == globalDepthInternal()) {
      ++_global_depth;
      const auto pre_size{_buckets.size()};
      _buckets.resize((pre_size << 1));
      for (auto i{0UL}; i < pre_size; ++i) {
        _buckets[i + pre_size] = _buckets[i];
      }
    }

    const auto flag_bit = (std::size_t{1} << (current_depth));
    auto [left_data, right_data] =
        bucket->split([this, &flag_bit](const auto& p) {
          return static_cast<bool>(!(this->index(p.first) & flag_bit));
        });

    auto left_node =
        std::make_shared<Bucket>(_bucket_max_size, current_depth + 1);
    auto right_node =
        std::make_shared<Bucket>(_bucket_max_size, current_depth + 1);
    left_node->data() = std::move(left_data);
    right_node->data() = std::move(right_data);

    for (auto i = (bucket_index & (flag_bit - 1)); i < _buckets.size();
         i += flag_bit) {
      if ((i & flag_bit)) {
        _buckets[i] = right_node;
        continue;
      }

      _buckets[i] = left_node;
    }
  }
}

template <typename Key, typename Value, typename Hasher>
inline bool ThreadSafeLookupTable<Key, Value, Hasher>::remove(const Key& key) {
  std::shared_lock lock(_mutex);
  const auto index = this->index(key);
  auto& bucket = _buckets[index];

  return bucket->remove(key);
}

template <typename Key, typename Value, typename Hasher>
inline std::size_t
ThreadSafeLookupTable<Key, Value, Hasher>::globalDepthInternal() const {
  return _global_depth;
}

template <typename Key, typename Value, typename Hasher>
inline std::size_t
ThreadSafeLookupTable<Key, Value, Hasher>::numBucketsInternal() const {
  return _current_number_bucket;
}

template <typename Key, typename Value, typename Hasher>
inline std::size_t ThreadSafeLookupTable<Key, Value, Hasher>::index(
    const Key& key) const {
  return Hasher{}(key) % _buckets.size();
}

template <typename Key, typename Value, typename Hasher>
inline bool ThreadSafeLookupTable<Key, Value, Hasher>::Bucket::isFull() const {
  return _max_size <= _data.size();
}

template <typename Key, typename Value, typename Hasher>
std::size_t ThreadSafeLookupTable<Key, Value, Hasher>::Bucket::size() const {
  return _data.size();
}

template <typename Key, typename Value, typename Hasher>
inline std::size_t ThreadSafeLookupTable<Key, Value, Hasher>::Bucket::depth()
    const {
  return _depth;
}

template <typename Key, typename Value, typename Hasher>
inline bool ThreadSafeLookupTable<Key, Value, Hasher>::Bucket::find(
    const Key& key, Value& value) const {
  std::shared_lock<std::shared_mutex> lock(_mutex);
  auto it = findEntry(key);
  if (it == _data.end()) {
    return false;
  }
  value = it->second;
  return true;
}

template <typename Key, typename Value, typename Hasher>
inline bool ThreadSafeLookupTable<Key, Value, Hasher>::Bucket::insert(
    const Key& key, Value value) {
  std::unique_lock<std::shared_mutex> lock(_mutex);
  BucketIterator it = findEntry(key);
  if (it != _data.end()) {
    it->second = std::move(value);
    return true;
  }

  if (isFull()) {
    return false;
  }

  _data.emplace_back(key, std::move(value));
  return true;
}

template <typename Key, typename Value, typename Hasher>
inline bool ThreadSafeLookupTable<Key, Value, Hasher>::Bucket::remove(
    const Key& key) {
  std::unique_lock<std::shared_mutex> lock(_mutex);
  auto it = findEntry(key);
  if (it == _data.end()) {
    return false;
  }

  _data.erase(it);
  return true;
}

template <typename Key, typename Value, typename Hasher>
inline auto ThreadSafeLookupTable<Key, Value, Hasher>::Bucket::findEntry(
    const Key& key) const {
  return std::find_if(_data.begin(), _data.end(),
                      [&key](const auto& item) { return item.first == key; });
}

template <typename Key, typename Value, typename Hasher>
inline auto ThreadSafeLookupTable<Key, Value, Hasher>::Bucket::findEntry(
    const Key& key) {
  // remove const call const
  // return const_cast<BucketIterator&>(
  //     static_cast<const Bucket&>(*this).findEntry(key));
  return std::find_if(_data.begin(), _data.end(),
                      [&key](const auto& item) { return item.first == key; });
}

}  // namespace exercises::t_s_s

#endif  // __ConcurrencyAction_THREAD_SAFE_LOOKUP_TABLE_H__
