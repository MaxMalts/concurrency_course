#include <wheels/test/test_framework.hpp>

// https://gitlab.com/Lipovsky/tinyfibers
#include <tinyfibers/api.hpp>
#include <tinyfibers/sync/mutex.hpp>
#include <tinyfibers/sync/wait_group.hpp>
#include <tinyfibers/runtime/deadlock.hpp>

#include <wheels/support/quick_exit.hpp>
#include <wheels/support/panic.hpp>

using tinyfibers::Mutex;
using tinyfibers::RunScheduler;
using tinyfibers::SetDeadlockHandler;
using tinyfibers::Spawn;
using tinyfibers::WaitGroup;
using tinyfibers::self::Yield;

TEST_SUITE(Deadlock) {
  // Deadlock with one fiber and one mutex
  TEST(OneFiber, wheels::test::TestOptions().ForceFork()) {
    RunScheduler([]() {
      Mutex mutex;

      auto locker = [&]() {
        mutex.Lock();
        mutex.Lock();

        // use mutex.Lock() / mutex.Unlock() to lock/unlock mutex
      };

      SetDeadlockHandler([]() {
        std::cout << "Fiber deadlocked!" << std::endl;
        // World is broken, leave it ASAP
        wheels::QuickExit(0);
      });

      Spawn(locker).Join();

      // We do not expect to reach this line
      WHEELS_PANIC("No deadlock =(");
    });
  }

  // Deadlock with two fibers
  TEST(TwoFibers, wheels::test::TestOptions().ForceFork()) {
    RunScheduler([]() {
      // Mutexes

      Mutex a;
      Mutex b;

      // Fiber routines

      int first_instances = 0;
      int second_instances = 0;

      auto first = [&]() {
        ++first_instances;
        if (first_instances == 3) {
          a.Lock();
        }
        // Use Yield() to reschedule current fiber
      };

      auto second = [&]() {
        ++second_instances;
        if (second_instances == 3) {
          a.Lock();
        }
      };

      // No deadlock with one fiber

      // No deadlock expected here
      // Run routine twice to check that
      // routine leaves mutexes in unlocked state
      Spawn(first).Join();
      Spawn(first).Join();

      // Same for `second`
      Spawn(second).Join();
      Spawn(second).Join();

      // Deadlock with two fibers

      SetDeadlockHandler([]() {
        std::cout << "Fibers deadlocked!" << std::endl;
        // World is broken, leave it ASAP
        wheels::QuickExit(0);
      });

      WaitGroup wg;
      wg.Spawn(first);
      wg.Spawn(second);
      wg.Wait();

      // We do not expect to reach this line
      WHEELS_PANIC("No deadlock =(");
    });
  }
}

RUN_ALL_TESTS()
