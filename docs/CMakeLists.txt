# SPDX-License-Identifier: BSD-3-Clause

# TODO: capture build output in log files rather than console?
# TODO: clean up on "make clean". Might require ADDITIONAL_CLEAN_FILES (cmake >= 3.15).
# TODO: Add an install target to install the docs? Might be needed for RTD?

find_package(Doxygen COMPONENTS dot)

# We ignore powercap-rapl-sysfs.h since it's deprecated
list(APPEND DOC_HEADERS ${PROJECT_SOURCE_DIR}/inc/powercap.h
                        ${PROJECT_SOURCE_DIR}/inc/powercap-sysfs.h
                        ${PROJECT_SOURCE_DIR}/inc/powercap-rapl.h)

if (DOXYGEN_FOUND)
  # configure Doxyfile
  set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
  set(DOXYGEN_PROJECT_NUMBER ${PROJECT_VERSION})
  set(DOXYGEN_PROJECT_BRIEF "A generic C interface to the Linux power capping framework")
  # TODO: A better way to do this (each entry must be quoted in Doxyfile)
  foreach(hdr IN LISTS DOC_HEADERS)
    string(APPEND DOXYGEN_INPUT " \"${hdr}\"")
  endforeach()
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  # run doxygen
  file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIRECTORY})
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html)
  add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                     COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                     MAIN_DEPENDENCY ${DOXYFILE_OUT}
                     DEPENDS ${DOC_HEADERS} ${DOXYFILE_IN}
                     COMMENT "Generating documentation with Doxygen")
  add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})
endif()
