#pragma once

// ============================================================================
// Shared Memory + POSIX Semaphore Demo
//
// 共享内存(Shared Memory)
//     负责在多个进程之间共享数据。
//
// POSIX 信号量(Semaphore)
//     负责进程间同步与通知。
//     Semaphore 本质上是一个内核维护的计数器：
//
//         sem_post() -> 计数 +1
//         sem_wait() -> 计数 -1
//
//     当计数为 0 时，sem_wait() 会阻塞等待。
//     当计数大于 0 时，sem_wait() 会立即返回。
//
// 注意：
//     Semaphore 只负责通知，不负责存储数据。
//     如果 Writer 连续发送多条消息，而共享内存只有一个数据槽位，
//     后面的消息会覆盖前面的消息。
//
//     Semaphore 会累计通知次数。
//     但如果你的共享内存只有一个数据槽位，那么 Writer 连续 sem_post() 3 次，
//     Reader 虽然能 sem_wait() 3 次，但读到的很可能都是最后一次写入的数据。
//
//     工业界通常采用：
//
//         Shared Memory
//             +
//         Semaphore/Futex
//             +
//         Ring Buffer
//
//     来实现高性能 IPC。
// ============================================================================

#include <cstdint>

// POSIX 共享内存对象名称
//
// Linux 下通常可以在：
//
//     /dev/shm
//
// 中看到对应对象。
//
// 名称必须以 '/' 开头。
constexpr const char *SHM_NAME = "/ipc_demo_sem_shm";

// POSIX 命名信号量名称
//
// Linux 下通常表现为：
//
//     /dev/shm/sem.ipc_demo_sem_sem
//
// 名称必须以 '/' 开头。
constexpr const char *SEM_NAME = "/ipc_demo_sem_sem";

// 消息缓冲区大小
//
// 当前 Demo 仅保存一条字符串消息。
// message 数组最大可存储 MSG_SIZE - 1 个字符，
// 最后一个字节用于 '\0' 结束符。
constexpr std::size_t MSG_SIZE = 1024;

// 共享内存中的数据结构
//
// Writer 与 Reader 必须使用完全相同的数据结构定义，
// 否则会导致内存布局不一致。
//
// 当前设计仅用于演示共享内存的基本读写：
//
//     value   -> 整数数据
//     message -> 字符串数据
//
// 注意：
//     当前结构只有一个消息槽位(Slot)。
//
//     如果 Writer 连续写入：
//
//         value = 1
//         value = 2
//         value = 3
//
//     Reader 最终只能看到最后一次写入的数据：
//
//         value = 3
//
//     若需要保存多条消息，应使用 Ring Buffer。
struct SharedData
{
  int value;

  char message[MSG_SIZE];
};