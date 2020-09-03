#
# Build tasking framework application with custom platform
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

BIN_PATH = ../../build/examples/bin
BUILD_PATH = ../../build/examples
T_INCLUDE_PATH = ../../build/tasking/include
T_LIB_PATH = ../../build/tasking/lib

-include ../../build/tasking/variant.mk

CXXFLAGS += -I$(T_INCLUDE_PATH)

.PHONY : customPlatform

customPlatform: | tasking $(BIN_PATH)
	$(CXX) $(CFLAGS) $(CXXFLAGS) -I. $(wildcard *.cpp) -L$(T_LIB_PATH) -ltasking -lpthread -o $(BIN_PATH)/customPlatform
	
tasking:
ifneq ("$(taskingVariant)", "custom")
	@echo "Example needs build of the Tasking Framework with custom scheduler"
	@echo "Do the following steps:"
	@echo "  cd <tasking main folder"
	@echo "  export CXXFLAGS=-Iexamples/customPlatform"
	@echo "  make install platform=custom"
	@echo "  export CXXFLAGS="
	@exit -1
endif
	
$(BIN_PATH): | $(BUILD_PATH)
	mkdir $(BIN_PATH)
	
$(BUILD_PATH): | ../../build
	mkdir $(BUILD_PATH)

../../build:
	mkdir ../../build
