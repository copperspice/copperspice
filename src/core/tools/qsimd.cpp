/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#if defined(Q_OS_WIN)
#include <intrin.h>

#elif defined(Q_OS_LINUX) && (defined(Q_PROCESSOR_ARM) || defined(Q_PROCESSOR_MIPS_32))
#include <qcore_unix_p.h>

// the kernel header definitions for HWCAP_*
// (the ones we need/may need anyway)

// copied from <asm/hwcap.h> (ARM)

#define HWCAP_CRUNCH    1024
#define HWCAP_THUMBEE   2048
#define HWCAP_NEON      4096
#define HWCAP_VFPv3     8192
#define HWCAP_VFPv3D16  16384

// copied from <linux/auxvec.h>
#define AT_HWCAP  16    /* arch dependent hints at CPU capabilities */

#endif

#if defined (Q_OS_NACL)
static inline uint detectProcessorFeatures()
{
   return 0;
}

#elif defined(Q_PROCESSOR_ARM)

static inline quint64 detectProcessorFeatures()
{
   quint64 features = 0;

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

               if (vector[i + 1] & HWCAP_NEON) {
                  features |= Q_UINT64_C(1) << CpuFeatureNEON;
               }
               break;
            }
      }

      ::qt_safe_close(auxv);
      return features;
   }
   // fall back if /proc/self/auxv wasn't found
#endif

#if defined(__ARM_NEON__)
   features = Q_UINT64_C(1) << CpuFeatureNEON;
#endif

   return features;
}

#elif defined(Q_PROCESSOR_X86)

#ifdef Q_PROCESSOR_X86_32
# define PICreg "%%ebx"

using cs_word  = qint32;
using cs_uword = quint32;

#else
# define PICreg "%%rbx"

using cs_word  = qint64;
using cs_uword = quint64;

#endif

static int maxBasicCpuidSupported()
{

#if defined(Q_CC_GNU)
   cs_word tmp1;

# if Q_PROCESSOR_X86 < 5
   long cpuid_supported;
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

   if (! cpuid_supported) {
      return 0;
   }
# endif
   int result;
   asm ("xchg " PICreg", %1\n"
        "cpuid\n"
        "xchg " PICreg", %1\n"
        : "=&a" (result), "=&r" (tmp1)
        : "0" (0)
        : "ecx", "edx");
   return result;

#elif defined(Q_OS_WIN)
   // Use the __cpuid function; if the CPUID instruction isn't supported, it will return 0
   int info[4];
   __cpuid(info, 0);
   return info[0];

#else
   return 0;

#endif
}

static void cpuidFeatures01(uint &ecx, uint &edx)
{
#if defined(Q_CC_GNU)
   cs_word tmp1;

   asm ("xchg " PICreg", %2\n"
        "cpuid\n"
        "xchg " PICreg", %2\n"
        : "=&c" (ecx), "=&d" (edx), "=&r" (tmp1)
        : "a" (1));

#elif defined(Q_OS_WIN)
   int info[4];
   __cpuid(info, 1);
   ecx = info[2];
   edx = info[3];
#endif

}

#ifdef Q_OS_WIN
inline void __cpuidex(int info[4], int, __int64) { memset(info, 0, 4 * sizeof(int));}
#endif

static void cpuidFeatures07_00(uint &ebx, uint &ecx)
{
#if defined(Q_CC_GNU)

   cs_uword rbx;             // in case it's 64-bit
   cs_uword rcx = 0;

   asm ("xchg " PICreg", %0\n"
        "cpuid\n"
        "xchg " PICreg", %0\n"
        : "=&r" (rbx), "+&c" (rcx)
        : "a" (7)
        : "%edx");
   ebx = rbx;
   ecx = rcx;

#elif defined(Q_OS_WIN)
   int info[4];
   __cpuidex(info, 7, 0);
   ebx = info[1];
   ecx = info[2];

#endif
}

#ifdef Q_OS_WIN
// fallback overload in case this intrinsic does not exist: unsigned __int64 _xgetbv(unsigned int);
inline quint64 _xgetbv(__int64) { return 0; }
#endif

static void xgetbv(uint in, uint &eax, uint &edx)
{
#if defined(Q_CC_GNU)
   asm (".byte 0x0F, 0x01, 0xD0" // xgetbv instruction
        : "=a" (eax), "=d" (edx)
        : "c" (in));

#elif defined (Q_OS_WIN)
   quint64 result = _xgetbv(in);
   eax = result;
   edx = result >> 32;
#endif
}
static quint64 detectProcessorFeatures()
{
   // Flags from the CR0 / XCR0 state register
   enum XCR0Flags {
      X87             = 1 << 0,
      XMM0_15         = 1 << 1,
      YMM0_15Hi128    = 1 << 2,
      BNDRegs         = 1 << 3,
      BNDCSR          = 1 << 4,
      OpMask          = 1 << 5,
      ZMM0_15Hi256    = 1 << 6,
      ZMM16_31        = 1 << 7,

      SSEState        = XMM0_15,
      AVXState        = XMM0_15 | YMM0_15Hi128,
      AVX512State     = AVXState | OpMask | ZMM0_15Hi256 | ZMM16_31
   };

   static const quint64 AllAVX512 = (Q_UINT64_C(1) << CpuFeatureAVX512F)  | (Q_UINT64_C(1) << CpuFeatureAVX512CD) |
                                    (Q_UINT64_C(1) << CpuFeatureAVX512ER) | (Q_UINT64_C(1) << CpuFeatureAVX512PF) |
                                    (Q_UINT64_C(1) << CpuFeatureAVX512BW) | (Q_UINT64_C(1) << CpuFeatureAVX512DQ) |
                                    (Q_UINT64_C(1) << CpuFeatureAVX512VL) |
                                    (Q_UINT64_C(1) << CpuFeatureAVX512IFMA) | (Q_UINT64_C(1) << CpuFeatureAVX512VBMI);
   static const quint64 AllAVX2 = (Q_UINT64_C(1) << CpuFeatureAVX2) | AllAVX512;
   static const quint64 AllAVX = (Q_UINT64_C(1) << CpuFeatureAVX) | AllAVX2;

   quint64 features = 0;
   int cpuidLevel = maxBasicCpuidSupported();

#if Q_PROCESSOR_X86 < 5
   if (cpuidLevel < 1) {
      return 0;
   }
#else
   Q_ASSERT(cpuidLevel >= 1);
#endif

   uint cpuid01ECX = 0, cpuid01EDX = 0;
   cpuidFeatures01(cpuid01ECX, cpuid01EDX);

   // the low 32-bits of features is cpuid01ECX
   // note: we need to check OS support for saving the AVX register state
   features = cpuid01ECX;

#if defined(Q_PROCESSOR_X86_32)
   // x86 might not have SSE2 support
   if (cpuid01EDX & (1u << 26)) {
      features |= Q_UINT64_C(1) << CpuFeatureSSE2;
   } else {
      features &= ~(Q_UINT64_C(1) << CpuFeatureSSE2);
   }
   // we should verify that the OS enabled saving of the SSE state...
#else
   // x86-64 or x32
   features |= Q_UINT64_C(1) << CpuFeatureSSE2;
#endif

   uint xgetbvA = 0, xgetbvD = 0;
   if (cpuid01ECX & (1u << 27)) {
      // XGETBV enabled
      xgetbv(0, xgetbvA, xgetbvD);
   }

   uint cpuid0700EBX = 0;
   uint cpuid0700ECX = 0;
   if (cpuidLevel >= 7) {
      cpuidFeatures07_00(cpuid0700EBX, cpuid0700ECX);

      // the high 32-bits of features is cpuid0700EBX
      features |= quint64(cpuid0700EBX) << 32;
   }

   if ((xgetbvA & AVXState) != AVXState) {
      // support for YMM registers is disabled, disable all AVX
      features &= ~AllAVX;
   } else if ((xgetbvA & AVX512State) != AVX512State) {
      // support for ZMM registers or mask registers is disabled, disable all AVX512
      features &= ~AllAVX512;
   } else {
      // this feature is out of order
      if (cpuid0700ECX & (1u << 1)) {
         features |= Q_UINT64_C(1) << CpuFeatureAVX512VBMI;
      } else {
         features &= ~(Q_UINT64_C(1) << CpuFeatureAVX512VBMI);
      }
   }

   return features;
}

#elif defined(Q_PROCESSOR_MIPS_32)

#if defined(Q_OS_LINUX)
//
// Do not use QByteArray: it could use SIMD instructions itself at
// some point, thus creating a recursive dependency. Instead, use a
// QSimpleBuffer, which has the bare minimum needed to use memory
// dynamically and read lines from /proc/cpuinfo of arbitrary sizes.
//
struct QSimpleBuffer {
   static const int chunk_size = 256;
   char *data;
   unsigned alloc;
   unsigned size;

   QSimpleBuffer(): data(0), alloc(0), size(0) {}
   ~QSimpleBuffer() { ::free(data); }

   void resize(unsigned newsize) {
      if (newsize > alloc) {
         unsigned newalloc = chunk_size * ((newsize / chunk_size) + 1);
         if (newalloc < newsize) { newalloc = newsize; }
         if (newalloc != alloc) {
            data = static_cast<char *>(::realloc(data, newalloc));
            alloc = newalloc;
         }
      }
      size = newsize;
   }
   void append(const QSimpleBuffer &other, unsigned appendsize) {
      unsigned oldsize = size;
      resize(oldsize + appendsize);
      ::memcpy(data + oldsize, other.data, appendsize);
   }
   void popleft(unsigned amount) {
      if (amount >= size) { return resize(0); }
      size -= amount;
      ::memmove(data, data + amount, size);
   }
   char *cString() {
      if (!alloc) { resize(1); }
      return (data[size] = '\0', data);
   }
};

// Uses a scratch "buffer" (which must be used for all reads done in the
// same file descriptor) to read chunks of data from a file, to read
// one line at a time. Lines include the trailing newline character ('\n').
// On EOF, line.size is zero.
//
static void bufReadLine(int fd, QSimpleBuffer &line, QSimpleBuffer &buffer)
{
   for (;;) {
      char *newline = static_cast<char *>(::memchr(buffer.data, '\n', buffer.size));

      if (newline) {
         unsigned piece_size = newline - buffer.data + 1;
         line.append(buffer, piece_size);
         buffer.popleft(piece_size);
         line.resize(line.size - 1);
         return;
      }

      if (buffer.size + QSimpleBuffer::chunk_size > buffer.alloc) {
         int oldsize = buffer.size;
         buffer.resize(buffer.size + QSimpleBuffer::chunk_size);
         buffer.size = oldsize;
      }

      ssize_t read_bytes = ::qt_safe_read(fd, buffer.data + buffer.size, QSimpleBuffer::chunk_size);

      if (read_bytes > 0) {
         buffer.size += read_bytes;

      } else {
         return;
      }
   }
}

// Checks if any line with a given prefix from /proc/cpuinfo contains
// a certain string, surrounded by spaces.

static bool procCpuinfoContains(const char *prefix, const char *string)
{
   int cpuinfo_fd = ::qt_safe_open("/proc/cpuinfo", O_RDONLY);

   if (cpuinfo_fd == -1) {
      return false;
   }

   unsigned string_len = ::strlen(string);
   unsigned prefix_len = ::strlen(prefix);
   QSimpleBuffer line, buffer;
   bool present = false;

   do {
      line.resize(0);
      bufReadLine(cpuinfo_fd, line, buffer);
      char *colon = static_cast<char *>(::memchr(line.data, ':', line.size));

      if (colon && line.size > prefix_len + string_len) {
         if (!::strncmp(prefix, line.data, prefix_len)) {
            // prefix matches, next character must be ':' or space
            if (line.data[prefix_len] == ':' || ::isspace(line.data[prefix_len])) {
               // Does it contain the string?
               char *found = ::strstr(line.cString(), string);
               if (found && ::isspace(found[-1]) &&
                     (::isspace(found[string_len]) || found[string_len] == '\0')) {
                  present = true;
                  break;
               }
            }
         }
      }

   } while (line.size);

   ::qt_safe_close(cpuinfo_fd);
   return present;
}
#endif
static inline quint64 detectProcessorFeatures()
{
   // NOTE: MIPS 74K cores are the only ones supporting DSPr2.
   quint64 flags = 0;

#if defined __mips_dsp
   flags |= Q_UINT64_C(1) << CpuFeatureDSP;
#  if defined __mips_dsp_rev && __mips_dsp_rev >= 2
   flags |= Q_UINT64_C(1) << CpuFeatureDSPR2;
#  elif defined(Q_OS_LINUX)
   if (procCpuinfoContains("cpu model", "MIPS 74Kc") || procCpuinfoContains("cpu model", "MIPS 74Kf")) {
      flags |= Q_UINT64_C(1) << CpuFeatureDSPR2;
   }
#  endif
#elif defined(Q_OS_LINUX)
   if (procCpuinfoContains("ASEs implemented", "dsp")) {
      flags |= Q_UINT64_C(1) << CpuFeatureDSP;
      if (procCpuinfoContains("cpu model", "MIPS 74Kc") || procCpuinfoContains("cpu model", "MIPS 74Kf")) {
         flags |= Q_UINT64_C(1) << CpuFeatureDSPR2;
      }
   }
#endif

   return flags;
}

#else
static inline uint detectProcessorFeatures()
{
   return 0;
}
#endif

/*
 * Use kdesdk/scripts/generate_string_table.pl to update the table below. Note
 * that the x86 version has a lot of blanks that must be kept and that the
 * offset table's type is changed to make the table smaller. We also remove the
 * terminating -1 that the script adds.
 */

// begin generated
#if defined(Q_PROCESSOR_ARM)
/* Data:
 neon
 */
static const char features_string[] = " neon\0";
static const int features_indices[] = { 0 };
#elif defined(Q_PROCESSOR_MIPS)
/* Data:
 dsp
 dspr2
*/
static const char features_string[] =
   " dsp\0"
   " dspr2\0"
   "\0";

static const int features_indices[] = {
   0,    5
};

#elif defined(Q_PROCESSOR_X86)

static const char features_string[] =

   " sse3\0"
   " sse2\0"
   " avx512vbmi\0"
   " ssse3\0"
   " fma\0"
   " cmpxchg16b\0"
   " sse4.1\0"
   " sse4.2\0"
   " movbe\0"
   " popcnt\0"
   " aes\0"
   " avx\0"
   " f16c\0"
   " rdrand\0"
   " bmi\0"
   " hle\0"
   " avx2\0"
   " bmi2\0"
   " rtm\0"
   " avx512f\0"
   " avx512dq\0"
   " rdseed\0"
   " avx512ifma\0"
   " avx512pf\0"
   " avx512er\0"
   " avx512cd\0"
   " sha\0"
   " avx512bw\0"
   " avx512vl\0"
   "\0";
static const quint8 features_indices[] = {
   0,    6,   12,    5,    5,    5,    5,    5,
   5,   24,    5,    5,   31,   36,    5,    5,
   5,    5,    5,   48,   56,    5,   64,   71,
   5,   79,    5,    5,   84,   89,   95,    5,
   5,    5,    5,  103,  108,  113,    5,    5,
   119,    5,    5,  125,    5,    5,    5,    5,
   130,  139,  149,    5,    5,  157,    5,    5,
   5,    5,  169,  179,  189,  199,  204,  214
};
#else
static const char features_string[] = "";
static const int features_indices[] = { };
#endif

// end generated

static const int features_count = (sizeof features_indices) / (sizeof features_indices[0]);

// record what CPU features were enabled by default in this Qt build
static const quint64 minFeature = qCompilerCpuFeatures;

#ifdef Q_OS_WIN

#if defined(Q_CC_GNU)
#  define ffsll __builtin_ffsll
#else

int ffsll(quint64 i)
{
#if defined(Q_OS_WIN64)
   unsigned long result;
   return _BitScanForward64(&result, i) ? result : 0;

#else
   unsigned long result;
   return _BitScanForward(&result, i) ? result :
          _BitScanForward(&result, i >> 32) ? result + 32 : 0;

#endif
}
#endif

#elif defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD) || defined(Q_OS_ANDROID) || defined(Q_OS_DARWIN)
# define ffsll __builtin_ffsll

#endif

Q_CORE_EXPORT std::atomic<quint64> cs_cpu_features{ 0 };

void qDetectCpuFeatures()
{


   quint64 f = detectProcessorFeatures();
   QByteArray disable = qgetenv("QT_NO_CPU_FEATURE");

   if (! disable.isEmpty()) {
      disable.prepend(' ');

      for (int i = 0; i < features_count; ++i) {
         if (disable.contains(features_string + features_indices[i])) {
            f &= ~(static_cast<quint64>(1) << i);
         }
      }
   }

   cs_cpu_features.store(f | quint32(QSimdInitialized));

}

void qDumpCPUFeatures()
{
   quint64 features = qCpuFeatures() & ~quint64(QSimdInitialized);
   printf("Processor features: ");

   for (int i = 0; i < features_count; ++i) {
      if (features & (static_cast<quint64>(1) << i)) {
         printf("%s%s", features_string + features_indices[i],
                minFeature & (Q_UINT64_C(1) << i) ? "[required]" : "");
      }
   }
   puts("");
}

