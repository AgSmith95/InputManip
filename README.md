# InputManip

You need Windows SDK.
* I used Win SDK v10.0.18362.1

You also need MS Visual Studio toolchain, which you can get with MS VS Community edition.
* I've just installed some MSVS Community 2019, which was latest at that point.
* Mind that "that point" was in early 2020.
* It is, probably, possible to use MS Build Tools instead of installing 10+Gb of MSVS, but I don't know how to do it.

You need CMake.
1. Go get it from https://cmake.org/download/
2. Then open the project folder in cmd and execute:
    * mkdir build
    * cd build
    * cmake ../
3. Part 2. should generate a project ready to be loaded in MSVS.
    * There is a way to build MSVS projects from console. Google it.
4. Instead of using console in Part 2. you can use CMake GUI tool.
