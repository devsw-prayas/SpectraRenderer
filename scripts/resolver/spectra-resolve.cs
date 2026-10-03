// Spectra Resolve - turns SpectraInstrumentation's integer-only records back into text.
//
//   dotnet run scripts/resolver/spectra-resolve.cs -- code <packed>             e.g. code 0xD0EA0319
//   dotnet run scripts/resolver/spectra-resolve.cs -- label <binary> <offset>   e.g. label bin/Release_win64/SpectraLauncher.exe 0x1A3F40
//
// Dump resolution waits on the capture format and on how a record names its image.

using System.Runtime.CompilerServices;
using System.Text;
using System.Text.Json;

if (args.Length == 0 || args[0] is "help" or "-h" or "--help")
{
    PrintUsage();
    return 0;
}

var rootDir = FindRepoRoot();
if (rootDir is null)
{
    Console.WriteLine("[ERROR] Could not locate the repo root (expected CMakeLists.txt two levels above this script).");
    return 1;
}

return args[0] switch
{
    "code" => ResolveCode(rootDir, args[1..]),
    "label" => ResolveLabel(rootDir, args[1..]),
    _ => Unknown(args[0]),
};

int Unknown(string p_Command)
{
    Console.WriteLine($"[ERROR] Unknown command \"{p_Command}\".");
    PrintUsage();
    return 1;
}

void PrintUsage()
{
    Console.WriteLine("Spectra Resolve - turns SpectraInstrumentation record integers back into text.");
    Console.WriteLine();
    Console.WriteLine("  code <packed>              Decode a packed DomainCode (0xD0 [EA|EB] [ModuleId] [Code]) against config/spectra-err.json / spectra-exec.json");
    Console.WriteLine("  label <binary> <offset>    Read the label string at an image-relative offset (FmtStringOffset) inside a PE binary");
    Console.WriteLine();
    Console.WriteLine("Numbers accept 0x-prefixed hex or decimal.");
}

// ---------------------------------------------------------------------------
// code
// ---------------------------------------------------------------------------

// Mirrors exec-gen's packing: [0xD0 fixed prefix][class: EA = err / EB = exec][ModuleId][Code].
int ResolveCode(string p_RootDir, string[] p_Rest)
{
    if (p_Rest.Length != 1 || ParseNumber(p_Rest[0]) is not { } packed || packed > uint.MaxValue)
    {
        Console.WriteLine("[ERROR] usage: code <packed 32-bit value>");
        return 1;
    }

    var value = (uint)packed;
    var prefix = (byte)(value >> 24);
    var classByte = (byte)(value >> 16);
    var moduleId = (byte)(value >> 8);
    var code = (byte)value;

    if (prefix != 0xD0)
    {
        Console.WriteLine($"[ERROR] 0x{value:X8} is not a Spectra code (prefix 0x{prefix:X2}, expected 0xD0).");
        return 1;
    }

    var (file, codesKey, moduleIdKey, domainKey, kind) = classByte switch
    {
        0xEA => ("spectra-err.json", "err_codes", "err_module_id", "err_domain_name", "fail-fast (ErrContext)"),
        0xEB => ("spectra-exec.json", "exec_codes", "exec_module_id", "exec_domain_name", "recoverable (InstrumentedException)"),
        _ => (null, null, null, null, null),
    };
    if (file is null)
    {
        Console.WriteLine($"[ERROR] 0x{value:X8} has unknown class byte 0x{classByte:X2} (expected 0xEA err or 0xEB exec).");
        return 1;
    }

    var path = Path.Combine(p_RootDir, "config", file);
    if (!File.Exists(path))
    {
        Console.WriteLine($"[ERROR] {path} not found.");
        return 1;
    }

    using var doc = JsonDocument.Parse(File.ReadAllText(path));
    foreach (var moduleEl in doc.RootElement.GetProperty("modules").EnumerateArray())
    {
        if (ParseNumber(moduleEl.GetProperty(moduleIdKey!).GetString()) != moduleId) continue;

        var moduleName = moduleEl.GetProperty("module").GetString();
        var domainName = moduleEl.GetProperty(domainKey!).GetString();
        Console.WriteLine($"0x{value:X8}  {kind}");
        Console.WriteLine($"  module : {moduleName} (id 0x{moduleId:X2}, domain {domainName})");

        // Code 0x00 is the bare domain constant (k*Domain in the generated header), not an entry.
        if (code == 0)
        {
            Console.WriteLine("  code   : 0x00 (domain constant, no specific code)");
            return 0;
        }

        if (moduleEl.TryGetProperty(codesKey!, out var codesEl))
        {
            foreach (var codeEl in codesEl.EnumerateArray())
            {
                if (ParseNumber(codeEl.GetProperty("code").GetString()) != code) continue;
                Console.WriteLine($"  code   : 0x{code:X2} {codeEl.GetProperty("name").GetString()}");
                if (codeEl.TryGetProperty("message", out var messageEl))
                    Console.WriteLine($"  message: {messageEl.GetString()}");
                return 0;
            }
        }

        Console.WriteLine($"  code   : 0x{code:X2} <not in {file} - table out of date with the binary?>");
        return 1;
    }

    Console.WriteLine($"[ERROR] No module with {moduleIdKey} 0x{moduleId:X2} in {file}.");
    return 1;
}

// ---------------------------------------------------------------------------
// label
// ---------------------------------------------------------------------------

// FmtStringOffset is an RVA. Only valid against the exact binary that wrote it: rebuilds move .rdata.
int ResolveLabel(string p_RootDir, string[] p_Rest)
{
    if (p_Rest.Length != 2 || ParseNumber(p_Rest[1]) is not { } rva || rva > uint.MaxValue)
    {
        Console.WriteLine("[ERROR] usage: label <binary> <offset>");
        return 1;
    }

    var binaryPath = Path.IsPathRooted(p_Rest[0]) ? p_Rest[0] : Path.Combine(p_RootDir, p_Rest[0]);
    if (!File.Exists(binaryPath))
    {
        Console.WriteLine($"[ERROR] {binaryPath} not found.");
        return 1;
    }

    var image = File.ReadAllBytes(binaryPath);
    if (RvaToFileOffset(image, (uint)rva) is not { } fileOffset)
    {
        Console.WriteLine($"[ERROR] 0x{rva:X} is not inside any section of {Path.GetFileName(binaryPath)} (wrong binary or not a label offset?).");
        return 1;
    }

    var end = fileOffset;
    while (end < image.Length && image[end] != 0) ++end;
    if (end == image.Length)
    {
        Console.WriteLine($"[ERROR] No string terminator after 0x{rva:X} - not a label offset.");
        return 1;
    }

    Console.WriteLine(Encoding.UTF8.GetString(image, fileOffset, end - fileOffset));
    return 0;
}

int? RvaToFileOffset(byte[] p_Image, uint p_Rva)
{
    if (p_Image.Length < 0x40 || p_Image[0] != 'M' || p_Image[1] != 'Z') return null;

    var peOffset = BitConverter.ToInt32(p_Image, 0x3C);
    if (peOffset < 0 || peOffset + 24 > p_Image.Length || BitConverter.ToUInt32(p_Image, peOffset) != 0x00004550) return null; // "PE\0\0"

    var coff = peOffset + 4;
    var sectionCount = BitConverter.ToUInt16(p_Image, coff + 2);
    var optionalHeaderSize = BitConverter.ToUInt16(p_Image, coff + 16);
    var sectionTable = coff + 20 + optionalHeaderSize;

    for (var i = 0; i < sectionCount; ++i)
    {
        var header = sectionTable + i * 40;
        if (header + 40 > p_Image.Length) return null;

        var virtualSize = BitConverter.ToUInt32(p_Image, header + 8);
        var virtualAddress = BitConverter.ToUInt32(p_Image, header + 12);
        var rawSize = BitConverter.ToUInt32(p_Image, header + 16);
        var rawPointer = BitConverter.ToUInt32(p_Image, header + 20);

        // Only the raw-backed part has bytes on disk; beyond it is zero-fill (.bss-like).
        if (p_Rva < virtualAddress || p_Rva >= virtualAddress + Math.Min(virtualSize, rawSize)) continue;
        var fileOffset = (long)rawPointer + (p_Rva - virtualAddress);
        return fileOffset < p_Image.Length ? (int)fileOffset : null;
    }
    return null;
}

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

ulong? ParseNumber(string? p_Text)
{
    if (string.IsNullOrWhiteSpace(p_Text)) return null;
    var text = p_Text.Trim();
    if (text.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
        return ulong.TryParse(text[2..], System.Globalization.NumberStyles.HexNumber, null, out var hex) ? hex : null;
    return ulong.TryParse(text, out var dec) ? dec : null;
}

string? FindRepoRoot()
{
    var scriptDir = Path.GetDirectoryName(ThisFilePath())!;
    var candidateRoot = Path.GetFullPath(Path.Combine(scriptDir, "..", ".."));
    return File.Exists(Path.Combine(candidateRoot, "CMakeLists.txt")) ? candidateRoot : null;
}

string ThisFilePath([CallerFilePath] string p_Path = "") => p_Path;
