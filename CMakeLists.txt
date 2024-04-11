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
    src/UD_ActorSlotManager.cpp
    src/UD_Serialization.cpp
    src/UD_ControlManager.cpp
    src/UD_ModEvents.cpp
    src/UD_PlayerStatus.cpp
    src/UD_Input.cpp
    src/UD_PapyrusDelegate.cpp
    src/UD_Materials.cpp
    src/UD_Diagnosis.cpp
    src/UD_Lockpick.cpp
    src/UD_FastTravelManager.cpp
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

# Copy localy compiled dll to bin folder
add_custom_command(TARGET UDNative POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_SOURCE_DIR}/build/release/UDNative.dll ${CMAKE_SOURCE_DIR}/SKSE/plugins)

# Copy pdb to bin folder. For now disabled as even ziped, this file is too big
add_custom_command(TARGET UDNative POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E tar "cf" ${CMAKE_SOURCE_DIR}/SKSE/plugins/UDNativePDB.zip --format=zip
                   -- ${CMAKE_SOURCE_DIR}/build/release/UDNative.pdb )

# Copy pdb to bin folder. This file is no under SC. Only reason for this is so its on the same place s dll after build
add_custom_command(TARGET UDNative POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_SOURCE_DIR}/build/release/UDNative.pdb ${CMAKE_SOURCE_DIR}/SKSE/plugins)