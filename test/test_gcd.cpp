/*
 * Copyright (C) 2024 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "../src/gcd.hpp"
#include <cassert>

int main() {              // NOLINT
    auto x = gcd(10, 5);  // NOLINT
    assert(x == 5);

    auto x2 = gcd<std::size_t>({10, 20, 30, 40, 35});  // NOLINT
    assert(x2 == 5);

    auto x3 = gcd<std::size_t>({10, 20, 30, 40, 37});  // NOLINT
    assert(x3 == 1);
}
