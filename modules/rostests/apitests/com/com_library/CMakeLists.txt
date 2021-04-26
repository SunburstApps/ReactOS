spec2def(com_test_library.dll comtest.spec)
add_idl_headers(rostests_com_library_idl comtest.idl)
add_typelib(comtest.idl)

list(APPEND SOURCE
    CTestClass.cpp
    comtest.rc
    ${CMAKE_CURRENT_BINARY_DIR}/com_test_library_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/com_test_library.def
)

list(APPEND comtest_rc_deps
    ${CMAKE_CURRENT_BINARY_DIR}/comtest.tlb
    ${CMAKE_CURRENT_SOURCE_DIR}/comtest.rgs
)
set_source_files_properties(comtest.rc PROPERTIES OBJECT_DEPENDS "${comtest_rc_deps}")

add_library(com_test_library MODULE ${SOURCE})
target_link_libraries(com_test_library atl_classes cpprt uuid)
if(MSVC)
    target_link_libraries(com_test_library runtmchk)
endif()

add_dependencies(com_test_library rostests_com_library_idl)
add_importlibs(com_test_library advapi32 ole32 shlwapi shell32 msvcrt kernel32 ntdll)
add_rostests_file(TARGET com_test_library)
