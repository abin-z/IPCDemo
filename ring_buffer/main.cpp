#include <fmt/core.h>

#include <atomic>
#include <chrono>
#include <random>
#include <thread>

#include "ring_buffer.hpp"

void test();
void test2();  // 并发测试

int main()
{
  fmt::print("=== Running Test 1: Single Thread ===\n");
  test();

  fmt::print("\n\n=== Running Test 2: Producer-Consumer ===\n");
  test2();

  return 0;
}

// ============================================================================
// Test 1: 单线程基本功能测试
// ============================================================================
void test()
{
  RingBuffer<int, 8> rb;

  fmt::print("Push:\n");

  for (int i = 1; i <= 7; ++i)
  {
    if (rb.push(i))
    {
      fmt::print("  push {}\n", i);
    }
  }

  fmt::print("\nPop:\n");

  int value{};

  while (rb.pop(value))
  {
    fmt::print("  pop {}\n", value);
  }
}

// ============================================================================
// Test 2: 生产者-消费者并发测试
// ============================================================================
void test2()
{
  // 容量16的环形缓冲区
  RingBuffer<int, 16> rb;

  // 控制标志
  std::atomic<bool> running{true};

  // 统计数据
  std::atomic<int> produced{0};
  std::atomic<int> consumed{0};

  // ========================================
  // 生产者线程
  // ========================================
  std::thread producer([&]() {
    int value = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay(1, 10);  // 1-10ms 随机延迟

    fmt::print("[Producer] Started\n");

    while (running.load(std::memory_order_relaxed))
    {
      ++value;

      // 尝试入队，如果满了就重试
      if (rb.push(value))
      {
        produced++;
        fmt::print("[Producer=====] → {}\n", value);
      }
      else
      {
        // 队列满，等待一下
        fmt::print("[Producer] Queue full, waiting...\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        continue;
      }

      // 模拟生产延迟
      std::this_thread::sleep_for(std::chrono::milliseconds(delay(gen)));
    }

    fmt::print("[Producer] Stopped. Total: {}\n", produced.load());
  });

  // ========================================
  // 消费者线程
  // ========================================
  std::thread consumer([&]() {
    int value = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay(5, 15);  // 5-15ms 随机延迟

    fmt::print("[Consumer--] Started\n");

    while (running.load(std::memory_order_relaxed))
    {
      // 尝试出队
      if (rb.pop(value))
      {
        consumed++;
        fmt::print("[Consumer--] ← {}\n", value);

        // 模拟处理延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(delay(gen)));
      }
      else
      {
        // 队列空，等待一下
        // 注意：如果生产者已经停止，退出循环
        if (!running.load(std::memory_order_relaxed))
        {
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
      }
    }

    // 清空剩余数据，并统计已消费数量
    int remaining = 0;
    while (rb.pop(value))
    {
      consumed++;
      remaining++;
    }

    fmt::print("[Consumer] Stopped. Total: {}, Drained: {}\n", consumed.load(), remaining);
  });

  // ========================================
  // 运行 3 秒
  // ========================================
  fmt::print("\n[Main] Running for 1 seconds...\n\n");
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // 停止生产者（消费者还会继续处理队列中的剩余数据）
  running.store(false, std::memory_order_release);
  fmt::print("\n[Main] Stopping...\n");

  // 等待线程结束
  producer.join();
  consumer.join();

  // ========================================
  // 输出统计
  // ========================================
  fmt::print("\n=== Final Statistics ===\n");
  fmt::print("Produced: {}\n", produced.load());
  fmt::print("Consumed: {}\n", consumed.load());
  fmt::print("Lost:     {}\n", produced.load() - consumed.load());
  fmt::print("Final size: {}\n", rb.size());
  fmt::print("Queue empty: {}\n", rb.empty() ? "Yes" : "No");
}