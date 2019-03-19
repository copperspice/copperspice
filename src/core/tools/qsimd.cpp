/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qsimd_p.h>
#include <QByteArray>
#include <stdio.h>

#if defined(Q_OS_WIN64)
#include <intrin.h>
#endif

#if defined(Q_OS_LINUX) && defined(__arm__)
#include <qcore_unix_p.h>

// the kernel header definitions for HWCAP_*
// (the ones we need/may need anyway)

// copied from <asm/hwcap.h> (ARM)
#define HWCAP_IWMMXT    512
#define HWCAP_CRUNCH    1024
#define HWCAP_THUMBEE   2048
#define HWCAP_NEON      4096
#define HWCAP_VFPv3     8192
#define HWCAP_VFPv3D16  16384

// copied from <linux/auxvec.h>
#define AT_HWCAP  16    /* arch dependent hints at CPU capabilities */

#endif

QT_BEGIN_NAMESPACE

#if defined (Q_OS_NACL)
static inline uint detectProcessorFeatures()
{
   return 0;
}

#elif defined(__arm__) || defined(__arm) || defined(QT_HAVE_IWMMXT) || defined(QT_HAVE_NEON)
static inline uint detectProcessorFeatures()
{
   uint features = 0;

#if defined(Q_OS_LINUX)
   int auxv = ::qt_safe_open("/proc/self/auxv", O_RDONLY);
   if (auxv != -1) {
      unsigned long vector[64];
      int nread;
      while (features == 0) {
         nread = ::qt_safe_read(auxv, (char *)vector, sizeof vector);
         if (nread <= 0) {
            // EOF or error
            break;
         }

         int max = nread / (sizeof vector[0]);
         for (int i = 0; i < max; i += 2)
            if (vector[i] == AT_HWCAP) {
               if (vector[i + 1] & HWCAP_IWMMXT) {
                  features |= IWMMXT;
               }
               if (vector[i + 1] & HWCAP_NEON) {
                  features |= NEON;
               }
               break;
            }
      }

      ::qt_safe_close(auxv);
      return features;
   }
   // fall back if /proc/self/auxv wasn't found
#endif

#if defined(QT_HAVE_IWMMXT)
   // runtime detection only available when running as a previlegied process
   features = IWMMXT;

#elif defined(QT_ALWAYS_HAVE_NEON)
   features = NEON;

#endif

   return features;
}

#elif defined(__i386__) || defined(_M_IX86)
static inline uint detectProcessorFeatures()
{
   uint features = 0;

   unsigned int extended_result = 0;
   unsigned int feature_result = 0;
   uint result = 0;
   /* see p. 118 of amd64 instruction set manual Vol3 */
#if defined(Q_CC_GNU)
   long cpuid_supported, tmp1;
   asm ("pushf\n"
        "pop %0\n"
        "mov %0, %1\n"
        "xor $0x00200000, %0\n"
        "push %0\n"
        "popf\n"
        "pushf\n"
        "pop %0\n"
        "xor %1, %0\n" // %eax is now 0 if CPUID is not supported
        : "=a" (cpuid_supported), "=r" (tmp1)
       );
   if (cpuid_supported) {
      asm ("xchg %%ebx, %2\n"
           "cpuid\n"
           "xchg %%ebx, %2\n"
           : "=c" (feature_result), "=d" (result), "=&r" (tmp1)
           : "a" (1));

      asm ("xchg %%ebx, %1\n"
           "cpuid\n"
           "cmp $0x80000000, %%eax\n"
           "jnbe 1f\n"
           "xor %0, %0\n"
           "jmp 2f\n"
           "1:\n"
           "mov $0x80000001, %%eax\n"
           "cpuid\n"
           "2:\n"
           "xchg %%ebx, %1\n"
           : "=d" (extended_result), "=&r" (tmp1)
           : "a" (0x80000000)
           : "%ecx"
          );
   }

#elif defined (Q_OS_WIN)
   _asm {
      push eax
      push ebx
      push ecx
      push edx
      pushfd
      pop eax
      mov ebx, eax
      xor eax, 00200000h
      push eax
      popfd
      pushfd
      pop eax
      mov edx, 0
      xor eax, ebx
      jz skip

      mov eax, 1
      cpuid
      mov result, edx
      mov feature_result, ecx
      skip:
      pop edx
      pop ecx
      pop ebx
      pop eax
   }

   _asm {
      push eax
      push ebx
      push ecx
      push edx
      pushfd
      pop eax
      mov ebx, eax
      xor eax, 00200000h
      push eax
      popfd
      pushfd
      pop eax
      mov edx, 0
      xor eax, ebx
      jz skip2

      mov eax, 80000000h
      cpuid
      cmp eax, 80000000h
      jbe skip2
      mov eax, 80000001h
      cpuid
      mov extended_result, edx
      skip2:
      pop edx
      pop ecx
      pop ebx
      pop eax
   }
#endif


   // result now contains the standard feature bits
   if (result & (1u << 15)) {
      features |= CMOV;
   }
   if (result & (1u << 23)) {
      features |= MMX;
   }
   if (extended_result & (1u << 22)) {
      features |= MMXEXT;
   }
   if (extended_result & (1u << 31)) {
      features |= MMX3DNOW;
   }
   if (extended_result & (1u << 30)) {
      features |= MMX3DNOWEXT;
   }
   if (result & (1u << 25)) {
      features |= SSE;
   }
   if (result & (1u << 26)) {
      features |= SSE2;
   }
   if (feature_result & (1u)) {
      features |= SSE3;
   }
   if (feature_result & (1u << 9)) {
      features |= SSSE3;
   }
   if (feature_result & (1u << 19)) {
      features |= SSE4_1;
   }
   if (feature_result & (1u << 20)) {
      features |= SSE4_2;
   }
   if (feature_result & (1u << 28)) {
      features |= AVX;
   }

   return features;
}

#elif defined(__x86_64) || defined(Q_OS_WIN64)
static inline uint detectProcessorFeatures()
{
   uint features = MMX | SSE | SSE2 | CMOV;
   uint feature_result = 0;

#if defined(Q_CC_GNU)
   quint64 tmp;
   asm ("xchg %%rbx, %1\n"
        "cpuid\n"
        "xchg %%rbx, %1\n"
        : "=c" (feature_result), "=&r" (tmp)
        : "a" (1)
        : "%edx"
       );
#elif defined (Q_OS_WIN64)
   {
      int info[4];
      __cpuid(info, 1);
      feature_result = info[2];
   }
#endif

   if (feature_result & (1u)) {
      features |= SSE3;
   }
   if (feature_result & (1u << 9)) {
      features |= SSSE3;
   }
   if (feature_result & (1u << 19)) {
      features |= SSE4_1;
   }
   if (feature_result & (1u << 20)) {
      features |= SSE4_2;
   }
   if (feature_result & (1u << 28)) {
      features |= AVX;
   }

   return features;
}

#elif defined(__ia64__)
static inline uint detectProcessorFeatures()
{
   return MMX | SSE | SSE2;
}

#else
static inline uint detectProcessorFeatures()
{
   return 0;
}
#endif

/*
 * Use kdesdk/scripts/generate_string_table.pl to update the table below.
 * Here's the data (don't forget the ONE leading space):
 mmx
 mmxext
 mmx3dnow
 mmx3dnowext
 sse
 sse2
 cmov
 iwmmxt
 neon
 sse3
 ssse3
 sse4.1
 sse4.2
 avx
  */

// begin generated
static const char features_string[] =
   " mmx\0"
   " mmxext\0"
   " mmx3dnow\0"
   " mmx3dnowext\0"
   " sse\0"
   " sse2\0"
   " cmov\0"
   " iwmmxt\0"
   " neon\0"
   " sse3\0"
   " ssse3\0"
   " sse4.1\0"
   " sse4.2\0"
   " avx\0"
   "\0";

static const int features_indices[] = {
   0,    5,   13,   23,   36,   41,   47,   53,
   61,   67,   73,   80,   88,   96,   -1
};
// end generated

const int features_count = (sizeof features_indices - 1) / (sizeof features_indices[0]);

uint qDetectCPUFeatures()
{
   static std::atomic<uint> features(std::numeric_limits<uint>::max());
   uint f = features.load(std::memory_order_acquire);

   if (f != std::numeric_limits<uint>::max()) {
      return f;
   }

   f = detectProcessorFeatures();
   QByteArray disable = qgetenv("QT_NO_CPU_FEATURE");

   if (! disable.isEmpty()) {
      disable.prepend(' ');

      for (int i = 0; i < features_count; ++i) {
         if (disable.contains(features_string + features_indices[i])) {
            f &= ~(1 << i);
         }
      }
   }

   features.store(f, std::memory_order_release);

   return f;
}

void qDumpCPUFeatures()
{
   uint features = qDetectCPUFeatures();
   printf("Processor features: ");

   for (int i = 0; i < features_count; ++i) {
      if (features & (1 << i)) {
         printf("%s", features_string + features_indices[i]);
      }
   }
   puts("");
}

QT_END_NAMESPACE
