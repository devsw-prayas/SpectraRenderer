// Spectra Bootstrap Driver - standalone dev-convenience task runner.
// Not part of the engine or the CMake build graph; shells out to cmake/scripts
// as opaque subprocess calls and reports results, nothing more.

using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

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
    "exec-gen" => ExecGen(rootDir, config, rest),
    "build" => BuildConfig(rootDir, config, rest),
    "rebuild" => RebuildConfig(rootDir, config, rest),
    "run" => RunTarget(rootDir, config, rest),
    "test" => TestSuites(rootDir, config, rest),
    "hades" => HadesPassthrough(rootDir, config, rest),
    "filters" => FiltersCommand(rootDir, config, rest),
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
    Console.WriteLine("  cmake-init [-f] [-preq] [-clangcl]                     Configure cmake (-f: delete CMakeCache.txt first, -preq: check cmake/VS2022 first, -clangcl: configure a separate clang-cl build tree)");
    Console.WriteLine("  submodule-update [--remote]                            git submodule update --init --recursive (--remote: also pull latest tracked branch)");
    Console.WriteLine("  module-gen -lib|-dll|-exe -cpp17|-cpp20|-cpp23 -n \"Name\" -dir <location>   Scaffold a new module");
    Console.WriteLine("  cu-check [-d]                                          Check for CUDA toolkit (-d: install if missing)");
    Console.WriteLine("  vk-check [-d]                                          Check for Vulkan SDK (-d: install if missing)");
    Console.WriteLine("  header-gen -p <Prefix> -np <Namespace> -dir <path>     Generate Compiler.h/Diagnostic.h pair");
    Console.WriteLine("  exec-gen                                               Regenerate <Module>.generated.h from config/spectra-err.json + spectra-exec.json (also runs automatically inside cmake-init)");
    Console.WriteLine("  build -c <Configuration> [-t <Target>] [-clangcl]      cmake --build (optionally a single target/submodule; -clangcl: build the clang-cl tree)");
    Console.WriteLine("  rebuild -c <Configuration> [-t <Target>] [-clangcl]    cmake --build --clean-first (optionally a single target/submodule; -clangcl: rebuild the clang-cl tree)");
    Console.WriteLine("  run -c <Configuration> [-clangcl]                      Launch the configured run target (-clangcl: from the clang-cl tree)");
    Console.WriteLine("  test [-c <Configuration>] [--fbt=<pattern>] [-ls]     Run every Hades suite listed in tests/test.config (-ls: list suites/tests instead)");
    Console.WriteLine("  hades [-c <Configuration>] <args...>                   Passthrough to Hades-Driver.exe (init-suite/find-suite/new-test/run/validate/...)");
    Console.WriteLine("  filters list                                           List gitFilters from config.json and whether each is enabled locally");
    Console.WriteLine("  filters enable <name>                                  Configure the named filter's clean/smudge commands for its appliesTo submodule(s)");
    Console.WriteLine("  filters disable <name>                                 Unset the named filter's clean/smudge commands (reverts to pass-through)");
    Console.WriteLine("  filters clean <name>                                   (internal - invoked by git itself as filter.<name>.clean) strip guarded blocks from stdin to stdout");
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
    var clangCl = p_Rest.Contains("-clangcl");

    if (p_Rest.Contains("-preq") && !CheckPrereqs())
        return 1;

    if (ExecGen(p_RootDir, p_Config, []) != 0)
        return 1;

    var buildDir = Path.Combine(p_RootDir, clangCl ? p_Config.ClangClBuildDir : p_Config.BuildDir);
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

    // clang-cl is a separate toolset within the same VS-generator family, not
    // a different generator - it gets its own build tree (a CMake cache can't
    // switch toolsets in place) and its own SPECTRA_BIN_SUBDIR so its output
    // never lands in the same bin/<Configuration> path as the default MSVC
    // build.
    var toolsetArgs = clangCl ? $" -T {p_Config.ClangClToolset}" : "";
    var binSubdirArgs = clangCl ? $" -DSPECTRA_BIN_SUBDIR=\"{p_Config.ClangClBinDir}\"" : "";

    Console.WriteLine($"[INFO] Configuring build in {buildDir}{(clangCl ? " (clang-cl)" : "")}... \n");
    var exitCode = Run("cmake", $"-S \"{p_RootDir}\" -B \"{buildDir}\" -G \"{p_Config.CmakeGenerator}\"{toolsetArgs}{binSubdirArgs}");
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
    var target = GetFlagValue(p_Rest, "-t");
    var clangCl = p_Rest.Contains("-clangcl");

    var buildDir = Path.Combine(p_RootDir, clangCl ? p_Config.ClangClBuildDir : p_Config.BuildDir);
    var targetArgs = target is null ? "" : $" --target {target}";
    Console.WriteLine($"[INFO] Building configuration: {cfg}{(target is null ? "" : $" (target: {target})")}{(clangCl ? " (clang-cl)" : "")} \n");
    var exitCode = Run("cmake", $"--build \"{buildDir}\" --config {cfg}{targetArgs}");
    Console.WriteLine(exitCode == 0 ? "[OK] Build succeeded." : $"[ERROR] Build failed for configuration {cfg}.");
    return exitCode;
}

int RebuildConfig(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var cfg = ResolveConfiguration(p_Config, p_Rest);
    if (cfg is null) return 1;
    var target = GetFlagValue(p_Rest, "-t");
    var clangCl = p_Rest.Contains("-clangcl");

    var buildDir = Path.Combine(p_RootDir, clangCl ? p_Config.ClangClBuildDir : p_Config.BuildDir);
    var targetArgs = target is null ? "" : $" --target {target}";
    Console.WriteLine($"[INFO] Rebuilding configuration: {cfg}{(target is null ? "" : $" (target: {target})")}{(clangCl ? " (clang-cl)" : "")} \n");
    var exitCode = Run("cmake", $"--build \"{buildDir}\" --config {cfg}{targetArgs} --clean-first");
    Console.WriteLine(exitCode == 0 ? "[OK] Rebuild succeeded." : $"[ERROR] Rebuild failed for configuration {cfg}.");
    return exitCode;
}

// run

int RunTarget(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var cfg = ResolveConfiguration(p_Config, p_Rest);
    if (cfg is null) return 1;
    var clangCl = p_Rest.Contains("-clangcl");

    var exePath = Path.Combine(p_RootDir, clangCl ? p_Config.ClangClBinDir : p_Config.BinDir, cfg, p_Config.RunTarget + ".exe");

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

// hades

int HadesPassthrough(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var cfg = ResolveConfiguration(p_Config, p_Rest);
    if (cfg is null) return 1;

    var hadesDriverExe = Path.Combine(p_RootDir, p_Config.BinDir, cfg, p_Config.HadesDriverTarget + ".exe");
    if (!File.Exists(hadesDriverExe))
    {
        Console.WriteLine($"[ERROR] {hadesDriverExe} not found. Build it first: driver.bat build -c {cfg} -t {p_Config.HadesDriverTarget}");
        return 1;
    }

    // Strip our own "-c <value>" pair, if present - everything else forwards
    // verbatim to Hades-Driver, which owns its own flag grammar entirely.
    // driver.bat has no opinion on what Hades' subcommands are or do.
    var forwardArgs = new List<string>();
    for (var i = 0; i < p_Rest.Length; i++)
    {
        if (p_Rest[i] == "-c") { i++; continue; }
        forwardArgs.Add(p_Rest[i]);
    }

    var psi = new ProcessStartInfo(hadesDriverExe) { UseShellExecute = false };
    foreach (var arg in forwardArgs) psi.ArgumentList.Add(arg);

    using var process = Process.Start(psi);
    process!.WaitForExit();
    return process.ExitCode;
}

// filters
//
// git clean/smudge filter support for debug-only instrumentation (e.g. Kerbecs)
// that must never persist in a submodule's committed history. Definitions live in
// config.json's "gitFilters" - "enable"/"disable" wire up local git config inside
// each of a filter's appliesTo submodules; "clean" is what git itself invokes (as
// filter.<name>.clean) on every `git add`/`git commit`, reading the working-tree
// content on stdin and writing the sanitized version to stdout. A non-zero exit
// from "clean" aborts the git operation - the safety-net check at the bottom of
// FiltersClean relies on this to fail loudly instead of silently letting a leaked
// reference through.

int FiltersCommand(string p_RootDir, Config p_Config, string[] p_Rest)
{
    if (p_Rest.Length == 0)
    {
        Console.WriteLine("[ERROR] Usage: filters list|enable <name>|disable <name>|clean <name>");
        return 1;
    }
    var sub = p_Rest[0];
    var subRest = p_Rest[1..];
    return sub switch
    {
        "list" => FiltersList(p_RootDir, p_Config),
        "enable" => FiltersEnable(p_RootDir, p_Config, subRest),
        "disable" => FiltersDisable(p_RootDir, p_Config, subRest),
        "clean" => FiltersClean(p_Config, subRest),
        _ => FiltersUnknownSub(sub)
    };
}

int FiltersUnknownSub(string p_Sub)
{
    Console.WriteLine($"[ERROR] Unknown filters subcommand: {p_Sub}");
    Console.WriteLine("[INFO] Usage: filters list|enable <name>|disable <name>|clean <name>");
    return 1;
}

int FiltersList(string p_RootDir, Config p_Config)
{
    if (p_Config.GitFilters.Count == 0)
    {
        Console.WriteLine("[INFO] No gitFilters defined in config.json.");
        return 0;
    }
    foreach (var (name, entry) in p_Config.GitFilters)
    {
        Console.WriteLine(name);
        Console.WriteLine($"    cMacro: {entry.CMacro}   cmakeOption: {entry.CmakeOption}");
        foreach (var relPath in entry.AppliesTo)
        {
            var targetDir = Path.Combine(p_RootDir, relPath);
            var enabled = Directory.Exists(targetDir)
                && !string.IsNullOrWhiteSpace(RunCaptureAt(targetDir, "git", $"config --get filter.{name}.clean"));
            Console.WriteLine($"    {relPath}: {(enabled ? "enabled" : "disabled")}");
        }
    }
    return 0;
}

int FiltersEnable(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var name = p_Rest.Length > 0 ? p_Rest[0] : null;
    if (name is null || !p_Config.GitFilters.TryGetValue(name, out var entry))
        return FiltersUnknownName(name, p_Config);

    var driverBatPath = Path.Combine(p_RootDir, "driver.bat");
    // Absolute path, quoted for git's own later shell-invocation of this command
    // (not for this process's own argument parsing) - submodules run filters with
    // their own working directory as CWD, so a relative path here would break.
    var cleanCmdValue = $"\"{driverBatPath}\" filters clean {name} %f";

    var ok = true;
    foreach (var relPath in entry.AppliesTo)
    {
        var targetDir = Path.Combine(p_RootDir, relPath);
        if (!Directory.Exists(targetDir))
        {
            Console.WriteLine($"[ERROR] filters enable: {targetDir} does not exist.");
            ok = false;
            continue;
        }
        var r1 = RunAt("git", $"config filter.{name}.clean \"{cleanCmdValue.Replace("\"", "\\\"")}\"", targetDir);
        var r2 = RunAt("git", $"config filter.{name}.smudge cat", targetDir);
        if (r1 != 0 || r2 != 0)
        {
            Console.WriteLine($"[ERROR] filters enable: failed to configure '{name}' for {relPath}.");
            ok = false;
            continue;
        }
        Console.WriteLine($"[OK] filters enable: '{name}' configured for {relPath}");
    }
    return ok ? 0 : 1;
}

int FiltersDisable(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var name = p_Rest.Length > 0 ? p_Rest[0] : null;
    if (name is null || !p_Config.GitFilters.TryGetValue(name, out var entry))
        return FiltersUnknownName(name, p_Config);

    var ok = true;
    foreach (var relPath in entry.AppliesTo)
    {
        var targetDir = Path.Combine(p_RootDir, relPath);
        if (!Directory.Exists(targetDir))
        {
            Console.WriteLine($"[ERROR] filters disable: {targetDir} does not exist.");
            ok = false;
            continue;
        }
        // --unset returns nonzero if the key was never set - not a real failure here.
        RunAt("git", $"config --unset filter.{name}.clean", targetDir);
        RunAt("git", $"config --unset filter.{name}.smudge", targetDir);
        Console.WriteLine($"[OK] filters disable: '{name}' unset for {relPath}");
    }
    return ok ? 0 : 1;
}

int FiltersUnknownName(string? p_Name, Config p_Config)
{
    Console.WriteLine($"[ERROR] Unknown filter: {p_Name}");
    Console.WriteLine("[INFO] Defined filters: " + string.Join(", ", p_Config.GitFilters.Keys));
    return 1;
}

int FiltersClean(Config p_Config, string[] p_Rest)
{
    var name = p_Rest.Length > 0 ? p_Rest[0] : null;
    if (name is null || !p_Config.GitFilters.TryGetValue(name, out var entry))
    {
        Console.Error.WriteLine($"[ERROR] filters clean: unknown filter '{name}'");
        return 1;
    }

    string content;
    using (var reader = new StreamReader(Console.OpenStandardInput()))
        content = reader.ReadToEnd();

    string stripped;
    try
    {
        stripped = StripCStyleBlocks(content, entry.CMacro);
        stripped = StripCMakeStyleBlocks(stripped, entry.CmakeOption);
    }
    catch (Exception ex)
    {
        Console.Error.WriteLine($"[ERROR] filters clean: '{name}' scanner failed: {ex.Message}");
        return 1;
    }

    // Cosmetic-only: reflows whitespace left behind by the strip above (and, since
    // there's no line-range scoping, the whole file) against the submodule's own
    // .clang-format. Never fails the filter - a missing clang-format or a formatting
    // hiccup shouldn't block a commit, only the safety net below should.
    var gitPath = p_Rest.Length > 1 ? p_Rest[1] : null;
    var clangFormatExe = FindOnPath("clang-format");
    if (clangFormatExe is not null && gitPath is not null)
    {
        try
        {
            var formatted = RunPipedCapture(clangFormatExe, $"-assume-filename=\"{gitPath}\" --style=file", stripped);
            if (!string.IsNullOrWhiteSpace(formatted))
                stripped = formatted;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[WARN] filters clean: '{name}' clang-format pass skipped: {ex.Message}");
        }
    }

    // Safety net: a leftover "Kerbecs"-ish reference after stripping means either a
    // call site was never guarded, or the scanner missed a block - fail loudly
    // (non-zero exit aborts the git add/commit) instead of letting it through.
    if (stripped.Contains("Kerbecs", StringComparison.OrdinalIgnoreCase)
        || stripped.Contains(entry.CMacro, StringComparison.Ordinal)
        || stripped.Contains(entry.CmakeOption, StringComparison.Ordinal))
    {
        Console.Error.WriteLine($"[ERROR] filters clean: stripped output for '{name}' still references Kerbecs/{entry.CMacro}/{entry.CmakeOption} - aborting.");
        return 1;
    }

    using var writer = new StreamWriter(Console.OpenStandardOutput());
    writer.Write(stripped);
    writer.Flush();
    return 0;
}

// StripCStyleBlocks/StripCMakeStyleBlocks
//
// Both grammars share one stack-based scanner (StripBlocks) - not regex-across-
// the-whole-file. Each frame tracks whether it's testing OUR macro/option (IsOwn)
// and, if so, whether the branch since its last #else/else() should be kept.
// p_Macro/p_Option is always treated as undefined/OFF: an #ifdef<macro>/if(<option>)
// branch is dropped, an #ifndef<macro> branch is kept, any #else/else() branch is
// kept, and #elif/elseif(...) on an own frame is rejected outright (a binary on/off
// guard can't represent a third branch). Suppressed() is true whenever ANY ancestor
// frame is an own-frame in its dropped branch - that's what makes a nested, unrelated
// #ifdef/if() sitting inside our own dropped branch get fully suppressed (directive
// lines included), while one outside our block (or inside our kept branch) passes
// through completely untouched, own directive lines included.

string StripBlocks(
    string p_Content,
    Func<string, (bool IsOwn, bool InitialKeep)?> p_MatchOpen,
    Func<string, bool> p_MatchElseIf,
    Func<string, bool> p_MatchElse,
    Func<string, bool> p_MatchClose)
{
    var stack = new Stack<(bool IsOwn, bool KeepCurrentBranch)>();
    var output = new StringBuilder();
    var lines = p_Content.Replace("\r\n", "\n").Split('\n');

    bool Suppressed() => stack.Any(f => f.IsOwn && !f.KeepCurrentBranch);

    for (var i = 0; i < lines.Length; i++)
    {
        var line = lines[i];
        var eol = i == lines.Length - 1 ? "" : "\n";
        var trimmed = line.TrimStart();

        if (p_MatchOpen(trimmed) is { } open)
        {
            var wasSuppressed = Suppressed();
            stack.Push((open.IsOwn, open.InitialKeep));
            if (open.IsOwn) continue;
            if (!wasSuppressed) output.Append(line).Append(eol);
            continue;
        }

        if (p_MatchElseIf(trimmed) && stack.Count > 0)
        {
            if (stack.Peek().IsOwn)
                throw new InvalidOperationException("#elif/elseif() inside an own guarded block is not supported - restructure to a plain if/else/endif.");
            if (!Suppressed()) output.Append(line).Append(eol);
            continue;
        }

        if (p_MatchElse(trimmed) && stack.Count > 0)
        {
            var top = stack.Pop();
            var wasSuppressed = Suppressed();
            if (top.IsOwn) { stack.Push((true, !top.KeepCurrentBranch)); continue; }
            stack.Push(top);
            if (!wasSuppressed) output.Append(line).Append(eol);
            continue;
        }

        if (p_MatchClose(trimmed) && stack.Count > 0)
        {
            var top = stack.Pop();
            if (top.IsOwn) continue;
            if (!Suppressed()) output.Append(line).Append(eol);
            continue;
        }

        if (!Suppressed()) output.Append(line).Append(eol);
    }

    return output.ToString();
}

string StripCStyleBlocks(string p_Content, string p_Macro)
{
    var ifdefRe = new Regex(@"^\s*#\s*ifdef\s+(\w+)\s*$");
    var ifndefRe = new Regex(@"^\s*#\s*ifndef\s+(\w+)\s*$");
    var elifRe = new Regex(@"^\s*#\s*elif\b");
    var elseRe = new Regex(@"^\s*#\s*else\b");
    var endifRe = new Regex(@"^\s*#\s*endif\b");

    (bool, bool)? MatchOpen(string t)
    {
        var mIfdef = ifdefRe.Match(t);
        if (mIfdef.Success) return (mIfdef.Groups[1].Value == p_Macro, false); // ifdef(macro): dropped
        var mIfndef = ifndefRe.Match(t);
        if (mIfndef.Success) return (mIfndef.Groups[1].Value == p_Macro, true); // ifndef(macro): kept
        return null;
    }

    return StripBlocks(p_Content, MatchOpen, t => elifRe.IsMatch(t), t => elseRe.IsMatch(t), t => endifRe.IsMatch(t));
}

string StripCMakeStyleBlocks(string p_Content, string p_Option)
{
    var ifRe = new Regex(@"^\s*if\s*\(([^)]*)\)\s*$", RegexOptions.IgnoreCase);
    var elseifRe = new Regex(@"^\s*elseif\s*\(", RegexOptions.IgnoreCase);
    var elseRe = new Regex(@"^\s*else\s*\(\s*\)\s*$", RegexOptions.IgnoreCase);
    var endifRe = new Regex(@"^\s*endif\s*\(", RegexOptions.IgnoreCase);

    (bool, bool)? MatchOpen(string t)
    {
        var m = ifRe.Match(t);
        if (!m.Success) return null;
        var isOwn = m.Groups[1].Value.Trim().Equals(p_Option, StringComparison.OrdinalIgnoreCase);
        return (isOwn, false); // own if(<option>): option treated OFF -> initial branch dropped
    }

    return StripBlocks(p_Content, MatchOpen, t => elseifRe.IsMatch(t), t => elseRe.IsMatch(t), t => endifRe.IsMatch(t));
}

// test

int TestSuites(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var testsDir = Path.Combine(p_RootDir, p_Config.TestsDir);
    var configPath = Path.Combine(testsDir, p_Config.TestConfigFile);
    if (!File.Exists(configPath))
    {
        Console.WriteLine($"[ERROR] {configPath} not found.");
        return 1;
    }

    var suiteNames = File.ReadAllLines(configPath)
        .Select(l => l.Trim())
        .Where(l => l.Length > 0 && !l.StartsWith("#"))
        .ToArray();

    if (suiteNames.Length == 0)
    {
        Console.WriteLine($"[INFO] No suites listed in {configPath}.");
        return 0;
    }

    if (p_Rest.Contains("-ls"))
        return ListSuites(testsDir, suiteNames);

    var cfg = ResolveConfiguration(p_Config, p_Rest);
    if (cfg is null) return 1;

    var hadesDriverExe = Path.Combine(p_RootDir, p_Config.BinDir, cfg, p_Config.HadesDriverTarget + ".exe");
    if (!File.Exists(hadesDriverExe))
    {
        Console.WriteLine($"[ERROR] {hadesDriverExe} not found. Build it first: driver.bat build -c {cfg} -t {p_Config.HadesDriverTarget}");
        return 1;
    }

    var fbt = GetFlagValue(p_Rest, "--fbt");
    var runArgs = "run" + (fbt is null ? "" : $" --fbt={fbt}");

    var failures = new List<string>();
    foreach (var suiteName in suiteNames)
    {
        var suiteDir = Path.Combine(testsDir, suiteName);
        if (!File.Exists(Path.Combine(suiteDir, "suite.toml")))
        {
            Console.WriteLine($"[WARN] {suiteName}: no suite.toml under {suiteDir}, skipping.");
            failures.Add(suiteName);
            continue;
        }

        Console.WriteLine($"[INFO] === {suiteName} ===");
        if (RunAt(hadesDriverExe, "find-suite .", suiteDir) != 0 || RunAt(hadesDriverExe, runArgs, suiteDir) != 0)
            failures.Add(suiteName);
    }

    Console.WriteLine();
    if (failures.Count == 0)
    {
        Console.WriteLine($"[OK] All {suiteNames.Length} suite(s) passed.");
        return 0;
    }

    Console.WriteLine($"[ERROR] {failures.Count}/{suiteNames.Length} suite(s) failed: {string.Join(", ", failures)}");
    return 1;
}

int ListSuites(string p_TestsDir, string[] p_SuiteNames)
{
    foreach (var suiteName in p_SuiteNames)
    {
        Console.WriteLine(suiteName);

        var suiteTomlPath = Path.Combine(p_TestsDir, suiteName, "suite.toml");
        if (!File.Exists(suiteTomlPath))
        {
            Console.WriteLine("    [WARN] suite.toml not found");
            continue;
        }

        var testIds = ExtractTestIds(File.ReadAllLines(suiteTomlPath));
        if (testIds.Count == 0)
        {
            Console.WriteLine("    (no tests)");
            continue;
        }
        foreach (var id in testIds)
            Console.WriteLine($"    - {id}");
    }
    return 0;
}

// suite.toml's grammar is a flat "key = value" list per [[test]] block (see
// Hades-Benchmark's TomlIO.h) - a full parse isn't needed just to list ids,
// so this only ever looks for a top-level "id = ..." key.
List<string> ExtractTestIds(string[] p_Lines)
{
    var ids = new List<string>();
    foreach (var rawLine in p_Lines)
    {
        var line = rawLine.Trim();
        var eq = line.IndexOf('=');
        if (eq < 0 || line[..eq].Trim() != "id") continue;
        ids.Add(line[(eq + 1)..].Trim().Trim('"'));
    }
    return ids;
}

int RunAt(string p_Exe, string p_Arguments, string p_WorkingDirectory)
{
    var psi = new ProcessStartInfo(p_Exe, p_Arguments) { UseShellExecute = false, WorkingDirectory = p_WorkingDirectory };
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

// exec-gen
//
// Reads config/spectra-err.json + config/spectra-exec.json (two SEPARATE domain/code
// namespaces - fail-fast ErrContext vs recoverable InstrumentedException, per the
// SpectraInstrumentation design doc's Domain/Code Resolution Model). Every value is a
// packed 32-bit constant: 0xD0 [class-byte: EA=err/EB=exec] [ModuleId] [Code], with
// ModuleId/Code each a single 00-FF byte from the JSON. For every module referenced in
// either file, emits src/<Module>/src/Internal/<Module>.generated.h - no message strings
// ever land in the binary, those resolve offline against the JSON via crash-analysis tooling.

int ExecGen(string p_RootDir, Config p_Config, string[] p_Rest)
{
    var configDir = Path.Combine(p_RootDir, p_Config.ConfigDir);
    var srcDir = Path.Combine(p_RootDir, p_Config.SrcDir);

    var errPath = Path.Combine(configDir, p_Config.ErrCodesFile);
    var execPath = Path.Combine(configDir, p_Config.ExecCodesFile);

    if (!File.Exists(errPath)) { Console.WriteLine($"[ERROR] exec-gen: {errPath} not found."); return 1; }
    if (!File.Exists(execPath)) { Console.WriteLine($"[ERROR] exec-gen: {execPath} not found."); return 1; }

    var errModules = ParseCodeFile(errPath, "err_codes", "err_module_id", "err_domain_name");
    if (errModules is null) return 1;

    var execModules = ParseCodeFile(execPath, "exec_codes", "exec_module_id", "exec_domain_name");
    if (execModules is null) return 1;

    if (!ValidateUniquePairs(errPath, errModules)) return 1;
    if (!ValidateUniquePairs(execPath, execModules)) return 1;

    var moduleNames = errModules.Keys.Union(execModules.Keys).OrderBy(n => n, StringComparer.Ordinal).ToArray();
    if (moduleNames.Length == 0)
    {
        Console.WriteLine("[INFO] exec-gen: no modules with err/exec codes yet, nothing to generate.");
        return 0;
    }

    foreach (var moduleName in moduleNames)
    {
        var moduleDir = Path.Combine(srcDir, moduleName);
        if (!Directory.Exists(moduleDir))
        {
            Console.WriteLine($"[ERROR] exec-gen: module \"{moduleName}\" is referenced in {p_Config.ErrCodesFile}/{p_Config.ExecCodesFile} but {moduleDir} does not exist.");
            return 1;
        }

        var internalDir = Path.Combine(moduleDir, "src", "Internal");
        Directory.CreateDirectory(internalDir);

        errModules.TryGetValue(moduleName, out var errEntry);
        execModules.TryGetValue(moduleName, out var execEntry);

        var headerPath = Path.Combine(internalDir, $"{moduleName}.generated.h");
        File.WriteAllText(headerPath, GeneratedHeaderTemplate(errEntry, execEntry));
        Console.WriteLine($"[OK] exec-gen: wrote {headerPath}");
    }

    return 0;
}

// p_CodesKey/p_ModuleIdKey/p_DomainNameKey let one parser serve both files -
// "err_codes"/"err_module_id"/"err_domain_name" or "exec_codes"/"exec_module_id"/"exec_domain_name".
// Using the wrong key name on the wrong file is itself the self-check the file-naming
// convention exists for: it fails loudly instead of silently reading zero entries.
Dictionary<string, ExecGenEntry>? ParseCodeFile(string p_Path, string p_CodesKey, string p_ModuleIdKey, string p_DomainNameKey)
{
    using var doc = JsonDocument.Parse(File.ReadAllText(p_Path));

    if (!doc.RootElement.TryGetProperty("modules", out var modulesEl) || modulesEl.ValueKind != JsonValueKind.Array)
    {
        Console.WriteLine($"[ERROR] exec-gen: {p_Path} is missing a top-level \"modules\" array.");
        return null;
    }

    var result = new Dictionary<string, ExecGenEntry>();
    foreach (var moduleEl in modulesEl.EnumerateArray())
    {
        if (!moduleEl.TryGetProperty("module", out var moduleNameEl) || moduleNameEl.GetString() is not { } moduleName)
        {
            Console.WriteLine($"[ERROR] exec-gen: {p_Path} has a module entry missing \"module\".");
            return null;
        }
        if (result.ContainsKey(moduleName))
        {
            Console.WriteLine($"[ERROR] exec-gen: {p_Path} has a duplicate \"module\": \"{moduleName}\" entry.");
            return null;
        }
        if (!moduleEl.TryGetProperty(p_ModuleIdKey, out var moduleIdEl) || moduleIdEl.ValueKind != JsonValueKind.String
            || ParseHexByte(moduleIdEl.GetString()) is not { } moduleId)
        {
            Console.WriteLine($"[ERROR] exec-gen: {p_Path}, module \"{moduleName}\" is missing a valid hex-byte \"{p_ModuleIdKey}\" (e.g. \"0x01\", range 0x00-0xFF).");
            return null;
        }
        if (!moduleEl.TryGetProperty(p_DomainNameKey, out var domainNameEl) || domainNameEl.GetString() is not { } domainName)
        {
            Console.WriteLine($"[ERROR] exec-gen: {p_Path}, module \"{moduleName}\" is missing \"{p_DomainNameKey}\".");
            return null;
        }

        var codes = new List<(byte Code, string Name)>();
        var codeNames = new HashSet<string>(StringComparer.Ordinal);
        if (moduleEl.TryGetProperty(p_CodesKey, out var codesEl) && codesEl.ValueKind == JsonValueKind.Array)
        {
            foreach (var codeEl in codesEl.EnumerateArray())
            {
                if (!codeEl.TryGetProperty("code", out var codeValEl) || codeValEl.ValueKind != JsonValueKind.String
                    || ParseHexByte(codeValEl.GetString()) is not { } codeByte)
                {
                    Console.WriteLine($"[ERROR] exec-gen: {p_Path}, module \"{moduleName}\" has a \"{p_CodesKey}\" entry missing a valid hex-byte \"code\" (e.g. \"0x01\", range 0x00-0xFF).");
                    return null;
                }
                if (!codeEl.TryGetProperty("name", out var codeNameEl) || codeNameEl.GetString() is not { } codeName)
                {
                    Console.WriteLine($"[ERROR] exec-gen: {p_Path}, module \"{moduleName}\" has a \"{p_CodesKey}\" entry missing \"name\".");
                    return null;
                }
                if (!codeNames.Add(codeName))
                {
                    Console.WriteLine($"[ERROR] exec-gen: {p_Path}, module \"{moduleName}\" has duplicate code name \"{codeName}\".");
                    return null;
                }
                codes.Add((codeByte, codeName));
            }
        }

        result[moduleName] = new ExecGenEntry(moduleId, domainName, codes);
    }
    return result;
}

// (ModuleId, Code) pairs must be unique across the WHOLE file, not just per module -
// two modules sharing a module_id by mistake and then reusing the same code byte would
// collide into the same packed 32-bit constant, making it ambiguous at crash-analysis time.
bool ValidateUniquePairs(string p_Path, Dictionary<string, ExecGenEntry> p_Modules)
{
    var seen = new Dictionary<(byte ModuleId, byte Code), string>();
    foreach (var (moduleName, entry) in p_Modules)
    {
        foreach (var (code, name) in entry.Codes)
        {
            var key = (entry.ModuleId, code);
            if (seen.TryGetValue(key, out var owner))
            {
                Console.WriteLine($"[ERROR] exec-gen: {p_Path}: (module_id=0x{entry.ModuleId:X2}, code=0x{code:X2}) is used by both \"{owner}\" and \"{moduleName}.{name}\".");
                return false;
            }
            seen[key] = $"{moduleName}.{name}";
        }
    }
    return true;
}

// Packs the doc's locked byte layout: [0xD0 : fixed Spectra prefix][class byte: EA
// err / EB exec][ModuleId][Code]. This packed value IS the DomainCode passed to
// ErrScope/PushException - RawCode stays a separate, caller-supplied VkResult/
// cudaError_t/HRESULT value, untouched by this scheme.
uint PackExecGenCode(byte p_ClassByte, byte p_ModuleId, byte p_Code) =>
    (0xD0u << 24) | ((uint)p_ClassByte << 16) | ((uint)p_ModuleId << 8) | p_Code;

uint PackErrCode(byte p_ModuleId, byte p_Code) => PackExecGenCode(0xEA, p_ModuleId, p_Code);
uint PackExecCode(byte p_ModuleId, byte p_Code) => PackExecGenCode(0xEB, p_ModuleId, p_Code);

string GeneratedHeaderTemplate(ExecGenEntry? p_Err, ExecGenEntry? p_Exec)
{
    var sb = new StringBuilder();
    sb.AppendLine("#pragma once");
    sb.AppendLine("// AUTO-GENERATED by exec-gen from config/spectra-err.json and config/spectra-exec.json.");
    sb.AppendLine("// Do not edit - regenerated on every cmake-init run.");
    sb.AppendLine();
    sb.AppendLine("#include <cstdint>");
    sb.AppendLine();

    if (p_Err is { } err)
    {
        sb.AppendLine("namespace Spectra::Generated::ErrCodes {");
        sb.AppendLine($"    constexpr uint32_t kErr{err.DomainName}Domain = 0x{PackErrCode(err.ModuleId, 0x00):X8};");
        foreach (var (code, name) in err.Codes)
            sb.AppendLine($"    constexpr uint32_t kErr{name} = 0x{PackErrCode(err.ModuleId, code):X8};");
        sb.AppendLine("}");
        sb.AppendLine();
    }

    if (p_Exec is { } exec)
    {
        sb.AppendLine("namespace Spectra::Generated::ExecCodes {");
        sb.AppendLine($"    constexpr uint32_t kExec{exec.DomainName}Domain = 0x{PackExecCode(exec.ModuleId, 0x00):X8};");
        foreach (var (code, name) in exec.Codes)
            sb.AppendLine($"    constexpr uint32_t kExec{name} = 0x{PackExecCode(exec.ModuleId, code):X8};");
        sb.AppendLine("}");
        sb.AppendLine();
    }

    return sb.ToString();
}

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

// module_id/code are written as "0x01"-style single-byte hex strings in the config
// JSON (the doc's locked [0xD0][EA/EB][ModuleId][Code] layout) - JSON has no hex
// numeric literal, so these arrive as strings and get parsed+range-checked here.
byte? ParseHexByte(string? p_Value)
{
    if (p_Value is null) return null;
    var trimmed = p_Value.StartsWith("0x", StringComparison.OrdinalIgnoreCase) ? p_Value[2..] : p_Value;
    return byte.TryParse(trimmed, System.Globalization.NumberStyles.HexNumber, null, out var v) ? v : null;
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

string RunCaptureAt(string p_WorkingDirectory, string p_Exe, string p_Arguments)
{
    var psi = new ProcessStartInfo(p_Exe, p_Arguments)
    {
        UseShellExecute = false,
        RedirectStandardOutput = true,
        WorkingDirectory = p_WorkingDirectory
    };
    using var process = Process.Start(psi);
    var output = process!.StandardOutput.ReadToEnd();
    process.WaitForExit();
    return output;
}

// Pipes p_StdinContent to the process and captures stdout - clang-format reads the
// file to format from stdin (no temp file needed) and -assume-filename tells it
// which language/style to apply and where to start its .clang-format directory walk.
string RunPipedCapture(string p_Exe, string p_Arguments, string p_StdinContent)
{
    var psi = new ProcessStartInfo(p_Exe, p_Arguments)
    {
        UseShellExecute = false,
        RedirectStandardInput = true,
        RedirectStandardOutput = true
    };
    using var process = Process.Start(psi);
    process!.StandardInput.Write(p_StdinContent);
    process.StandardInput.Close();
    var output = process.StandardOutput.ReadToEnd();
    process.WaitForExit();
    if (process.ExitCode != 0)
        throw new InvalidOperationException($"{p_Exe} exited with code {process.ExitCode}");
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

    Dictionary<string, GitFilterEntry> GetGitFilters()
    {
        var result = new Dictionary<string, GitFilterEntry>(StringComparer.Ordinal);
        if (!root.TryGetProperty("gitFilters", out var filtersEl) || filtersEl.ValueKind != JsonValueKind.Object)
            return result;

        foreach (var filterProp in filtersEl.EnumerateObject())
        {
            var entryEl = filterProp.Value;
            if (!entryEl.TryGetProperty("cMacro", out var cMacroEl) || cMacroEl.GetString() is not { } cMacro)
            {
                Console.WriteLine($"[ERROR] {configPath}: gitFilters.\"{filterProp.Name}\" is missing \"cMacro\".");
                Environment.Exit(1);
                return result;
            }
            if (!entryEl.TryGetProperty("cmakeOption", out var cmakeOptionEl) || cmakeOptionEl.GetString() is not { } cmakeOption)
            {
                Console.WriteLine($"[ERROR] {configPath}: gitFilters.\"{filterProp.Name}\" is missing \"cmakeOption\".");
                Environment.Exit(1);
                return result;
            }
            if (!entryEl.TryGetProperty("appliesTo", out var appliesToEl) || appliesToEl.ValueKind != JsonValueKind.Array)
            {
                Console.WriteLine($"[ERROR] {configPath}: gitFilters.\"{filterProp.Name}\" is missing array \"appliesTo\".");
                Environment.Exit(1);
                return result;
            }
            var appliesTo = appliesToEl.EnumerateArray().Select(e => e.GetString() ?? "").ToArray();
            result[filterProp.Name] = new GitFilterEntry(cMacro, cmakeOption, appliesTo);
        }
        return result;
    }

    return new Config(
        GetString("buildDir"),
        GetString("binDir"),
        GetString("clangClBuildDir"),
        GetString("clangClBinDir"),
        GetString("cmakeGenerator"),
        GetString("clangClToolset"),
        GetString("defaultConfig"),
        GetString("runTarget"),
        GetStringArray("buildConfigurations"),
        GetString("cudaCompilerExe"),
        GetString("cudaInstallerScript"),
        GetString("vulkanHeaderRelPath"),
        GetString("vulkanInstallerScript"),
        GetString("testsDir"),
        GetString("testConfigFile"),
        GetString("hadesDriverTarget"),
        GetString("srcDir"),
        GetString("configDir"),
        GetString("errCodesFile"),
        GetString("execCodesFile"),
        GetGitFilters());
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
    string ClangClBuildDir,
    string ClangClBinDir,
    string CmakeGenerator,
    string ClangClToolset,
    string DefaultConfig,
    string RunTarget,
    string[] BuildConfigurations,
    string CudaCompilerExe,
    string CudaInstallerScript,
    string VulkanHeaderRelPath,
    string VulkanInstallerScript,
    string TestsDir,
    string TestConfigFile,
    string HadesDriverTarget,
    string SrcDir,
    string ConfigDir,
    string ErrCodesFile,
    string ExecCodesFile,
    Dictionary<string, GitFilterEntry> GitFilters);

record ExecGenEntry(byte ModuleId, string DomainName, List<(byte Code, string Name)> Codes);

// cMacro: C/C++ preprocessor macro guarding #ifdef blocks in .h/.cpp files.
// cmakeOption: CMake option() guarding if()/endif() blocks in CMakeLists.txt.
// appliesTo: submodule-relative paths (from repo root) this filter is scoped to -
// "enable"/"disable" run `git config` inside each of these, not the outer repo.
record GitFilterEntry(string CMacro, string CmakeOption, string[] AppliesTo);
