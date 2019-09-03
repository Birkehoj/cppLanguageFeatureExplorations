cmake_minimum_required(VERSION 2.8)

project(ThreadedLoggerExample)
add_executable(${PROJECT_NAME} "main.cpp")

SET( THREADS_PREFER_PTHREAD_FLAG TRUE )
FIND_PACKAGE( Threads REQUIRED )
TARGET_LINK_LIBRARIES( ThreadedLoggerExample Threads::Threads )
TARGET_COMPILE_OPTIONS( ThreadedLoggerExample INTERFACE Threads::Threads )
