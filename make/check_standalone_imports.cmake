# Post-build check of the MSVC standalone executable (see make/CMakeLists_app.txt):
# objects created by libwinevbs must never reach the system OLE implementation, so the
# executable must not import from ole32/oleaut32/combase (see make/winevbs64-msvc.def).
#
# Usage: cmake -DDUMPBIN=<dumpbin.exe> -DEXE=<executable> -DDEF=<def file> -P check_standalone_imports.cmake

execute_process(
   COMMAND "${DUMPBIN}" /nologo /dependents "${EXE}"
   OUTPUT_VARIABLE _deps
   RESULT_VARIABLE _result
)
if(NOT _result EQUAL 0)
   message(FATAL_ERROR "dumpbin failed (${_result}) on ${EXE}")
endif()

string(TOLOWER "${_deps}" _deps_lower)
foreach(_dll ole32.dll oleaut32.dll combase.dll)
   string(REGEX MATCH "[ \t\r\n]${_dll}" _found "${_deps_lower}")
   if(_found)
      message(FATAL_ERROR "${EXE} imports from the system OLE library ${_dll}, see ${DEF}. Dependencies:\n${_deps}")
   endif()
endforeach()
