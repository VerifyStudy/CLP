                  Overview:
CLP is a C/C++ Preprocessor implemented in C++. 
Copyright (C) 2020-2021 .Its main function is to 
organize the C language syntax in advance and make 
it more concise.It faithfully records all aspects 
of preprocessing that make debugging preprocessing 
far easier.

              License and Copyright:
CLP is licensed under the ubuntu with copyright by 
hui,yuan,sai and liang.

               Getting Started with CLP:
CLP is based on Ubuntu and is developed with Visual 
Studio Code based on LLVM, so it must be based on 
Ubuntu when it is used. 

               Features:
1:Conformant C/C++ preprocessor.
2:Gives programatic access to every preprocessing 
token and the state of the preprocessor at any point 
during preprocessing.
3:Requires ubuntu(linux) llvm and clang.
4:It can generate preprocessor visualisations from 
the command line.

             LLVM setup steps:
The README briefly describes how to get started 
with building llvm.
Taken from https://github.com/llvm/llvm-project.
The LLVM project has multiple components. The core 
of the project is clang. 
             First:
Download the source code,by command :git clone 
https://github.com/llvm/llvm-project.git , git 
clone https://gitee.com/ythslzy/llvm-project.git
             Second:
Download the GCC compiler,by command:sudo 
update-alternatives --config gcc
             Third:
Download cmake,by command:git clone 
https://gitee.com/my_mirrors/CMake.git,
next input:./bootstrap(If something goes wrong, 
try switching to the lower version of GCC),then 
input:make,finally input:sudo make install.The 
installation is successful.
             Fourth:
Configure and build LLVM and Clang:
by command：cd llvm-project and cmake -S llvm -B 
build -G <generator>[options]
             Finally:
Select the specified build system. Default is NINJA 
or MAKE,by command:sudo cmake --build.--target install 
-- -j3,or Or enter instructions individually, step by 
step:make,sudo make install,llvm-config --targets -built

             Introduction to all documents:
main.cpp:The file is mainly responsible for the C 
language preprocessing of the source code, the subsequent 
addition or modification of functions are based on the file 
for implementation. 

test folder:This folder contains all the files are C language 
code, mainly to test whether each function can preprocess the 
test code.

.vscode folder:Linux VSCODE C/C ++ three file configuration.
 --c_cpp_properties.json:C/C++ Edit configuration;Reference 
 library file configuration
 --launch.json:Learn about the properties using IntelliSense;
 Task allocation;
 --settings.json:Specify file type
 --tasks.json:Task,configure default build task;Compiler configuration

 --compile_commands.json:Vscode uses compile_commands. Json to 
 configure the includePath environment.  

                   Run a test file:
You can run the test files using commands by:
cd test/
./../src/main loop.cpp
from the Ubuntu terminal interface.You can then see the result 
of the processing in output.cpp(and outputs.cpp with line number).  Of course, you can also run the test file in vscode.  

                 Contact Me:
For any questions, please ping me via my github account. 
Changes and additions are always welcome.