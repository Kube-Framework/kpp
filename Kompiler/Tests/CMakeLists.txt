find_package(GTest REQUIRED)

add_executable(KompilerTests
    tests_Lexer.cpp
)

target_link_libraries(KompilerTests
    KompilerLib
    GTest::GTest
    GTest::Main
)

add_test(NAME KompilerTests COMMAND KompilerTests)
