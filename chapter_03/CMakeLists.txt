add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world ${MPI_CXX_LIBRARIES})

add_executable(mpi_trap_integral mpi_trap_integral.cpp)
target_link_libraries(mpi_trap_integral ${MPI_CXX_LIBRARIES})

add_executable(mpi_math mpi_math.cpp)
target_link_libraries(mpi_math ${MPI_CXX_LIBRARIES})

add_executable(mpi_carlo_pi mpi_carlo_pi.cpp)
target_link_libraries(mpi_carlo_pi ${MPI_CXX_LIBRARIES})

add_executable(mpi_ping_pong mpi_ping_pong.cpp)
target_link_libraries(mpi_ping_pong ${MPI_CXX_LIBRARIES})
