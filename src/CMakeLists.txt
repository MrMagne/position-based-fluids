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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(io)
add_subdirectory(visual)
add_subdirectory(ocl)

set(KERNELS
  "${HESP_SOURCE_DIR}/src/hesp.hpp"
  "${HESP_SOURCE_DIR}/src/kernels/apply_vorticity_and_viscosity.cl"
  "${HESP_SOURCE_DIR}/src/kernels/calc_hash.cl"
  "${HESP_SOURCE_DIR}/src/kernels/compute_delta.cl"
  "${HESP_SOURCE_DIR}/src/kernels/compute_scaling.cl"
  "${HESP_SOURCE_DIR}/src/kernels/find_cells.cl"
  "${HESP_SOURCE_DIR}/src/kernels/init_cells.cl"
  "${HESP_SOURCE_DIR}/src/kernels/init_cells_old.cl"
  "${HESP_SOURCE_DIR}/src/kernels/predict_positions.cl"
  "${HESP_SOURCE_DIR}/src/kernels/radix_histogram.cl"
  "${HESP_SOURCE_DIR}/src/kernels/radix_paste.cl"
  "${HESP_SOURCE_DIR}/src/kernels/radix_reorder.cl"
  "${HESP_SOURCE_DIR}/src/kernels/update_cells.cl"
  "${HESP_SOURCE_DIR}/src/kernels/update_positions.cl"
  "${HESP_SOURCE_DIR}/src/kernels/update_predicted.cl"
  "${HESP_SOURCE_DIR}/src/kernels/update_velocities.cl"
)

SET(SHADERS
  "${HESP_SOURCE_DIR}/src/shaders/particlevertex.glsl"
  "${HESP_SOURCE_DIR}/src/shaders/particlefragment.glsl"
  "${HESP_SOURCE_DIR}/src/shaders/shadervertex.glsl"
  "${HESP_SOURCE_DIR}/src/shaders/shaderfragment.glsl"
)

SET(SCENARIOS
  "${HESP_SOURCE_DIR}/assets/scenarios/dam_miles.par"
  "${HESP_SOURCE_DIR}/assets/scenarios/dam_miles.in"
  "${HESP_SOURCE_DIR}/assets/scenarios/dam_coarse.par"
  "${HESP_SOURCE_DIR}/assets/scenarios/dam_coarse.in"
)

SET(TEXTURES
  "${HESP_SOURCE_DIR}/assets/textures/wall.tga"
)

# also headers are not necessary for cmake, 
# include them here so they appear in IDE like Xcode
add_executable(hesp ${SOURCE} ${HEADER} ${KERNELS} ${SHADERS} ${SCENARIOS} ${TEXTURES})

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

add_custom_target(copy ALL
    COMMENT "Copying support files")

foreach(KERNEL ${KERNELS})
  get_filename_component(FILENAME ${KERNEL} NAME)
  set(SRC "${KERNEL}")
  set(DST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/kernels/${FILENAME}")

  add_custom_command(
    TARGET copy
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
    )
endforeach(KERNEL)

foreach(SHADER ${SHADERS})
  get_filename_component(FILENAME ${SHADER} NAME)
  set(SRC "${SHADER}")
  set(DST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/shaders/${FILENAME}")

  add_custom_command(
    TARGET copy
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
    )
endforeach(SHADER)

foreach(SCENARIO ${SCENARIOS})
  get_filename_component(FILENAME ${SCENARIO} NAME)
  set(SRC "${SCENARIO}")
  set(DST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/scenarios/${FILENAME}")

  add_custom_command(
    TARGET copy
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
    )
endforeach(SCENARIO)

foreach(TEXTURE ${TEXTURES})
  get_filename_component(FILENAME ${TEXTURE} NAME)
  set(SRC "${TEXTURE}")
  set(DST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/textures/${FILENAME}")

  add_custom_command(
    TARGET copy
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
    )
endforeach(TEXTURE)