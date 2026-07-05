include(FetchContent)

FetchContent_Declare(utils-cpp
    GIT_REPOSITORY https://github.com/ihor-drachuk/utils-cpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(utils-cpp)