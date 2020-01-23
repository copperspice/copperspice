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

#include "qmachparser_p.h"

#if defined(Q_OF_MACH_O)

#include <qendian.h>
#include <qlibrary_p.h>

#include <mach-o/loader.h>
#include <mach-o/fat.h>

#if defined(Q_PROCESSOR_X86_64)
#  define MACHO64
static const cpu_type_t my_cputype = CPU_TYPE_X86_64;
#elif defined(Q_PROCESSOR_X86_32)
static const cpu_type_t my_cputype = CPU_TYPE_X86;
#elif defined(Q_PROCESSOR_POWER_64)
#  define MACHO64
static const cpu_type_t my_cputype = CPU_TYPE_POWERPC64;
#elif defined(Q_PROCESSOR_POWER_32)
static const cpu_type_t my_cputype = CPU_TYPE_POWERPC;
#elif defined(Q_PROCESSOR_ARM)
static const cpu_type_t my_cputype = CPU_TYPE_ARM;
#else
#  error "Unknown CPU type"
#endif

#ifdef MACHO64
#  undef MACHO64
typedef mach_header_64 my_mach_header;
typedef segment_command_64 my_segment_command;
typedef section_64 my_section;
static const uint32_t my_magic = MH_MAGIC_64;
#else
typedef mach_header my_mach_header;
typedef segment_command my_segment_command;
typedef section my_section;
static const uint32_t my_magic = MH_MAGIC;
#endif

static int ns(const QString &reason, const QString &library, QString *errorString)
{
   if (errorString)
      *errorString = QLibrary::tr("'%1' is not a valid Mach-O binary (%2)")
         .formatArg(library, reason.isEmpty() ? QLibrary::tr("file is corrupt") : reason);

   return QMachOParser::NotSuitable;
}

int QMachOParser::parse(const char *m_s, ulong fdlen, const QString &library, QString *errorString, long *pos, ulong *sectionlen)
{
   // The minimum size of a Mach-O binary we're interested in.
   // It must have a full Mach header, at least one segment and at least one
   // section. It's probably useless with just the "qtmetadata" section, but
   // it's valid nonetheless.
   // A fat binary must have this plus the fat header, of course.
   static const size_t MinFileSize = sizeof(my_mach_header) + sizeof(my_segment_command) + sizeof(my_section);
   static const size_t MinFatHeaderSize = sizeof(fat_header) + 2 * sizeof(fat_arch);

   if (fdlen < MinFileSize) {
      return ns(QLibrary::tr("file too small"), library, errorString);
   }

   // find out if this is a fat Mach-O binary first
   const my_mach_header *header = 0;
   const fat_header *fat = reinterpret_cast<const fat_header *>(m_s);

   if (fat->magic == qToBigEndian(FAT_MAGIC)) {
      // find our architecture in the binary
      const fat_arch *arch = reinterpret_cast<const fat_arch *>(fat + 1);
      if (Q_UNLIKELY(fdlen < MinFatHeaderSize)) {
         return ns(QLibrary::tr("file too small"), library, errorString);
      }

      int count = qFromBigEndian(fat->nfat_arch);
      if (fdlen < sizeof(*fat) + sizeof(*arch) * count) {
         return ns(QString(), library, errorString);
      }

      for (int i = 0; i < count; ++i) {
         if (arch[i].cputype == qToBigEndian(my_cputype)) {
            // ### should we check the CPU subtype? Maybe on ARM?
            uint32_t size = qFromBigEndian(arch[i].size);
            uint32_t offset = qFromBigEndian(arch[i].offset);
            if (Q_UNLIKELY(size > fdlen) || Q_UNLIKELY(offset > fdlen)
               || Q_UNLIKELY(size + offset > fdlen) || Q_UNLIKELY(size < MinFileSize)) {
               return ns(QString(), library, errorString);
            }

            header = reinterpret_cast<const my_mach_header *>(m_s + offset);
            fdlen = size;
            break;
         }
      }
      if (!header) {
         return ns(QLibrary::tr("no suitable architecture in fat binary"), library, errorString);
      }

      // check the magic again
      if (Q_UNLIKELY(header->magic != my_magic)) {
         return ns(QString(), library, errorString);
      }
   } else {
      header = reinterpret_cast<const my_mach_header *>(m_s);
      fat = 0;

      // check magic
      if (header->magic != my_magic)
         return ns(QLibrary::tr("invalid magic %1").formatArg(qFromBigEndian(header->magic), 8, 16, QLatin1Char('0')),
               library, errorString);
   }

   // from this point on, fdlen is specific to this architecture
   // from this point on, everything is in host byte order
   *pos = reinterpret_cast<const char *>(header) - m_s;

   // (re-)check the CPU type
   // ### should we check the CPU subtype? Maybe on ARM?
   if (header->cputype != my_cputype) {
      if (fat) {
         return ns(QString(), library, errorString);
      }
      return ns(QLibrary::tr("wrong architecture"), library, errorString);
   }

   // check the file type
   if (header->filetype != MH_BUNDLE && header->filetype != MH_DYLIB) {
      return ns(QLibrary::tr("not a dynamic library"), library, errorString);
   }

   // find the __TEXT segment, "qtmetadata" section
   const my_segment_command *seg = reinterpret_cast<const my_segment_command *>(header + 1);
   ulong minsize = sizeof(*header);

   for (uint i = 0; i < header->ncmds; ++i,
      seg = reinterpret_cast<const my_segment_command *>(reinterpret_cast<const char *>(seg) + seg->cmdsize)) {
      // We're sure that the file size includes at least one load command
      // but we have to check anyway if we're past the first
      if (fdlen < minsize + sizeof(load_command)) {
         return ns(QString(), library, errorString);
      }

      // cmdsize can't be trusted until validated
      // so check it against fdlen anyway
      // (these are unsigned operations, with overflow behavior specified in the standard)
      minsize += seg->cmdsize;
      if (fdlen < minsize || fdlen < seg->cmdsize) {
         return ns(QString(), library, errorString);
      }

      const uint32_t MyLoadCommand = sizeof(void *) > 4 ? LC_SEGMENT_64 : LC_SEGMENT;
      if (seg->cmd != MyLoadCommand) {
         continue;
      }

      // is this the __TEXT segment?
      if (strcmp(seg->segname, "__TEXT") == 0) {
         const my_section *sect = reinterpret_cast<const my_section *>(seg + 1);
         for (uint j = 0; j < seg->nsects; ++j) {
            // is this the "qtmetadata" section?
            if (strcmp(sect[j].sectname, "qtmetadata") != 0) {
               continue;
            }

            // found it!
            if (Q_UNLIKELY(fdlen < sect[j].offset) || Q_UNLIKELY(fdlen < sect[j].size)
               || Q_UNLIKELY(fdlen < sect[j].offset + sect[j].size)) {
               return ns(QString(), library, errorString);
            }

            *pos += sect[j].offset;
            *sectionlen = sect[j].size;
            return QtMetaDataSection;
         }
      }

      // other type of segment
      seg = reinterpret_cast<const my_segment_command *>(reinterpret_cast<const char *>(seg) + seg->cmdsize);
   }

   //    // No Qt section was found, but at least we know that where the proper architecture's boundaries are
   //    return NoQtSection;
   if (errorString) {
      *errorString = QLibrary::tr("'%1' is not a Qt plugin").formatArg(library);
   }

   return NotSuitable;
}

#endif
