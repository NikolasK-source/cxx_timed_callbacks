#
# Copyright (C) 2024 Nikolas Koesling <nikolas@koesling.info>.
# This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

enable_testing()

add_executable(test_${Target} timer_callback_test.cpp)
add_test(NAME test_${Target} COMMAND test_${Target})
target_link_libraries(test_${Target} ${Target} cxxitimer)

add_executable(test_${Target}_gcd test_gcd.cpp ../src/gcd.hpp)
add_test(NAME test_${Target}_gcd COMMAND test_${Target}_gcd)
