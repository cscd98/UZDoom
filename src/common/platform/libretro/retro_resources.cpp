#include "retro_resources.h"
#include <cstdint>

extern "C"
{
extern const unsigned char uzdoom_pk3_start[];
extern const unsigned char uzdoom_pk3_end[];
extern const unsigned char game_support_pk3_start[];
extern const unsigned char game_support_pk3_end[];
}

namespace Libretro
{

const unsigned char *EmbeddedResourceData(const char *path)
{
    if (!IsEmbeddedResource(path))
        return nullptr;

    return !std::strcmp(path, EmbeddedSupportPath) ? game_support_pk3_start : uzdoom_pk3_start;
}

size_t EmbeddedResourceSize(const char *path)
{
    auto start = EmbeddedResourceData(path);

    if (!start)
        return 0;

    auto end = !std::strcmp(path, EmbeddedSupportPath) ? game_support_pk3_end : uzdoom_pk3_end;

    return reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(start);
}

} // namespace Libretro
