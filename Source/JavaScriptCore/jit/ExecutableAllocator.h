/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ExecutableAllocator_h
#define ExecutableAllocator_h
#include "JITCompilationEffort.h"
#include <stddef.h> // for ptrdiff_t
#include <limits>
#include <wtf/Assertions.h>
#include <wtf/Lock.h>
#include <wtf/MetaAllocatorHandle.h>
#include <wtf/MetaAllocator.h>
#include <wtf/PageAllocation.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>

#if OS(DARWIN)
#include <libkern/OSCacheControl.h>
#endif

#if OS(DARWIN)
#include <sys/mman.h>
#endif

#if CPU(MIPS) && OS(LINUX)
#include <sys/cachectl.h>
#endif

#if CPU(SH4) && OS(LINUX)
#include <asm/cachectl.h>
#include <asm/unistd.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

#define JIT_ALLOCATOR_LARGE_ALLOC_SIZE (pageSize() * 4)

#define EXECUTABLE_POOL_WRITABLE true

namespace JSC {

class VM;

static const unsigned jitAllocationGranule = 32;

typedef WTF::MetaAllocatorHandle ExecutableMemoryHandle;

#if ENABLE(ASSEMBLER)

#if ENABLE(EXECUTABLE_ALLOCATOR_DEMAND)
class DemandExecutableAllocator;
#endif

#if ENABLE(EXECUTABLE_ALLOCATOR_FIXED)
#if defined(FIXED_EXECUTABLE_MEMORY_POOL_SIZE_IN_MB) && FIXED_EXECUTABLE_MEMORY_POOL_SIZE_IN_MB > 0
static const size_t fixedExecutableMemoryPoolSize = FIXED_EXECUTABLE_MEMORY_POOL_SIZE_IN_MB * 1024 * 1024;
#elif CPU(ARM) || CPU(MIPS)
static const size_t fixedExecutableMemoryPoolSize = 16 * 1024 * 1024;
#elif CPU(ARM64)
static const size_t fixedExecutableMemoryPoolSize = 32 * 1024 * 1024;
#elif CPU(X86_64)
static const size_t fixedExecutableMemoryPoolSize = 1024 * 1024 * 1024;
#else
static const size_t fixedExecutableMemoryPoolSize = 32 * 1024 * 1024;
#endif
#if CPU(ARM)
static const double executablePoolReservationFraction = 0.15;
#else
static const double executablePoolReservationFraction = 0.25;
#endif

extern uintptr_t startOfFixedExecutableMemoryPool;
#endif

class ExecutableAllocator {
    enum ProtectionSetting { Writable, Executable };

public:
    ExecutableAllocator(VM&);
    ~ExecutableAllocator();

    static void initializeAllocator();

    bool isValid() const;

    static bool underMemoryPressure();

    static double memoryPressureMultiplier(size_t addedMemoryUsage);

#if ENABLE(META_ALLOCATOR_PROFILE)
    static void dumpProfile();
#else
    static void dumpProfile() { }
#endif

    RefPtr<ExecutableMemoryHandle> allocate(VM&, size_t sizeInBytes, void* ownerUID, JITCompilationEffort);

    bool isValidExecutableMemory(const LockHolder&, void* address);

    static size_t committedByteCount();

    Lock& getLock() const;
};

#endif // ENABLE(JIT) && ENABLE(ASSEMBLER)

} // namespace JSC

#endif // !defined(ExecutableAllocator)
