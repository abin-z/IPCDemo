#include <fcntl.h>
#include <fmt/core.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>

#include "shared_memory.hpp"

int main()
{
  // 打开共享内存对象
  int fd = shm_open(SHM_NAME, O_RDWR, 0666);
  if (fd == -1)
  {
    fmt::print("Failed to open shared memory\n");
    return 1;
  }

  // 将共享内存映射到进程地址空间
  auto *sharedData =
    static_cast<SharedData *>(mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  if (sharedData == MAP_FAILED)
  {
    fmt::print("Failed to map shared memory\n");
    close(fd);
    return 1;
  }

  // 读取共享内存中的数据
  fmt::print("Reader:\n");
  fmt::print("value   = {}\n", sharedData->value);
  fmt::print("message = {}\n", sharedData->message);

  // 解除映射和关闭文件描述符
  munmap(sharedData, sizeof(SharedData));
  close(fd);

  // 删除共享内存对象
  shm_unlink(SHM_NAME);

  fmt::print("Reader finished\n");
  return 0;
}