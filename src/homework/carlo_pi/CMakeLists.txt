aux_source_directory(. CARLO_PI_SRC)
add_library(carlo_pi OBJECT ${CARLO_PI_SRC})
# IMPORTANT: omp_target need to link with OpenMP::OpenMP_CXX before being homework lib
target_link_libraries(carlo_pi PUBLIC OpenMP::OpenMP_CXX)
set(ALL_OBJECT_FILES ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:carlo_pi> PARENT_SCOPE)
