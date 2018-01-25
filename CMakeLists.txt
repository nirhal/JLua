# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)

find_package(JNI)

if (JNI_FOUND)
    message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
else()
    # Set it manually if JNI not found

    # Example for windows:
    set(JNI_INCLUDE_DIRS "C:/Program Files/Java/jdk1.8.0_161/include" "C:/Program Files/Java/jdk1.8.0_161/include/win32")
endif()

add_library( # Specifies the name of the library.
             lua

             # Sets the library as a static library.
             STATIC

             # Provides a relative path to your source file(s).
             src/c/lua/lapi.c
             src/c/lua/lauxlib.c
             src/c/lua/lbaselib.c
             src/c/lua/lcode.c
             src/c/lua/ldblib.c
             src/c/lua/ldebug.c
             src/c/lua/ldo.c
             src/c/lua/ldump.c
             src/c/lua/lfunc.c
             src/c/lua/lgc.c
             src/c/lua/linit.c
             src/c/lua/liolib.c
             src/c/lua/llex.c
             src/c/lua/lmathlib.c
             src/c/lua/lmem.c
             src/c/lua/loadlib.c
             src/c/lua/lobject.c
             src/c/lua/lopcodes.c
             src/c/lua/loslib.c
             src/c/lua/lparser.c
             src/c/lua/lstate.c
             src/c/lua/lstring.c
             src/c/lua/lstrlib.c
             src/c/lua/ltable.c
             src/c/lua/ltablib.c
             src/c/lua/ltm.c
             src/c/lua/lundump.c
             src/c/lua/lvm.c
             src/c/lua/lzio.c
             src/c/lua/print.c )

add_library( # Specifies the name of the library.
             jlua

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/c/jlua/jlua.c
             src/c/jlua/jluajni.c)

include_directories( src/c/lua )

######## We don't need this in Android #######

include_directories(${JNI_INCLUDE_DIRS})

##############################################


target_link_libraries( jlua lua )