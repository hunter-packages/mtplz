# Explicitly list the source files for this subdirectory
#
# If you add any source files to this subdirectory
#    that should be included in the kenlm library,
#        (this excludes any unit test files)
#    you should add them to the following list:
#
# Because we do not set PARENT_SCOPE in the following definition,
#    CMake files in the parent directory won't be able to access this variable.
#
set(KENLM_UTIL_SOURCE
  float_memcpy.hh
  bit_packing.cc
  ersatz_progress.cc
  exception.cc
  file.cc
  file_piece.cc
  float_to_string.cc
  integer_to_string.cc
  mmap.cc
  murmur_hash.cc
  mutable_vocab.cc
  parallel_read.cc
  pool.cc
  read_compressed.cc
  scoped.cc
  spaces.cc
  string_piece.cc
  usage.cc)

set(READ_COMPRESSED_FLAGS)
set(READ_COMPRESSED_LIBS)

hunter_add_package(ZLIB)
find_package(ZLIB CONFIG REQUIRED)
set(READ_COMPRESSED_FLAGS "${READ_COMPRESSED_FLAGS} -DHAVE_ZLIB")
set(READ_COMPRESSED_LIBS ${READ_COMPRESSED_LIBS} ZLIB::zlib)

hunter_add_package(BZip2)
find_package(BZip2 CONFIG REQUIRED)
set(READ_COMPRESSED_FLAGS "${READ_COMPRESSED_FLAGS} -DHAVE_BZLIB")
set(READ_COMPRESSED_LIBS ${READ_COMPRESSED_LIBS} BZip2::bz2)

hunter_add_package(lzma)
find_package(lzma CONFIG REQUIRED)
set(READ_COMPRESSED_FLAGS "${READ_COMPRESSED_FLAGS} -DHAVE_XZLIB")
set(READ_COMPRESSED_LIBS ${READ_COMPRESSED_LIBS} lzma::lzma)

set_source_files_properties(read_compressed.cc PROPERTIES COMPILE_FLAGS ${READ_COMPRESSED_FLAGS})
set_source_files_properties(read_compressed_test.cc PROPERTIES COMPILE_FLAGS ${READ_COMPRESSED_FLAGS})
set_source_files_properties(file_piece_test.cc PROPERTIES COMPILE_FLAGS ${READ_COMPRESSED_FLAGS})

# This directory has children that need to be processed
add_subdirectory(double-conversion)
add_subdirectory(stream)

# Finding 'rt' library:
# * https://github.com/ruslo/hunter/blob/eaf4656a3e34798e7f306cccad472e27501c735c/cmake/templates/BoostConfig.cmake.in#L17-L33
include(CheckCXXSymbolExists)
set(TIMER_LINK)
check_cxx_symbol_exists("clock_gettime" "time.h" mtplz_have_clock_gettime)
if(NOT mtplz_have_clock_gettime)
  include(CheckLibraryExists)
  check_library_exists(rt clock_gettime "" mtplz_check_rt_library)

  if(mtplz_check_rt_library)
    set(TIMER_LINK rt)
  endif()
endif()

# Group these objects together for later use.
add_library(kenlm_util ${KENLM_UTIL_DOUBLECONVERSION_SOURCE} ${KENLM_UTIL_STREAM_SOURCE} ${KENLM_UTIL_SOURCE})
target_link_libraries(kenlm_util Boost::boost ${READ_COMPRESSED_LIBS} Threads::Threads ${TIMER_LINK})

install(
    TARGETS kenlm_util
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "${include_install_dir}"
)

AddExes(EXES probing_hash_table_benchmark
        LIBRARIES kenlm_util ${Boost_LIBRARIES} Threads::Threads)
AddExes(EXES cat_compressed
        LIBRARIES kenlm_util ${Boost_LIBRARIES} Threads::Threads)

# Only compile and run unit tests if tests should be run
if(BUILD_TESTING)
  set(KENLM_BOOST_TESTS_LIST
    bit_packing_test
    integer_to_string_test
    joint_sort_test
    layout_test
    multi_intersection_test
    pcqueue_test
    probing_hash_table_test
    read_compressed_test
    sized_iterator_test
    sorted_uniform_test
    string_stream_test
    tokenize_piece_test
  )

  AddTests(TESTS ${KENLM_BOOST_TESTS_LIST}
           LIBRARIES kenlm_util ${Boost_LIBRARIES} Threads::Threads)

  # file_piece_test requires an extra command line parameter
  KenLMAddTest(TEST file_piece_test
               LIBRARIES kenlm_util ${Boost_LIBRARIES} Threads::Threads
               TEST_ARGS ${CMAKE_CURRENT_SOURCE_DIR}/file_piece.cc)
endif()
