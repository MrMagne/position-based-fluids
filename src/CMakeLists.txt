include_directories(
	${CMAKE_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/lib/glm
	${CMAKE_SOURCE_DIR}/lib/glfw/include
	${CMAKE_SOURCE_DIR}/lib/soil/src/
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pedantic -Wall -Wextra -Werror -Wfatal-errors")
add_definitions(-DUSE_LINKEDCELL)

set(SOURCE
	main.cpp
	Runner.cpp
	Simulation.cpp
  DataLoader.cpp
)

set(HEADER
  hesp.hpp
  Particle.hpp
  Runner.hpp
  Simulation.hpp
  DataLoader.hpp
)

add_subdirectory(io)
add_subdirectory(visual)
add_subdirectory(ocl)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# also headers are not necessary for cmake, 
# include them here so they appear in IDE like Xcode
add_executable(hesp ${SOURCE} ${HEADER})

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

add_custom_command(TARGET hesp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${HESP_SOURCE_DIR}/assets/scenarios"
        $<TARGET_FILE_DIR:hesp>/data/scenarios)

add_custom_command(TARGET hesp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${HESP_SOURCE_DIR}/assets/scenarios/dam_miles.par"
        $<TARGET_FILE_DIR:hesp>/data/scenarios)

add_custom_command(TARGET hesp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${HESP_SOURCE_DIR}/assets/scenarios/dam_miles.par"
        $<TARGET_FILE_DIR:hesp>/data/scenarios)

add_custom_command(TARGET hesp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${HESP_SOURCE_DIR}/assets/textures"
        $<TARGET_FILE_DIR:hesp>/data/textures)

add_custom_command(TARGET hesp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${HESP_SOURCE_DIR}/src/kernels"
        $<TARGET_FILE_DIR:hesp>/data/kernels)

add_custom_command(TARGET hesp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${HESP_SOURCE_DIR}/src/hesp.hpp"
        $<TARGET_FILE_DIR:hesp>/data/kernels)

add_custom_command(TARGET hesp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${HESP_SOURCE_DIR}/src/shaders"
        $<TARGET_FILE_DIR:hesp>/data/shaders)