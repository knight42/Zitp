CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(Zitp)
ADD_EXECUTABLE(Zitp src/main.cpp src/zitp.cpp src/Term.cpp src/value.cpp)
SET_TARGET_PROPERTIES(Zitp PROPERTIES OUTPUT_NAME "zitp")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-switch -std=c++1y")

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DDEBUG_MODE=1)
ENDIF()

# Test
ENABLE_TESTING()
function(addTest)
    foreach(t ${ARGN})
        ADD_TEST(test_${t} ${CMAKE_SOURCE_DIR}/run_test.sh ${t})
    endforeach()
endfunction()

addTest(io
    app_func1 app_func2 app_func3
    ret_func currying high_order iter_fact
    short_circuit
    while_loop)
