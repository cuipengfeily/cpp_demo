cmake_minimum_required (VERSION 2.6 FATAL_ERROR)
project      (wait_for)

include_directories (${PROJECT_SOURCE_DIR})

set  (project_SOURCES wait_for.cpp main.cpp)

set  (INCLUDE_FILE wait_for.h)

ADD_EXECUTABLE (wait_for ${project_SOURCES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT wait_for)