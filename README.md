# LoadOrderTest

This is a simple SKSE plugin to help troubleshoot issues with plugins not loading in Skyrim SE.  
It is based on [this xmake template](https://github.com/libxse/commonlibsse-ng-template) by [qudix](https://github.com/qudix).

## Requirements
* [XMake](https://xmake.io) [2.8.2+]
* C++23 Compiler (MSVC, Clang-CL)

## How to build
Clone the project.
```bat
git clone --recurse-submodules https://github.com/AtomCrafty/LoadOrderTest.git
cd LoadOrderTest
```

To build the project, run the following command.  
The output files will be located in `build/windows/x64/releasedbg/`.
```bat
xmake build
```

If you want to generate a Visual Studio project, run the following command.  
The solution file will be located in `vsxmakeXXXX/`.
```bat
xmake project -k vsxmake
```
