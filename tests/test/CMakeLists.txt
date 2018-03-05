include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(rbtree_test_start
        individual.h
        def_dumper.h
        rbtree_prv1_test.cpp
        rbtree_pub1_test.cpp
    ${CMAKE_SOURCE_DIR}/src/rbtree.h
    ${CMAKE_SOURCE_DIR}/src/rbtree.hpp
)

target_link_libraries(rbtree_test_start gtest gtest_main)
