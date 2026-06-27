#pragma once

// ============================================================================
// Lock-Free Ring Buffer (Single Producer / Single Consumer)
//
// Ring Buffer（环形缓冲区）是一种固定大小的循环队列，常用于：
//
//   - 高性能消息队列
//   - Producer / Consumer 模型
//   - 共享内存 IPC
//   - 网络收发缓冲区
//   - 音视频数据流
//
// 相比 std::queue：
//
//   - 固定容量，不需要动态内存分配
//   - 数据连续存储，Cache 友好
//   - O(1) 入队 / 出队
//   - 适合实时系统
//
// 当前实现特点：
//
//   ✔ Single Producer（一个生产者）
//   ✔ Single Consumer（一个消费者）
//   ✔ Lock-Free（无锁）
//   ✔ 基于 std::atomic 实现线程安全
//
// 注意：
//
//   本实现仅适用于 SPSC（Single Producer Single Consumer）。
//   如果存在多个 Producer 或多个 Consumer，需要采用不同的算法。
// ============================================================================

#include <array>
#include <atomic>
#include <cstddef>

template <typename T, std::size_t Capacity>
class RingBuffer
{
 public:
  // Ring Buffer 至少需要两个槽位。
  //
  // 本实现使用：
  //
  //     write == read
  //
  // 来表示缓冲区为空。
  //
  // 因此必须始终保留一个空槽位，用于区分：
  //
  //     空：
  //         write == read
  //
  //     满：
  //         (write + 1) % Capacity == read
  //
  // 若 Capacity == 1，则唯一的槽位始终为空，
  // 无法存储任何数据，因此至少需要两个槽位。
  static_assert(Capacity >= 2, "Capacity must be >= 2.");

  bool push(const T &value)
  {
    // Producer 只修改 write_，
    // 只读取 read_。

    const std::size_t write = write_.load(std::memory_order_relaxed);
    const std::size_t next = (write + 1) % Capacity;

    // Buffer 已满：
    //
    // 保留一个空槽位，
    // next == read_ 表示没有可写空间。
    //
    // acquire：
    // 保证读取到 Consumer 最新更新的 read_。
    if (next == read_.load(std::memory_order_acquire))
    {
      return false;
    }

    // 写入数据
    buffer_[write] = value;

    // 发布新的写位置。
    //
    // release：
    // 保证数据已经写入 buffer_ 后，
    // 再更新 write_。
    write_.store(next, std::memory_order_release);

    return true;
  }

  bool pop(T &value)
  {
    // Consumer 只修改 read_，
    // 只读取 write_。

    const std::size_t read = read_.load(std::memory_order_relaxed);

    // Buffer 为空：
    //
    // acquire：
    // 保证读取到 Producer 最新发布的数据。
    if (read == write_.load(std::memory_order_acquire))
    {
      return false;
    }

    // 读取数据
    value = buffer_[read];

    // 更新读位置。
    //
    // release：
    // 保证数据已经读取完成，
    // 再更新 read_。
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
  // 环形缓冲区存储空间
  std::array<T, Capacity> buffer_{};

  // 写索引（仅 Producer 修改）
  std::atomic<std::size_t> write_{0};

  // 读索引（仅 Consumer 修改）
  std::atomic<std::size_t> read_{0};
};