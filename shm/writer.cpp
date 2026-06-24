#include <fcntl.h>
#include <fmt/core.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>

#include "shared_memory.hpp"

int main()
{
  // 创建共享内存对象
  int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
  if (fd == -1)
  {
    fmt::print("Failed to open shared memory\n");
    return 1;
  }

  // 设置共享内存大小
  if (ftruncate(fd, sizeof(SharedData)) == -1)
  {
    fmt::print("Failed to set shared memory size\n");
    close(fd);
    shm_unlink(SHM_NAME);
    return 1;
  }

  // 将共享内存映射到进程地址空间
  auto *sharedData =
    static_cast<SharedData *>(mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  if (sharedData == MAP_FAILED)
  {
    fmt::print("Failed to map shared memory\n");
    close(fd);
    shm_unlink(SHM_NAME);
    return 1;
  }

  // 写入数据到共享内存
  sharedData->value = 96;
  std::strcpy(sharedData->message, "Hello Shared Memory");  // 使用 std::strcpy 复制字符串, 末尾会自动添加 '\0'
  fmt::print("Writer:\n");
  fmt::print("value   = {}\n", sharedData->value);
  fmt::print("message = {}\n", sharedData->message);

  // 解除映射和关闭文件描述符
  munmap(sharedData, sizeof(SharedData));
  close(fd);

  fmt::print("Writer finished\n");
  return 0;
}