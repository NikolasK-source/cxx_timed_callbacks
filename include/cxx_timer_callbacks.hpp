/*
 * Copyright (C) 2024 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include <csignal>
#include <cxxitimer.hpp>
#include <functional>
#include <memory>
#include <thread>
#include <unordered_set>

/**
 * @brief Timer callback class
 * @details
 *      A TimerCallback object is added to the TimerCallbackHive.
 *      The TimerCallback object stores a variable amount of callback functions
 *      that are called by the TimerCallbackHive with the interval of the
 *      TimerCallback object
 */
class TimerCallback final {
public:
    //* callback function type ( void() )
    using callback_t = void (*)() noexcept;

private:
    std::unordered_set<callback_t> callbacks;    //*< callback functions
    std::size_t                    interval_ms;  //*< callback interval (milliseconds)

    /**
     * @brief create timer callback
     * @details
     *      the static function TimerCallback::create is used to create TimerCallback objects
     * @param interval_ms timer callback interval (milliseconds)
     */
    explicit TimerCallback(std::size_t interval_ms);

public:
    /**
     * @details create TimerCallback object
     * @param interval_ms timer callback interval (milliseconds)
     * @return
     */
    static std::shared_ptr<TimerCallback> create(std::size_t interval_ms);

    /**
     * @brief add a callback function
     * @details
     *      A function can only be added once.
     *      If the function is added multiple times, it still will only be called once.
     * @param callback_function callback function to add
     */
    void add_callback_function(callback_t callback_function) noexcept;

    /**
     * @brief remove a callback function
     * @param callback_function callback function to remove
     */
    void remove_callback_function(callback_t callback_function) noexcept;

    /**
     * @brief get callback interval
     * @return callback interval in seconds
     */
    double get_interval() const noexcept;

private:
    /**
     * @brief call all callback functions
     * @details is called by the TimerCallbackHive
     */
    void invoke();

public:
    friend class TimerCallbackHive;
};

/**
 * @brief Singleton class TimerCallbackHive
 * @details
 *      Stores TimerCallback objects that are called with their specified interval
 *      once the TimerCallbackHive is active.
 *
 *      TimerCallbackHive uses ITIMER_REAL (and therefore SIGALRM) to create the callbacks.
 *      Neither ITIMER_REAL nor SIGALRM should be used in other parts of the application.
 */
class TimerCallbackHive final {
private:
    /**
     * @brief additional callback data
     */
    struct TimerCallbackData {
        explicit TimerCallbackData(std::size_t value = 0) : counter(value), counter_init(value) {}

        /**
         * @brief current counter
         * @details
         *      the counter is decremented with each timer tick.
         *      once 0, the callback is invoked.
         */
        std::size_t counter;
        std::size_t counter_init;  //*< value to which the counter is set after invocation

        /**
         * @brief TimerCallback object
         * @details
         *      storing the shared pointer in the TimerCallbackHive ensures that the object is not deleted while used.
         */
        std::shared_ptr<TimerCallback> callback;
    };

    static std::unique_ptr<TimerCallbackHive> instance;  //*< singleton instance

    std::unordered_map<TimerCallback *, TimerCallbackData> callbacks;     //*< all callback objects
    cxxitimer::ITimer_Real                                 timer;         //*< interval timer
    std::unique_ptr<std::thread>                           timer_thread;  //<* thread that invokes the callbacks

    /**
     * @brief create a TimerCallbackHive
     * @details
     *      Singleton class --> access instance with TimerCallbackHive::get()
     */
    TimerCallbackHive() = default;

public:
    TimerCallbackHive(const TimerCallbackHive &other)            = delete;
    TimerCallbackHive(TimerCallbackHive &&other)                 = delete;
    TimerCallbackHive &operator=(const TimerCallbackHive &other) = delete;
    TimerCallbackHive &operator=(TimerCallbackHive &&other)      = delete;

    /**
     * @brief destroy the TimerCallbackHive
     */
    ~TimerCallbackHive();

    /**
     * @brief get instance
     * @return reference to TimerCallbackHive instance
     */
    static TimerCallbackHive &get();

    /**
     * @brief add a callback
     * @param callback callback to add
     * @throws std::logic_error if callbacks are activated
     */
    void add_callback(const std::shared_ptr<TimerCallback> &callback);

    /**
     * @brief remove a callback
     * @param callback callback to remove
     * @throws std::logic_error if callbacks are activated
     */
    void remove_callback(const std::shared_ptr<TimerCallback> &callback);

    /**
     * @brief activate callbacks
     * @return timer tick interval in seconds
     * @throws std::logic_error if already activated
     */
    double start();

    /**
     * @brief deactivate callbacks
     * @throws std::logic_error if not activated
     */
    void stop();

    /**
     * @brief remove all callbacks
     */
    void clear();

private:
    /**
     * @brief thread that handles the callbacks
     */
    static void thread();
};
