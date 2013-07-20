#ifndef __HESP_HPP
#define __HESP_HPP

#ifdef __OPENCL_VERSION__

// Enable atomic functions
#if defined(cl_khr_global_int32_base_atomics) && (cl_khr_global_int32_extended_atomics)
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#else
#error "Device does not support atomic operations!"
#endif // cl_khr_global_int32_base_atomics && cl_khr_global_int32_extended_atomics

// Enable CL/GL sharing
#if defined(USE_CGL_SHARING)

#if defined(cl_khr_gl_sharing)
#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#else
#if defined(__APPLE__) || defined(__MACOSX)
#pragma OPENCL EXTENSION cl_APPLE_gl_sharing : enable
#else
#error "Device does not support OpenCL/OpenGL sharing!"
#endif // cl_APPLE_gl_sharing
#endif // cl_khr_gl_sharing

#endif // USE_CGL_SHARING

#else

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define __CL_ENABLE_EXCEPTIONS

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wc++11-extra-semi"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#if defined(__APPLE__) || defined(__MACOSX)
#include "ocl/cl.hpp"
#else
#include <CL/cl.hpp>
#endif // __APPLE__
#pragma GCC diagnostic pop

#endif // __OPENCL_VERSION__

#endif // __HESP_HPP
