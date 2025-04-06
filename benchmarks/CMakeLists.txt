add_executable(bench_original bench_original.cpp)
add_executable(bench_fast bench_fast.cpp)


target_link_libraries(bench_original PRIVATE ChessLib)
target_link_libraries(bench_fast PRIVATE ChessLib FAST-SAN-LIB)
