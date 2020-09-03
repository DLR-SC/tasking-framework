# Configuration script for the tasking bare metal model linux folder
# Execute cmake one level up to (re)generate the
# Projectfiles/Makefiles

message ("Generate platform None")

include_directories(
    ../../include
    .
)

add_library(platformObjects OBJECT
  clockExecutionModel.cpp
  schedulerExecutionModel.cpp
  mutex.cpp
  signaler.cpp
)
