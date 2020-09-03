# Configuration script for the folder.
# Execute cmake one level up to (re)generate the
# Projectfiles/Makefiles


set(GTEST_PATH ${PROJECT_SOURCE_DIR}/contrib)
set(GTEST_SRC ${GTEST_PATH}/gtest/gtest-all.cc)
set(GTEST_MAIN ${GTEST_PATH}/gtest/gtest_main.cc)
include_directories(
	../include
	../../include
	../../include/arch/${PLATFORM}
	./
	${GTEST_PATH}
)

link_directories( 
	${CMAKE_CURRENT_BINARY_DIR}/src
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")

add_executable( test_${PROJECT_NAME}-channels
	${GTEST_SRC}
	${GTEST_MAIN}
	testFifoReader.cpp
	testFifo.cpp
)


target_link_libraries ( test_${PROJECT_NAME}-channels
	${CMAKE_THREAD_LIBS_INIT}
	${PROJECTNAME}
	${PROJECTNAME}-channels
	rt
	${GTEST_LIBRARY_DEBUG} ${GTEST_MAIN_LIBRARY_DEBUG}
)

add_test(tests-${PROJECT_NAME}-channels test_${PROJECT_NAME}-channels --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/xunit.xml)
# Adding custom test target "check" because it is the only way to show the full output of test errors on console
add_custom_target(check-${PROJECT_NAME}-channels COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure)
