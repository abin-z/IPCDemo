#include <fmt/core.h>

#include "ring_buffer.hpp"

int main()
{
  RingBuffer<int, 8> rb;

  fmt::print("Push:\n");

  for (int i = 1; i <= 7; ++i)
  {
    if (rb.push(i))
    {
      fmt::print("push {}\n", i);
    }
  }

  fmt::print("\nPop:\n");

  int value{};

  while (rb.pop(value))
  {
    fmt::print("pop {}\n", value);
  }
}