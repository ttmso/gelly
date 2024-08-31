include(FetchContent)

function(get_nvapi)
    # Since using NVAPI is an extremely explicit choice,
    # we don't include it in the submodules to improve build times.
    # So we will just fetch it here.
    FetchContent_Declare(
            nvapi
            GIT_REPOSITORY https://github.com/NVIDIA/nvapi.git
            GIT_TAG 4ba3384
    )

    # It has no CMakelists.txt, so we just fetch the path and return it.
    message(STATUS "Fetching NVAPI from GitHub")
    FetchContent_Populate(nvapi)
    message(STATUS "Fetched NVAPI from GitHub at ${nvapi_SOURCE_DIR}")
    set(NVAPI_PATH ${nvapi_SOURCE_DIR} PARENT_SCOPE)
endfunction()