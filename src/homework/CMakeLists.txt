add_subdirectory(histogram_bin)
add_subdirectory(trap_integral)
add_subdirectory(carlo_pi)
add_subdirectory(matrix_multi_vector)

add_library(
  homework
  STATIC
  ${ALL_OBJECT_FILES}
)

set(HOMEWORK_LIBS histogram_bin trap_integral carlo_pi matrix_multi_vector)

target_link_libraries(homework PUBLIC  ${PROJECET_NAME_THIRD_LIBRARIES} ${HOMEWORK_LIBS})
