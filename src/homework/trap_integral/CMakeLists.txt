aux_source_directory(. TRAP_INTEGRAL_SRC)
add_library(trap_integral OBJECT ${TRAP_INTEGRAL_SRC})
set(ALL_OBJECT_FILES ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:trap_integral> PARENT_SCOPE)
