cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project("alglib")

add_library(specialfunctions specialfunctions.cpp ap.cpp alglibinternal.cpp)

