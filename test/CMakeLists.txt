IF (DIRECT_LINK_TESTS)
  MESSAGE(STATUS "Configuring tests for direct linking against the connector")
ELSE()
  MESSAGE(STATUS "Configuring tests for linking against the DM library")
ENDIF()

# Seems like libodbc needs it
IF(NOT WIN32)
  INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindIconv.cmake)
  IF (ICONV_EXTERNAL)
    SET(PLATFORM_DEPENDENCIES ${PLATFORM_DEPENDENCIES} ${ICONV_LIBRARIES})
  ENDIF()
  SET(PLATFORM_DEPENDENCIES ${PLATFORM_DEPENDENCIES} "pthread" "dl")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ENABLE_TESTING()

SET (ODBC_TESTS
        "basic" "types" "blob" "desc" "info" "tran" "catalog1" "catalog2"
        "use_result" "scroll" "bulk" "prepare" "datetime" "keys"
        "curext" "relative" "unicode" "cursor" "dyn_cursor"
        "error" "escape_sequences" "param" "result1" "result2" "multistatement"
        "sqlcolumns" "sql_procedure_columns" "sql_get_type_info" "sql_special_columns"
        "client_side" "sql_get_info" "sql_row_count" "sql_get_functions"
        "sql_get_diag_field" "sql_get_diag_rec" "sql_execute" "sql_buffers" "connect")
SET(COMMON_TEST_SOURCES tap.h)

# Interactive makes sense on WIN32 only atm
IF (WIN32 AND (BUILD_INTERACTIVE_TESTS OR USE_INTERACTIVE_TESTS))
  MESSAGE(STATUS "Configuring to build interactive test")
  SET (ODBC_TESTS ${ODBC_TESTS} "interactive")
ENDIF()
IF(NOT WIN32)
  SET(COMMON_TEST_SOURCES ${COMMON_TEST_SOURCES} "${CMAKE_SOURCE_DIR}/ma_conv_charset.c")
ENDIF()
# iOdbc has
IF(WITH_IODBC)
#  Doesn't work with newer Clang
# -Wno-pointer-sign: command not found
#  SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wno-pointer-sign")
ENDIF()
FOREACH (ODBC_TEST ${ODBC_TESTS})
  IF (${ODBC_TEST} STREQUAL "interactive")
    ADD_EXECUTABLE(odbc_${ODBC_TEST} ${ODBC_TEST}.c ${COMMON_TEST_SOURCES})
  ELSE()
    ADD_EXECUTABLE(odbc_${ODBC_TEST} ${ODBC_TEST}.c ${COMMON_TEST_SOURCES})
  ENDIF()
  FOREACH(CONNECTOR_DRIVER_TYPE_SHORT ${CONNECTOR_DRIVER_TYPES_SHORT})
    IF (DIRECT_LINK_TESTS)
      TARGET_LINK_LIBRARIES(odbc_${ODBC_TEST} ssodbc${CONNECTOR_DRIVER_TYPE_SHORT} ${PLATFORM_DEPENDENCIES})
    ELSE()
      TARGET_LINK_LIBRARIES(odbc_${ODBC_TEST} ${ODBC_LIBS} ${PLATFORM_DEPENDENCIES} ssodbc${CONNECTOR_DRIVER_TYPE_SHORT})
      ADD_DEPENDENCIES(odbc_${ODBC_TEST} ssodbc${CONNECTOR_DRIVER_TYPE_SHORT})
    ENDIF()
  ENDFOREACH(CONNECTOR_DRIVER_TYPE_SHORT)
  IF (NOT ${ODBC_TEST} STREQUAL "interactive" OR USE_INTERACTIVE_TESTS)
    ADD_TEST(odbc_${ODBC_TEST} ${EXECUTABLE_OUTPUT_PATH}/odbc_${ODBC_TEST})
    SET_TESTS_PROPERTIES(odbc_${ODBC_TEST} PROPERTIES TIMEOUT 240)
    IF (${ODBC_TEST} STREQUAL "connect")
      SET_TESTS_PROPERTIES(odbc_${ODBC_TEST} PROPERTIES TIMEOUT 480)
    ENDIF()
    IF(NOT WIN32)
      SET_PROPERTY(TEST odbc_${ODBC_TEST} APPEND PROPERTY
              ENVIRONMENT ODBCINI=${CMAKE_BINARY_DIR}/test/odbc.ini)
      SET_PROPERTY(TEST odbc_${ODBC_TEST} APPEND PROPERTY
              ENVIRONMENT ODBCSYSINI=${CMAKE_BINARY_DIR}/test)
    ENDIF()
  ENDIF()
  SET_TARGET_PROPERTIES(odbc_${ODBC_TEST} PROPERTIES C_STANDARD 90)
ENDFOREACH()

ADD_EXECUTABLE(odbc_connstring connstring.c ${CMAKE_SOURCE_DIR}/ma_dsn.c ${COMMON_TEST_SOURCES})
TARGET_LINK_LIBRARIES(odbc_connstring ${ODBC_LIBS} ${ODBC_INSTLIBS} mariadbclient ${PLATFORM_DEPENDENCIES})
FOREACH(CONNECTOR_DRIVER_TYPE_SHORT ${CONNECTOR_DRIVER_TYPES_SHORT})
  ADD_DEPENDENCIES(odbc_connstring ssodbc${CONNECTOR_DRIVER_TYPE_SHORT})
ENDFOREACH(CONNECTOR_DRIVER_TYPE_SHORT)
ADD_TEST(odbc_connstring ${EXECUTABLE_OUTPUT_PATH}/odbc_connstring)
SET_TESTS_PROPERTIES(odbc_connstring PROPERTIES TIMEOUT 240)
IF(NOT WIN32)
  SET_PROPERTY(TEST odbc_connstring APPEND PROPERTY
          ENVIRONMENT ODBCINI=${CMAKE_BINARY_DIR}/test/odbc.ini)
  SET_PROPERTY(TEST odbc_connstring APPEND PROPERTY
          ENVIRONMENT ODBCSYSINI=${CMAKE_BINARY_DIR}/test)
ENDIF()

ADD_EXECUTABLE(odbc_browser_auth browser_auth.c)
TARGET_LINK_LIBRARIES(odbc_browser_auth ssodbca ${PLATFORM_DEPENDENCIES})
