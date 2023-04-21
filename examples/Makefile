#
# Build tasking framework examples
#
# Copyright 2012-2020 German Aerospace Center (DLR) SC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

BIN_PATH = ../build/examples/bin
BUILD_PATH = ../build/examples
T_INCLUDE_PATH = ../build/tasking/include
T_LIB_PATH = ../build/tasking/lib

-include ../build/tasking/variant.mk

CXXFLAGS += -I$(T_INCLUDE_PATH)
 
# Size reduction by eliminating dead code
CXXFLAGS += -Wl,--gc-sections 

.PHONY : all help channelExample ioChannelExample multiparallelExample periodicTaskExample \
  timeOutExample customPlatform clean tasking

all: channelExample ioChannelExample multiparallelExample periodicTaskExample timeOutExample filterExample

help:
	@echo "Make targets:"
	@echo "  all                 : Compile all examples except custom platform"
	@echo "  channelExample      : Compute Fibonacci numbers"
	@echo "  ioChannelExample    : Specialize channels for IO"
	@echo "  multiparallelExample: Usage of a barrier to synchronize parallel executions"
	@echo "  periodicTaskExample : Usage of an event for periodic task execution"
	@echo "  timeOutExample      : Set up a task with time out behavior"
	@echo "  customPlatform      : Implement an application specific scheduler platform"

channelExample: | tasking $(BIN_PATH)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) channelExample.cpp -L$(T_LIB_PATH) -ltasking -lpthread -o $(BIN_PATH)/channelExample
	
ioChannelExample: | tasking $(BIN_PATH)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) ioChannelExample.cpp -L$(T_LIB_PATH) -ltasking -lpthread -o $(BIN_PATH)/ioChannelExample

multiparallelExample: | tasking $(BIN_PATH)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) multiparallelExample.cpp -L$(T_LIB_PATH) -ltasking -lpthread -o $(BIN_PATH)/multiparallelExample

periodicTaskExample: | tasking $(BIN_PATH)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) periodicTaskExample.cpp -L$(T_LIB_PATH) -ltasking -lpthread -o $(BIN_PATH)/periodicTaskExample

timeOutExample: | tasking $(BIN_PATH)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) timeOutExample.cpp -L$(T_LIB_PATH) -ltasking -lpthread -o $(BIN_PATH)/timeOutExample
	
filterExample: | tasking $(BIN_PATH)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) filterExample.cpp -L$(T_LIB_PATH) -ltasking -lpthread -o $(BIN_PATH)/filterExample

customPlatform:
	@cd customPlatform && $(MAKE) customPlatform MAKEFLAGS=

tasking:
ifdef taskingVariant
	@cd .. && $(MAKE) clean MAKEFLAGS= 
endif
	@cd .. && $(MAKE) install platform=linux MAKEFLAGS= 
	
clean: 
	@rm -r $(BUILD_PATH)

$(BIN_PATH): | $(BUILD_PATH)
	@mkdir $(BIN_PATH)
	
$(BUILD_PATH): | ../build
	@mkdir $(BUILD_PATH)

../build:
	@mkdir ../build
