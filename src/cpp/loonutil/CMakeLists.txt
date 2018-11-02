cmake_minimum_required(VERSION 3.6)

project(loonutil_lib)

if(NOT DEFINED LOGGER_LEVEL)
    message(FATAL_ERROR "LOGGER_LEVEL (0,1,...,5) is not defined")
else()
    message(STATUS "LOGGER_LEVEL=${LOGGER_LEVEL}")
endif()

option(install_submodule "Install loonutil_lib" OFF)

set(UTIL_HEADERS cedar.h cedarpp.h global.h logger.h multi-array.h progress.h relabel.h relabelImpl.h timer.h util.h array.h iobin.h exception.h simpleHelp.h)

add_library(loonutil global.cpp logger.cpp progress.cpp timer.cpp util.cpp BinWriter.cpp BinReader.cpp exception.cpp simpleHelp.cpp)
target_compile_definitions(loonutil PUBLIC -DLOGGER_LEVEL=${LOGGER_LEVEL})

if(install_submodule)
    install(TARGETS loonutil DESTINATION lib)
    install(FILES ${UTIL_HEADERS} DESTINATION include/loonutil)
endif()
