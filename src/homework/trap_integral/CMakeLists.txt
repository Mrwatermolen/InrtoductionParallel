aux_source_directory(. trap_integral_SRC)
add_library(trap_integral OBJECT ${trap_integral_SRC})
set(ALL_OBJECT_FILES ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:trap_integral> PARENT_SCOPE)
