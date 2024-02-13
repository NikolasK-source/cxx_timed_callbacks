/*
 * Copyright (C) 2024 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include <stdexcept>
#include <vector>

/**
 * @brief get the greatest common divisor
 * @details recursive function!
 * @tparam T data type
 * @param a first value
 * @param b second value
 * @return greatest common divisor of a and b
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
T gcd(T a, T b) {
    if (a == 0) return b;
    return gcd(b % a, a);
}

/**
 * @brief get the greatest common divisor
 * @details recursive function!
 * @tparam T data type
 * @param values list of values
 * @return reatest common divisor of <values>
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
T gcd(std::vector<T> values) {
    if (values.empty()) throw std::invalid_argument("empty vector");

    T result = values.front();
    for (std::size_t i = 1; i < values.size(); ++i) {
        result = gcd(values[i], result);
        if (result == 1) break;
    }
    return result;
}
