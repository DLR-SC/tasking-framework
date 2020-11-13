# Tasking Framework #

Tasking Framework is an event-driven multithreading execution platform and software development library.
  It is dedicated to develop embedded software with static memory requirements as well as non-embedded software. Therefore, the applicability of Tasking Framework covers developing embedded software for illustrative case studies for educational purpose, prototypes, on-board software, etc.

## Copyrights ##
Copyright 2012 German Aerospace Center (DLR) SC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

### External Libraries ###
We use the following libraries with their respective licenses: 

  - contrib/gtest: *BSD 3-Clause*
  - contrib/outpost-core: *Mozilla Public License, v. 2.0*.
  
## Prerequisites ##
The framework was developed and tested on recent 64-Bit Linux systems. The 
pthread library is needed.


## Build ##
To build library in folder build/lib 

    make lib

To generate an folder with includes and library in build/tasking 

    make install
  
To print the options, write

    make help

To clean

    make clean


For the selection of a platform model use option platform=<model>. The model is one of the subfolder names in the source directory, namely: linux, none, outpost, none. Default is linux. 

 When platform=outpost is selected, you need to address the outpost architecture by setting outpostArch to: freertos, none, posix, or rtems. To clone outpost:
 
     git submodule init
     
     git submodule update --recursive
 
 When platform=custom is selected, you need to develop the scheduler interfaces and provide the include path in the CXXFLAGS.
 

### Examples ###
First go to examples/ folder

    cd examples
 
 Then to build the examples except the ones in examples/customPlatform
 
    make all
    
Or you can build each example alone by:

    make 'example_name'
    
To build the examples in customPlatform

    make customPlatform
    

 
### Test ###
The framework has a plain C++ testsuite in the test/ folder. To run the tests:

    make test
    
To see the outputs even if everything wents fine call

    ./build/test_tasking 

## Documentation ##
If doxygen is installed the build target
  
    make doc 

is available to generate HTML output in the build/doc/ folder.

##How to contribute##
The development of Tasking Framework is conducted internally. Releases will be mirrored to the external repository on GitHub. 

However, you can use github to report bugs, suggest bug fixes, suggest features, etc. The development team will consider your report and contact you. 

In case of contribution, your contribution will be tested and evaluated. If it would be accepted to be merged to Tasking Framework, your name will be added to the contributor list (CONTIBUTORS.md), and to the related commit. 
