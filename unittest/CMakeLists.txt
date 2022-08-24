cmake_minimum_required (VERSION 3.15)

macro(make_test)
	add_executable (${ARGV0} ${ARGV1})
	target_link_libraries(${ARGV0} poly_vector)
	add_test(${ARGV0} ${ARGV0})

if (MSVC)
	#target_compile_options(${ARGV0} PRIVATE  )
	#target_compile_options(${ARGV0} PRIVATE /RTC- /fsanitize=address)
	#target_link_options(${ARGV0} PRIVATE -fsanitize=undefined)
else()
	target_compile_options(${ARGV0} PRIVATE -fsanitize=undefined,address)
	target_link_options(${ARGV0} PRIVATE -fsanitize=undefined,address)
endif()

endmacro()

make_test (unittest_poly_vector "unittest_poly_vector.cpp")
