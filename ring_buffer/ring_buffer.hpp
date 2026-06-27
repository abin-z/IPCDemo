#pragma once

#include <array>
#include <atomic>
#include <cstddef>

template <typename T, std::size_t Capacity>
class RingBuffer
{
 public:
  static_assert(Capacity >= 2, "Capacity must be >= 2.");

  bool push(const T &value)
  {
    // Writer 只写 m_write，只读 m_read

    const std::size_t write = write_.load(std::memory_order_relaxed);
    const std::size_t next = (write + 1) % Capacity;

    // acquire：
    // 确保读取到 Consumer 最新更新的 read_index
    if (next == read_.load(std::memory_order_acquire))
    {
      return false;  // Buffer Full
    }

    buffer_[write] = value;

    // release：
    // 保证数据已经写入 buffer 后，再更新 write_index
    write_.store(next, std::memory_order_release);

    return true;
  }

  bool pop(T &value)
  {
    // Consumer 只写 m_read，只读 m_write

    const std::size_t read = read_.load(std::memory_order_relaxed);

    // acquire：
    // 保证读取到 Producer 最新发布的数据
    if (read == write_.load(std::memory_order_acquire))
    {
      return false;  // Buffer Empty
    }

    value = buffer_[read];

    // release：
    // 保证数据已经读取完成，再更新 read_index
    read_.store((read + 1) % Capacity, std::memory_order_release);

    return true;
  }

  [[nodiscard]] bool empty() const
  {
    return read_.load() == write_.load();
  }

  [[nodiscard]] bool full() const
  {
    return (write_.load() + 1) % Capacity == read_.load();
  }

  [[nodiscard]] std::size_t size() const
  {
    const auto write = write_.load();
    const auto read = read_.load();

    return (write + Capacity - read) % Capacity;
  }

 private:
  std::array<T, Capacity> buffer_{};

  // Producer 修改
  std::atomic<std::size_t> write_{0};

  // Consumer 修改
  std::atomic<std::size_t> read_{0};
};