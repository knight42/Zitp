CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(Zitp)
ADD_EXECUTABLE(Zitp src/Term.cpp)
SET_TARGET_PROPERTIES(Zitp PROPERTIES OUTPUT_NAME "zitp")
ENABLE_TESTING()

function(addTest)
    foreach(t ${ARGN})
        ADD_TEST(test_${t} ${CMAKE_SOURCE_DIR}/run_test.sh ${t})
    endforeach()
endfunction()

addTest(io app_func1 app_func2 app_func3 currying iter_fact)
