FindOpenCL.cmake
================

:Author:    Sergiu Dotenco
:Date:      September 16, 2011


Overview
--------

This module allows to locate OpenCL_ and to determine its version while
configuring the project.

.. _OpenCL: http://www.khronos.org/opencl/


Customizable variables
----------------------

  OPENCL_ROOT_DIR
    Specifies OpenCL's root directory. The find module uses this variable to
    locate OpenCL. The variable will be filled automatically unless explicitly
    set using CMake's ``-D`` command-line option. Instead of setting a CMake
    variable, an environment variable called ``OCLROOT`` can be used. While
    locating the root directory, the module will try to detect OpenCL
    implementations provided by `AMD's Accelerated Parallel Processing SDK`__,
    `NVIDIA's GPU Computing Toolkit`__ and `Intel's OpenCL SDK`__ by examining
    the ``AMDAPPSDKROOT``, ``CUDA_PATH`` and ``INTELOCLSDKROOT`` environment
    variables, respectively.

__ AMDAPP_
__ NVIDIACUDA_
__ INTELOCL_

.. _AMDAPP: http://developer.amd.com/sdks/AMDAPPSDK/Pages/default.aspx
.. _NVIDIACUDA: http://developer.nvidia.com/cuda-toolkit-40
.. _INTELOCL: http://software.intel.com/en-us/articles/opencl-sdk


Read-only variables
-------------------

  OPENCL_FOUND
    Indicates whether the library has been found.

  OPENCL_INCLUDE_DIRS
    Specifies the OpenCL include directories.

  OPENCL_LIBRARIES
    Specifies the OpenCL libraries that should be passed to
    ``target_link_libararies``.

.. vi: spell spelllang=en
