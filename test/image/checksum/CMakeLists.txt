#
# Copyright (c) 2019 Mateusz Loskot <mateusz at loskot dot net>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
# See the accompanying README.md
#
add_executable(test_core_image_checksum_generate
  generate.cpp
  sample_image_data.cpp)
target_link_libraries(test_core_image_checksum_generate
  PRIVATE
    gil_compile_options
    gil_include_directories
    gil_dependencies)

add_executable(test_core_image_checksum_calculate
  calculate.cpp
  sample_image_data.cpp)
target_link_libraries(test_core_image_checksum_calculate
  PRIVATE
    gil_compile_options
    gil_include_directories
    gil_dependencies)
add_test(test.core.image.checksum.calculate
  test_core.image_checksum_calculate
  ${CMAKE_CURRENT_SOURCE_DIR}/reference_checksums.txt)
