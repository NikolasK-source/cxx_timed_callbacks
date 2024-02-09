#
# Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
# This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

add_executable(test_${Target} template_test.cpp)

enable_testing()
add_test(NAME test_${Target}  COMMAND test_${Target})

target_link_libraries(test_${Target} ${Target})
