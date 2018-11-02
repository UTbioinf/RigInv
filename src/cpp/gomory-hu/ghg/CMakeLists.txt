cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project("gomory_hu_tree_ghg")

add_executable(ho_ghg ho.c random.c timer.c compact.c tree.c graph.c fprintfll.c)
# LOON_ALLOW_PRINT
target_compile_definitions(ho_ghg PUBLIC -DNDEBUG -DSAVECUT -DHO -DNO_PR)
target_link_libraries(ho_ghg -lm)

install(TARGETS ho_ghg DESTINATION libexec/bin)
