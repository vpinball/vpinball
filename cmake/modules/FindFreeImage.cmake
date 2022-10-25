# Find the FreeImage library.
#
# Once done this will define
#  FreeImage_FOUND - System has FreeImage
#  FreeImage_INCLUDE_DIRS - The FreeImage include directories
#  FreeImage_LIBRARIES - The libraries needed to use FreeImage
#  FreeImage_DEFINITIONS - Compiler switches required for using FreeImage
#
# It also creates the IMPORTED target: FreeImage::FreeImage

find_path(FreeImage_INCLUDE_DIR NAMES FreeImage.h)
find_library(FreeImage_LIBRARY_RELEASE NAMES FreeImage freeimage)
find_library(FreeImage_LIBRARY_DEBUG
  NAMES FreeImaged FreeImage_d FreeImage-debug
  PATH_SUFFIXES debug
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeImage  DEFAULT_MSG
                                  FreeImage_LIBRARY_RELEASE
                                  FreeImage_INCLUDE_DIR)



mark_as_advanced(FreeImage_INCLUDE_DIR
                 FreeImage_LIBRARY_DEBUG
                 FreeImage_LIBRARY_RELEASE)

set(FreeImage_LIBRARY "")
if (FreeImage_LIBRARY_DEBUG)
    list(APPEND FreeImage_LIBRARY debug ${FreeImage_LIBRARY_DEBUG})
endif()
if (FreeImage_LIBRARY_RELEASE)
    list(APPEND FreeImage_LIBRARY optimized ${FreeImage_LIBRARY_RELEASE})
endif()

set(FreeImage_INCLUDE_DIRS ${FreeImage_INCLUDE_DIR})
set(FreeImage_LIBRARIES ${FreeImage_LIBRARY})

if(FreeImage_FOUND)
  if (NOT TARGET FreeImage::FreeImage)
    add_library(FreeImage::FreeImage UNKNOWN IMPORTED)
  endif()
  if (FreeImage_LIBRARY_RELEASE)
    set_property(TARGET FreeImage::FreeImage APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(FreeImage::FreeImage PROPERTIES
      IMPORTED_LOCATION_RELEASE "${FreeImage_LIBRARY_RELEASE}"
    )
  endif()
  if (FreeImage_LIBRARY_DEBUG)
    set_property(TARGET FreeImage::FreeImage APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(FreeImage::FreeImage PROPERTIES
      IMPORTED_LOCATION_DEBUG "${FreeImage_LIBRARY_DEBUG}"
    )
  endif()
  set_target_properties(FreeImage::FreeImage PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${FreeImage_INCLUDE_DIR}"
  )
endif()
