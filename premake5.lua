-- Copyright (c) 2026 JC Technolabs
-- License: MIT

-- premake5.lua
workspace "PolarionMockup"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    startproject "mockup"

project "mockup"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir "bin/%{cfg.buildcfg}"
    objdir ("bin-int/%{cfg.buildcfg}")

    defines
    {
        "MOCKUP_VERSION=\"0.1\""
    }

    files
    {
        "src/**.h",
        "src/**.cpp",
        "vendor/simdjson/simdjson.cpp",
        "vendor/simdjson/simdjson.h",
    }

    includedirs
    {
        "src/",
        "vendor/",
        "vendor/spdlog/include",
        "vendor/thread-pool/include",
        "vendor/tracy/include",
        "vendor/openssl/include",
        "vendor/crow/include/crow",
        "vendor/asio/asio/include",
    }

    -- ================================================================
    -- Linux
    -- ================================================================
    filter "system:linux"

        linkoptions {
            "-fno-pie -no-pie",
            "-rdynamic"
        }

        links {
            "pthread",
            "dl",
            "ssl",
            "crypto",
            "z",
        }

        defines {
            "LINUX",
        }

    -- ================================================================
    -- macOS
    -- ================================================================
    filter "system:macosx"

        linkoptions {
            "-framework CoreFoundation",
            "-framework SystemConfiguration"
        }

        links {
            "ssl",
            "crypto",
            "z",
        }

    -- ================================================================
    -- GCC / Clang warning flags (gmake builds)
    -- ================================================================
    filter { "action:gmake*", "configurations:Debug" }
        buildoptions { "-ggdb -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter -Wno-reorder -Wno-expansion-to-defined" }

    filter { "action:gmake*", "configurations:Release" }
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter -Wno-reorder -Wno-expansion-to-defined" }

    -- ================================================================
    -- Windows
    -- ================================================================
    filter "system:windows"
        systemversion "latest"
        buildoptions { "/utf-8", "/bigobj" }
        defines { "NOMINMAX" }

        links {
            "ws2_32",
            "advapi32",
            "crypt32",
            "ssl",
            "crypto",
        }

    -- ================================================================
    -- Debug / Release configurations
    -- ================================================================
    filter "configurations:Debug"
        defines
        {
            "DEBUG"
        }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines
        {
            "NDEBUG"
        }
        runtime "Release"
        optimize "on"

    filter {}

    -- ================================================================
    -- Clean
    -- ================================================================
    if _ACTION == "clean" then
        print("clean the build...")

        os.rmdir("bin")
        os.rmdir("bin-int")

        os.remove("*.make")
        os.remove("**/*.make")
        os.remove("Makefile")
        os.remove("vendor/Makefile")

        os.rmdir("vendor/openssl/bin")
        os.rmdir("vendor/openssl/bin-int")

        print("done.")
    end

    include "vendor/openssl/crypto.lua"
    include "vendor/openssl/ssl.lua"
