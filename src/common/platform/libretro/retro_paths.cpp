#include "retro_platform.h"
#include "zstring.h"

FString GetUserFile(const char *file) { return FString(Libretro::SaveDirectory()) + "/" + file; }
FString M_GetAppDataPath(bool) { return Libretro::SystemDirectory(); }
FString M_GetCachePath(bool, FString) { return Libretro::SaveDirectory(); }
FString M_GetAutoexecPath() { return GetUserFile("autoexec.cfg"); }
FString M_GetConfigPath(bool) { return GetUserFile("uzdoom-libretro.ini"); }
FString M_GetDocumentsPath() { return FString(Libretro::SaveDirectory()) + "/"; }
FString M_GetScreenshotsPath() { return Libretro::SaveDirectory(); }
FString M_GetSavegamesPath() { return M_GetDocumentsPath(); }
FString M_GetDemoPath() { return M_GetDocumentsPath(); }
FString M_GetNormalizedPath(const char *path) { return path; }
const char *GetConfigPath() { return Libretro::SaveDirectory(); }
const char *GetDataPath() { return Libretro::SystemDirectory(); }
const char *GetCachePath() { return Libretro::SaveDirectory(); }
const char *GetPicturesPath() { return Libretro::SaveDirectory(); }

#ifdef __APPLE__
FString M_GetMacAppSupportPath(bool) { return Libretro::SystemDirectory(); }
void M_GetMacSearchDirectories(FString &documents, FString &userSupport, FString &localSupport)
{
    documents = Libretro::SaveDirectory();
    userSupport = Libretro::SystemDirectory();
    localSupport = Libretro::SystemDirectory();
}
#endif
