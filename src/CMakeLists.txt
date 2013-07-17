include_directories(
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_SOURCE_DIR}/lib/glfw/include
	${CMAKE_SOURCE_DIR}/lib/soil/src/
)

add_definitions(-DGLFW3 -DUSE_LINKEDCELL -DUSE_CGL_SHARING)

set(SOURCE
	main.cpp
	Runner.cpp
	Simulation.cpp
)

add_subdirectory(io)
add_subdirectory(visual)
add_subdirectory(ocl)

add_executable(hesp ${SOURCE})

target_link_libraries(hesp
	glfw
	soil
	${OPENGL_LIBRARY}
	${OPENCL_LIBRARY}
	${X11_LIBRARY}
)
