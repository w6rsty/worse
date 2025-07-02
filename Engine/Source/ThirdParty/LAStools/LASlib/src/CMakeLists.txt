if (!MSVC)
	add_compile_options(
		-Wno-deprecated 
		-Wno-write-strings 
		-Wno-unused-result 
		-Wno-unused-variable
		-Wno-unused-parameter
		-Wno-unused-function
		-Wno-sign-compare
		-Wno-conversion
		-Wno-implicit-fallthrough
		-Wno-missing-field-initializers
		-Wno-reorder
		-Wno-switch
		-Wno-format
		-Wno-parentheses
		-Wno-maybe-uninitialized
		-Wno-char-subscripts
		-Wno-strict-aliasing
		-std=c++11
	)
endif()
add_definitions(-DNDEBUG -DUNORDERED -DHAVE_UNORDERED_MAP)

set(LAS_SRC
	lasreader.cpp
	lasignore.cpp
	laswriter.cpp
	lasreader_las.cpp
	lasreader_bin.cpp
	lasreader_shp.cpp
	lasreader_asc.cpp
	lasreader_bil.cpp
	lasreader_dtm.cpp
	lasreader_ply.cpp
	lasreader_qfit.cpp
	lasreader_txt.cpp
	lasreadermerged.cpp
	lasreaderbuffered.cpp
	lasreaderstored.cpp
	lasreaderpipeon.cpp
	laswriter_las.cpp
	laswriter_bin.cpp
	laswriter_qfit.cpp
	laswriter_wrl.cpp
	laswriter_txt.cpp
	laswritercompatible.cpp
	laswaveform13reader.cpp
	laswaveform13writer.cpp
	lasutility.cpp
	lasfilter.cpp
	lastransform.cpp
	laskdtree.cpp
	lascopc.cpp
	fopen_compressed.cpp
)

set(LAZ_SRC
	lasreadpoint.cpp
	lasreaditemcompressed_v1.cpp
	lasreaditemcompressed_v2.cpp
	lasreaditemcompressed_v3.cpp
	lasreaditemcompressed_v4.cpp
	laswritepoint.cpp
	laswriteitemcompressed_v1.cpp
	laswriteitemcompressed_v2.cpp
	laswriteitemcompressed_v3.cpp
	laswriteitemcompressed_v4.cpp
	integercompressor.cpp
	arithmeticdecoder.cpp
	arithmeticencoder.cpp
	arithmeticmodel.cpp
	lasindex.cpp
	lasquadtree.cpp
	lasinterval.cpp
	laszip.cpp
	mydefs.cpp
	lasmessage.cpp
)

foreach(file ${LAZ_SRC})
	list(APPEND LAZ_SRC_FULL ../../LASzip/src/${file})
endforeach(file)

file(GLOB_RECURSE LAS_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/../../LASzip/src/*.hpp 
	${CMAKE_CURRENT_SOURCE_DIR}/../inc/*.hpp
)

add_library(LASlib ${LAS_SRC} ${LAZ_SRC_FULL})
set_property(TARGET LASlib PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET LASlib PROPERTY CXX_STANDARD 11)

if (BUILD_SHARED_LIBS)
	target_compile_definitions(LASlib PRIVATE "COMPILE_AS_DLL")
endif()

target_include_directories(LASlib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../inc>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../LASzip/src>
  $<INSTALL_INTERFACE:include/LASlib>
)
set_target_properties(LASlib PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

if(MSVC)
   target_compile_options(LASlib PRIVATE 
       /Zi
       /wd4996  # 'function': was declared deprecated
       /wd4244  # conversion from 'type1' to 'type2', possible loss of data
       /wd4267  # conversion from 'size_t' to 'type', possible loss of data
       /wd4305  # truncation from 'type1' to 'type2'
       /wd4018  # signed/unsigned mismatch
       /wd4101  # unreferenced local variable
       /wd4189  # local variable is initialized but not referenced
       /wd4100  # unreferenced formal parameter
       /wd4706  # assignment within conditional expression
       /wd4146  # unary minus operator applied to unsigned type
       /wd4090  # different 'const' qualifiers
   )

   # Tell linker to include symbol data
    set_target_properties(LASlib PROPERTIES 
        LINK_FLAGS "/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF"
    )

    # Set file name & location
    set_target_properties(LASlib PROPERTIES 
        COMPILE_PDB_NAME LASlib 
        COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}
    )
endif()

install(FILES ${LAS_INCLUDES} DESTINATION include/LASlib)

if (MSVC)
	foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
		install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib/${OUTPUTCONFIG} DESTINATION lib/LASlib)
	endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
else()
	install(TARGETS LASlib EXPORT laslib-targets
		ARCHIVE DESTINATION lib/LASlib
		LIBRARY DESTINATION lib/LASlib
		RUNTIME DESTINATION lib/LASlib)
	install(EXPORT laslib-targets DESTINATION lib/cmake/LASlib)
	install(FILES ${CMAKE_SOURCE_DIR}/LASlib/src/laslib-config.cmake DESTINATION lib/cmake/LASlib)
endif(MSVC)
