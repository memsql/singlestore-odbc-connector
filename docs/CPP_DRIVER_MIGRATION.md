# C++ driver migration plan

## Goal and scope

Migrate the SingleStore ODBC driver implementation from C to C++ without
changing its ODBC behavior, supported platforms, or public C ABI. The migration
covers the driver sources listed in `SS_ODBC_SOURCES` and the ANSI/Unicode
frontends in the root `CMakeLists.txt`.

The bundled Connector/C (`libmariadb`), vendored libraries, tests, setup
programs, and generated flex/bison sources remain C unless a concrete need to
change them is identified. Keeping these boundaries in C limits the review
surface and allows upstream code to be updated independently.

## Implementation status

* Phase 0 established the mixed C/C++ build and migrated `ma_driver`.
* The C linkage macros and layout checks are the next boundary pull request.
* `ma_debug` and `ma_environment` are migrated as separate language-only pull requests.

## Compatibility contract

Every migration PR must preserve these invariants:

* Exported ODBC symbols retain C linkage, calling conventions, names, and
  visibility on Linux, macOS, and Windows.
* Public and cross-language structures retain their field order, size, and
  alignment. A structure can become an implementation class only after its C
  layout is no longer exposed across a boundary.
* ANSI and Unicode drivers continue to build and pass the same test suite.
* The driver continues to interoperate with the C driver manager,
  Connector/C, authentication plugins, setup library, and generated parser.
* No C++ exception may cross an ODBC or C callback boundary. Until explicit
  boundary guards exist, migrated code must use the existing return/error
  conventions and non-throwing allocation paths.
* Memory must be released by the allocator family that created it. Existing
  buffers shared with C keep using the `MADB_*` allocation macros.
* A language-only conversion and an ownership/behavior refactor happen in
  separate commits or PRs so regressions can be attributed and reverted.

The initial baseline is C++14. It is supported by the repository's current
CMake minimum and compiler matrix. Raising the language level can be evaluated
separately after the migration is established.

## Delivery phases

### 0. Establish a mixed-language build

1. Enable C and C++ in the top-level CMake project.
2. Apply the same sanitizer instrumentation to both languages.
3. Set the driver target to require portable C++14 without compiler
   extensions.
4. Convert `ma_driver.c`, a small leaf module, to `ma_driver.cc`.
5. Give its C-facing declarations explicit `extern "C"` linkage.
6. Add C++ compiler packages to documented build prerequisites.

Exit criteria:

* ANSI and Unicode libraries build on Linux, macOS, and Windows.
* The existing test matrix passes unchanged.
* Exported-symbol lists are unchanged from `master`.

### 1. Lock down cross-language boundaries

1. Inventory ODBC exports from `mariadb-odbc-driver.def.in`, `maodbc.def`, and
   `odbc_3_api.c`; add an automated ABI check for symbol additions/removals.
2. Add a shared linkage macro/header for declarations consumed by both C and
   C++, including platform calling-convention attributes.
3. Add compile-time layout checks for handles and descriptors passed across C
   boundaries.
4. Document ownership and nullability for Connector/C, driver-manager, parser,
   and setup-library interfaces.
5. Add catch-all guards that translate unexpected exceptions into ODBC
   diagnostics before using throwing C++ facilities.

Exit criteria:

* CI detects symbol or layout drift.
* Every exported entry point and C callback has an explicit C linkage and
  exception boundary.

### 2. Convert stateless utilities

Convert one cohesive module per PR, initially preserving implementation and
tests:

1. `ma_common`, `ma_string`, and `ma_helper`
2. `ma_error` and `ma_debug`
3. `ma_parse`, `ma_typeconv`, and `ma_conv_charset`
4. `escape_sequences/ast` while leaving generated lexer/parser sources in C

After each language-only conversion, make focused follow-up changes that use
`constexpr`, scoped enums, small value types, and standard algorithms where
they improve safety without changing boundary layouts.

Exit criteria for each module:

* Both driver variants build with strict C++ diagnostics.
* Existing focused tests pass under AddressSanitizer and UndefinedBehaviorSanitizer.
* No new exported symbols or behavior changes are introduced.

### 3. Introduce internal ownership types

1. Wrap driver-owned strings, arrays, and Connector/C resources in
   single-purpose RAII types with explicit deleters.
2. Migrate result and descriptor internals (`ma_result`, `ma_desc`) while
   retaining C-compatible handle shells.
3. Migrate environment state (`ma_environment`) and make cleanup idempotent.
4. Replace `goto` cleanup only where RAII has made all partial-construction
   paths safe.

Exit criteria:

* Allocation-failure and early-return tests cover each migrated owner.
* Leak/undefined-behavior sanitizer runs are clean.
* Handle layout checks and C integration tests remain green.

### 4. Migrate statement execution

Split `ma_statement` into reviewed responsibilities before changing ownership:

1. Parameter binding and conversion
2. Prepared-statement lifecycle
3. Result fetching and rowset state
4. Metadata and diagnostics

Move each responsibility behind an internal C++ interface, keeping the
`MADB_Stmt` ABI shell and method table stable until all C callers are migrated.
Add regression tests for cancellation, partial initialization, array binding,
client/server prepared statements, and error paths.

Exit criteria:

* Statement tests pass in ANSI/Unicode and client/server prepared modes.
* Fault-injection cleanup tests and sanitizer runs are clean.

### 5. Migrate connection, DSN, and authentication

1. Convert `ma_dsn` parsing/serialization and add round-trip tests for all DSN
   options.
2. Convert `ma_connection` in slices: attributes, connect/disconnect,
   transactions, prompting, and diagnostics.
3. Convert `plugins/browser_auth` and `ma_fake_request`, preserving the plugin
   C API and platform behavior.
4. Convert POSIX and Windows platform adapters independently.

Exit criteria:

* Connection-string and DSN compatibility tests cover old and new binaries.
* Prompt/setup integration passes on Windows and authentication flows pass on
  all supported platforms.
* Connection failure at every initialization stage is leak-free.

### 6. Convert and harden the ODBC entry layer

1. Convert `odbc_3_api`, `ma_ansi`, and `ma_unicode` last, after their callees
   have stable C++ interfaces.
2. Keep exports as thin, non-throwing adapters that validate handles, dispatch
   internally, and translate errors.
3. Convert `ma_dll` only after Windows load/unload behavior is covered.
4. Remove obsolete C-only shims and method tables once no C production caller
   depends on them.

Exit criteria:

* Export sets and calling conventions match the pre-migration release.
* The full test and packaging matrix passes.
* Built packages load through unixODBC, iODBC, and the Windows Driver Manager.

### 7. Final cleanup

1. Remove temporary compatibility macros and mixed-language allowances.
2. Enable a consistent warning policy for C++ sources and fail CI on new
   warnings.
3. Run performance and binary-size comparisons for connect, prepare/execute,
   fetch, and metadata workloads; investigate material regressions.
4. Update contributor and architecture documentation.
5. Decide whether C++17 or newer is justified, as a separate compatibility
   change.

## Pull request and validation strategy

Keep migration PRs small and ordered: boundary infrastructure, language-only
conversion, then behavior-preserving modernization. Each PR should include:

* its module's ownership/boundary notes;
* focused tests for newly exposed error paths;
* ANSI and Unicode builds on every supported OS;
* exported-symbol and structure-layout checks;
* sanitizer results for Linux;
* a note confirming whether package contents or runtime dependencies changed.

If a phase causes a regression, revert the most recent module conversion
without reverting already-stable boundary infrastructure. Do not maintain a
long-lived parallel implementation: small mixed-language steps keep `master`
releasable and avoid two sources of truth.

## First PR

The first PR implements phase 0 only. `ma_driver` is deliberately used as the
pilot because it has three internal functions, one C caller, no exported ODBC
entry points, and simple ownership. Its implementation remains structurally
unchanged except for the explicit cast required by C++, making this PR a test
of toolchains, linkage, and packaging rather than a behavior rewrite.
