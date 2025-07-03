#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Open3D::open3d_torch_ops" for configuration "Release"
set_property(TARGET Open3D::open3d_torch_ops APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Open3D::open3d_torch_ops PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "torch_cpu;Open3D::Open3D;Open3D::tbb"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/open3d_torch_ops.dylib"
  IMPORTED_SONAME_RELEASE "@rpath/open3d_torch_ops.dylib"
  )

list(APPEND _cmake_import_check_targets Open3D::open3d_torch_ops )
list(APPEND _cmake_import_check_files_for_Open3D::open3d_torch_ops "${_IMPORT_PREFIX}/lib/open3d_torch_ops.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
