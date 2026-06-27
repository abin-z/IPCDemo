#include <fcntl.h>
#include <fmt/core.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>

#include "shared_memory.hpp"

int main()
{
  // 创建（或打开）一个 POSIX 共享内存对象
  //
  // 参数说明：
  // SHM_NAME          : 共享内存名称，必须以 '/' 开头
  // O_RDWR            : 以读写方式打开
  // O_CREAT           : 如果对象不存在则创建
  // 0666              : 权限 rw-rw-rw-
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

  // 设置共享内存大小
  //
  // 新创建的共享内存大小默认为 0 字节，
  // 必须通过 ftruncate() 指定容量。
  //
  // 参数说明：
  // fd                 : shm_open 返回的文件描述符
  // sizeof(SharedData) : 共享内存总大小
  //
  // 返回值：
  // 成功 -> 0
  // 失败 -> -1
  if (ftruncate(fd, sizeof(SharedData)) == -1)
  {
    fmt::print("Failed to set shared memory size\n");

    close(fd);
    shm_unlink(SHM_NAME);

    return 1;
  }

  // 将共享内存映射到当前进程的虚拟地址空间
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
  //     对内存的修改会同步到共享内存对象，
  //     其他进程可见
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
    shm_unlink(SHM_NAME);

    return 1;
  }

  // 打开信号量
  sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);  // 初始值0
  if (sem == SEM_FAILED)
  {
    fmt::print("Failed to sem_open\n");
    munmap(sharedData, sizeof(SharedData));
    close(fd);
    shm_unlink(SHM_NAME);
    return 1;
  }

  // 写入共享内存
  //
  // 此时 sharedData 指向共享内存区域，
  // 对其读写就像普通结构体一样。
  sharedData->value = 101;

  // strcpy 会复制字符串内容，
  // 并自动复制结尾 '\0'
  std::strcpy(sharedData->message, "Hello Shared Memory + Semaphore");

  fmt::print("Writer:\n");
  fmt::print("value   = {}\n", sharedData->value);
  fmt::print("message = {}\n", sharedData->message);

  //
  // 通知Reader
  //
  sem_post(sem);

  fmt::print("Notification sent\n");
  // 关闭 semaphore（关键）
  sem_close(sem);

  // 解除映射
  //
  // 参数：
  // sharedData          : mmap 返回的地址
  // sizeof(SharedData)  : 映射区域大小
  //
  // 注意：
  // munmap 只解除当前进程映射，
  // 不会删除共享内存对象。
  munmap(sharedData, sizeof(SharedData));

  // 关闭文件描述符
  //
  // 注意：
  // close 只关闭 fd，
  // 不会删除共享内存对象。
  close(fd);

  // 如果这里执行：
  //
  // shm_unlink(SHM_NAME);
  //
  // 则会删除共享内存对象的名字，
  // 后续进程将无法再通过 shm_open() 找到它。
  //
  // 当前 Demo 需要 Reader 继续访问，
  // 因此不在 Writer 中执行 shm_unlink()。

  fmt::print("Writer finished\n");

  return 0;
}