cmake_minimum_required (VERSION 3.0)
project (drtaint_test)

add_library(drtaint_test SHARED
drtaint_test_client.cpp
../../core/drtaint.cpp
../../core/shadow_memory.c
../../core/shadow_registers.c
../../core/shadow_blocks.cpp
../../core/drtaint_helper.cpp
../../core/propagation.cpp
)

configure_DynamoRIO_client(drtaint_test)
use_DynamoRIO_extension(drtaint_test "drreg")
use_DynamoRIO_extension(drtaint_test "drmgr")
use_DynamoRIO_extension(drtaint_test "drutil")
use_DynamoRIO_extension(drtaint_test "drx")
use_DynamoRIO_extension(drtaint_test "umbra")
use_DynamoRIO_extension(drtaint_test "drsyscall")

# Download and build google test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  drtaint_test_app
  drtaint_test_app.cpp
)

target_link_libraries(
  drtaint_test_app
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(drtaint_test_app)