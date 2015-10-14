-- premake5 script for crawler - https://github.com/r-lyeh/crawler
-- rlyeh, public domain

newoption { trigger = "gen-licenses", description = "Generate LICENSES and update README.md" }

-- if _OPTIONS["android"] then
-- elseif
-- end

-- http://industriousone.com/scripting-reference

local action = _ACTION or ""

    --put these projects in a virtual folder named "premaked"
    --group "premaked"
    --    include "libs/audio/soloud/build/premake4.lua"
    --group ""

    --os.copyfile("tools/unix2dos/unix2dos.exe", ".build/")
    --put these projects in a virtual folder named "data"
    --group "data"
        --include "assets/"
        --dofile "assets/premake5.lua"
    --group ""

    solution "crawler"
    location (".prj")
    configurations { "Debug", "DebugOpt", "Release", "Shipping" }
    prebuildcommands { "premake5 vs2013" }
    platforms {"native", "x64", "x32"}
    language "C"
    targetdir (".obj")
    debugdir  (".obj")
    objdir    (".obj")
    kind "StaticLib"

    -- filter { "platforms:x32" }
    --    architecture "x32"

    -- filter { "platforms:x64" }
    --    architecture "x64"

-- taken from http://svn.wildfiregames.com/public/ps/trunk/build/premake/premake4.lua, MIT licensed
-- detect CPU architecture (simplistic, currently only supports x86, amd64 and ARM)
arch = "x86"
if _OPTIONS["android"] then
    arch = "arm"
elseif os.is("windows") then
    -- if string.find(os.getenv("PROCESSOR_ARCHITECTURE"), "64")~=nil or string.find(os.getenv("PROCESSOR_ARCHITEW6432"), "64")~=nil then
    if os.getenv("Platform")~=nil and string.find(os.getenv("Platform"), "64")~=nil then
        arch = "amd64"
    end
else
    arch = os.getenv("HOSTTYPE")
    if arch == "x86_64" or arch == "amd64" then
        arch = "amd64"
    else
        os.execute("gcc -dumpmachine > .gccmachine.tmp")
        local f = io.open(".gccmachine.tmp", "r")
        local machine = f:read("*line")
        f:close()
        if string.find(machine, "x86_64") == 1 or string.find(machine, "amd64") == 1 then
            arch = "amd64"
        elseif string.find(machine, "i.86") == 1 then
            arch = "x86"
        elseif string.find(machine, "arm") == 1 then
            arch = "arm"
        else
            print("WARNING: Cannot determine architecture from GCC, assuming x86")
        end
    end
end

if arch == "x86" then
    tag = "x86"
elseif arch == "amd64" then
    tag = "x64"
else
    tag = "arm"
end

if os.is64bit() then
   print("This is a 64-bit system")
else
   print("This is NOT a 64-bit system")
end

-- print("[ OK ] Build arch-"..tag)

FBX_SDK_ROOT = os.getenv("FBX_SDK_ROOT")
if FBX_SDK_ROOT then
    -- avert your eyes children!
    if string.find(_ACTION, "xcode") then
        -- TODO: i'm sure we could do some string search+replace trickery to make
        --       this more general-purpose
        -- take care of the most common case where the FBX SDK is installed to the
        -- default location and part of the path contains a space
        -- god help you if you install the FBX SDK using a totally different path
        -- that contains a space AND you want to use Xcode
        -- Premake + Xcode combined fuck this up so badly making it nigh-impossible
        -- to do any kind of _proper_ path escaping here (I wasted an hour on this)
        -- (maybe I should have used CMake ....)
        FBX_SDK_ROOT = string.gsub(FBX_SDK_ROOT, "FBX SDK", "'FBX SDK'")
    end
    includedirs {
        (FBX_SDK_ROOT .. "/include"),
    }
    defines {
        "FBXSDK_NEW_API",
    }
end



--    filter { "platforms:Win32" }
--        system "Windows"
--        architecture "x32"

--    filter { "platforms:Win64" }
--        system "Windows"
--        architecture "x64"

    filter { "system:unix" }
        buildoptions "-std=c++11 -pthread"

    filter { "system:macosx and (**.cpp or **.cc or **.cxx)" }
        buildoptions { "-std=c++11" }
    filter { "system:macosx and (**.c)" }
        buildoptions { "-std=c99" }
    filter { "system:macosx" }
        linkoptions "-stdlib=libc++"

    filter "action:vs*"
        defines { "_SCL_SECURE_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS", "NOMINMAX" }
        flags {  "NoEditAndContinue", "Maps" } -- "StaticRuntime" }  --NoMinimalRebuild
--      vectorextensions "SSE" -- "SSE2" -- vs2013 pro only
--      flags { "ExtraWarnings", "FatalWarnings", "MultiProcessorCompile", }
--      flags { "NoRTTI" }
--      flags( "NoExceptions" )
--      defines { "_SECURE_SCL=0", "_SECURE_SCL_THROWS=0" } -- @master?
--      defines { "_WIN32_WINNT=0x0501"} -- needed for boost
        buildoptions {
            "/wd4018", -- '<' : signed/unsigned mismatch
            "/wd4099", -- PDB 'xxx.pdb' was not found with ...\yyy.pdb'; linking object as if no debug info (issue with 3rdparty libs)
            "/wd4100", -- unreferenced formal parameter
            "/wd4127", -- conditional expression is constant
            "/wd4244", -- 'initializing' : conversion from 'X' to 'Y', possible loss of data
            "/wd4267", -- conversion from 'T' to 'U', possible loss of data
            "/wd4309", -- truncation of constant value
            "/wd4503", -- decorated name length exceeded, name was truncated
            "/wd4530", -- C++ exception handler used, but unwind semantics are not enabled.
            "/wd4800", -- forcing value to bool 'true' or 'false' (performance warning)
            "/wd4996", -- this function or variable may be unsafe. Consider using asctime_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS.
        }

    filter "Debug"
        targetdir (".obj")
        defines { "DEBUG","_DEBUG", "HAS_DEBUGINFO=3" }
        flags { --[["NoRTTI",]] "NoExceptions", --[["StaticRuntime",]] "Symbols" }
        floatingpoint "Fast" -- "Strict" raises errors on stb :(
        optimize "Off"
        targetsuffix "-d"

    filter "DebugOpt"
        targetdir (".obj")
        defines { "DEBUG","_DEBUG", "HAS_DEBUGINFO=2", "HAS_OPTIMIZATIONS=1", }
        flags { --[["NoRTTI",]] "NoExceptions", "StaticRuntime", "Symbols", "NoBufferSecurityCheck", "NoRuntimeChecks" }
        floatingpoint "Fast"
        optimize "Debug"
        targetsuffix "-do"

    filter "Release"
        targetdir (".obj")
        defines { "NDEBUG","_NDEBUG", "HAS_DEBUGINFO=1", "HAS_OPTIMIZATIONS=2" }
        flags { --[["NoRTTI",]] "NoExceptions", --[["StaticRuntime",]] "Symbols", "NoBufferSecurityCheck", "NoRuntimeChecks" }
        floatingpoint "Fast"
        optimize "On"
        targetsuffix "-r"

    filter "Shipping"
        targetdir (".obj")
        defines { "NDEBUG","_NDEBUG","RETAIL","SHIPPING","MASTER","GOLD", "HAS_OPTIMIZATIONS=3" }
        flags { --[["NoRTTI",]] "NoExceptions", "StaticRuntime", --[["Symbols",]] "NoBufferSecurityCheck", "NoRuntimeChecks", "NoFramePointer", "LinkTimeOptimization" }
        floatingpoint "Fast"
        optimize "Full"
        targetsuffix "-sh"


    --[[
    filter "system:windows"
       prebuildcommands { "copy /y tools\\unix2dos\\unix2dos.exe .build\\" }
    ]]


    -- excerpt from libgdx/fbx-conv {
    -- TODO: Because there are a few remaining things ...
    --    1. x86/x64 switching
    --    2. clean this file up because I'm sure it could be organized better
    --    3. consider maybe switching to CMake because of the ugly hack below
    --
    -- NOTE: I am intentionally leaving out a "windows+gmake" configuration
    --       as trying to compile against the FBX SDK using MinGW results in
    --       compile errors. Some quick googling seems to indicate MinGW is
    --       not supported by the FBX SDK?
    --       If you try to use this script to build with MinGW you will end
    --       up with a Makefile that has god knows what in it
    if FBX_SDK_ROOT then
        --- VISUAL STUDIO --------------------------------------------------
        configuration "vs*"
            defines {
                "_CRT_SECURE_NO_WARNINGS",
                "_CRT_NONSTDC_NO_WARNINGS"
            }
            libdirs {
                (FBX_SDK_ROOT .. "/lib/vs2013/x86"),
            }
        configuration { "vs*", "Debug*" }
            links {
                "libfbxsdk-md",
            }
            libdirs {
                (FBX_SDK_ROOT .. "/lib/vs2013/x86/debug"),
            }
        configuration { "vs*", "Re*" }
            links {
                "libfbxsdk-mt",
            }
            libdirs {
                (FBX_SDK_ROOT .. "/lib/vs2013/x86/release"),
            }
        --- LINUX ----------------------------------------------------------
        configuration { "linux" }
            -- TODO: while using x64 will likely be fine for most people nowadays,
            --       we still need to make this configurable
            libdirs {
            }
            links {
                "pthread",
                "fbxsdk",
                "dl",
            }
        configuration { "linux", "Debug*" }
            libdirs {
                (FBX_SDK_ROOT .. "/lib/gcc4/x64/debug"),
            }
        configuration { "linux", "Re*" }
            libdirs {
                (FBX_SDK_ROOT .. "/lib/gcc4/x64/release"),
            }
        --- MAC ------------------------------------------------------------
        configuration { "macosx" }
            libdirs {
                (FBX_SDK_ROOT .. "/lib/gcc4/ub"),
            }
            links {
                "CoreFoundation.framework",
                "fbxsdk",
            }
        configuration { "macosx", "Debug*" }
            libdirs {
                (FBX_SDK_ROOT .. "/lib/gcc4/ub/debug"),
            }
        configuration { "macosx", "Re*" }
            libdirs {
                (FBX_SDK_ROOT .. "/lib/gcc4/ub/release"),
            }
        printf("[ XX ] FBX SDK compilation support (FBX_SDK_ROOT environment variable found)")
    end
    -- } end of excerpt




    -- GL Extensions
    project "glew"
        language "C"
        includedirs {
            "libs/extensions/glew/include/",
        }
        files {
            "libs/extensions/glew/include/GL/*.h",
            "libs/extensions/glew/src/glew.c"
        }
        defines "GLEW_STATIC"

    project "gl3w"
        language "C"
        includedirs {
            "libs/extensions/gl3w/",
        }
        files {
            "libs/extensions/gl3w/gl3w.c"
        }

    project "glxw"
        language "C"
        includedirs {
            "libs/extensions/glxw/include/",
        }
        files {
            "libs/extensions/glxw/include/**.h",
            "libs/extensions/glxw/src/glxw.c",
            os.is("windows") and "libs/extensions/glxw/src/glxw_wgl.c" or "",
        }
        defines {
            os.is("windows") and "WIN32" or "",
        }

    -- GL/Frameworks
    project "glwt"
        language "C"
        includedirs {
            "libs/extensions/glxw/include/",
            "libs/frameworks/glwt/include/",
            "libs/frameworks/glwt/src/",
        }
        files {
            "libs/frameworks/glwt/src/common/**.c",
            "libs/frameworks/glwt/src/win32/**.c",
            "libs/frameworks/glwt/src/wgl/**.c",
        }
        defines {
            os.is("windows") and "WIN32" or "",
        }

    project "GLFW3"
        language "C"
        includedirs { "libs/frameworks/glfw3/include" }
        files {
            "libs/frameworks/glfw3/include/GLFW/*.h",
            "libs/frameworks/glfw3/src/context.c",
            "libs/frameworks/glfw3/src/init.c",
            "libs/frameworks/glfw3/src/input.c",
            --"libs/frameworks/glfw3/src/joystick.c",
            "libs/frameworks/glfw3/src/monitor.c",
            --"libs/frameworks/glfw3/src/time.c",
            "libs/frameworks/glfw3/src/window.c",
        }
        defines { "_GLFW_USE_OPENGL" }
        configuration "windows"
            defines { "_GLFW_WIN32", "_GLFW_WGL" }
            files {
                "libs/frameworks/glfw3/src/win32*.c", -- posix_* mach_time x11_* xkb_* wl_*
                "libs/frameworks/glfw3/src/wgl_context.c", -- egl_ glx_
                "libs/frameworks/glfw3/src/winmm_joystick.c", -- linux_joystick
            }

    project "SFML2"
        language "C++"
        includedirs {
            "libs/extensions/glew/include",
            "libs/frameworks/SFML2/include",
            "libs/frameworks/SFML2/src",
            "libs/frameworks/SFML2/extlibs/headers/AL",
            "libs/frameworks/SFML2/extlibs/headers/jpeg/",
            os.is("windows") and "libs/frameworks/SFML2/extlibs/headers/libsndfile/windows" or "",
            os.is("windows") and "libs/frameworks/SFML2/extlibs/headers/libfreetype/windows" or "",
         }
        files {
            "libs/frameworks/SFML2/src/SFML/Audio/*.cpp",
            "libs/frameworks/SFML2/src/SFML/Graphics/**.c*",
            "libs/frameworks/SFML2/src/SFML/Main/*.cpp",
            "libs/frameworks/SFML2/src/SFML/Network/*.cpp",
            "libs/frameworks/SFML2/src/SFML/System/*.cpp",
            "libs/frameworks/SFML2/src/SFML/Window/*.cpp",
            "patches/sfml2/ImageLoader.cpp",
        }
        excludes {
            "libs/frameworks/SFML2/src/SFML/Graphics/ImageLoader.cpp",
        }
        defines { "SFML_STATIC", "GLEW_STATIC", "STBI_FAILURE_USERMSG" }
        configuration "windows"
            files {
                "libs/frameworks/SFML2/src/SFML/Network/Win32/*.cpp",
                "libs/frameworks/SFML2/src/SFML/System/Win32/*.cpp",
                "libs/frameworks/SFML2/src/SFML/Window/Win32/*.cpp",
            }

    project "freeglut"
        language "C"
        includedirs { "libs/frameworks/freeglut/include" }
        files {
            "libs/frameworks/freeglut/src/*.c", --freeglut.c",
        }
        defines {
            "FREEGLUT_STATIC",
            "FREEGLUT_LIB_PRAGMAS=0",
        }

    -- GUI

    project "nanovg"
        language "C"
        includedirs { "libs/render/nanovg/src", }
        files { "libs/render/nanovg/src/*.c", }

    project "stb"
        language "C"
        includedirs { "libs/data/stb" }
        files {
            "libs/data/stb/stb/*.h",
            "libs/data/stb/*.h",
            "libs/data/stb/stb_lib.c",
            "libs/data/stb/stb/stb_image.c",
            "libs/data/stb/stb/stb_image_write.c",
        }

    project "imgui"
        language "C++"
        includedirs {
            "libs/extensions/glew/include",
            "libs/ui/imgui/",
        }
        files {
            "libs/ui/imgui/*.h*",
            "libs/ui/imgui/**.cpp",
        }

    project "nfd" -- native file dialog
        language "C++"
        includedirs {
            "libs/ui/nativefiledialog/src/include",
        }
        files {
            "libs/ui/nativefiledialog/src/*.h",
            "libs/ui/nativefiledialog/src/nfd_common.c",
            os.is("windows") and "libs/ui/nativefiledialog/src/nfd_win.cpp" or "",
            os.is("macosx")  and "libs/ui/nativefiledialog/src/nfd_cocoa.m" or "",
            os.is("linux")   and "libs/ui/nativefiledialog/src/nfd_gtk.c"   or "",
        }

    -- AUDIO

    project "soloud"
        language "C++"
        includedirs {
            "libs/audio/soloud/include",
            "libs/audio/soloud/ext/libmodplug/src",
            "libs/audio/soloud/ext/libmodplug/src/libmodplug",
        }
        files {
            "libs/audio/soloud/src/audiosource/**.c*",
            os.is("windows") and "libs/audio/soloud/src/backend/winmm/*.c*" or "",
            os.is("macosx") and "libs/audio/soloud/src/backend/openal/*.c*" or "",
            "libs/audio/soloud/src/core/*.c*",
            "libs/audio/soloud/src/filter/*.c*",
            "libs/audio/soloud/ext/libmodplug/src/*.c*"
        }
        defines {
            "MODPLUG_STATIC",
            "WITH_MODPLUG",
            os.is("windows") and "WITH_WINMM" or "",
            os.is("macosx") and "WITH_OPENAL" or "",
            os.is("macosx") and "HAVE_SINF" or "",
            os.is("macosx") and "HAVE_SETENV" or "",
        }

    project "musepack"
        language "C++"
        includedirs {
            "libs/audio/wave/deps/musepack/include",
        }
        files {
            "libs/audio/wave/deps/musepack/libmpcdec/**.c*",
        }
        defines {
        }

    -- NETWORK
    project "enet"
        language "C"
        includedirs {
            "libs/network/enet/btgui/enet/include/",
        }
        files {
            "libs/network/enet/btgui/enet/**.h",
            "libs/network/enet/btgui/enet/**.c",
        }
        defines {
            os.is("windows") and "WIN32" or "",
        }

    -- DATABASES
    project "sqlite3"
        language "C"
        includedirs {
            "libs/databases/sqlite3/",
        }
        files {
            "libs/databases/sqlite3/sqlite3.c",
        }
        defines {
            "SQLITE_HAVE_ISNAN"
        }

    -- CREATIVE
    project "sketch2d"
        language "C++"
        includedirs {
            "libs/",
            "libs/extensions/glew/include/",
            "libs/frameworks/glfw3/include/",
            "libs/render/nanovg/src/",
            "libs/data/stb/stb",
            "libs/image/spot/redist/deps/soil2/",
        }
        files {
            "libs/creative/sketch2d/sketch2d.h",
            "libs/creative/sketch2d/sketch2d.cpp",
        }
        defines "GLEW_STATIC"
        defines "NANOVG_GL2_IMPLEMENTATION"

    -- IMMEDIATE MODE

    project "glim"
        kind "StaticLib"
        language "C++"
        includedirs {
            "libs/extensions/glew/include",
            "libs/render/glim",
        }
        files {
            "libs/render/glim/*.h*",
            "libs/render/glim/*.c*",
        }
        defines "GLEW_STATIC"

    -- UTILS

    project "pugixml"
        kind "StaticLib"
        language "C++"
        includedirs {
            "libs/xml/pugixml/src/",
        }
        files {
            "libs/xml/pugixml/src/*.*pp",
        }
        defines "PUGIXML_NO_EXCEPTIONS"

    -- SCRIPT

    project "lua"
        kind "StaticLib"
        language "C"
        --method #1
        --os.copyfile("libs/script/lua/src/luaconf.h.orig", "libs/script/lua/src/luaconf.h")
        includedirs {
            "libs/script/lua/src/",
            "patches/lua/", -- method #2, luaconf.h
        }
        files {
            "libs/script/lua/src/*.h",
            "libs/script/lua/src/*.c",
        }
        excludes {
            "libs/script/lua/src/lua.c",
            "libs/script/lua/src/luac.c",
            "libs/script/lua/src/wmain.c",
            "libs/script/lua/src/loadlib_rel.c",
        }
        defines {
        }

    -- PHYSICS

    --[[
    project "bullet"
        os.copyfile("patches/bullet3/b3Vector3.cpp", "libs/physics/bullet3/src/Bullet3Common/b3Vector3.cpp")
        kind "StaticLib"
        language "C++"
        includedirs {
            "libs/physics/bullet3/src",
        }
        files {
            "libs/physics/bullet3/src/**.h",
            "libs/physics/bullet3/src/**.cpp",
        }
        excludes {
            "libs/physics/bullet3/src/Bullet3OpenCL/**.cpp",
        }
    ]]

    project "bullet"
        kind "StaticLib"
        language "C++"
        includedirs {
            "libs/physics/bullet2",
        }
        files {
            "libs/physics/bullet2/**.h",
            "libs/physics/bullet2/**.cpp",
        }

    -- ANIMATION

    project "spine"
        kind "StaticLib"
        language "C"
        includedirs {
            "libs/animation/spine-runtimes/spine-c/include",
        }
        files {
            "libs/animation/spine-runtimes/spine-c/src/**.c",
        }

    project "assimp"
        kind "StaticLib"
        language "C++"
        files {
            "libs/mesh/assimp/code/**.h",
            "libs/mesh/assimp/code/**.cpp",
            "libs/mesh/assimp/code/**.cpp",
            "libs/mesh/assimp/include/**.h",
            "libs/mesh/assimp/include/**.inl",
            "libs/mesh/assimp/include/**.hpp",
        }
        -- contrib libraries
        files {
            "libs/mesh/assimp/contrib/clipper/**.h",
            "libs/mesh/assimp/contrib/clipper/**.cpp",
            "libs/mesh/assimp/contrib/ConvertUTF/**.h",
            "libs/mesh/assimp/contrib/ConvertUTF/**.c",
            "libs/mesh/assimp/contrib/irrXML/**.h",
            "libs/mesh/assimp/contrib/irrXML/**.cpp",
            "libs/mesh/assimp/contrib/poly2tri/poly2tri/**.h",
            "libs/mesh/assimp/contrib/poly2tri/poly2tri/**.cc",
            "libs/mesh/assimp/contrib/unzip/**.h",
            "libs/mesh/assimp/contrib/unzip/**.c",
            "libs/mesh/assimp/contrib/zlib/**.h",
            "libs/mesh/assimp/contrib/zlib/**.c",
        }
        includedirs {
            "libs/mesh/assimp/code/BoostWorkaround/",
            "patches/assimp/",
        }
        defines {
            -- importers
            --"ASSIMP_BUILD_NO_X_IMPORTER",
            "ASSIMP_BUILD_NO_3DS_IMPORTER",
            --"ASSIMP_BUILD_NO_MD3_IMPORTER",
            "ASSIMP_BUILD_NO_MDL_IMPORTER",
            "ASSIMP_BUILD_NO_MD2_IMPORTER",
            "ASSIMP_BUILD_NO_PLY_IMPORTER",
            "ASSIMP_BUILD_NO_ASE_IMPORTER",
            --"ASSIMP_BUILD_NO_OBJ_IMPORTER",
            "ASSIMP_BUILD_NO_HMP_IMPORTER",
            --"ASSIMP_BUILD_NO_SMD_IMPORTER",
            "ASSIMP_BUILD_NO_MDC_IMPORTER",
            --"ASSIMP_BUILD_NO_MD5_IMPORTER",
            "ASSIMP_BUILD_NO_STL_IMPORTER",
            "ASSIMP_BUILD_NO_LWO_IMPORTER",
            "ASSIMP_BUILD_NO_DXF_IMPORTER",
            --"ASSIMP_BUILD_NO_NFF_IMPORTER",
            "ASSIMP_BUILD_NO_RAW_IMPORTER",
            "ASSIMP_BUILD_NO_OFF_IMPORTER",
            "ASSIMP_BUILD_NO_AC_IMPORTER",
            "ASSIMP_BUILD_NO_BVH_IMPORTER",
            "ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
            "ASSIMP_BUILD_NO_IRR_IMPORTER",
            "ASSIMP_BUILD_NO_Q3D_IMPORTER",
            "ASSIMP_BUILD_NO_B3D_IMPORTER",
            --"ASSIMP_BUILD_NO_COLLADA_IMPORTER",
            "ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
            "ASSIMP_BUILD_NO_CSM_IMPORTER",
            "ASSIMP_BUILD_NO_3D_IMPORTER",
            "ASSIMP_BUILD_NO_LWS_IMPORTER",
            --"ASSIMP_BUILD_NO_OGRE_IMPORTER",
            "ASSIMP_BUILD_NO_MS3D_IMPORTER",
            "ASSIMP_BUILD_NO_COB_IMPORTER",
            --"ASSIMP_BUILD_NO_BLEND_IMPORTER",
            "ASSIMP_BUILD_NO_NDO_IMPORTER",
            -- options
            --"ASSIMP_BUILD_NO_CALCTANGENTS_PROCESS",
            --"ASSIMP_BUILD_NO_JOINVERTICES_PROCESS",
            --"ASSIMP_BUILD_NO_MAKELEFTHANDED_PROCESS",
            --"ASSIMP_BUILD_NO_FLIPUVS_PROCESS",
            --"ASSIMP_BUILD_NO_FLIPWINDINGORDER_PROCESS",
            --"ASSIMP_BUILD_NO_TRIANGULATE_PROCESS",
            --"ASSIMP_BUILD_NO_GENFACENORMALS_PROCESS",
            --"ASSIMP_BUILD_NO_GENVERTEXNORMALS_PROCESS",
            --"ASSIMP_BUILD_NO_REMOVEVC_PROCESS",
            --"ASSIMP_BUILD_NO_SPLITLARGEMESHES_PROCESS",
            --"ASSIMP_BUILD_NO_PRETRANSFORMVERTICES_PROCESS",
            --"ASSIMP_BUILD_NO_LIMITBONEWEIGHTS_PROCESS",
            --"ASSIMP_BUILD_NO_VALIDATEDS_PROCESS",
            --"ASSIMP_BUILD_NO_IMPROVECACHELOCALITY_PROCESS",
            --"ASSIMP_BUILD_NO_FIXINFACINGNORMALS_PROCESS",
            --"ASSIMP_BUILD_NO_REMOVE_REDUNDANTMATERIALS_PROCESS",
            --"ASSIMP_BUILD_NO_FINDINVALIDDATA_PROCESS",
            --"ASSIMP_BUILD_NO_FINDDEGENERATES_PROCESS",
            --"ASSIMP_BUILD_NO_SORTBYPTYPE_PROCESS",
            --"ASSIMP_BUILD_NO_GENUVCOORDS_PROCESS",
            --"ASSIMP_BUILD_NO_TRANSFORMTEXCOORDS_PROCESS",
            --"ASSIMP_BUILD_NO_FINDINSTANCES_PROCESS",
            --"ASSIMP_BUILD_NO_OPTIMIZEMESHES_PROCESS",
            --"ASSIMP_BUILD_NO_OPTIMIZEGRAPH_PROCESS",
        }

    project "spine-sfml"
        kind "StaticLib"
        language "C++"
        includedirs {
            "libs/frameworks/SFML2/include",
            "libs/animation/spine-runtimes/spine-c/include",
            "libs/animation/spine-runtimes/spine-sfml/src",
        }
        files {
            "libs/animation/spine-runtimes/spine-sfml/**.h",
            "libs/animation/spine-runtimes/spine-sfml/src/**.cpp",
        }
        defines { "SFML_STATIC" }

    -- DUMMY
    project "dummy"
        kind "StaticLib"
        language "C++"
        files {"libs/dummy.cpp"}

    -- LET'S START
    function create_project( folder, example_path )
        example_path = string.sub(example_path, string.len(folder) + 1);
        project (example_path)
            kind "ConsoleApp"
            language "C++"

            --warnings "extra"
            targetdir (folder .. example_path)
            flags {
                -- "StaticRuntime",
            }  --NoMinimalRebuild
            files {
                folder .. example_path .. "/**.h",
                folder .. example_path .. "/**.hh",
                folder .. example_path .. "/**.hpp",
                folder .. example_path .. "/**.hxx",

                folder .. example_path .. "/**.c",
                folder .. example_path .. "/**.cc",
                folder .. example_path .. "/**.cpp",
                folder .. example_path .. "/**.cxx",

                folder .. example_path .. "/**.inl",
                folder .. example_path .. "/**.ipp",

                folder .. example_path .. "/**.mm",
            }

            if os.is("windows") then
            -- execute build version tool
            -- os.execute("tools/buildver/buildver.exe "..folder..example_path.."/build.h")
            -- debug & release
            os.copyfile("libs/frameworks/SDL2/lib/"..tag.."/SDL2.dll", folder..example_path.."/SDL2.dll")
            --[[
            os.copyfile("libs/frameworks/SDL2/extlibs/bin/"..tag.."/openal32.dll", "bin/openal32.dll")
            os.copyfile("libs/frameworks/SDL2/extlibs/bin/"..tag.."/libsndfile-1.dll", "bin/libsndfile-1.dll")
            ]]
            end

            if os.isfile( folder..example_path.."/pch.h" ) then
                --pchheader folder..example_path.."/pch.h"
                --pchsource folder..example_path.."/pch.cpp"
            end

            includedirs {
                folder .. example_path .. "/",
                folder .. example_path .. "/*/",
                folder .. example_path .. "/3rd-party/",
                folder .. example_path .. "/3rd-party/*/",
                folder .. example_path .. "/3rd/",
                folder .. example_path .. "/3rd/*/",
                folder .. example_path .. "/3rdparty/",
                folder .. example_path .. "/3rdparty/*/",
                folder .. example_path .. "/dep/",
                folder .. example_path .. "/dep/*/",
                folder .. example_path .. "/dependencies/",
                folder .. example_path .. "/dependencies/*/",
                folder .. example_path .. "/deps/",
                folder .. example_path .. "/deps/*/",
                folder .. example_path .. "/ext/",
                folder .. example_path .. "/ext/*/",
                folder .. example_path .. "/external/",
                folder .. example_path .. "/external/*/",
                folder .. example_path .. "/externals/",
                folder .. example_path .. "/externals/*/",
                folder .. example_path .. "/exts/",
                folder .. example_path .. "/exts/*/",
                folder .. example_path .. "/inc/",
                folder .. example_path .. "/inc/*/",
                folder .. example_path .. "/include/",
                folder .. example_path .. "/include/*/",
                folder .. example_path .. "/includes/",
                folder .. example_path .. "/includes/*/",
                folder .. example_path .. "/incs/",
                folder .. example_path .. "/incs/*/",
                folder .. example_path .. "/third-party/",
                folder .. example_path .. "/third-party/*/",
                folder .. example_path .. "/thirdparty/",
                folder .. example_path .. "/thirdparty/*/",
                folder .. example_path .. "/contrib/",
                folder .. example_path .. "/contrib/*/",
                folder .. example_path .. "/contribs/",
                folder .. example_path .. "/contribs/*/",
                folder .. example_path .. "/vendor/",
                folder .. example_path .. "/vendor/*/",
                folder .. example_path .. "/vendors/",
                folder .. example_path .. "/vendors/*/",

                --"addons/**/",

                "libs/",
                "libs/*/",
                "libs/mesh/assimp/include/",
                "libs/audio/soloud/include/",
                "libs/math/glm/",
                "libs/math/eigen/",
                "libs/frameworks/freeglut/include/",
                "libs/frameworks/glfw3/include/",
                "libs/frameworks/SFML2/include/",
                "libs/frameworks/SDL2/include/",
                "libs/extensions/glxw/include/",
                "libs/extensions/glew/include/",
                "libs/extensions/gl3w/",
                "libs/render/nanovg/src/",
                "libs/creative/sketch2d/",
                "libs/xml/pugixml/src/",

                "libs/script/lua/src/",
                "patches/lua/",

                "libs/animation/spine-runtimes/spine-c/include/",
                "libs/animation/spine-runtimes/spine-sfml/src/",

                "libs/physics/bullet2/",
                "libs/physics/bullet3/src/",

                "libs/databases/sqlite3/",

                "libs/extensions/glxw/include/",
                "libs/frameworks/glwt/include/",
            }

            libdirs {
                --"addons/**/",
                ".obj/",

                "libs/frameworks/SDL2/lib/" .. tag,
                "libs/frameworks/SFML2/extlibs/libs-msvc/" .. tag .. "/",
            }

            function lib( name )
                return not os.isfile( folder..example_path.."/@"..name ) and name or "dummy"
            end
            function oslib( os_name, name )
                return os.is( os_name ) and name or "dummy"
            end

            links {
                lib("GLFW3"),
                lib("glew"),
                lib("gl3w"),
                lib("nanovg"),
                lib("stb"),
                lib("sketch2d"),
                lib("soloud"),
                lib("imgui"),
                lib("enet"),
                lib("pugixml"),
                lib("lua"),

                lib("spine"),
                lib("spine-sfml"),

                lib("glxw"),
                lib("glwt"),

                lib("bullet"),

                lib("sqlite3"),

                lib("SDL2"),

                lib("SFML2"), lib("freetype"), lib("jpeg"), lib("openal32"), lib("sndfile"), oslib("windows", "winmm"),

                oslib("windows", "OpenGL32"),

                lib("freeglut"), oslib("windows", "glu32"),

                lib("assimp"),

                lib("nfd"),
            }

            defines {
                "GLEW_STATIC",
                "SFML_STATIC",
                "FREEGLUT_STATIC",
                "FREEGLUT_LIB_PRAGMAS=0",
                "PUGIXML_NO_EXCEPTIONS",
                "EIGEN_DONT_ALIGN_STATICALLY=1",
                os.is("windows") and "WIN32" or "",
            }
    end

    local examples = os.matchdirs("examples/*")
    for _, example in ipairs(examples) do
        create_project("examples/", example)
    end

    local tutorials = os.matchdirs("tutorials/*")
    for _, tutorial in ipairs(tutorials) do
        create_project("tutorials/", tutorial)
    end

    local apps = os.matchdirs("apps/*")
    for _, app in ipairs(apps) do
        create_project("apps/", app)
    end

    if _OPTIONS["gen-licenses"] then

        function compose_license(libname,content)
            local t = ""
            libname = libname .. " LICENSE"
            t = t .. "- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -\n\n"
            t = t .. string.rep(" ", (39-string.len(libname)/2)) .. string.upper(libname) .. "\n\n"
            t = t .. "- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -\n\n"
            t = t .. content
            t = t .. "\n\n"
            return t
        end

        function compose_link(name,url,license)
            return "- ["..name.."]("..url.."), "..license.." licensed.\n"
        end

        function detect_license(txt)
            local found

            txt = string.upper(txt)

            -- look for legal text first
            if not found and string.findlast(txt, "LESSER GENERAL PUBLIC LICENSE") then found = "LGPL" end
            if not found and string.findlast(txt, "GENERAL PUBLIC LICENSE") then found = "GPL" end
            if not found and string.findlast(txt, "BOOST SOFTWARE LICENSE") then found = "BOOST" end
            if not found and string.findlast(txt, "OSS.SGI.COM") then found = "SGI" end
            if not found and string.findlast(txt, "CREATIVE COMMONS CORPORATION") then found = "CC" end
            if not found and string.findlast(txt, "SUBMISSION OF CONTRIBUTIONS") then found = "APACHE2" end
            if not found and string.findlast(txt, "SUBJECT TO THE FOLLOWING RESTRICTIONS") then found = "ZLIB/LIBPNG" end
            if not found and string.findlast(txt, "PERMISSION IS HEREBY GRANTED") then found = "MIT" end
            if not found and string.findlast(txt, "MERCHANTABILITY AND FITNESS") then found = "BSD" end
            if not found and string.findlast(txt, "APACHE LICENSE, VERSION 2.0") then found = "APACHE2" end

            -- look for explicit text
            if not found and string.findlast(txt, "PUBLIC DOMAIN") then found = "PD" end
            if not found and string.findlast(txt, "UNLICENSE") then found = "PD" end
            if not found and string.findlast(txt, "WTFPL") then found = "WTFPL" end
            if not found and string.findlast(txt, "ZLIB LICENSE") then found = "ZLIB/LIBPNG" end
            if not found and string.findlast(txt, "LIBPNG LICENSE") then found = "ZLIB/LIBPNG" end
            if not found and string.findlast(txt, "MIT LICENSE") then found = "MIT" end
            if not found and string.findlast(txt, "BSD LICENSE") then found = "BSD" end
            if not found and string.findlast(txt, "BOOST LICENSE") then found = "BOOST" end
            if not found and string.findlast(txt, "APACHE LICENSE") then found = "APACHE2" end
            if not found and string.findlast(txt, "SGI LICENSE") then found = "SGI" end
            if not found and string.findlast(txt, "LGPL LICENSE") then found = "LGPL" end
            if not found and string.findlast(txt, "GPL LICENSE") then found = "GPL" end

            return found
        end

        function find_licenses(pathmask,head,body)
            local h = head
            local d = body
            local licenses = os.matchfiles(pathmask)
            for _, license in ipairs(licenses) do
                local libpath = path.getdirectory(license)
                local libname = string.sub(libpath, 1 + string.findlast(libpath, "/", true) )

                local fp = io.open( path.translate(license), "r" )
                local content = fp:read("*all")
                fp:close()

                d = d .. compose_license(libname,content)
                local found = detect_license(content)
                if found then
                    h = h .. compose_link(libname,license,detect_license(content))
                else
                    print("cannot detect license! (",license,") {\n",content,"\n}\n")
                end
            end
            return h, d
        end

        function detect_licenses(pathmask,head,body)
            local h = head
            local d = body
            local readmes = os.matchfiles(pathmask)
            for _, readme in ipairs(readmes) do
                local libpath = path.getdirectory(readme)
                local libname = string.sub(libpath, 1 + string.findlast(libpath, "/", true) )

                local process = true

                if process and os.isfile(libpath.."/licence") then process = false end
                if process and os.isfile(libpath.."/LICENCE") then process = false end
                if process and os.isfile(libpath.."/licence.md") then process = false end
                if process and os.isfile(libpath.."/LICENCE.md") then process = false end
                if process and os.isfile(libpath.."/licence.txt") then process = false end
                if process and os.isfile(libpath.."/LICENCE.txt") then process = false end
                if process and os.isfile(libpath.."/LICENCE.TXT") then process = false end
                if process and os.isfile(libpath.."/licences") then process = false end
                if process and os.isfile(libpath.."/LICENCES") then process = false end
                if process and os.isfile(libpath.."/licences.md") then process = false end
                if process and os.isfile(libpath.."/LICENCES.md") then process = false end
                if process and os.isfile(libpath.."/licences.txt") then process = false end
                if process and os.isfile(libpath.."/LICENCES.txt") then process = false end
                if process and os.isfile(libpath.."/LICENCES.TXT") then process = false end
                if process and os.isfile(libpath.."/license") then process = false end
                if process and os.isfile(libpath.."/LICENSE") then process = false end
                if process and os.isfile(libpath.."/license.md") then process = false end
                if process and os.isfile(libpath.."/LICENSE.md") then process = false end
                if process and os.isfile(libpath.."/license.txt") then process = false end
                if process and os.isfile(libpath.."/LICENSE.txt") then process = false end
                if process and os.isfile(libpath.."/LICENSE.TXT") then process = false end
                if process and os.isfile(libpath.."/licenses") then process = false end
                if process and os.isfile(libpath.."/LICENSES") then process = false end
                if process and os.isfile(libpath.."/licenses.md") then process = false end
                if process and os.isfile(libpath.."/LICENSES.md") then process = false end
                if process and os.isfile(libpath.."/licenses.txt") then process = false end
                if process and os.isfile(libpath.."/LICENSES.txt") then process = false end
                if process and os.isfile(libpath.."/LICENSES.TXT") then process = false end

                if process and os.isfile(libpath.."/unlicense") then process = false end
                if process and os.isfile(libpath.."/unlicense.md") then process = false end
                if process and os.isfile(libpath.."/unlicense.txt") then process = false end
                if process and os.isfile(libpath.."/UNLICENSE") then process = false end
                if process and os.isfile(libpath.."/UNLICENSE.md") then process = false end
                if process and os.isfile(libpath.."/UNLICENSE.txt") then process = false end
                if process and os.isfile(libpath.."/UNLICENSE.TXT") then process = false end

                if process and os.isfile(libpath.."/copying") then process = false end
                if process and os.isfile(libpath.."/copying.md") then process = false end
                if process and os.isfile(libpath.."/copying.txt") then process = false end
                if process and os.isfile(libpath.."/COPYING") then process = false end
                if process and os.isfile(libpath.."/COPYING.md") then process = false end
                if process and os.isfile(libpath.."/COPYING.txt") then process = false end
                if process and os.isfile(libpath.."/COPYING.TXT") then process = false end

                if process == true then
                    local fp = io.open( path.translate(readme), "r" )
                    local content = fp:read("*all")
                    fp:close()

                    local found = detect_license(content)
                    if found then
                        d = d .. compose_license(libname,found)
                        h = h .. compose_link(libname,readme,found)
                    else
                        if string.len(content) > 0 then
                            print("cannot detect license! (",readme,") {\n",content,"\n}\n")
                        end
                    end
                end
            end
            return h,d
        end

        local head = ""
        local body = ""

        head, body = find_licenses("libs/**/licen*e*",head,body)
        head, body = find_licenses("libs/**/LICEN*E*",head,body)
        head, body = find_licenses("libs/**/copying*",head,body)
        head, body = find_licenses("libs/**/COPYING*",head,body)
        head, body = detect_licenses("libs/**/readme*",head,body)
        head, body = detect_licenses("libs/**/README*",head,body)

        local lc = io.open("LICENSES.md","a")
        lc:write("-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8<\n")
        lc:write("### licenses (autogenerated; may be wrong)\n")
        lc:write("\n")
        lc:write("- Check [LICENSE](LICENSE) file as well.\n")
        lc:write(head)
        lc:close()

        local lc = io.open("LICENSE","w")
        lc:write(body)
        lc:close()

        os.exit()
    end

-------------------------------------------------------------------------------
