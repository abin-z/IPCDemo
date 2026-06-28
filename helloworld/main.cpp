#include <fmt/core.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

// 控制主循环是否继续运行。
// 当收到 Ctrl+C(SIGINT) 时，将其修改为 false，
// 主线程检测到后退出循环，并完成资源清理。
static std::atomic<bool> running{true};

// SIGINT(Ctrl+C) 信号处理函数
//
// 当用户在终端按下 Ctrl+C 时：
//
//      Ctrl+C
//         │
//         ▼
//      Terminal
//         │
//         ▼
//   向前台进程发送 SIGINT
//         │
//         ▼
//      signal_handler()
//
// 如果程序没有注册 SIGINT 处理函数，
// Linux 会执行 SIGINT 的默认行为(Default Action)：
//
//      Terminate Process（终止程序）
//
// 即程序会被内核直接结束。
//
// 如果程序注册了自己的处理函数，
// Linux 就不会再执行默认行为，而是调用该函数。
// 因此，需要由程序自己决定如何处理该信号。
//
// 在工程中，推荐不要在这里直接退出程序，
// 而是仅设置一个退出标志，让主线程完成资源释放，
// 例如：关闭 socket、释放共享内存、关闭信号量等。
void signal_handler(int)
{
  running = false;
}

int main()
{
  // 注册 SIGINT(Ctrl+C) 的处理函数。
  //
  // 常见写法：
  //
  // signal(SIGINT, signal_handler);
  //     注册自定义处理函数。
  //
  // signal(SIGINT, SIG_DFL);
  //     恢复系统默认行为（Ctrl+C 直接终止程序）。
  //
  // signal(SIGINT, SIG_IGN);
  //     忽略 Ctrl+C。
  std::signal(SIGINT, signal_handler);

  fmt::print("Press Ctrl+C to exit.\n");

  while (running)
  {
    fmt::print("hello world.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  fmt::print("\nCtrl+C detected, exiting...\n");

  return 0;
}