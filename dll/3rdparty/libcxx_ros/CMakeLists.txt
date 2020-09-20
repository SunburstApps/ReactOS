set_cpp(WITH_RTTI WITH_EXCEPTIONS)

include_directories(include)
include_directories(${REACTOS_SOURCE_DIR}/sdk/include/c++)
add_definitions(-D_LIBCPP_BUILDING_LIBRARY)

set(SOURCE
    algorithm.cpp
    any.cpp
    bind.cpp
    charconv.cpp
    chrono.cpp
    condition_variable.cpp
    condition_variable_destructor.cpp
    debug.cpp
    exception.cpp
    functional.cpp
    future.cpp
    hash.cpp
    ios.cpp
    iostream.cpp
    locale.cpp
    memory.cpp
    mutex.cpp
    mutex_destructor.cpp
    new.cpp
    optional.cpp
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

# Use SHARED here so that CMake will create the implib for us.
# I don't want to do this manually due to all the mangled function names.
add_library(libcxx_ros SHARED ${SOURCE})
set_module_type(libcxx_ros win32dll)
add_importlibs(libcxx_ros msvcrt kernel32)
add_cd_file(TARGET libcxx_ros DESTINATION reactos/system32 FOR all)
