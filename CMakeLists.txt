cmake_minimum_required(VERSION 3.14)
include(FetchContent)

project(greenbutton_reader)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(greenbutton_reader greenbutton.cpp)
target_compile_features(greenbutton_reader PUBLIC cxx_std_20)
set_target_properties(greenbutton_reader PROPERTIES CXX_EXTENSIONS OFF)


find_package(libpqxx QUIET)
if (!LIBPQXX_FOUND)
  pkg_search_module(PQ REQUIRED pq)
  pkg_search_module(PQXX REQUIRED pqxx)
  if (LIBPQXX_FOUND)
    message("Found libpqxx with pkg_config...")
  else()
    message(FATAL_ERROR "Could not find libpqxx.")
  endif()
endif()

FetchContent_Declare(fmt
		GIT_REPOSITORY https://github.com/fmtlib/fmt
		GIT_TAG 8.0.1
)
FetchContent_MakeAvailable(fmt)


target_link_libraries(greenbutton_reader PRIVATE pqxx fmt)
