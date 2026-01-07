# Basic test support
enable_testing()

# Add a simple test executable
add_executable(test_basic test_basic.cpp)
target_link_libraries(test_basic PRIVATE delaunay_interfaces)

add_test(NAME BasicTest COMMAND test_basic)

# You can add more sophisticated testing with Google Test or Catch2
# For now, this is a minimal test setup
