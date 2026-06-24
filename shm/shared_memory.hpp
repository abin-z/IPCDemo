#pragma once

#include <cstdint>

constexpr const char *SHM_NAME = "/ipc_demo_shm";  // 共享内存对象名称
constexpr std::size_t SHM_SIZE = 1024;             // 共享内存大小

struct SharedData
{
  int32_t value;
  char message[SHM_SIZE];
};