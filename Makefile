#
# Build tasking framework
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

.PHONY : help doc lib clean depend test install

# Get platform specific sources for the scheduler
ifeq (none, $(platform))
schedulerFolder = $(wildcard arch/none)
CXXFLAGS += -Iarch/none
else
ifeq (outpost, $(platform))
schedulerFolder = $(wildcard arch/outpost)
CXXFLAGS += -Iarch/outpost 
CXXFLAGS += -Icontrib/outpost-core/modules/base/src
CXXFLAGS += -Icontrib/outpost-core/modules/rtos/arch/$(outpostArch)
CXXFLAGS += -Icontrib/outpost-core/modules/rtos/src
CXXFLAGS += -Icontrib/outpost-core/modules/time/src
else
ifeq (custom, $(platform))
else
platform = linux
schedulerFolder = $(wildcard arch/linux)
CXXFLAGS += -Iarch/linux
endif
endif
endif

# Find out object files of scheduler and convert to objects in build folder
schedulerSources= $(wildcard $(schedulerFolder)/*.cpp)
schedulerDependencies = $(patsubst $(schedulerFolder)/%,build/%,$(schedulerSources:.cpp=.d))
schedulerObjects = $(patsubst $(schedulerFolder)/%,build/%,$(schedulerSources:.cpp=.o))

# Find out object files of API and convert to objects in build folder
srcSources = $(wildcard src/*.cpp)
srcDependencies = $(patsubst src/%,build/%,$(srcSources:.cpp=.d))
srcObjects = $(patsubst src/%,build/%,$(srcSources:.cpp=.o))

testSource = $(wildcard test/*cpp)
testObjects = $(patsubst %,build/%,$(testSource:.cpp=.o))

# Setup includes and other flags for the build
CXXFLAGS += -Iinclude

help :
	@echo "Targets for make:"
	@echo "  doc     : Generates documentation of API"
	@echo "  depend  : Generate dependancies for source files without platform option as"
	@echo "            Linux scheduler"
	@echo "  lib     : Generates library without platform option as Linux scheduler"
	@echo "            Call 'make clean' if you generate for a different platform"
	@echo "  install : Generate the tasking directory with lib and include folder."
	@echo "            Platform option is necessary if it is different from Linux."
	@echo "  clean   : Remove the build folder"
	@echo "  examples: Compile all examples"
	@echo
	@echo "Optional arguments"
	@echo "  platform = linux   : Generate scheduler for Posix thread functionalities"
	@echo "  platform = none    : Generate scheduler without any functionality, e.g. for"
	@echo "               unit tests"
	@echo "  platform = outpost : Generate scheduler for outpost thread functionalities"
	@echo "               Generating with outpost need to address the outpost architecture"
	@echo "               by setting outpostArch to one of freertos, none, posix, or rtems."
	@echo "  platform = custom  : Generate without scheduler. The application software has"
	@echo "                       to provide the scheduler interfaces and provide the"
	@echo "                       include path in the CXXFLAGS."

# Generate lib file for the Tasking Framework
lib: $(schedulerObjects) $(srcObjects) | build/lib
	@echo "<<<< Create lib for scheduler type $(platform) >>>>"
	ar rcs build/lib/libtasking.a $(schedulerObjects) $(srcObjects)

# Generate an install folder to roll out
install: lib | build/tasking
	@cp build/lib/libtasking.a build/tasking/lib/libtasking.a
	@cp include/*.h build/tasking/include
	@cp -r include/impl build/tasking/include/impl
ifneq ("$(platform)", "custom")
	@cp $(schedulerFolder)/*.h build/tasking/include
endif
	@cp LICENSE build/tasking
	@echo "taskingVariant = $(platform)" > build/tasking/variant.mk
	@echo "Tasking framework for $(platform) provided in folder build/tasking."
	
# Update dependencies
depend: $(srcDependencies) $(schedulerDependencies)

build/%.d: src/%.cpp | build
	@$(CXX) -MM $(CXXFLAGS) $< > $@
	@sed -i '1s/.*/build\/&/; $$s/.*/& | build/' $@
	@echo "\t$(CXX) -c $(CXXFLAGS) $< -o $@" >> $@
	@sed -i '$$s/\.d/.o/' $@
build/%.d: $(schedulerFolder)/%.cpp | build
	@$(CXX) -MM $(CXXFLAGS) $< > $@
	@sed -i '1s/.*/build\/&/; $$s/.*/& | build/' $@
	@echo "\t$(CXX) -c $(CXXFLAGS) $< -o $@" >> $@
	@sed -i '$$s/\.d/.o/' $@
	
# Generate unit tests (gtest required)
#test: build/testTasking
#build/testTasking: lib $(testObjects) | build/test
#	@echo "<<<< Generate unit tests >>>>"
#	$(CXX) $(CFLAGS) $(CXXFLAGS) -Icontrib contrib/gtest/gtest-all.cc $(wildcard build/test/*.o) -Lbuild/lib -ltasking -lpthread -o build/tasking_test
#	
#build/test/%.o: test/%.cpp | build/test
#	$(CXX) -c $(CFLAGS) $(CXXFLAGS) -Icontrib  $< -o $@

# Generate documentation 
doc : | build
	@doxygen DoxyfileMake.in

# Cleaning compile results by removing build folder
clean :
	@rm -r build

# Generate build folders if not existing
build/test: | build
	@mkdir build/test

build/lib: | build
	@mkdir build/lib

build/tasking: | build
	mkdir build/tasking
	mkdir build/tasking/lib
	mkdir build/tasking/include

build:
	@mkdir build
	
-include $(srcDependencies) $(schedulerDependencies)
	
