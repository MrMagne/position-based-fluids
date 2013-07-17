#ifndef __HESP_HPP
#define __HESP_HPP

#ifdef __OPENCL_VERSION__

#pragma OPENCL EXTENSION cl_amd_printf : enable

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

#ifdef USE_DOUBLE_PRECISION

#if defined(cl_khr_fp64)
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64)
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#else
#error "Device does not support double precision extensions!"
#endif // cl_khr_fp64
typedef double hesp_float;
typedef double3 hesp_float3;
typedef double4 hesp_float4;
#else
typedef float hesp_float;
typedef float3 hesp_float3;
typedef float4 hesp_float4;

#endif // USE_DOUBLE_PRECISION

#else

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif // __APPLE__

#ifdef USE_DOUBLE_PRECISION
typedef cl_double hesp_float;
typedef cl_double3 hesp_float3;
typedef cl_double4 hesp_float4;
#else
typedef cl_float hesp_float;
typedef cl_float3 hesp_float3;
typedef cl_float4 hesp_float4;
#endif // USE_DOUBLE_PRECISION

#endif // __OPENCL_VERSION__

#endif // __HESP_HPP
