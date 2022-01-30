cmake_minimum_required (VERSION 3.0)
project (ucb-holmes)

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-ot REQUIRED)
include_directories(${EMP-OT_INCLUDE_DIRS})

find_package(emp-zk-holmes REQUIRED)
include_directories(${EMP-ZK-HOLMES_INCLUDE_DIRS})

find_package(GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})
find_package(FLINT REQUIRED)
include_directories(${FLINT_INCLUDE_DIR}/flint)
find_package(MPFR REQUIRED)
include_directories(${MPFR_INCLUDE_DIR})

find_package(OpenMP)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -ggdb -Wno-ignored-attributes -Xpreprocessor -fopenmp")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes -Xpreprocessor -fopenmp")
else()
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -ggdb -Wno-ignored-attributes -fopenmp")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes -fopenmp")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

set(binaries_bench bench_dataset_1 bench_dataset_2 bench_dataset_3 bench_range_check bench_histogram_numeric bench_histogram_nominal bench_mean_check bench_trimmed_mean bench_variance_check bench_jl bench_strawman_jl bench_test)
foreach(bin ${binaries_bench})
   add_executable("${bin}" "bench/${bin}.cpp")
   target_link_libraries("${bin}" ${EMP-OT_LIBRARIES} ${EMP-ZK-HOLMES_LIBRARIES} ${GMP_LIBRARIES} ${MPFR_LIBRARIES} ${FLINT_LIBRARIES} OpenMP::OpenMP_CXX)
endforeach()