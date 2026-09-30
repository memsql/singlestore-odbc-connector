#include <ma_odbc.h>

#include <cstddef>
#include <type_traits>

/* These checks fail the build if a C handle or boundary structure stops being
   a standard-layout type or gains, loses, or reorders a field. */
static_assert(std::is_standard_layout<MADB_Drv>::value,
              "MADB_Drv must remain a C-compatible standard-layout type");
static_assert(offsetof(MADB_Drv, DriverName) == 0,
              "MADB_Drv::DriverName must remain the first field");
static_assert(offsetof(MADB_Drv, OdbcLibrary) > offsetof(MADB_Drv, DriverName),
              "MADB_Drv field order changed");
static_assert(offsetof(MADB_Drv, SetupLibrary) > offsetof(MADB_Drv, OdbcLibrary),
              "MADB_Drv field order changed");
static_assert(sizeof(MADB_Drv) == 3 * sizeof(char *),
              "MADB_Drv size changed");

static_assert(std::is_standard_layout<MADB_ERROR>::value,
              "MADB_ERROR must remain a C-compatible standard-layout type");
static_assert(std::is_standard_layout<MADB_Error>::value,
              "MADB_Error must remain a C-compatible standard-layout type");
static_assert(offsetof(MADB_Error, NativeError) > offsetof(MADB_Error, SqlState),
              "MADB_Error field order changed");
static_assert(offsetof(MADB_Error, SqlErrorMsg) > offsetof(MADB_Error, NativeError),
              "MADB_Error field order changed");
static_assert(offsetof(MADB_Error, PrefixLen) > offsetof(MADB_Error, SqlErrorMsg),
              "MADB_Error field order changed");
static_assert(offsetof(MADB_Error, ReturnValue) > offsetof(MADB_Error, PrefixLen),
              "MADB_Error field order changed");
static_assert(offsetof(MADB_Error, ErrRecord) > offsetof(MADB_Error, ReturnValue),
              "MADB_Error field order changed");
static_assert(offsetof(MADB_Error, ErrorNum) > offsetof(MADB_Error, ErrRecord),
              "MADB_Error field order changed");

static_assert(std::is_standard_layout<MADB_Env>::value,
              "MADB_Env must remain a C-compatible standard-layout type");
static_assert(offsetof(MADB_Env, cs) > offsetof(MADB_Env, Error),
              "MADB_Env field order changed");
static_assert(offsetof(MADB_Env, Dbcs) > offsetof(MADB_Env, cs),
              "MADB_Env field order changed");
static_assert(offsetof(MADB_Env, Trace) > offsetof(MADB_Env, Dbcs),
              "MADB_Env field order changed");
static_assert(offsetof(MADB_Env, TraceFile) > offsetof(MADB_Env, Trace),
              "MADB_Env field order changed");
static_assert(offsetof(MADB_Env, OdbcVersion) > offsetof(MADB_Env, TraceFile),
              "MADB_Env field order changed");
static_assert(offsetof(MADB_Env, OutputNTS) > offsetof(MADB_Env, OdbcVersion),
              "MADB_Env field order changed");
