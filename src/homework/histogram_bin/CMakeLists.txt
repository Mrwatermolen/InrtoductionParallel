aux_source_directory(. HISTOGRAM_BIN_SRC)
add_library(histogram_bin OBJECT ${HISTOGRAM_BIN_SRC})
set(ALL_OBJECT_FILES ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:histogram_bin> PARENT_SCOPE)
