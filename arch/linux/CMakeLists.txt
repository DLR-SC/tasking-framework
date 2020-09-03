# Configuration script for the tasking bare metal model linux folder
# Execute cmake one level up to (re)generate the
# Projectfiles/Makefiles

message ("Generate platform Linux")

find_package(Threads REQUIRED)

include_directories(
    ../../include
    .
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fPIC")

add_library(platformObjects OBJECT
  clockExecutionModel.cpp
  mutex.cpp
  schedulerExecutionModel.cpp
  signaler.cpp
)
