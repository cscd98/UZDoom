#include "retro_platform.h"
#include "c_cvars.h"
#include "i_system.h"
#include "i_interface.h"
#include "st_start.h"
#include "m_argv.h"
#include "printf.h"
#include "stringtable.h"
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#include <direct.h>
#else
#include <unistd.h>
#if defined(__ANDROID__) && __ANDROID_API__ < 28
#include <sys/syscall.h>
#elif !defined(__APPLE__) && !defined(__WEBOS__)
#include <sys/random.h>
#endif
#include <sys/utsname.h>
#endif
#include <chrono>
#include <cstdlib>
#ifdef __WEBOS__
#include <cstdio>
#endif

extern const char * const BACKEND = "libretro";
FString sys_ostype;
FArgs *Args = nullptr;
double PerfToSec = 0, PerfToMillisec = 0;
bool PerfAvailable = false;
CVAR(String, queryiwad_key, "", CVAR_GLOBALCONFIG | CVAR_ARCHIVE);

FString I_DetectOS()
{
#ifdef _WIN32
    sys_ostype = "Windows (libretro)";
#else
    utsname info{};
    if (uname(&info) == 0) sys_ostype.Format("%s %s on %s", info.sysname, info.release, info.machine);
#endif
    return sys_ostype;
}

void CalculateCPUSpeed() {} // Use the engine's portable timing implementation.

void I_ShowFatalError(const char *message)
{
    Libretro::Log(RETRO_LOG_ERROR, message);
}

void I_PrintStr(const char *message)
{
    Libretro::Log(RETRO_LOG_INFO, message);
}

void I_SetIWADInfo() {}

bool HoldingQueryKey(const char *)
{
    return false;
}

bool I_PickIWad(bool, FStartupSelectionInfo &info)
{
    return info.Wads && info.DefaultIWAD >= 0 &&
        static_cast<unsigned>(info.DefaultIWAD) < info.Wads->Size();
}

bool I_SetCursor(FGameTexture *)
{
    return false;
}

void I_SetWindowTitle(const char *) {}

void I_OpenShellFolder(const char *) {} // The core must not launch host applications.

bool I_IsDarkMode() { return false; }

static FString clipboard;

void I_PutInClipboard(const char *text)
{
    clipboard = text ? text : "";
}

FString I_GetFromClipboard(bool)
{
    return clipboard;
}

FString I_GetCWD()
{
#ifdef _WIN32
    wchar_t *cwd = _wgetcwd(nullptr, 0);
    FString result(cwd ? cwd : L"");
#else
    char *cwd = getcwd(nullptr, 0);
    FString result(cwd ? cwd : "");
#endif
    free(cwd);

    return result;
}

bool I_ChDir(const char *path)
{
#ifdef _WIN32
    return SetCurrentDirectoryW(WideString(path).c_str()) != 0;
#else
    return chdir(path) == 0;
#endif
}

unsigned int I_MakeRNGSeedWebOS32()
{
    unsigned int seed = 0;

    FILE* f = fopen("/dev/urandom", "rb");
    if (f)
    {
        size_t n = fread(&seed, 1, sizeof(seed), f);
        fclose(f);

        if (n == sizeof(seed))
            return seed;
    }

    return static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count());
}

unsigned I_MakeRNGSeed()
{
    unsigned seed;
#if defined(_WIN32)
    if (BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(&seed), sizeof(seed), BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0)
        return seed;
#elif defined(__APPLE__)
    return arc4random();
#elif defined(__WEBOS__)
    return I_MakeRNGSeedWebOS32();
#elif defined(__ANDROID__) && __ANDROID_API__ < 28
    if (syscall(SYS_getrandom, &seed, sizeof(seed), 0) == sizeof(seed))
        return seed;
#else
    if (getrandom(&seed, sizeof(seed), 0) == sizeof(seed))
        return seed;
#endif
    return static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count());
}

extern "C" int I_FileAvailable(const char *)
{
    return 0;
}

FStartupScreen *FStartupScreen::CreateInstance(int progress)
{
    return new FStartupScreen(progress);
}

#ifdef _WIN32
FString I_GetSteamPath() { return ""; }
TArray<FString> I_GetGogPaths() { return {}; }
TArray<FString> I_GetBethesdaPath() { return {}; }

bool I_WriteIniFailed(const char *filename)
{
    FString message;
    message.Format("Could not save config file %s", filename);
    Libretro::Log(RETRO_LOG_ERROR, message.GetChars());

    return false;
}
#endif

#ifdef _WIN32
FString FStringTable::GetSystemLocale()
{
    wchar_t name[LOCALE_NAME_MAX_LENGTH];

    return GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH) > 0 ? FString(name) : FString("en-US");
}
#endif
