#include <chrono>
#include <thread>

#include "common/common/lock_guard.h"
#include "common/common/mutex_tracer_impl.h"

#include "test/test_common/contention.h"
#include "test/test_common/test_base.h"

#include "absl/synchronization/mutex.h"

namespace Envoy {

class MutexTracerTest : public TestBase {
protected:
  void SetUp() override { tracer_.reset(); }

  // Since MutexTracerImpl::contentionHook is a private method, MutexTracerTest is a friend class.
  void sendWaitCyclesToContentionHook(int64_t wait_cycles) {
    tracer_.contentionHook(nullptr, nullptr, wait_cycles);
  }

  Thread::MutexBasicLockable mu_;
  MutexTracerImpl& tracer_{MutexTracerImpl::getOrCreateTracer()};
};

// Call the contention hook manually.
TEST_F(MutexTracerTest, AddN) {
  EXPECT_EQ(tracer_.numContentions(), 0);
  EXPECT_EQ(tracer_.currentWaitCycles(), 0);
  EXPECT_EQ(tracer_.lifetimeWaitCycles(), 0);

  sendWaitCyclesToContentionHook(2);

  EXPECT_EQ(tracer_.numContentions(), 1);
  EXPECT_EQ(tracer_.currentWaitCycles(), 2);
  EXPECT_EQ(tracer_.lifetimeWaitCycles(), 2);

  sendWaitCyclesToContentionHook(3);

  EXPECT_EQ(tracer_.numContentions(), 2);
  EXPECT_EQ(tracer_.currentWaitCycles(), 3);
  EXPECT_EQ(tracer_.lifetimeWaitCycles(), 5);

  sendWaitCyclesToContentionHook(0);

  EXPECT_EQ(tracer_.numContentions(), 3);
  EXPECT_EQ(tracer_.currentWaitCycles(), 0);
  EXPECT_EQ(tracer_.lifetimeWaitCycles(), 5);
}

// Call the contention hook in a real contention scenario.
TEST_F(MutexTracerTest, OneThreadNoContention) {
  // Regular operation doesn't cause contention.
  { Thread::LockGuard lock(mu_); }

  EXPECT_EQ(tracer_.numContentions(), 0);
  EXPECT_EQ(tracer_.currentWaitCycles(), 0);
  EXPECT_EQ(tracer_.lifetimeWaitCycles(), 0);
}

TEST_F(MutexTracerTest, TryLockNoContention) {
  // TryLocks don't cause contention.
  {
    Thread::LockGuard lock(mu_);
    EXPECT_FALSE(mu_.tryLock());
  }

  EXPECT_EQ(tracer_.numContentions(), 0);
  EXPECT_EQ(tracer_.currentWaitCycles(), 0);
  EXPECT_EQ(tracer_.lifetimeWaitCycles(), 0);
}

TEST_F(MutexTracerTest, TwoThreadsWithContention) {
  for (int i = 1; i <= 10; ++i) {
    int64_t curr_num_lifetime_wait_cycles = tracer_.lifetimeWaitCycles();

    Thread::TestUtil::ContentionGenerator contention_generator;

    contention_generator.generateContention(tracer_);
    EXPECT_EQ(tracer_.numContentions(), i);
    EXPECT_GT(tracer_.currentWaitCycles(), 0); // This shouldn't be hardcoded.
    EXPECT_GT(tracer_.lifetimeWaitCycles(), 0);
    EXPECT_GT(tracer_.lifetimeWaitCycles(), curr_num_lifetime_wait_cycles);
  }
}

} // namespace Envoy
