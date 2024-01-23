add_executable(mpi_math mpi_math.cpp)
target_link_libraries(mpi_math ${MPI_CXX_LIBRARIES})

add_executable(mpi_ping_pong mpi_ping_pong.cpp)
target_link_libraries(mpi_ping_pong ${MPI_CXX_LIBRARIES})

