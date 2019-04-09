Phasar a LLVM-based Static Analysis Framework - Swift Version
=============================================

Currently supported version of LLVM
-----------------------------------
Phasar is currently set up to support LLVM-5.0.0/ LLVM-5.0.1.

What is Phasar?
---------------
Phasar is a LLVM-based static analysis framework written in C++. It allows users
to specify arbitrary data-flow problems which are then solved in a 
fully-automated manner on the specified LLVM IR target code. Computing points-to
information, call-graph(s), etc. is done by the framework, thus you can focus on
what matters.

Why does this fork exist?
---------------------------------
This fork is for implementing Swift aware analysis on top of Phasar and developing a toolchain for analyzing Swift applications. 

Installation
------------

Notes: Some of these instructions are borrowed from the original Readme and some from phasar.org. This fork has modified `CMakeLists.txt` in order to be MacOS compatible. You can feel free to try changing the `CMAKE_CXX_STANDARD` version from `14` to `17` in `CMakeLists.txt`.

#### Note: Reference macOS version is 10.14.4

### Dependencies
* LLVM/Clang 5.0.0 or 5.0.1 (you can use the custom installation script under `/utils` but this does *not* work on Mac)
* SQLite 3.11.0 or newer (libsqlite3-dev)
* MySqlConnector (libmysqlcppconn-dev)
* LibCurl (libcurl4-openssl-dev)
* Zlib (zlib1g-dev)
* Boost 1.63.0 or newer (for common Linux distributions no stable package is available, it has to be self compiled, for Homebrew (MacOS) version 1.66.0 is available and it works)
* make
* CMake
* Python 3 (helpful, but not necessary)

To install most of these dependencies on a Debian or Ubuntu Linux you can use these command:  
`apt-get install sqlite3 libsqlite3-dev bear python3 git make cmake zlib1g-dev libncurses5-dev graphviz doxygen libcurl4-gnutls-dev libmysqlcppconn-dev`

You may need to install `python-dev`, `subversion`, and maybe some others. If you get any errors, it should be clear which dependencies you are missing.

To install these dependencies on a Mac you can use Homebrew’s bundle functionality to install all dependencies except for LLVM (execute this directly in the cloned repository):  
`brew bundle`

#### Installing LLVM/Clang 5.0.0/5.0.1 on Mac
Using `brew install llvm@5` does not work because that would install `5.0.2`, which is currently unsupported. You must find a way to install `5.0.0` or `5.0.1` on Mac. This is a problem that is actively being solved.

### Compile Phasar
First, if you would like to enable Swift analysis, you must set the environment variable for PHASAR to your `phasar/` directory. The Swift JSON file for defining sources and sinks can be found in `config/`.

Set the system's variables for the C and C++ compiler to clang. On Mac this may be optional.

Possible Linux example:
```
$ export CC=/usr/local/bin/clang
$ export CXX=/usr/local/bin/clang++
```
Possible MacOS example:
```
$ export CC=/usr/local/opt/llvm\@5/bin/clang
$ export CXX=/usr/localopt/llvm\@5/bin/clang++
```

Run `git submodule update --init --recursive` in `phasar/` (your cloned repo directory).

Navigate into the Phasar directory. The following commands will do the job and compile the Phasar framework:

```
$ mkdir build
$ cd build/
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j $(nproc) # or use a different number of cores to compile it
```

After compilation using cmake the following two binaries can be found in the build/ directory:

+ phasar - the actual Phasar command-line tool
+ myphasartool - an example tool that shows how tools can be build on top of Phasar

Use the command:

`$ ./phasar --help`

in order to display the manual and help message.

To be able to run `phasar` anywhere, run the following in `build/`

`$ sudo make install`

Please be careful and check if errors occur during the compilation.

#### IDEs
I recommend using CLion since it is really easy to get a CMake project going with it. XCode is another alternative but is more tricky to setup CMake with.

### Usage

When using CMake to compile Phasar the following optional parameters can be used:

| Parameter : Type|  Effect |
|-----------|--------|
| <b>BUILD_SHARED_LIBS</b> : BOOL | Build shared libraries (default is OFF) |
| <b>CMAKE_BUILD_TYPE</b> : STRING | Build Phasar in 'Debug' or 'Release' mode <br> (default is 'Debug') |
| <b>CMAKE_INSTALL_PREFIX</b> : PATH | Path where Phasar will be installed if <br> “make install” is invoked or the “install” <br> target is built (default is /usr/local) |
| <b>PHASAR_BUILD_DOC</b> : BOOL | Build Phasar documentation (default is OFF) |
| <b>PHASAR_BUILD_UNITTESTS</b> : BOOL | Build Phasar unittests (default is OFF) |
| <b>PHASAR_ENABLE_PAMM</b> : STRING | Enable the performance measurement mechanism <br> ('Off', 'Core' or 'Full', default is Off) |
| <b>PHASAR_ENABLE_PIC</b> : BOOL | Build Position-Independed Code (default is ON) |
| <b>PHASAR_ENABLE_WARNINGS</b> : BOOL | Enable compiler warnings (default is ON) |


#### A remark on compile time
C++'s long compile times are always a pain. As shown in the above, when using cmake the compilation can easily be run in parallel, resulting in shorter compilation times. Make use of it!

## Please refer to the original repo's Readme for more info on this tool
