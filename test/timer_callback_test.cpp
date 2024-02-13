/*
 * Copyright (C) 2024 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxx_timer_callbacks.hpp"

#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

static std::array<std::size_t, 8> counter;  // NOLINT

int main() {
    counter.fill(0);

    auto &callback_hive = TimerCallbackHive::get();
    auto  callback1     = TimerCallback::create(100);   // NOLINT
    auto  callback2     = TimerCallback::create(500);   // NOLINT
    auto  callback3     = TimerCallback::create(1000);  // NOLINT
    auto  callback4     = TimerCallback::create(250);   // NOLINT
    callback_hive.add_callback(callback1);
    callback_hive.add_callback(callback2);
    callback_hive.add_callback(callback3);
    callback_hive.add_callback(callback4);

    callback1->add_callback_function([]() noexcept { counter[0]++; });  // NOLINT
    callback1->add_callback_function([]() noexcept { counter[4]++; });  // NOLINT

    callback2->add_callback_function([]() noexcept { counter[1]++; });  // NOLINT
    callback2->add_callback_function([]() noexcept { counter[5]++; });  // NOLINT

    callback3->add_callback_function([]() noexcept { counter[2]++; });  // NOLINT
    callback3->add_callback_function([]() noexcept { counter[6]++; });  // NOLINT

    callback4->add_callback_function([]() noexcept { counter[3]++; });  // NOLINT
    callback4->add_callback_function([]() noexcept { counter[7]++; });  // NOLINT

    const auto tick_time_s = callback_hive.start();
    std::cerr << "tick_time_s: " << tick_time_s << std::endl;  // NOLINT
    assert(std::abs(tick_time_s - 0.05) < 0.0001);             // NOLINT

    std::this_thread::sleep_for(std::chrono::seconds(5));  // NOLINT

    for (std::size_t i = 0; i < counter.size(); ++i) {
        std::cerr << "counter " << i << ": " << counter[i] << std::endl;  // NOLINT
    }

    assert(counter[0] == 50);
    assert(counter[1] == 10);
    assert(counter[2] == 5);
    assert(counter[3] == 20);
    assert(counter[4] == 50);
    assert(counter[5] == 10);
    assert(counter[6] == 5);
    assert(counter[7] == 20);

    try {
        callback_hive.clear();
        assert(false);
    } catch (std::logic_error &e) { std::cerr << e.what() << std::endl; }  // NOLINT

    callback_hive.stop();
    callback_hive.remove_callback(callback4);

    callback_hive.add_callback(callback1);
    callback_hive.add_callback(callback2);
    callback_hive.add_callback(callback3);

    const auto tick_time_s2 = callback_hive.start();
    std::cerr << "tick_time_s: " << tick_time_s2 << std::endl;  // NOLINT
    assert(std::abs(tick_time_s2 - 0.1) < 0.0001);              // NOLINT
}
