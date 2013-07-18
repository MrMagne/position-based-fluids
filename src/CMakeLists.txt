include_directories(
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_SOURCE_DIR}/lib/glfw/include
	${CMAKE_SOURCE_DIR}/lib/soil/src/
)

add_definitions(-DUSE_LINKEDCELL -DUSE_CGL_SHARING)

set(SOURCE
	main.cpp
	Runner.cpp
	Simulation.cpp
)

add_subdirectory(io)
add_subdirectory(visual)
add_subdirectory(ocl)

add_executable(hesp ${SOURCE})

if (APPLE)
  target_link_libraries(hesp
  	glfw
  	soil
  	${OPENGL_LIBRARY}
  	${OPENCL_LIBRARY}
    ${COREFOUNDATION_LIBRARY}
    ${COCOA_LIB}
    ${IOKIT_LIB}
  )
else()
  target_link_libraries(hesp
    glfw
    soil
    ${OPENGL_LIBRARY}
    ${OPENCL_LIBRARY}
  )
endif (APPLE)
