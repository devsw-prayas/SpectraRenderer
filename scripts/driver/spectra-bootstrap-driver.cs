// Spectra Bootstrap Driver - standalone dev-convenience task runner.
// Not part of the engine or the CMake build graph; shells out to cmake/scripts
// as opaque subprocess calls and reports results, nothing more.

using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text.Json;

if (args.Length == 0 || args[0] is "help" or "-h" or "--help")
{
    PrintUsage();
    return 0;
}

var rootDir = FindRepoRoot();
if (rootDir is null)
{
    Console.WriteLine("[ERROR] Could not locate repo root (no CMakeLists.txt found above this executable).");
    return 1;
}

var config = LoadConfig(rootDir);
var command = args[0];
var rest = args[1..];

return command switch
{
    "cmake-init" => CmakeInit(rootDir, config, rest),
    "submodule-update" => SubmoduleUpdate(rootDir, rest),
    "module-gen" => ModuleGen(rest),
    "cu-check" => CuCheck(rootDir, config, rest),
    "vk-check" => VkCheck(rootDir, config, rest),
    "header-gen" => HeaderGen(rest),
    "build" => BuildConfig(rootDir, config, rest),
    "rebuild" => RebuildConfig(rootDir, config, rest),
    "run" => RunTarget(rootDir, config, rest),
    _ => UnknownCommand(command)
};

// Dispatch helpers

void PrintUsage()
{
    Banner("Spectra Bootstrap Driver");
    Console.WriteLine("Usage: driver <command> [options]");
    Console.WriteLine();
    Console.WriteLine("Commands:");
    Console.WriteLine("  help, -h, --help                                       Show this message");
    Console.WriteLine("  cmake-init [-f] [-preq]                                Configure cmake (-f: delete CMakeCache.txt first, -preq: check cmake/VS2022 first)");
    Console.WriteLine("  submodule-update [--remote]                            git submodule update --init --recursive (--remote: also pull latest tracked branch)");
    Console.WriteLine("  module-gen -lib|-dll|-exe -cpp17|-cpp20|-cpp23 -n \"Name\" -dir <location>   Scaffold a new module");
    Console.WriteLine("  cu-check [-d]                                          Check for CUDA toolkit (-d: install if missing)");
    Console.WriteLine("  vk-check [-d]                                          Check for Vulkan SDK (-d: install if missing)");
    Console.WriteLine("  header-gen -p <Prefix> -np <Namespace> -dir <path>     Generate Compiler.h/Diagnostic.h pair");
    Console.WriteLine("  build -c <Configuration>                               cmake --build");
    Console.WriteLine("  rebuild -c <Configuration>                             cmake --build --clean-first");
    Console.WriteLine("  run -c <Configuration>                                 Launch the configured run target");
}

int UnknownCommand(string name)
{
    Console.WriteLine($"[ERROR] Unknown command: {name}");
    PrintUsage();
    return 1;
}

// cmake-init

int CmakeInit(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var force = p_Rest.Contains("-f");

    if (p_Rest.Contains("-preq") && !CheckPrereqs())
        return 1;

    var buildDir = Path.Combine(p_RootDir, p_Config.BuildDir);
    Directory.CreateDirectory(buildDir);

    if (force)
    {
        var cachePath = Path.Combine(buildDir, "CMakeCache.txt");
        if (File.Exists(cachePath))
        {
            File.Delete(cachePath);
            Console.WriteLine($"[OK] Deleted {cachePath}");
        }
    }

    Console.WriteLine($"[INFO] Configuring build in {buildDir}... \n");
    var exitCode = Run("cmake", $"-S \"{p_RootDir}\" -B \"{buildDir}\" -G \"{p_Config.CmakeGenerator}\"");
    Console.WriteLine(exitCode == 0 ? "[OK] CMake configured." : "[ERROR] CMake configure failed.");
    return exitCode;
}

// submodule-update

int SubmoduleUpdate(string p_RootDir, string[] p_Rest)
{
    var remote = p_Rest.Contains("--remote");
    var args = "submodule update --init --recursive" + (remote ? " --remote" : "");

    Console.WriteLine("[INFO] Updating submodules... \n");
    var exitCode = Run("git", $"-C \"{p_RootDir}\" {args}");
    Console.WriteLine(exitCode == 0 ? "[OK] Submodules up to date." : "[ERROR] Submodule update failed.");
    return exitCode;
}

// build / rebuild

int BuildConfig(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var cfg = ResolveConfiguration(p_Config, p_Rest);
    if (cfg is null) return 1;

    var buildDir = Path.Combine(p_RootDir, p_Config.BuildDir);
    Console.WriteLine($"[INFO] Building configuration: {cfg} \n");
    var exitCode = Run("cmake", $"--build \"{buildDir}\" --config {cfg}");
    Console.WriteLine(exitCode == 0 ? "[OK] Build succeeded." : $"[ERROR] Build failed for configuration {cfg}.");
    return exitCode;
}

int RebuildConfig(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var cfg = ResolveConfiguration(p_Config, p_Rest);
    if (cfg is null) return 1;

    var buildDir = Path.Combine(p_RootDir, p_Config.BuildDir);
    Console.WriteLine($"[INFO] Rebuilding configuration: {cfg} \n");
    var exitCode = Run("cmake", $"--build \"{buildDir}\" --config {cfg} --clean-first");
    Console.WriteLine(exitCode == 0 ? "[OK] Rebuild succeeded." : $"[ERROR] Rebuild failed for configuration {cfg}.");
    return exitCode;
}

// run

int RunTarget(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var cfg = ResolveConfiguration(p_Config, p_Rest);
    if (cfg is null) return 1;

    var exePath = Path.Combine(p_RootDir, p_Config.BinDir, cfg, p_Config.RunTarget + ".exe");

    if (!File.Exists(exePath))
    {
        Console.WriteLine($"[ERROR] {exePath} not found. Build configuration '{cfg}' first.");
        return 1;
    }

    Console.WriteLine($"[INFO] Launching {exePath}...");
    var psi = new ProcessStartInfo(exePath) { UseShellExecute = false };
    using var process = Process.Start(psi);
    process!.WaitForExit();
    return process.ExitCode;
}

// cu-check / vk-check

int CuCheck(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var download = p_Rest.Contains("-d");

    if (FindOnPath(p_Config.CudaCompilerExe) is not null)
    {
        Console.WriteLine($"[OK] CUDA toolkit ({p_Config.CudaCompilerExe}) found.");
        return 0;
    }

    Console.WriteLine($"[MISSING] CUDA toolkit ({p_Config.CudaCompilerExe}) not found on PATH.");
    if (!download) return 1;

    var installer = Path.Combine(p_RootDir, p_Config.CudaInstallerScript);
    Console.WriteLine($"[INFO] Running {installer}...");
    var exitCode = Run(installer, "");
    if (exitCode != 0)
    {
        Console.WriteLine("[ERROR] CUDA install failed.");
        return exitCode;
    }

    if (FindOnPath(p_Config.CudaCompilerExe) is not null)
    {
        Console.WriteLine($"[OK] CUDA toolkit ({p_Config.CudaCompilerExe}) found after install.");
        return 0;
    }

    Console.WriteLine("[ERROR] CUDA toolkit still not found on PATH after install.");
    return 1;
}

int VkCheck(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var download = p_Rest.Contains("-d");

    if (CheckVulkanSdk(p_Config))
        return 0;

    if (!download) return 1;

    var installer = Path.Combine(p_RootDir, p_Config.VulkanInstallerScript);
    Console.WriteLine($"[INFO] Running {installer}...");
    var exitCode = Run(installer, "");
    if (exitCode != 0)
    {
        Console.WriteLine("[ERROR] Vulkan SDK install failed.");
        return exitCode;
    }

    return CheckVulkanSdk(p_Config) ? 0 : 1;
}

bool CheckVulkanSdk(Config p_Config)
{
    var vulkanSdk = Environment.GetEnvironmentVariable("VULKAN_SDK");
    if (string.IsNullOrEmpty(vulkanSdk))
    {
        Console.WriteLine("[MISSING] VULKAN_SDK not set.");
        return false;
    }
    if (!File.Exists(Path.Combine(vulkanSdk, p_Config.VulkanHeaderRelPath)))
    {
        Console.WriteLine($"[MISSING] VULKAN_SDK is set but Vulkan headers weren't found at {vulkanSdk}.");
        return false;
    }
    Console.WriteLine("[OK] Vulkan SDK found.");
    return true;
}

// header-gen

int HeaderGen(string[] p_Rest)
{
    var prefix = GetFlagValue(p_Rest, "-p");
    var ns = GetFlagValue(p_Rest, "-np");
    var dir = GetFlagValue(p_Rest, "-dir");

    if (prefix is null || ns is null || dir is null)
    {
        Console.WriteLine("[ERROR] Usage: header-gen -p <Prefix> -np <Namespace> -dir <TargetDir>");
        return 1;
    }

    var targetDir = Path.GetFullPath(dir);
    if (!Directory.Exists(targetDir))
    {
        Console.WriteLine($"[ERROR] Target directory does not exist: {targetDir}");
        return 1;
    }

    var prefixUpper = prefix.ToUpperInvariant();
    var moduleHeader = $"Spectra{ns}";

    var compilerFileName = $"Spec{prefix}Compiler.h";
    var diagnosticFileName = $"Spec{prefix}Diagnostic.h";

    var compilerPath = Path.Combine(targetDir, compilerFileName);
    var diagnosticPath = Path.Combine(targetDir, diagnosticFileName);

    File.WriteAllText(compilerPath, CompilerTemplate(ns, prefixUpper));
    Console.WriteLine($"[OK] Wrote {compilerPath}");

    File.WriteAllText(diagnosticPath, DiagnosticTemplate(prefixUpper, moduleHeader, compilerFileName));
    Console.WriteLine($"[OK] Wrote {diagnosticPath}");

    return 0;
}

string CompilerTemplate(string p_ns, string p_prefix) => $$"""
#pragma once

namespace Spectra::{{p_ns}} {
#if defined(_MSC_VER)
#define SPEC_{{p_prefix}}_COMPILER_MSVC 1
#else
#define SPEC_{{p_prefix}}_COMPILER_MSVC 0
#endif

#if defined(__clang__)
#define SPEC_{{p_prefix}}_COMPILER_CLANG 1
#else
#define SPEC_{{p_prefix}}_COMPILER_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define SPEC_{{p_prefix}}_COMPILER_GCC 1
#else
#define SPEC_{{p_prefix}}_COMPILER_GCC 0
#endif
}

#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_FORCEINLINE __forceinline
#define SPEC_{{p_prefix}}_NOINLINE    __declspec(noinline)
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_FORCEINLINE inline __attribute__((always_inline))
#define SPEC_{{p_prefix}}_NOINLINE    __attribute__((noinline))
#else
#define SPEC_{{p_prefix}}_FORCEINLINE inline
#define SPEC_{{p_prefix}}_NOINLINE
#endif

#define SPEC_{{p_prefix}}_INLINE inline

#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_COMPILER_BARRIER() _ReadWriteBarrier()
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_COMPILER_BARRIER() asm volatile("" ::: "memory")
#else
#define SPEC_{{p_prefix}}_COMPILER_BARRIER()
#endif

#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_OPTIMIZE_OFF __pragma(optimize("", off))
#define SPEC_{{p_prefix}}_OPTIMIZE_ON  __pragma(optimize("", on))
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_OPTIMIZE_OFF _Pragma("clang optimize off")
#define SPEC_{{p_prefix}}_OPTIMIZE_ON  _Pragma("clang optimize on")
#else
#define SPEC_{{p_prefix}}_OPTIMIZE_OFF
#define SPEC_{{p_prefix}}_OPTIMIZE_ON
#endif

#if SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_LIKELY(x)   __builtin_expect(!!(x), 1)
#define SPEC_{{p_prefix}}_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SPEC_{{p_prefix}}_LIKELY(x)   (x)
#define SPEC_{{p_prefix}}_UNLIKELY(x) (x)
#endif

#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_DEBUG_BREAK() __debugbreak()
#define SPEC_{{p_prefix}}_TRAP()        __debugbreak()
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_DEBUG_BREAK() __builtin_trap()
#define SPEC_{{p_prefix}}_TRAP()        __builtin_trap()
#else
#include <cstdlib>
#define SPEC_{{p_prefix}}_DEBUG_BREAK() std::abort()
#define SPEC_{{p_prefix}}_TRAP()        std::abort()
#endif

#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_UNREACHABLE() __assume(0)
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_UNREACHABLE() __builtin_unreachable()
#else
#define SPEC_{{p_prefix}}_UNREACHABLE() SPEC_{{p_prefix}}_TRAP()
#endif

#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_PRAGMA(x) __pragma(x)
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_PRAGMA(x) _Pragma(#x)
#else
#define SPEC_{{p_prefix}}_PRAGMA(x)
#endif

#define SPEC_{{p_prefix}}_DIAGNOSTIC_PUSH SPEC_{{p_prefix}}_PRAGMA(diagnostic push)
#define SPEC_{{p_prefix}}_DIAGNOSTIC_POP  SPEC_{{p_prefix}}_PRAGMA(diagnostic pop)

#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_DISABLE_WARNING(w) SPEC_{{p_prefix}}_PRAGMA(warning(disable : w))
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_DISABLE_WARNING(w) SPEC_{{p_prefix}}_PRAGMA(clang diagnostic ignored w)
#else
#define SPEC_{{p_prefix}}_DISABLE_WARNING(w)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define SPEC_{{p_prefix}}_FALLTHROUGH [[fallthrough]]
#else
#define SPEC_{{p_prefix}}_FALLTHROUGH
#endif
#else
#define SPEC_{{p_prefix}}_FALLTHROUGH
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define SPEC_{{p_prefix}}_NODISCARD [[nodiscard]]
#if __cplusplus >= 202002L
#define SPEC_{{p_prefix}}_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#else
#define SPEC_{{p_prefix}}_NODISCARD_MSG(msg) [[nodiscard]]
#endif
#else
#define SPEC_{{p_prefix}}_NODISCARD
#define SPEC_{{p_prefix}}_NODISCARD_MSG(msg)
#endif
#else
#define SPEC_{{p_prefix}}_NODISCARD
#define SPEC_{{p_prefix}}_NODISCARD_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define SPEC_{{p_prefix}}_MAYBE_UNUSED [[maybe_unused]]
#else
#define SPEC_{{p_prefix}}_MAYBE_UNUSED
#endif
#else
#define SPEC_{{p_prefix}}_MAYBE_UNUSED
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(deprecated)
#define SPEC_{{p_prefix}}_DEPRECATED [[deprecated]]
#define SPEC_{{p_prefix}}_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#else
#define SPEC_{{p_prefix}}_DEPRECATED
#define SPEC_{{p_prefix}}_DEPRECATED_MSG(msg)
#endif
#else
#define SPEC_{{p_prefix}}_DEPRECATED
#define SPEC_{{p_prefix}}_DEPRECATED_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(noreturn)
#define SPEC_{{p_prefix}}_NORETURN [[noreturn]]
#else
#define SPEC_{{p_prefix}}_NORETURN
#endif
#else
#define SPEC_{{p_prefix}}_NORETURN
#endif

#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_RESTRICT __restrict
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_RESTRICT __restrict__
#else
#define SPEC_{{p_prefix}}_RESTRICT
#endif

#define SPEC_{{p_prefix}}_ALIGNAS(n) alignas(n)

#if SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), (n))
#else
#define SPEC_{{p_prefix}}_ASSUME_ALIGNED(ptr, n) (ptr)
#endif

#if SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_HOT  __attribute__((hot))
#define SPEC_{{p_prefix}}_COLD __attribute__((cold))
#else
#define SPEC_{{p_prefix}}_HOT
#define SPEC_{{p_prefix}}_COLD
#endif
""" + "\n";

string DiagnosticTemplate(string p_prefix, string p_moduleHeader, string p_compilerFileName) => $$"""
#pragma once
#include "{{p_moduleHeader}}.h"
#include "{{p_compilerFileName}}"

#if defined(_DEBUG) || defined(DEBUG)
#define SPEC_{{p_prefix}}_BUILD_DEBUG 1
#define SPEC_{{p_prefix}}_BUILD_RELEASE 0
#else
#define SPEC_{{p_prefix}}_BUILD_DEBUG 0
#define SPEC_{{p_prefix}}_BUILD_RELEASE 1
#endif

#if SPEC_{{p_prefix}}_BUILD_DEBUG

#define SPEC_{{p_prefix}}_ASSERT(expr)                                     \
        do {                                                   \
            if (!(expr)) {                                    \
                SPEC_{{p_prefix}}_DEBUG_BREAK();                              \
                SPEC_{{p_prefix}}_TRAP();                                     \
            }                                                  \
        } while (0)

#else

#define SPEC_{{p_prefix}}_ASSERT(expr) do { (void)sizeof(expr); } while (0)

#endif

#if SPEC_{{p_prefix}}_BUILD_DEBUG
#define SPEC_{{p_prefix}}_ASSUME(expr) SPEC_{{p_prefix}}_ASSERT(expr)
#else
#if SPEC_{{p_prefix}}_COMPILER_MSVC
#define SPEC_{{p_prefix}}_ASSUME(expr) __assume(expr)
#elif SPEC_{{p_prefix}}_COMPILER_CLANG || SPEC_{{p_prefix}}_COMPILER_GCC
#define SPEC_{{p_prefix}}_ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#else
#define SPEC_{{p_prefix}}_ASSUME(expr) do { } while (0)
#endif
#endif

#if SPEC_{{p_prefix}}_BUILD_DEBUG
#define SPEC_{{p_prefix}}_DEBUG_ASSERT(expr) SPEC_{{p_prefix}}_ASSERT(expr)
#define SPEC_{{p_prefix}}_DEBUG_ASSUME(expr) SPEC_{{p_prefix}}_ASSUME(expr)
#else
#define SPEC_{{p_prefix}}_DEBUG_ASSERT(expr) do {} while (0)
#define SPEC_{{p_prefix}}_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define SPEC_{{p_prefix}}_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

#define SPEC_{{p_prefix}}_UNUSED(x) (void)(x)
""" + "\n";

// module-gen

int ModuleGen(string[] p_Rest)
{
    string? type = null;
    if (p_Rest.Contains("-lib")) type = "lib";
    if (p_Rest.Contains("-dll")) type = "dll";
    if (p_Rest.Contains("-exe")) type = "exe";

    var cxxStd = "20";
    if (p_Rest.Contains("-cpp17")) cxxStd = "17";
    if (p_Rest.Contains("-cpp20")) cxxStd = "20";
    if (p_Rest.Contains("-cpp23")) cxxStd = "23";

    var nameArg = GetFlagValue(p_Rest, "-n");
    var dirArg = GetFlagValue(p_Rest, "-dir");

    if (type is null || nameArg is null || dirArg is null)
    {
        Console.WriteLine("[ERROR] Usage: module-gen -lib|-dll|-exe -cpp17|-cpp20|-cpp23 -n \"Name\" -dir <location>");
        return 1;
    }

    var name = nameArg.Replace(" ", "");
    var parentDir = Path.GetFullPath(dirArg);
    var root = Path.Combine(parentDir, name);

    Directory.CreateDirectory(Path.Combine(root, "src", "Public"));
    Directory.CreateDirectory(Path.Combine(root, "src", "Private"));
    Directory.CreateDirectory(Path.Combine(root, "build"));
    Directory.CreateDirectory(Path.Combine(root, "bin"));
    Console.WriteLine("[OK] Directories created.");

    switch (type)
    {
        case "exe": WriteExeSource(root, name); break;
        case "dll": WriteDllSource(root, name); break;
        case "lib": WriteLibSource(root, name); break;
    }
    Console.WriteLine("[OK] Source files written.");

    WriteCMakeLists(root, name, type, cxxStd);
    Console.WriteLine("[OK] CMakeLists.txt written.");

    WriteModuleGitignore(root);
    Console.WriteLine("[OK] .gitignore written.");

    Console.WriteLine();
    Console.WriteLine($"Done! Project ready at: {root}");
    return 0;
}

void WriteExeSource(string p_Root, string p_Name)
{
    File.WriteAllText(Path.Combine(p_Root, "src", "Public", $"{p_Name}.h"), $$"""
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
""" + "\n");

    File.WriteAllText(Path.Combine(p_Root, "src", "Private", "main.cpp"), $$"""
#include "{{p_Name}}.h"

int main() {
    std::cout << "Hello from {{p_Name}}\n";
    return 0;
}
""" + "\n");
}

void WriteDllSource(string p_Root, string p_Name)
{
    File.WriteAllText(Path.Combine(p_Root, "src", "Public", $"{p_Name}.h"), $$"""
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#ifdef {{p_Name}}_EXPORTS
#  define {{p_Name}}_API __declspec(dllexport)
#else
#  define {{p_Name}}_API __declspec(dllimport)
#endif

{{p_Name}}_API void Init();
""" + "\n");

    File.WriteAllText(Path.Combine(p_Root, "src", "Private", $"{p_Name}.cpp"), $$"""
#include "{{p_Name}}.h"

void Init() {
    std::cout << "{{p_Name}} initialised\n";
}
""" + "\n");
}

void WriteLibSource(string p_Root, string p_Name)
{
    File.WriteAllText(Path.Combine(p_Root, "src", "Public", $"{p_Name}.h"), $$"""
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class {{p_Name}} {
public:
    void SayHello() const;
};
""" + "\n");

    File.WriteAllText(Path.Combine(p_Root, "src", "Private", $"{p_Name}.cpp"), $$"""
#include "{{p_Name}}.h"

void {{p_Name}}::SayHello() const {
    std::cout << "Hello from {{p_Name}}\n";
}
""" + "\n");
}

void WriteCMakeLists(string p_Root, string p_Name, string p_Type, string p_CxxStd)
{
    var targetDecl = p_Type switch
    {
        "exe" => $$"""
add_executable({{p_Name}}
    ${{{p_Name}}_HEADERS}
    ${{{p_Name}}_SOURCE}
    ${{{p_Name}}_INL}
)
""",
        "dll" => $$"""
add_library({{p_Name}} SHARED
    ${{{p_Name}}_HEADERS}
    ${{{p_Name}}_SOURCE}
    ${{{p_Name}}_INL}
)
target_compile_definitions({{p_Name}} PRIVATE {{p_Name}}_EXPORTS)
""",
        _ => $$"""
add_library({{p_Name}} STATIC
    ${{{p_Name}}_HEADERS}
    ${{{p_Name}}_SOURCE}
    ${{{p_Name}}_INL}
)
"""
    };

    var content = $$"""
cmake_minimum_required(VERSION 3.20)
project({{p_Name}} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD {{p_CxxStd}})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE {{p_Name}}_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE {{p_Name}}_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE {{p_Name}}_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

{{targetDecl}}
target_include_directories({{p_Name}} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers({{p_Name}} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/{{p_Name}}.h)

target_compile_options({{p_Name}} PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra>
)
""" + "\n";

    File.WriteAllText(Path.Combine(p_Root, "CMakeLists.txt"), content);
}

void WriteModuleGitignore(string p_Root)
{
    File.WriteAllText(Path.Combine(p_Root, ".gitignore"), """
build/
bin/
.cache/
CMakeFiles/
CMakeCache.txt
cmake_install.cmake
*.pdb
*.ilk
*.exp
""");
}

// Shared helpers

void Banner(string title)
{
    Console.WriteLine();
    Console.WriteLine(new string('=', 60));
    Console.WriteLine($"              {title}");
    Console.WriteLine(new string('=', 60));
    Console.WriteLine();
}

string? GetFlagValue(string[] p_Args, string p_Flag)
{
    var idx = Array.IndexOf(p_Args, p_Flag);
    if (idx == -1 || idx + 1 >= p_Args.Length) return null;
    return p_Args[idx + 1];
}

bool CheckPrereqs()
{
    var ok = true;
    ok &= CheckCommand("cmake", "CMake not found in PATH. Install CMake 3.20+ and try again.");
    ok &= CheckVisualStudio2022();
    return ok;
}

bool CheckCommand(string p_Name, string p_ErrorMessage)
{
    if (FindOnPath(p_Name) is not null)
    {
        Console.WriteLine($"[OK] {p_Name} found.");
        return true;
    }
    Console.WriteLine($"[ERROR] {p_ErrorMessage}");
    return false;
}

bool CheckVisualStudio2022()
{
    var programFilesX86 = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86);
    var vswhere = Path.Combine(programFilesX86, "Microsoft Visual Studio", "Installer", "vswhere.exe");
    if (File.Exists(vswhere))
    {
        var output = RunCapture(vswhere, "-version \"[17.0,18.0)\" -property installationPath");
        if (!string.IsNullOrWhiteSpace(output))
        {
            Console.WriteLine("[OK] Visual Studio 2022 found.");
            return true;
        }
    }
    Console.WriteLine("[ERROR] Visual Studio 2022 not found. Install it before building.");
    return false;
}

string RunCapture(string p_Exe, string p_Arguments)
{
    var psi = new ProcessStartInfo(p_Exe, p_Arguments)
    {
        UseShellExecute = false,
        RedirectStandardOutput = true
    };
    using var process = Process.Start(psi);
    var output = process!.StandardOutput.ReadToEnd();
    process.WaitForExit();
    return output;
}

string? FindOnPath(string p_ExeName)
{
    var pathVar = Environment.GetEnvironmentVariable("PATH") ?? "";
    var exts = (Environment.GetEnvironmentVariable("PATHEXT") ?? ".EXE;.BAT;.CMD").Split(';');
    foreach (var dir in pathVar.Split(Path.PathSeparator))
    {
        foreach (var ext in exts)
        {
            var candidate = Path.Combine(dir, p_ExeName + ext);
            if (File.Exists(candidate)) return candidate;
        }
    }
    return null;
}

int Run(string p_Exe, string p_Arguments)
{
    var psi = new ProcessStartInfo(p_Exe, p_Arguments) { UseShellExecute = false };
    using var process = Process.Start(psi);
    process!.WaitForExit();
    return process.ExitCode;
}

// dotnet run compiles file-based apps into a temp cache dir outside the repo
// (%TEMP%\dotnet\runfile\...), so AppContext.BaseDirectory can't be used to
// locate the repo. This file's own compile-time path is stable instead: it
// always lives at <root>/scripts/driver/spectra-bootstrap-driver.cs.
string ThisFilePath([CallerFilePath] string p_Path = "") => p_Path;

string? FindRepoRoot()
{
    var scriptDir = Path.GetDirectoryName(ThisFilePath())!;
    var candidateRoot = Path.GetFullPath(Path.Combine(scriptDir, "..", ".."));
    return File.Exists(Path.Combine(candidateRoot, "CMakeLists.txt")) ? candidateRoot : null;
}

// config.json is tracked build-tooling config (like CMakeLists.txt), never
// generated by the driver - a missing field is a config-authoring error, not
// something to paper over with a hardcoded fallback here.
Config LoadConfig(string p_RootDir)
{
    var configPath = Path.Combine(p_RootDir, "scripts", "driver", "config", "config.json");
    if (!File.Exists(configPath))
    {
        Console.WriteLine($"[ERROR] Config not found at {configPath}.");
        Environment.Exit(1);
    }

    using var doc = JsonDocument.Parse(File.ReadAllText(configPath));
    var root = doc.RootElement;

    string GetString(string p_Name)
    {
        if (!root.TryGetProperty(p_Name, out var v) || v.GetString() is not { } s)
        {
            Console.WriteLine($"[ERROR] {configPath} is missing required field \"{p_Name}\".");
            Environment.Exit(1);
            return "";
        }
        return s;
    }

    string[] GetStringArray(string p_Name)
    {
        if (!root.TryGetProperty(p_Name, out var v) || v.ValueKind != JsonValueKind.Array)
        {
            Console.WriteLine($"[ERROR] {configPath} is missing required array field \"{p_Name}\".");
            Environment.Exit(1);
            return [];
        }
        return v.EnumerateArray().Select(e => e.GetString() ?? "").ToArray();
    }

    return new Config(
        GetString("buildDir"),
        GetString("binDir"),
        GetString("cmakeGenerator"),
        GetString("defaultConfig"),
        GetString("runTarget"),
        GetStringArray("buildConfigurations"),
        GetString("cudaCompilerExe"),
        GetString("cudaInstallerScript"),
        GetString("vulkanHeaderRelPath"),
        GetString("vulkanInstallerScript"));
}

string? ResolveConfiguration(Config p_Config, string[] p_Rest)
{
    var cfg = GetFlagValue(p_Rest, "-c") ?? p_Config.DefaultConfig;
    if (!p_Config.BuildConfigurations.Contains(cfg))
    {
        Console.WriteLine($"[ERROR] Unknown configuration: {cfg}");
        Console.WriteLine("[INFO] Valid configurations (from config.json): " + string.Join(", ", p_Config.BuildConfigurations));
        return null;
    }
    return cfg;
}

record Config(
    string BuildDir,
    string BinDir,
    string CmakeGenerator,
    string DefaultConfig,
    string RunTarget,
    string[] BuildConfigurations,
    string CudaCompilerExe,
    string CudaInstallerScript,
    string VulkanHeaderRelPath,
    string VulkanInstallerScript);
