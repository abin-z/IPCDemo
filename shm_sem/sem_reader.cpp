#include <fcntl.h>
#include <fmt/core.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>

#include "fmt/base.h"
#include "shared_memory.hpp"

int main()
{
  // 打开已存在的共享内存对象
  //
  // 参数说明：
  // SHM_NAME : 共享内存名称
  // O_RDWR   : 以读写方式打开
  // 0666     : 权限参数（仅 O_CREAT 时有效）
  //
  // 注意：
  // Reader 负责连接已有共享内存，
  // 因此不使用 O_CREAT。
  //
  // 返回值：
  // 成功 -> 文件描述符(fd)
  // 失败 -> -1
  int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
  if (fd == -1)
  {
    fmt::print("Failed to open shared memory\n");
    return 1;
  }

  // 将共享内存映射到当前进程地址空间
  //
  // mmap 参数说明：
  //
  // nullptr
  //     让内核自动选择映射地址
  //
  // sizeof(SharedData)
  //     映射区域大小
  //
  // PROT_READ | PROT_WRITE
  //     映射区域可读可写
  //
  // MAP_SHARED
  //     共享映射
  //     对共享内存的修改会同步到其他进程
  //
  // fd
  //     shm_open 返回的文件描述符
  //
  // 0
  //     从共享内存起始位置开始映射
  //
  // 返回值：
  // 成功 -> 映射地址
  // 失败 -> MAP_FAILED
  auto *sharedData =
    static_cast<SharedData *>(mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));

  if (sharedData == MAP_FAILED)
  {
    fmt::print("Failed to map shared memory\n");

    close(fd);

    return 1;
  }

  // 打开信号量
  sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);  // 初始值0
  if (sem == SEM_FAILED)
  {
    fmt::print("Failed to sem_open\n");
    munmap(sharedData, sizeof(SharedData));
    close(fd);
    // shm_unlink(SHM_NAME);
    return 1;
  }

  fmt::print("Waiting for data...\n");

  //
  // 阻塞等待
  //
  sem_wait(sem);
  fmt::print("======");

  // 读取共享内存中的数据
  //
  // mmap 成功后，
  // sharedData 就像普通结构体指针一样使用。
  fmt::print("Reader:\n");
  fmt::print("value   = {}\n", sharedData->value);
  fmt::print("message = {}\n", sharedData->message);

  // 关闭 semaphore（关键）
  sem_close(sem);

  // 解除映射
  //
  // 参数：
  // sharedData         : mmap 返回的地址
  // sizeof(SharedData) : 映射区域大小
  //
  // 注意：
  // munmap 仅解除当前进程与共享内存的映射关系，
  // 不会删除共享内存对象。
  munmap(sharedData, sizeof(SharedData));

  // 关闭文件描述符
  //
  // 注意：
  // close 仅关闭当前进程持有的 fd，
  // 不会删除共享内存对象。
  close(fd);

  // 删除共享内存对象
  //
  // shm_unlink() 的作用是删除共享内存对象的名字，
  // 类似于文件系统中的 unlink()。
  //
  // 注意：
  // 1. 不会立即销毁共享内存
  // 2. 已经 mmap 的进程仍可继续访问
  // 3. 新进程将无法通过 shm_open() 找到该对象
  // 4. 当最后一个进程 munmap() 且 close() 后，
  //    内核才会真正回收共享内存
  //
  // 通常由共享内存的创建者(Writer)负责删除，
  // Reader 一般不执行 shm_unlink()。
  //
  // shm_unlink(SHM_NAME);

  fmt::print("Reader finished\n");

  return 0;
}