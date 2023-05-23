cmake_minimum_required (VERSION 3.12)

project ("aos_2022_cpp")

# Add source to this project's executable.
add_executable (aos_2022_cpp "main.cpp"  "days.hpp" "days.cpp" "d00.cpp" "d01.cpp" "d02.cpp" "d03.cpp" "d04.cpp" "d05.cpp" "d06.cpp" "d07.cpp" "d08.cpp" "d09.cpp" "d10.cpp" "d11.cpp" "d12.cpp" "d13.cpp" "d14.cpp" "d15.cpp" "d16.cpp" "d17.cpp" "d18.cpp" "d19.cpp" "d20.cpp" "d21.cpp" "d22.cpp")

set_property(TARGET aos_2022_cpp PROPERTY CXX_STANDARD 23)

target_compile_options(aos_2022_cpp PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

target_link_libraries(aos_2022_cpp
	PUBLIC gtest
)