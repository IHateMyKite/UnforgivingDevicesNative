# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)

# Set your project name. This will be the name of your SKSE .dll file.
project(UDNative VERSION 1.0.0 LANGUAGES CXX)

# === Set source files ===
# Add new .cpp here, otherwise the build will end with link error
set(source_files ${source_files}
    src/UD_Config.cpp
    src/UD_GameEvents.cpp
    src/UD_MinigameEffect.cpp
    src/UD_RegisterPapyrus.cpp
    src/UD_Utility.cpp
    src/UD_UI.cpp
    src/UD_Inventory.cpp
    src/UD_Keywords.cpp
    src/UD_Animation.cpp
    src/UD_Updater.cpp
    src/UD_Lib.cpp
    src/UD_Skill.cpp
    src/UD_Modifiers.cpp
    src/UD_ActorSlotManager.cpp
    src/UD_Serialization.cpp
    src/UD_ControlManager.cpp
    src/UD_ModEvents.cpp
    src/UD_PlayerStatus.cpp
    src/UD_PapyrusDelegate.cpp
    src/UD_Materials.cpp
    src/OrgasmSystem/OrgasmEvents.cpp
    src/OrgasmSystem/OrgasmManager.cpp
    src/OrgasmSystem/OrgasmData.cpp
    src/OrgasmSystem/OrgasmConfig.cpp
)

# Setup your SKSE plugin as an SKSE plugin!
find_package(CommonLibSSE CONFIG REQUIRED)
add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${source_files} plugin.cpp) # <--- specifies plugin.cpp
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PROJECT_NAME} PRIVATE PCH.h) # <--- PCH.h is required!
target_include_directories(${PROJECT_NAME} PRIVATE include)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")