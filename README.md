# Tasking Framework #

Tasking Framework is an event-driven multithreading execution platform and software development library.
 It is dedicated to develop space applications, which perform on-board data processing and sophisticated control algorithms.

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
To build 

    make install
  
To print the options, write

    make help

To clean

    make clean


For the selection of a platform model use option platform=<model>. The model
is one of the subfolder names in the source directory. Default is linux.

### Examples ###
To build the provided examples:

    cd examples
 
    make all
 
## Documentation ##
If doxygen is installed the build target
  
    make doc 

is available to generate HTML output in the build/doc/ folder.
