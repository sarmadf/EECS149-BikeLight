cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(EE149Proj)

# add the executables
add_executable(${PROJECT_NAME} main.c brakes.c brakes.h turn.h turn.c states.c states.h)
