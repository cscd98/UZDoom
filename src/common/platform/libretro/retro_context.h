#pragma once
#include <cstddef>
#include <cstdint>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdexcept>

struct RetroContext
{
    void *fiber = nullptr;
    bool converted = false;
    void (*entry)() = nullptr;
};

inline void SwitchContext(RetroContext *, RetroContext *to) { SwitchToFiber(to->fiber); }

inline void InitContext(RetroContext &context, void *, size_t size,
    void (*entry)(), RetroContext &caller)
{
    caller.converted = !IsThreadAFiber();
    caller.fiber = caller.converted ? ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH) : GetCurrentFiber();

    if (!caller.fiber)
    {
        caller = {};
        throw std::runtime_error("Could not convert frontend thread to a fiber");
    }

    context.entry = entry;
    context.fiber = CreateFiberEx(size, size, FIBER_FLAG_FLOAT_SWITCH,
        [](void *opaque) { static_cast<RetroContext *>(opaque)->entry(); }, &context);

    if (!context.fiber)
    {
        if (caller.converted)
            ConvertFiberToThread();
        caller = {};
        throw std::runtime_error("Could not create engine fiber");
    }
}
inline void DestroyContext(RetroContext &context, RetroContext &caller)
{
    if (context.fiber)
        DeleteFiber(context.fiber);
    if (caller.converted)
        ConvertFiberToThread();
    context = {}; caller = {};
}

#elif defined(__ANDROID__) || (defined(__APPLE__) && defined(__aarch64__))
// AAPCS64 callee-saved integer registers, low halves of v8-v15, SP.
struct alignas(16) RetroContext { uint64_t registers[22]{}; };
extern "C" void SwitchContext(RetroContext *from, const RetroContext *to);

inline void InitContext(RetroContext &context, void *stack, size_t size,
    void (*entry)(), RetroContext &)
{
    context = {};
    context.registers[11] = reinterpret_cast<uintptr_t>(entry); // x30 / return PC
    context.registers[20] = (reinterpret_cast<uintptr_t>(stack) + size) & ~uintptr_t(15);
}
#else
#include <ucontext.h>

using RetroContext = ucontext_t;
inline void SwitchContext(RetroContext *from, RetroContext *to)
{
    swapcontext(from, to);
}

inline void InitContext(RetroContext &context, void *stack, size_t size,
    void (*entry)(), RetroContext &caller)
{
    getcontext(&context);
    context.uc_stack.ss_sp = stack;
    context.uc_stack.ss_size = size;
    context.uc_link = &caller;
    makecontext(&context, entry, 0);
}
#endif

#ifndef _WIN32
inline void DestroyContext(RetroContext &, RetroContext &)
{
}
#endif
