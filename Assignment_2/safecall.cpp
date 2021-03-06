#include "safecall.h"
#include <cstddef>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// ====================================================================
// this is the function you need to reimplement
// ====================================================================
//
// safecall(n) calls unsafe(n) and returns values as follows.
//
// if the call to unsafe() crashes, safecall() will return -2
//
// if unsafe() does not return within 1s, safecall() will return -1
//   please note that safecall() must always return within 1s
//
// if the call to unsafe() returns within 1s, safecall() will return
// the value that unsafe() returned
//
int safecall(int i)
{
  // This is not a good implementation.
  // You need to change it... :)

  // My recommendation:
  //
  // remove temporary file
  unlink("/tmp/talhaSafeCall");
  pid_t pid = fork();
  // //If Child
  if (pid == 0)
  {
    int result = unsafe(i);
    FILE *pFile = fopen("/tmp/talhaSafeCall", "wb");
    if (pFile != NULL)
    {
      fwrite(&result, sizeof(int), 1, pFile);
    }
    fclose(pFile);
    exit(0);
  }
  else
  {
    auto start_time = std::chrono::steady_clock::now();
    for (;;)
    {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
      pid_t n = waitpid(pid, NULL, WNOHANG);
      if (n > 1)
      {
        int num;
        FILE *pFile = fopen("/tmp/talhaSafeCall", "rb");
        if (pFile == NULL)
        {
          return -2;
        }
        uint8_t stat = fread(&num, sizeof(int), 1, pFile);
        fclose(pFile);

        if (stat == 0)
        {
          return -2;
        }
        return num;
      }
      auto curr_time = std::chrono::steady_clock::now();
      double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(curr_time - start_time).count() / 1000000.0;
      if (elapsed > 1.0)
      {
        kill(pid, SIGKILL);
        return -1;
      }
    }
  }
  // pid = fork()
  // in child process:
  //   call result=unsafe(i)
  //   write result to temporary file
  //   exit
  // in parent process:
  //   remember current time
  //   loop:
  //     sleep for 0.000001s
  //     waitpid(pid, NULL, WNOHANG)
  //     if waitpid successful
  //       read results from file
  //       if reading results was successful, return result
  //       othersise return -2
  //     measure elapsed time since loop started
  //     if elapsed time > 1s
  //       kill child
  //       return -1
  //

  // Alternative implementation:
  //
  // Very similar to the above, but instead of using a busy loop with
  // sleep, we can spawn an additional child process. The 2nd child
  // will simply sleep for 1s and then exit.
  //
  // In the parent, we can call wait(NULL) to see which child finished
  // first. if the child that calls unsafe() finished first, then we
  // know it finished under 1s. We can kill the 2nd child and collect
  // results...
  //
  // If the 2nd child finished first, we know that the unsafe() is still
  // running even after 1s. We can kill the 1st child and return -1.
  //
  return 0;
}
