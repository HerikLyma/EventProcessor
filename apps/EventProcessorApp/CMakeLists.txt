# Set the project name to 'EventProcessorLib'.
set(PROJECT_NAME EventProcessorApp)

# Globally search for all source files with the extension .cpp.
file(GLOB_RECURSE SOURCES *.cpp)

# Globally search for all header files with the extension .h.
file(GLOB_RECURSE HEADERS *.h)

# Set libraries
# set(LIBS EventProcessorLib)

# Include the 'include' directory of the project's library dependency.
include_directories(include)

# Create a static library named after the project using the gathered sources and headers.
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

# Link the project library to its private dependencies specified in ${LIBS}.
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
