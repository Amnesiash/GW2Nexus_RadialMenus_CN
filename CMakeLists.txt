cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

project(gw2_radial_menus)

# 体积开关（默认精简：Os + gc-sections + strip；按原版基线对比时用 -DSLIM=OFF）
option(SLIM "optimize for size: -Os, -gc-sections, strip" ON)
option(NOSTRIP "keep symbols (for crash address resolution)" OFF)
option(DBGSYM "add -g debug info with release layout (for addr2line)" OFF)

if(DBGSYM)
    # 与 Release 布局一致，仅附加 -g，便于用崩溃偏移定位
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -g")
elseif(SLIM)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -Os -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -Os -ffunction-sections -fdata-sections")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")
endif()

include_directories(src src/thirdparty src/thirdparty/imgui)

# 资源编译（PNG 纹理打包进 DLL）
# 上游 .rc 为 UTF-16（MSVC 生成），MinGW windres 只吃 UTF-8：先 iconv 转码再编译
set(MINGW_BIN ${CMAKE_CURRENT_SOURCE_DIR}/../.tools/mingw64/bin)
set(RC_UTF8 ${CMAKE_CURRENT_BINARY_DIR}/GW2-RadialMenus.rc.utf8)
add_custom_command(
    OUTPUT ${RC_UTF8}
    COMMAND cmd /c "${MINGW_BIN}/iconv.exe -f UTF-16 -t UTF-8 ${CMAKE_CURRENT_SOURCE_DIR}/src/GW2-RadialMenus.rc > ${RC_UTF8}"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/GW2-RadialMenus.rc
    VERBATIM
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rc.obj
    COMMAND ${MINGW_BIN}/windres.exe
            -I${CMAKE_CURRENT_SOURCE_DIR}/src
            -I${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty
            --input ${RC_UTF8}
            --output ${CMAKE_CURRENT_BINARY_DIR}/rc.obj
            -O coff
    DEPENDS ${RC_UTF8}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    VERBATIM
)

add_library(radials SHARED
    src/entry.cpp
    src/imgui_extensions.cpp
    src/Shared.cpp
    src/Util.cpp
    src/Core/Addon.cpp
    src/Core/Conditions.cpp
    src/Core/ItemProcessor.cpp
    src/Core/RadialContext.cpp
    src/Core/RadialMenu.cpp
    src/Core/StateObserver.cpp
    src/Core/LanguageConfig.cpp
    src/thirdparty/ImAnimate/ImAnimate.cpp
    src/thirdparty/imgui/imgui.cpp
    src/thirdparty/imgui/imgui_draw.cpp
    src/thirdparty/imgui/imgui_tables.cpp
    src/thirdparty/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/rc.obj
)

# 界面文案包含简体中文：源文件为 UTF-8，且执行字符集也需按 UTF-8 编码
# (MSVC: /utf-8；MinGW/Clang: -finput-charset/-fexec-charset)
if(MSVC)
    target_compile_options(radials PRIVATE /utf-8)
else()
    target_compile_options(radials PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 与 vcxproj 一致的预处理定义
target_compile_definitions(radials PRIVATE GW2RADIALMENUS_EXPORTS _WINDOWS _USRDLL)

set_target_properties(radials PROPERTIES OUTPUT_NAME "Radial_Menus_CN" PREFIX "")
if(SLIM AND NOT NOSTRIP)
    # 仅在精简模式下去符号与丢弃未引用段
    target_link_options(radials PRIVATE -s -Wl,--gc-sections)
    add_custom_command(TARGET radials POST_BUILD
        COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:radials>
        VERBATIM
    )
endif()
