add_subdirectory(atm)
add_subdirectory(bank)
add_subdirectory(screen)

add_executable(main main.cpp)

target_link_libraries(main atm bank screen)
