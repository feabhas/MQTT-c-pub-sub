#Specify the version being used as well as the language
cmake_minimum_required(VERSION 2.6)
#Name your project here
project(mqttpub)
project(mqttsub)

#Sends the -std=c99 flag to the gcc compiler
add_definitions(-std=c99)
include_directories(src)

#This tells CMake to main.c and name it mqttpub
add_executable(mqttpub pubmain/main.c src/mqtt.c)
add_executable(mqttsub submain/main.c src/mqtt.c)



