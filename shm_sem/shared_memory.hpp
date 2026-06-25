#pragma once

// 共享内存
// +
// POSIX Semaphore

#include <cstdint>

constexpr const char *SHM_NAME = "/ipc_demo_sem_shm";  // 共享内存对象名称
constexpr const char *SEM_NAME = "/ipc_demo_sem_sem";  // 信号量,
constexpr std::size_t MSG_SIZE = 1024;                 // 共享内存大小

struct SharedData
{
  int value;
  char message[MSG_SIZE];
};