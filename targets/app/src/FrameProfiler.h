#ifndef FRAME_PROFILER_H
#define FRAME_PROFILER_H

#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>

class FrameProfiler {
public:
    enum class Bucket {
        Frame,
        World,
        Terrain,
        ChunkCull,
        ChunkCollect,
        ChunkPlayback,
        ChunkDirtyScan,
        ChunkRebuildSchedule,
        ChunkRebuildBody,
        ChunkPrepass,
        ChunkBlockShape,
        ChunkBlockFaceCull,
        ChunkBlockLighting,
        ChunkBlockEmit,
        RenderableTileEntityCleanup,
        TileEntityUnloadCleanup,
        Entity,
        Particle,
        WeatherSky,
        UIHud,
        Lightmap,
        Count
    };

    struct BucketDescriptor {
        Bucket bucket;
        std::string_view label;
    };

    static constexpr std::size_t BucketCount() noexcept {
        return static_cast<std::size_t>(Bucket::Count);
    }

    static constexpr std::size_t BucketIndex(Bucket bucket) noexcept {
        return static_cast<std::size_t>(bucket);
    }

#ifdef ENABLE_FRAME_PROFILER
    static bool IsEnabled() noexcept;
    static void Record(Bucket bucket, std::uint64_t elapsedNs) noexcept;
    static void EndFrame(std::uint64_t elapsedNs) noexcept;

    class Scope {
    public:
        explicit Scope(Bucket bucket) noexcept;
        ~Scope() noexcept;

        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;

    private:
        std::uint64_t m_startNs;
        Bucket m_bucket;
        bool m_enabled;
    };

    class FrameScope {
    public:
        FrameScope() noexcept;
        ~FrameScope() noexcept;

        FrameScope(const FrameScope&) = delete;
        FrameScope& operator=(const FrameScope&) = delete;

    private:
        std::uint64_t m_startNs;
        bool m_enabled;
    };
#else
    // Safe fallback stubs so the game compiles without overhead when the profiler is disabled
    static constexpr bool IsEnabled() noexcept { return false; }
    static inline void Record(Bucket, std::uint64_t) noexcept {}
    static inline void EndFrame(std::uint64_t) noexcept {}

    class Scope {
    public:
        explicit Scope(Bucket) noexcept {}
        ~Scope() noexcept = default;
    };

    class FrameScope {
    public:
        FrameScope() noexcept {}
        ~FrameScope() noexcept = default;
    };
#endif
};

#endif // FRAME_PROFILER_H