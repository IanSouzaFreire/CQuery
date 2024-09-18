#ifndef CQUERY_UTIL_MEMORY_HPP
#define CQUERY_UTIL_MEMORY_HPP

namespace CQuery {

template <typename ReturnType, typename... Args>
class Cache {
  std::function<ReturnType(Args...)> func_;
  std::unordered_map<std::tuple<Args...>, ReturnType> cache_;
  size_t max_size_;
  size_t hits_ = 0;
  size_t misses_ = 0;

public:
  Cache(std::function<ReturnType(Args...)> func, size_t max_size = 0)
    : func_(std::move(func)), max_size_(max_size) {}

  ReturnType operator()(Args... args) {
    auto key = std::make_tuple(args...);
    auto it = cache_.find(key);
    
    if (it != cache_.end()) {
      ++hits_;
      return it->second;
    }

    ++misses_;
    ReturnType result = func_(args...);
    cache_[key] = result;

    if (max_size_ > 0 && cache_.size() > max_size_) {
      // Evict the least recently used item
      cache_.erase(cache_.begin());
    }

    return result;
  }

  void cache_clear() {
    cache_.clear();
    hits_ = 0;
    misses_ = 0;
  }

  size_t cache_size() const {
    return cache_.size();
  }

  size_t cache_max_size() const {
    return max_size_;
  }

  size_t cache_hits() const {
    return hits_;
  }

  size_t cache_misses() const {
    return misses_;
  }
};

template <typename ReturnType, typename... Args>
class TypedCache {
  std::function<ReturnType(Args...)> func_;
  std::unordered_map<std::tuple<std::type_index, Args...>, ReturnType> cache_;
  size_t max_size_;
  size_t hits_ = 0;
  size_t misses_ = 0;

public:
  TypedCache(std::function<ReturnType(Args...)> func, size_t max_size = 0)
    : func_(std::move(func)), max_size_(max_size) {}

  ReturnType operator()(Args... args) {
    auto key = std::make_tuple(std::type_index(typeid(args))..., args...);
    auto it = cache_.find(key);
    
    if (it != cache_.end()) {
      ++hits_;
      return it->second;
    }

    ++misses_;
    ReturnType result = func_(args...);
    cache_[key] = result;

    if (max_size_ > 0 && cache_.size() > max_size_) {
      // Evict the least recently used item
      cache_.erase(cache_.begin());
    }

    return result;
  }

  void cache_clear() {
    cache_.clear();
    hits_ = 0;
    misses_ = 0;
  }

  size_t cache_size() const {
    return cache_.size();
  }

  size_t cache_max_size() const {
    return max_size_;
  }

  size_t cache_hits() const {
    return hits_;
  }

  size_t cache_misses() const {
    return misses_;
  }
};

}

#endif