# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.
project("avtranscoder")


SET(RECORDE_LIB_PATH ${CMAKE_SOURCE_DIR}/source)
add_subdirectory (${RECORDE_LIB_PATH})