#ifndef _RATE_QUEUE_TEST_
#define _RATE_QUEUE_TEST_

#include <pingfs/util/rate_queue.hpp>

#include <chrono>
#include <memory>

#include "gtest/gtest.h"


/**
 * Ensure that RateQueue rate-limits servicing
 * its elements.
 */
TEST(RateQueue, TimedRun) {
    std::chrono::milliseconds delay(100);
    std::shared_ptr<pingfs::RateQueue> rate_queue =
        pingfs::RateQueue::start_queue(delay);

    auto start_time = std::chrono::system_clock::now();
    std::vector<std::chrono::milliseconds> duration_vec;

    std::function<void()> func_to_run = [&start_time, &duration_vec]() {
        duration_vec.push_back(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - start_time));
        start_time = std::chrono::system_clock::now();
    };

    std::size_t num_adds = 10;
    for (std::size_t i = 0; i < num_adds; ++i) {
        rate_queue->add(func_to_run);
    }
    std::chrono::milliseconds wait(delay * (num_adds + 5));
    std::this_thread::sleep_for(wait);
    rate_queue->stop();

    // Test that we have expected number of added durations
    // and that each duration is approximately delay
    // milliseconds
    ASSERT_EQ(duration_vec.size(), num_adds);

    // Each duration should be equal to delay +/- count_tolerance
    // ticks.
    int count_tolerance =
        static_cast<int>(0.20 * static_cast<double>(delay.count()));
    // start at 1 because the first element may have been
    // added in the middle of a rate limit.
    for (std::size_t i = 1; i < duration_vec.size(); ++i) {
        std::chrono::milliseconds dur = duration_vec[i];
        ASSERT_TRUE(dur.count() >= (delay.count() - count_tolerance));
        ASSERT_TRUE(dur.count() <= (delay.count() + count_tolerance));
    }
}


#endif
