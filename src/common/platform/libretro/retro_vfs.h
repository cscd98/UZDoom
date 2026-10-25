#pragma once

#include <cstddef>
#include <cstdio>
#include "libretro.h"

namespace FileSys { class FileReaderInterface; class FileWriter; }

namespace Libretro
{
bool InitVFS(retro_environment_t environment);
bool HasVFS();
FileSys::FileWriter *OpenVFSWriter(const char *path);
FILE *OpenVFSFile(const char *path, const char *mode);
bool StatVFS(const char *path, bool *directory, size_t *size = nullptr);
const retro_vfs_interface *VFS();

FileSys::FileReaderInterface *OpenVFSReader(const char *path,
    ptrdiff_t start = 0, ptrdiff_t length = -1);
}
