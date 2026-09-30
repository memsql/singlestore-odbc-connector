#ifndef MA_C_API_H
#define MA_C_API_H

/* Declarations shared with C translation units and the ODBC ABI. */
#ifdef __cplusplus
#define MADB_C_BEGIN extern "C" {
#define MADB_C_END }
#else
#define MADB_C_BEGIN
#define MADB_C_END
#endif

#endif /* MA_C_API_H */
