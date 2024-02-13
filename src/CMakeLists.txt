#
# Copyright (C) 2024 Nikolas Koesling <nikolas@koesling.info>.
# This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

# ---------------------------------------- source files (*.cpp, *.cc, ...) ---------------------------------------------
# ======================================================================================================================

target_sources(${Target} PRIVATE cxx_timer_callbacks.cpp)

# ---------------------------------------- header files (*.hpp, *.h, ...) ----------------------------------------------
# -------------------- place only header files in the src folder that are required only internally. --------------------
# ======================================================================================================================

target_sources(${Target} PRIVATE gcd.hpp)

# ---------------------------------------- subdirectories --------------------------------------------------------------
# ======================================================================================================================
