aux_source_directory(. EXERCISES_SRC)
add_library(exercises STATIC ${EXERCISES_SRC})
target_link_libraries(exercises PUBLIC ${PROJECET_NAME_THIRD_LIBRARIES} OpenMP::OpenMP_CXX)

add_subdirectory(thread)
add_subdirectory(mpi)
add_subdirectory(thread_safe_structure)
