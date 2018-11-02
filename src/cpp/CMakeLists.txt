cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project("riginv_cpp")

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD 11)

##find_package(loonlib REQUIRED)
##if(loonlib_FOUND)
##    include_directories(${LOONLIB_INCLUDE_DIRS})
##endif()

## # Gomory-Hu Tree GHG implementation
## add_subdirectory(gomory-hu/ghg/)
## add_subdirectory(gomory-hu/aux/)

include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(alglib/cpp/src/)
add_subdirectory(loonutil/)

set(LOONLIB_LIBRARIES loonutil)
set(rigvin_cpp_install_list "")

add_executable(sort_brief_alignment sort_brief_alignment.cpp)
target_link_libraries(sort_brief_alignment ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} sort_brief_alignment)

add_executable(concordant_aln_analysis concordant_aln_analysis.cpp)
target_link_libraries(concordant_aln_analysis ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} concordant_aln_analysis)

add_executable(discordant_type1 discordant_type1.cpp)
target_link_libraries(discordant_type1 ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} discordant_type1)

add_executable(discordant_type2 discordant_type2.cpp)
target_link_libraries(discordant_type2 ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} discordant_type2)

add_executable(discordant_type3 discordant_type3.cpp)
target_link_libraries(discordant_type3 ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} discordant_type3)


# segment_prediciton unfinished!!!

# not required for now
### add_executable(brute_force_rect_prediction brute_force_rect_prediction.cpp)
### target_link_libraries(brute_force_rect_prediction ${LOONLIB_LIBRARIES})
### set(rigvin_cpp_install_list ${rigvin_cpp_install_list} brute_force_rect_prediction)

add_executable(partition_disconnected_rects partition_disconnected_rects.cpp)
target_link_libraries(partition_disconnected_rects ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} partition_disconnected_rects)

#not required for now
### add_executable(prob_rect_prediction prob_rect_prediction.cpp)
### target_link_libraries(prob_rect_prediction ${LOONLIB_LIBRARIES})
### set(rigvin_cpp_install_list ${rigvin_cpp_install_list} prob_rect_prediction)

add_executable(cluster_by_maximal_coverage cluster_by_maximal_coverage.cpp)
target_link_libraries(cluster_by_maximal_coverage ${LOONLIB_LIBRARIES} specialfunctions)
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} cluster_by_maximal_coverage)

add_executable(refine_type2 refine_type2.cpp)
target_link_libraries(refine_type2 ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} refine_type2)

add_executable(sampling_lines sampling_lines.cpp)
target_link_libraries(sampling_lines ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} sampling_lines)

# The following shall not be used. It's not correct.
## add_executable(test_new_prob test_new_prob.cpp)
## target_link_libraries(test_new_prob ${LOONLIB_LIBRARIES} specialfunctions)
## set(rigvin_cpp_install_list ${rigvin_cpp_install_list} test_new_prob)

add_executable(merge_files merge_files.cpp)
target_link_libraries(merge_files ${LOONLIB_LIBRARIES})
set(rigvin_cpp_install_list ${rigvin_cpp_install_list} merge_files)

install(TARGETS ${rigvin_cpp_install_list} DESTINATION libexec/bin)
