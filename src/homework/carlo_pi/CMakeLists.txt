aux_source_directory(. CARLO_PI_SRC)
add_library(carlo_pi OBJECT ${CARLO_PI_SRC})
set(ALL_OBJECT_FILES ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:carlo_pi> PARENT_SCOPE)
