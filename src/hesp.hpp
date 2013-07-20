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

#if defined(PLATFORM_APPLE)

#if (OS_X_VERSION < 1083)
#pragma OPENCL EXTENSION cl_APPLE_gl_sharing : enable
#else
#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#endif

#else

#if !defined(PLATFORM_AMD)
#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#endif

#endif

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
