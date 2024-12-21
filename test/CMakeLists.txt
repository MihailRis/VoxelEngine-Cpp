project(VoxelEngineTest)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

find_package(GTest)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(
  ${PROJECT_NAME}
  VoxelEngineSrc
  GTest::gtest_main
)

# HACK: copy res to test/ folder for fixing
# problem compatibility MultiConfig and non
# MultiConfig builds. Delete in future and
# use only root res folder
add_custom_command(
  TARGET ${PROJECT_NAME} 
  POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different 
          ${CMAKE_SOURCE_DIR}/res 
          $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
  )

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
