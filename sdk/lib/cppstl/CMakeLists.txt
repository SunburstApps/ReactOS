set(SOURCE
    algorithm.cpp
    any.cpp
    atomic.cpp
    barrier.cpp
    bind.cpp
    charconv.cpp
    chrono.cpp
    condition_variable_destructor.cpp
    condition_variable.cpp
    debug.cpp
    exception.cpp
    functional.cpp
    future.cpp
    hash.cpp
    ios.cpp
    ios.instantiations.cpp
    iostream.cpp
    locale.cpp
    memory.cpp
    mutex_destructor.cpp
    mutex.cpp
    new.cpp
    optional.cpp
    random_shuffle.cpp
    random.cpp
    regex.cpp
    shared_mutex.cpp
    stdexcept.cpp
    string.cpp
    strstream.cpp
    system_error.cpp
    thread.cpp
    typeinfo.cpp
    utility.cpp
    valarray.cpp
    variant.cpp
    vector.cpp

    experimental/memory_resource.cpp
    filesystem/directory_iterator.cpp
    filesystem/int128_builtins.cpp
    filesystem/operations.cpp
    support/win32/locale_win32.cpp
    support/win32/support.cpp
    support/win32/thread_win32.cpp
)

add_library(cppstl STATIC ${SOURCE})
target_include_directories(cppstl PUBLIC ${REACTOS_SOURCE_DIR}/sdk/include/c++)
target_include_directories(cppstl PRIVATE include)
target_compile_definitions(cppstl PRIVATE
    _LIBCPP_BUILDING_LIBRARY _LIBCPP_DISABLE_NEW_DELETE_DEFINITIONS
    _LIBCPP_LINK_PTHREAD_LIB _LIBCPP_LINK_RT_LIB _LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER
    _LIBCPP_DISABLE_MACRO_CONFLICT_WARNINGS _LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS
)

# libc++ emits a huge number of warnings, many of which I can do nothing about.
target_compile_options(cppstl PRIVATE -w)

if(GCC)
    target_compile_definitions(cppstl PRIVATE __GLIBCXX__)
endif()

if(MSVC)
    target_link_libraries(cppstl INTERFACE cpprt oldnames)
    # We set this properties through our INTERFACE library
    set_target_properties(cppstl PROPERTIES INTERFACE_WITH_CXX_STL TRUE)
endif()
