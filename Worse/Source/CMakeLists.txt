include(${CMAKE_SOURCE_DIR}/CMake/Config.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/Message.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

PrintHeader("Worse Build Configuration" 80)

# Collect dependencies
list(PREPEND CMAKE_PREFIX_PATH "${WORSE_ROOT_DIR}/Source/ThirdParty/VulkanMemoryAllocator")
MessageScope(CollectThirdPartyTargets THIRD_PARTY_TARGETS)

MessageScope(CollectEngineModules ENGINE_MODULE_TARGETS)

MessageScope(ResolveEngineModules ${ENGINE_MODULE_TARGETS} ${THIRD_PARTY_TARGETS})
