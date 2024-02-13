/*
 * Copyright (C) 2024 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxx_timer_callbacks.hpp"
#include "gcd.hpp"

#include <iostream>
#include <pthread.h>
#include <stdexcept>

std::unique_ptr<TimerCallbackHive> TimerCallbackHive::instance;

TimerCallback::TimerCallback(std::size_t interval_ms) : interval_ms(interval_ms) {
    if (interval_ms == 0) throw std::invalid_argument("ionterval of 0 ms is not possible");
}

std::shared_ptr<TimerCallback> TimerCallback::create(std::size_t interval_ms) {
    return std::shared_ptr<TimerCallback>(new TimerCallback(interval_ms));
}

void TimerCallback::add_callback_function(callback_t callback_function) noexcept {
    callbacks.insert(callback_function);
}

void TimerCallback::remove_callback_function(callback_t callback_function) noexcept {
    callbacks.erase(callback_function);
}

double TimerCallback::get_interval() const noexcept {
    return static_cast<double>(interval_ms);
}

void TimerCallback::invoke() {
    for (auto &function : callbacks) {
        function();
    }
}

TimerCallbackHive &TimerCallbackHive::get() {
    if (!instance) instance = std::unique_ptr<TimerCallbackHive>(new TimerCallbackHive());
    return *instance;
}

void TimerCallbackHive::add_callback(const std::shared_ptr<TimerCallback> &callback) {
    if (timer.is_running()) throw std::logic_error("cannot add callbacks to hive once active");

    // already added --> do nothing
    if (callbacks.find(callback.get()) != callbacks.end()) return;

    callbacks[callback.get()]          = TimerCallbackData();
    callbacks[callback.get()].callback = callback;
}

void TimerCallbackHive::remove_callback(const std::shared_ptr<TimerCallback> &callback) {
    if (timer.is_running()) throw std::logic_error("cannot remove callbacks from hive once active");
    callbacks.erase(callback.get());
}

double TimerCallbackHive::start() {
    if (timer.is_running()) throw std::logic_error("already active");
    if (callbacks.empty()) throw std::logic_error("no callbacks in hive");

    // determine timer tick
    std::vector<std::size_t> callback_intervals;
    callback_intervals.reserve(callbacks.size());
    for (auto &callback_pair : callbacks) {
        auto &callback = *callback_pair.first;
        callback_intervals.push_back(callback.interval_ms);
    }

    const auto timer_tick_ms = gcd(callback_intervals);
    const auto timer_tick_s  = static_cast<double>(timer_tick_ms) * 0.001;

    // calculate timer counter
    for (auto &callback_pair : callbacks) {
        auto &callback       = *callback_pair.first;
        callbacks[&callback] = TimerCallbackData(callback.interval_ms / timer_tick_ms);
    }

    // block SIGALRM
    sigset_t set;
    sigemptyset(&set);

    if (sigaddset(&set, SIGALRM) == -1) {
        perror("sigaddset");
        exit(EXIT_FAILURE);
    }

    if (pthread_sigmask(SIG_BLOCK, &set, nullptr) != 0) {
        perror("pthread_sigmask");
        exit(EXIT_FAILURE);
    }

    timer_thread = std::make_unique<std::thread>(TimerCallbackHive::thread);

    timer.set_interval(timer_tick_s);
    timer.start();

    return timer_tick_s;
}

void TimerCallbackHive::stop() {
    if (!timer.is_running()) throw std::logic_error("not active");
    timer.stop();
    if (pthread_cancel(timer_thread->native_handle()) != 0) {
        throw std::system_error(errno, std::generic_category(), "pthread_cancel failed");
    }
    timer_thread->join();
}

void TimerCallbackHive::thread() {
    auto &instance = TimerCallbackHive::get();

    // setup sigwait
    sigset_t set;
    sigemptyset(&set);

    if (sigaddset(&set, SIGALRM) != 0) {
        perror("sigaddset");
        exit(EXIT_FAILURE);
    }

    if (pthread_sigmask(SIG_BLOCK, &set, nullptr) != 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    while (true) {
        int  sig = 0;
        auto tmp = sigwait(&set, &sig);
        if (tmp != 0) {
            perror("sigwait");
            exit(EXIT_FAILURE);
        }

        for (auto &callback_pair : instance.callbacks) {
            auto &callback      = *callback_pair.first;
            auto &callback_data = callback_pair.second;

            if (--callback_data.counter == 0) {
                callback_data.counter = callback_data.counter_init;
                callback.invoke();
            }
        }
    }
}

TimerCallbackHive::~TimerCallbackHive() {  // NOLINT: An exception may be thrown
                                           // only possible if pthread_cancel fails.
                                           // This would indicate a massive internal error.
    if (timer.is_running()) stop();
}

void TimerCallbackHive::clear() {
    if (timer.is_running()) throw std::logic_error("cannot clear while active");
    callbacks.clear();
}
