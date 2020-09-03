# Configuration script for the folder.
# Execute cmake one level up to (re)generate the
# Projectfiles/Makefiles

find_package(Threads)

include_directories(
        ../include
        ../../include
        ../../include/arch/${PLATFORM}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_library(${PROJECT_NAME}-channels STATIC
        fifoGeneric.cpp
)

target_link_libraries ( ${PROJECT_NAME}-channels
        ${PROJECT_NAME}
        ${CMAKE_THREAD_LIBS_INIT}
        rt
)