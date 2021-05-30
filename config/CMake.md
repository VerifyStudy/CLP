
         Overview
This folder is mainly used to store 
the configuration files for setting 
up the LLVM framework. How to set up 
the LLVM and the clang has been described in the 
general README, please refer to that 
for detailed steps.  

Building LLVM with CMake
CMake is a cross-platform build-generator tool. 
CMake does not build the project, it generates 
the files needed by your build tool for building LLVM.

Usuage
1:Download and install CMake.
2:Open a shell.
3:Create a buid directory.
   $ mkdir mybuilder
   $ cd mybuilder
4:After CMake has finished running, proceed to 
use IDE project files, or start the build from 
the build directory
   $cmake --build.
5:After LLVM has finished building, install it 
from the build directory
   $cmake --build . --target install
   or
   $ cmake -DCMAKE_INSTALL_PREFIX=/tmp/llvm -P cmake_install.cmake


Executing the Tests
Testing is performed when the check-all target is built. For instance, if you are using Makefiles, execute this command in the root of your build directory.On Visual Studio, you may run tests by building the project "check-all"
   $make check-all