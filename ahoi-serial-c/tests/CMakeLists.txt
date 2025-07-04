
add_executable(example_test test_example.c)

target_link_libraries(example_test cmocka)

add_test(NAME example_test COMMAND example_test)
