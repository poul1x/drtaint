cmake_minimum_required (VERSION 3.14)
project (drtaint)

if (NOT DEFINED DynamoRIO_DIR)
  message(FATAL_ERROR "Need to specify directory DynamoRIO_DIR containing DynamoRIOConfig.cmake")
endif ()

if (NOT DEFINED DrMemoryFramework_DIR)
  message(FATAL_ERROR "Need to specify directory DrMemoryFramework_DIR containing DRMFTarget32.cmake")
endif ()

find_package(DynamoRIO REQUIRED)
find_package(DrMemoryFramework REQUIRED)

set(CMAKE_CXX_STANDARD 11)

include_directories("core/api")

add_subdirectory(app/drtaint_test drtaint_test)
add_subdirectory(app/drtaint_only drtaint_only)

