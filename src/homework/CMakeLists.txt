add_subdirectory(histogram_bin)
add_subdirectory(trap_integral)
add_subdirectory(carlo_pi)

add_library(
  homework
  STATIC
  ${ALL_OBJECT_FILES}
)

set(HOMEWORK_LIBS histogram_bin trap_integral carlo_pi)

target_link_libraries(homework PUBLIC ${HOMEWORK_LIBS} ${PROJECET_NAME_THIRD_LIBRARIES})
