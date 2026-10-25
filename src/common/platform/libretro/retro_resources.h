#pragma once
#include <cstddef>
#include <cstring>

namespace Libretro
{
inline constexpr const char *EmbeddedResourcePath = "uzdoom-internal:/uzdoom.pk3";
inline constexpr const char *EmbeddedSupportPath = "uzdoom-internal:/game_support.pk3";

inline const char *EmbeddedPathForName(const char *name)
{
    if (!name)
        return nullptr;

    if (!std::strcmp(name, "uzdoom.pk3"))
        return EmbeddedResourcePath;

    if (!std::strcmp(name, "game_support.pk3"))
        return EmbeddedSupportPath;

    return nullptr;
}

inline bool IsEmbeddedResource(const char *path)
{
    return path && (!std::strcmp(path, EmbeddedResourcePath) || !std::strcmp(path, EmbeddedSupportPath));
}

const unsigned char *EmbeddedResourceData(const char *path = EmbeddedResourcePath);

size_t EmbeddedResourceSize(const char *path = EmbeddedResourcePath);

} // namespace Libretro
