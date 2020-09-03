# Configuration script for the tasking source folder
# Execute cmake one level up to (re)generate the
# Projectfiles/Makefiles

include_directories(
    ../include
    ../arch/${PLATFORM}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_library(${PROJECT_NAME} STATIC
    clock.cpp
    schedulePolicyFifo.cpp
    schedulePolicyLifo.cpp
    schedulePolicyPriority.cpp
    scheduler.cpp
    schedulerUnitTest.cpp
    task.cpp
    taskBarrier.cpp
    taskChannel.cpp
    taskEvent.cpp
    taskGroup.cpp
    taskInput.cpp
    taskInputArray.cpp
    taskTypes.cpp
    taskPeriodicSchedule.cpp
    $<TARGET_OBJECTS:platformObjects>
)

if (PLATFORM EQUAL "linux")
	target_link_libraries ( ${PROJECT_NAME}
    	${CMAKE_THREAD_LIBS_INIT}
    	rt
	)
else ()
	target_link_libraries ( ${PROJECT_NAME} )
endif()
