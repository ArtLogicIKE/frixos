# Writes a header with the current build timestamp. Invoked by a custom target
# on every build (see main/CMakeLists.txt) so the reported compile_time is
# always fresh, regardless of which source files changed.
string(TIMESTAMP BUILD_TS "%Y-%m-%d %H:%M:%S" UTC)
set(CONTENT "#pragma once\n#define BUILD_TIMESTAMP \"${BUILD_TS} UTC\"\n")

# Only rewrite when the value actually changes within a single configure, to
# avoid needless churn; the timestamp differs build-to-build so this still
# refreshes every build.
if(EXISTS "${OUTFILE}")
    file(READ "${OUTFILE}" EXISTING)
else()
    set(EXISTING "")
endif()
if(NOT EXISTING STREQUAL CONTENT)
    file(WRITE "${OUTFILE}" "${CONTENT}")
endif()
