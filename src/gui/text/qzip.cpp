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

#include <qglobal.h>

#ifndef QT_NO_TEXTODFWRITER

#include <qzipreader_p.h>
#include <qzipwriter_p.h>
#include <qdatetime.h>

#include <qendian.h>
#include <qdebug.h>
#include <qdir.h>

#include <zlib.h>

// Zip standard version for archives handled by this API
// (actually, the only basic support of this version is implemented but it is enough for now)
#define ZIP_VERSION 20

#if 0
#define ZDEBUG qDebug
#else
#define ZDEBUG if (0) qDebug
#endif

static inline uint readUInt(const uchar *data)
{
   return (data[0]) + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
}

static inline ushort readUShort(const uchar *data)
{
   return (data[0]) + (data[1] << 8);
}

static inline void writeUInt(uchar *data, uint i)
{
   data[0] = i & 0xff;
   data[1] = (i >> 8) & 0xff;
   data[2] = (i >> 16) & 0xff;
   data[3] = (i >> 24) & 0xff;
}

static inline void writeUShort(uchar *data, ushort i)
{
   data[0] = i & 0xff;
   data[1] = (i >> 8) & 0xff;
}

static inline void copyUInt(uchar *dest, const uchar *src)
{
   dest[0] = src[0];
   dest[1] = src[1];
   dest[2] = src[2];
   dest[3] = src[3];
}

static inline void copyUShort(uchar *dest, const uchar *src)
{
   dest[0] = src[0];
   dest[1] = src[1];
}

static void writeMSDosDate(uchar *dest, const QDateTime &dt)
{
   if (dt.isValid()) {
      quint16 time =
         (dt.time().hour() << 11)    // 5 bit hour
         | (dt.time().minute() << 5)   // 6 bit minute
         | (dt.time().second() >> 1);  // 5 bit double seconds

      dest[0] = time & 0xff;
      dest[1] = time >> 8;

      quint16 date =
         ((dt.date().year() - 1980) << 9) // 7 bit year 1980-based
         | (dt.date().month() << 5)           // 4 bit month
         | (dt.date().day());                 // 5 bit day

      dest[2] = char(date);
      dest[3] = char(date >> 8);
   } else {
      dest[0] = 0;
      dest[1] = 0;
      dest[2] = 0;
      dest[3] = 0;
   }
}

static int inflate(Bytef *dest, ulong *destLen, const Bytef *source, ulong sourceLen)
{
   z_stream stream;
   int err;

   stream.next_in = const_cast<Bytef *>(source);
   stream.avail_in = (uInt)sourceLen;
   if ((uLong)stream.avail_in != sourceLen) {
      return Z_BUF_ERROR;
   }

   stream.next_out = dest;
   stream.avail_out = (uInt) * destLen;
   if ((uLong)stream.avail_out != *destLen) {
      return Z_BUF_ERROR;
   }

   stream.zalloc = (alloc_func)0;
   stream.zfree = (free_func)0;

   err = inflateInit2(&stream, -MAX_WBITS);
   if (err != Z_OK) {
      return err;
   }

   err = inflate(&stream, Z_FINISH);
   if (err != Z_STREAM_END) {
      inflateEnd(&stream);
      if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0)) {
         return Z_DATA_ERROR;
      }
      return err;
   }
   *destLen = stream.total_out;

   err = inflateEnd(&stream);
   return err;
}

static int deflate (Bytef *dest, ulong *destLen, const Bytef *source, ulong sourceLen)
{
   z_stream stream;
   int err;

   stream.next_in = const_cast<Bytef *>(source);
   stream.avail_in = (uInt)sourceLen;
   stream.next_out = dest;
   stream.avail_out = (uInt) * destLen;
   if ((uLong)stream.avail_out != *destLen) {
      return Z_BUF_ERROR;
   }

   stream.zalloc = (alloc_func)0;
   stream.zfree = (free_func)0;
   stream.opaque = (voidpf)0;

   err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
   if (err != Z_OK) {
      return err;
   }

   err = deflate(&stream, Z_FINISH);
   if (err != Z_STREAM_END) {
      deflateEnd(&stream);
      return err == Z_OK ? Z_BUF_ERROR : err;
   }
   *destLen = stream.total_out;

   err = deflateEnd(&stream);
   return err;
}

namespace WindowsFileAttributes {
enum {
   Dir        = 0x10, // FILE_ATTRIBUTE_DIRECTORY
   File       = 0x80, // FILE_ATTRIBUTE_NORMAL
   TypeMask   = 0x90,

   ReadOnly   = 0x01, // FILE_ATTRIBUTE_READONLY
   PermMask   = 0x01
};
}

namespace UnixFileAttributes {
enum {
   Dir        = 0040000, // __S_IFDIR
   File       = 0100000, // __S_IFREG
   SymLink    = 0120000, // __S_IFLNK
   TypeMask   = 0170000, // __S_IFMT

   ReadUser   = 0400, // __S_IRUSR
   WriteUser  = 0200, // __S_IWUSR
   ExeUser    = 0100, // __S_IXUSR
   ReadGroup  = 0040, // __S_IRGRP
   WriteGroup = 0020, // __S_IWGRP
   ExeGroup   = 0010, // __S_IXGRP
   ReadOther  = 0004, // __S_IROTH
   WriteOther = 0002, // __S_IWOTH
   ExeOther   = 0001, // __S_IXOTH
   PermMask   = 0777
};
}
static QFile::Permissions modeToPermissions(quint32 mode)
{
   QFile::Permissions ret;

   if (mode & UnixFileAttributes::ReadUser) {
      ret |= QFile::ReadOwner | QFile::ReadUser;
   }
   if (mode & UnixFileAttributes::WriteUser) {
      ret |= QFile::WriteOwner | QFile::WriteUser;
   }
   if (mode & UnixFileAttributes::ExeUser) {
      ret |= QFile::ExeOwner | QFile::ExeUser;
   }
   if (mode & UnixFileAttributes::ReadGroup) {
      ret |= QFile::ReadGroup;
   }
   if (mode & UnixFileAttributes::WriteGroup) {
      ret |= QFile::WriteGroup;
   }
   if (mode & UnixFileAttributes::ExeGroup) {
      ret |= QFile::ExeGroup;
   }
   if (mode & UnixFileAttributes::ReadOther) {
      ret |= QFile::ReadOther;
   }
   if (mode & UnixFileAttributes::WriteOther) {
      ret |= QFile::WriteOther;
   }
   if (mode & UnixFileAttributes::ExeOther) {
      ret |= QFile::ExeOther;
   }
   return ret;
}
static quint32 permissionsToMode(QFile::Permissions perms)
{
   quint32 mode = 0;
   if (perms & (QFile::ReadOwner | QFile::ReadUser)) {
      mode |= UnixFileAttributes::ReadUser;
   }
   if (perms & (QFile::WriteOwner | QFile::WriteUser)) {
      mode |= UnixFileAttributes::WriteUser;
   }
   if (perms & (QFile::ExeOwner | QFile::ExeUser)) {
      mode |= UnixFileAttributes::WriteUser;
   }
   if (perms & QFile::ReadGroup) {
      mode |= UnixFileAttributes::ReadGroup;
   }
   if (perms & QFile::WriteGroup) {
      mode |= UnixFileAttributes::WriteGroup;
   }
   if (perms & QFile::ExeGroup) {
      mode |= UnixFileAttributes::ExeGroup;
   }
   if (perms & QFile::ReadOther) {
      mode |= UnixFileAttributes::ReadOther;
   }
   if (perms & QFile::WriteOther) {
      mode |= UnixFileAttributes::WriteOther;
   }
   if (perms & QFile::ExeOther) {
      mode |= UnixFileAttributes::ExeOther;
   }
   return mode;
}

static QDateTime readMSDosDate(const uchar *src)
{
   uint dosDate = readUInt(src);
   quint64 uDate;
   uDate = (quint64)(dosDate >> 16);
   uint tm_mday = (uDate & 0x1f);
   uint tm_mon =  ((uDate & 0x1E0) >> 5);
   uint tm_year = (((uDate & 0x0FE00) >> 9) + 1980);
   uint tm_hour = ((dosDate & 0xF800) >> 11);
   uint tm_min =  ((dosDate & 0x7E0) >> 5);
   uint tm_sec =  ((dosDate & 0x1f) << 1);

   return QDateTime(QDate(tm_year, tm_mon, tm_mday), QTime(tm_hour, tm_min, tm_sec));
}

// for details, see http://www.pkware.com/documents/casestudies/APPNOTE.TXT

enum HostOS {
   HostFAT      = 0,
   HostAMIGA    = 1,
   HostVMS      = 2,  // VAX/VMS
   HostUnix     = 3,
   HostVM_CMS   = 4,
   HostAtari    = 5,  // what if it's a minix filesystem? [cjh]
   HostHPFS     = 6,  // filesystem used by OS/2 (and NT 3.x)
   HostMac      = 7,
   HostZ_System = 8,
   HostCPM      = 9,
   HostTOPS20   = 10, // pkzip 2.50 NTFS
   HostNTFS     = 11, // filesystem used by Windows NT
   HostQDOS     = 12, // SMS/QDOS
   HostAcorn    = 13, // Archimedes Acorn RISC OS
   HostVFAT     = 14, // filesystem used by Windows 95, NT
   HostMVS      = 15,
   HostBeOS     = 16, // hybrid POSIX/database filesystem
   HostTandem   = 17,
   HostOS400    = 18,
   HostOSX      = 19
};

enum GeneralPurposeFlag {
   Encrypted = 0x01,
   AlgTune1 = 0x02,
   AlgTune2 = 0x04,
   HasDataDescriptor = 0x08,
   PatchedData = 0x20,
   StrongEncrypted = 0x40,
   Utf8Names = 0x0800,
   CentralDirectoryEncrypted = 0x2000
};

enum CompressionMethod {
   CompressionMethodStored = 0,
   CompressionMethodShrunk = 1,
   CompressionMethodReduced1 = 2,
   CompressionMethodReduced2 = 3,
   CompressionMethodReduced3 = 4,
   CompressionMethodReduced4 = 5,
   CompressionMethodImploded = 6,
   CompressionMethodReservedTokenizing = 7, // reserved for tokenizing
   CompressionMethodDeflated = 8,
   CompressionMethodDeflated64 = 9,
   CompressionMethodPKImploding = 10,

   CompressionMethodBZip2 = 12,

   CompressionMethodLZMA = 14,

   CompressionMethodTerse = 18,
   CompressionMethodLz77 = 19,

   CompressionMethodJpeg = 96,
   CompressionMethodWavPack = 97,
   CompressionMethodPPMd = 98,
   CompressionMethodWzAES = 99
};
struct LocalFileHeader {
   uchar signature[4]; //  0x04034b50
   uchar version_needed[2];
   uchar general_purpose_bits[2];
   uchar compression_method[2];
   uchar last_mod_file[4];
   uchar crc_32[4];
   uchar compressed_size[4];
   uchar uncompressed_size[4];
   uchar file_name_length[2];
   uchar extra_field_length[2];
};

struct DataDescriptor {
   uchar crc_32[4];
   uchar compressed_size[4];
   uchar uncompressed_size[4];
};

struct CentralFileHeader {
   uchar signature[4]; // 0x02014b50
   uchar version_made[2];
   uchar version_needed[2];
   uchar general_purpose_bits[2];
   uchar compression_method[2];
   uchar last_mod_file[4];
   uchar crc_32[4];
   uchar compressed_size[4];
   uchar uncompressed_size[4];
   uchar file_name_length[2];
   uchar extra_field_length[2];
   uchar file_comment_length[2];
   uchar disk_start[2];
   uchar internal_file_attributes[2];
   uchar external_file_attributes[4];
   uchar offset_local_header[4];
   LocalFileHeader toLocalHeader() const;
};

struct EndOfDirectory {
   uchar signature[4]; // 0x06054b50
   uchar this_disk[2];
   uchar start_of_directory_disk[2];
   uchar num_dir_entries_this_disk[2];
   uchar num_dir_entries[2];
   uchar directory_size[4];
   uchar dir_start_offset[4];
   uchar comment_length[2];
};

struct FileHeader {
   CentralFileHeader h;
   QByteArray file_name;
   QByteArray extra_field;
   QByteArray file_comment;
};

class QZipPrivate
{
 public:
   QZipPrivate(QIODevice *device, bool ownDev)
      : device(device), ownDevice(ownDev), dirtyFileTree(true), start_of_directory(0) {
   }

   ~QZipPrivate() {
      if (ownDevice) {
         delete device;
      }
   }

   QZipReader::FileInfo fillFileInfo(int index) const;

   QIODevice *device;
   bool ownDevice;
   bool dirtyFileTree;
   QVector<FileHeader> fileHeaders;
   QByteArray comment;
   uint start_of_directory;
};

QZipReader::FileInfo QZipPrivate::fillFileInfo(int index) const
{
   QZipReader::FileInfo fileInfo;
   FileHeader header = fileHeaders.at(index);
   quint32 mode = readUInt(header.h.external_file_attributes);
   const HostOS hostOS = HostOS(readUShort(header.h.version_made) >> 8);
   switch (hostOS) {
      case HostUnix:
         mode = (mode >> 16) & 0xffff;
         switch (mode & UnixFileAttributes::TypeMask) {
            case UnixFileAttributes::SymLink:
               fileInfo.isSymLink = true;
               break;
            case UnixFileAttributes::Dir:
               fileInfo.isDir = true;
               break;
            case UnixFileAttributes::File:
            default: // ### just for the case; should we warn?
               fileInfo.isFile = true;
               break;
         }
         fileInfo.permissions = modeToPermissions(mode);
         break;
      case HostFAT:
      case HostNTFS:
      case HostHPFS:
      case HostVFAT:
         switch (mode & WindowsFileAttributes::TypeMask) {
            case WindowsFileAttributes::Dir:
               fileInfo.isDir = true;
               break;
            case WindowsFileAttributes::File:
            default:
               fileInfo.isFile = true;
               break;
         }
         fileInfo.permissions |= QFile::ReadOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther;
         if ((mode & WindowsFileAttributes::ReadOnly) == 0) {
            fileInfo.permissions |= QFile::WriteOwner | QFile::WriteUser | QFile::WriteGroup | QFile::WriteOther;
         }
         if (fileInfo.isDir) {
            fileInfo.permissions |= QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther;
         }
         break;

      default:
         qWarning("QZip: Zip entry format at %d is not supported.", index);
         return fileInfo; // we don't support anything else
   }

   ushort general_purpose_bits = readUShort(header.h.general_purpose_bits);

   // if bit 11 is set, the filename and comment fields must be encoded using UTF-8
   const bool inUtf8     = (general_purpose_bits & Utf8Names) != 0;
   fileInfo.filePath     = inUtf8 ? QString::fromUtf8(header.file_name) : QString::fromLatin1(header.file_name);
   fileInfo.crc          = readUInt(header.h.crc_32);
   fileInfo.size         = readUInt(header.h.uncompressed_size);
   fileInfo.lastModified = readMSDosDate(header.h.last_mod_file);

   // fix the file path, if broken (convert separators, eat leading and trailing ones)
   fileInfo.filePath = QDir::fromNativeSeparators(fileInfo.filePath);

   while (!fileInfo.filePath.isEmpty() && (fileInfo.filePath.at(0) == QLatin1Char('.') || fileInfo.filePath.at(0) == QLatin1Char('/'))) {
      fileInfo.filePath = fileInfo.filePath.mid(1);
   }
   while (!fileInfo.filePath.isEmpty() && fileInfo.filePath.at(fileInfo.filePath.size() - 1) == QLatin1Char('/')) {
      fileInfo.filePath.chop(1);
   }

   return fileInfo;
}

class QZipReaderPrivate : public QZipPrivate
{
 public:
   QZipReaderPrivate(QIODevice *device, bool ownDev)
      : QZipPrivate(device, ownDev), status(QZipReader::NoError)
   { }

   void scanFiles();

   QZipReader::Status status;
};

class QZipWriterPrivate : public QZipPrivate
{
 public:
   QZipWriterPrivate(QIODevice *device, bool ownDev)
      : QZipPrivate(device, ownDev),
        status(QZipWriter::NoError),
        permissions(QFile::ReadOwner | QFile::WriteOwner),
        compressionPolicy(QZipWriter::AlwaysCompress) {
   }

   QZipWriter::Status status;
   QFile::Permissions permissions;
   QZipWriter::CompressionPolicy compressionPolicy;

   enum EntryType { Directory, File, Symlink };

   void addEntry(EntryType type, const QString &fileName, const QByteArray &contents);
};

LocalFileHeader CentralFileHeader::toLocalHeader() const
{
   LocalFileHeader h;
   writeUInt(h.signature, 0x04034b50);
   copyUShort(h.version_needed, version_needed);
   copyUShort(h.general_purpose_bits, general_purpose_bits);
   copyUShort(h.compression_method, compression_method);
   copyUInt(h.last_mod_file, last_mod_file);
   copyUInt(h.crc_32, crc_32);
   copyUInt(h.compressed_size, compressed_size);
   copyUInt(h.uncompressed_size, uncompressed_size);
   copyUShort(h.file_name_length, file_name_length);
   copyUShort(h.extra_field_length, extra_field_length);
   return h;
}

void QZipReaderPrivate::scanFiles()
{
   if (!dirtyFileTree) {
      return;
   }

   if (! (device->isOpen() || device->open(QIODevice::ReadOnly))) {
      status = QZipReader::FileOpenError;
      return;
   }

   if ((device->openMode() & QIODevice::ReadOnly) == 0) { // only read the index from readable files.
      status = QZipReader::FileReadError;
      return;
   }

   dirtyFileTree = false;
   uchar tmp[4];
   device->read((char *)tmp, 4);
   if (readUInt(tmp) != 0x04034b50) {
      qWarning() << "QZip: not a zip file!";
      return;
   }

   // find EndOfDirectory header
   int i = 0;
   int start_of_directory = -1;
   int num_dir_entries = 0;
   EndOfDirectory eod;
   while (start_of_directory == -1) {
      const int pos = device->size() - int(sizeof(EndOfDirectory)) - i;
      if (pos < 0 || i > 65535) {
         qWarning() << "QZip: EndOfDirectory not found";
         return;
      }

      device->seek(pos);
      device->read((char *)&eod, sizeof(EndOfDirectory));
      if (readUInt(eod.signature) == 0x06054b50) {
         break;
      }
      ++i;
   }

   // have the eod
   start_of_directory = readUInt(eod.dir_start_offset);
   num_dir_entries = readUShort(eod.num_dir_entries);
   ZDEBUG("start_of_directory at %d, num_dir_entries=%d", start_of_directory, num_dir_entries);
   int comment_length = readUShort(eod.comment_length);
   if (comment_length != i) {
      qWarning() << "QZip: failed to parse zip file.";
   }
   comment = device->read(qMin(comment_length, i));


   device->seek(start_of_directory);
   for (i = 0; i < num_dir_entries; ++i) {
      FileHeader header;
      int read = device->read((char *) &header.h, sizeof(CentralFileHeader));
      if (read < (int)sizeof(CentralFileHeader)) {
         qWarning() << "QZip: Failed to read complete header, index may be incomplete";
         break;
      }
      if (readUInt(header.h.signature) != 0x02014b50) {
         qWarning() << "QZip: invalid header signature, index may be incomplete";
         break;
      }

      int l = readUShort(header.h.file_name_length);
      header.file_name = device->read(l);
      if (header.file_name.length() != l) {
         qWarning() << "QZip: Failed to read filename from zip index, index may be incomplete";
         break;
      }
      l = readUShort(header.h.extra_field_length);
      header.extra_field = device->read(l);
      if (header.extra_field.length() != l) {
         qWarning() << "QZip: Failed to read extra field in zip file, skipping file, index may be incomplete";
         break;
      }
      l = readUShort(header.h.file_comment_length);
      header.file_comment = device->read(l);
      if (header.file_comment.length() != l) {
         qWarning() << "QZip: Failed to read read file comment, index may be incomplete";
         break;
      }

      ZDEBUG("found file '%s'", header.file_name.data());
      fileHeaders.append(header);
   }
}

void QZipWriterPrivate::addEntry(EntryType type, const QString &fileName, const QByteArray &contents)
{
#ifndef NDEBUG
   static const char *const entryTypes[] = {
      "directory",
      "file     ",
      "symlink  "
   };

   ZDEBUG() << "adding" << entryTypes[type] << ":" << fileName.constData() << (type == 2 ? QByteArray(" -> " +
            contents).constData() : "");
#endif

   if (! (device->isOpen() || device->open(QIODevice::WriteOnly))) {
      status = QZipWriter::FileOpenError;
      return;
   }
   device->seek(start_of_directory);

   // don't compress small files
   QZipWriter::CompressionPolicy compression = compressionPolicy;
   if (compressionPolicy == QZipWriter::AutoCompress) {
      if (contents.length() < 64) {
         compression = QZipWriter::NeverCompress;
      } else {
         compression = QZipWriter::AlwaysCompress;
      }
   }

   FileHeader header;
   memset(&header.h, 0, sizeof(CentralFileHeader));
   writeUInt(header.h.signature, 0x02014b50);

   writeUShort(header.h.version_needed, ZIP_VERSION);
   writeUInt(header.h.uncompressed_size, contents.length());
   writeMSDosDate(header.h.last_mod_file, QDateTime::currentDateTime());
   QByteArray data = contents;
   if (compression == QZipWriter::AlwaysCompress) {
      writeUShort(header.h.compression_method, CompressionMethodDeflated);

      ulong len = contents.length();
      // shamelessly copied form zlib
      len += (len >> 12) + (len >> 14) + 11;
      int res;

      do {
         data.resize(len);
         res = deflate((uchar *)data.data(), &len, (const uchar *)contents.constData(), contents.length());

         switch (res) {
            case Z_OK:
               data.resize(len);
               break;

            case Z_MEM_ERROR:
               qWarning("QZip: Z_MEM_ERROR: Not enough memory to compress file, skipping");
               data.resize(0);
               break;

            case Z_BUF_ERROR:
               len *= 2;
               break;
         }
      } while (res == Z_BUF_ERROR);
   }

   // TODO add a check if data.length() > contents.length().  Then try to store the original and revert the compression method to be uncompressed
   writeUInt(header.h.compressed_size, data.length());
   uint crc_32 = ::crc32(0, 0, 0);
   crc_32      = ::crc32(crc_32, (const uchar *)contents.constData(), contents.length());

   writeUInt(header.h.crc_32, crc_32);

   // if bit 11 is set, the filename and comment fields must be encoded using UTF-8
   ushort general_purpose_bits = Utf8Names; // always use utf-8
   writeUShort(header.h.general_purpose_bits, general_purpose_bits);

   const bool inUtf8 = (general_purpose_bits & Utf8Names) != 0;
   header.file_name = inUtf8 ? fileName.toUtf8() : fileName.toLatin1();

   if (header.file_name.size() > 0xffff) {
      qWarning("QZip: Filename is too long, chopping it to 65535 bytes");
      header.file_name = header.file_name.left(0xffff);
   }

   if (header.file_comment.size() + header.file_name.size() > 0xffff) {
      qWarning("QZip: File comment is too long, chopping it to 65535 bytes");
      header.file_comment.truncate(0xffff - header.file_name.size()); // ### don't break the utf-8 sequence, if any
   }
   writeUShort(header.h.file_name_length, header.file_name.length());
   //h.extra_field_length[2];

   writeUShort(header.h.version_made, HostUnix << 8);
   //uchar internal_file_attributes[2];
   //uchar external_file_attributes[4];
   quint32 mode = permissionsToMode(permissions);

   switch (type) {
      case Symlink:
         mode |= UnixFileAttributes::SymLink;
         break;
      case Directory:
         mode |= UnixFileAttributes::Dir;
         break;
      case File:
         mode |= UnixFileAttributes::File;
         break;
      default:
         // error, may want to throw
         break;
   }

   writeUInt(header.h.external_file_attributes, mode << 16);
   writeUInt(header.h.offset_local_header, start_of_directory);

   fileHeaders.append(header);

   LocalFileHeader h = header.h.toLocalHeader();
   device->write((const char *)&h, sizeof(LocalFileHeader));
   device->write(header.file_name);
   device->write(data);
   start_of_directory = device->pos();
   dirtyFileTree = true;
}

QZipReader::QZipReader(const QString &archive, QIODevice::OpenMode mode)
{
   QScopedPointer<QFile> f(new QFile(archive));
   QZipReader::Status status;

   if (f->open(mode) && f->error() == QFile::NoError) {
      status = NoError;

   } else {
      if (f->error() == QFile::ReadError) {
         status = FileReadError;
      } else if (f->error() == QFile::OpenError) {
         status = FileOpenError;
      } else if (f->error() == QFile::PermissionsError) {
         status = FilePermissionsError;
      } else {
         status = FileError;
      }
   }

   d = new QZipReaderPrivate(f.data(), /*ownDevice=*/true);
   f.take();
   d->status = status;
}

QZipReader::QZipReader(QIODevice *device)
   : d(new QZipReaderPrivate(device, /*ownDevice=*/false))
{
   Q_ASSERT(device);
}


QZipReader::~QZipReader()
{
   close();
   delete d;
}


QIODevice *QZipReader::device() const
{
   return d->device;
}


bool QZipReader::isReadable() const
{
   return d->device->isReadable();
}

/*!
    Returns true if the file exists; otherwise returns false.
*/
bool QZipReader::exists() const
{
   QFile *f = qobject_cast<QFile *> (d->device);
   if (f == 0) {
      return true;
   }
   return f->exists();
}

/*!
    Returns the list of files the archive contains.
*/
QVector<QZipReader::FileInfo> QZipReader::fileInfoList() const
{
   d->scanFiles();
   QVector<FileInfo> files;

   const int numFileHeaders = d->fileHeaders.size();
   files.reserve(numFileHeaders);

   for (int i = 0; i < numFileHeaders; ++i) {
      files.append(d->fillFileInfo(i));
   }


   return files;

}

int QZipReader::count() const
{
   d->scanFiles();
   return d->fileHeaders.count();
}

QZipReader::FileInfo QZipReader::entryInfoAt(int index) const
{
   d->scanFiles();

   if (index >= 0 && index < d->fileHeaders.count()) {
      return d->fillFileInfo(index);
   }
   return QZipReader::FileInfo();
}
QByteArray QZipReader::fileData(const QString &fileName) const
{
   d->scanFiles();
   int i;

   for (i = 0; i < d->fileHeaders.size(); ++i) {
      if (QString::fromLatin1(d->fileHeaders.at(i).file_name) == fileName) {
         break;
      }
   }

   if (i == d->fileHeaders.size()) {
      return QByteArray();
   }

   FileHeader header = d->fileHeaders.at(i);

   ushort version_needed = readUShort(header.h.version_needed);
   if (version_needed > ZIP_VERSION) {
      qWarning("QZip: .ZIP specification version %d implementationis needed to extract the data.", version_needed);
      return QByteArray();
   }

   ushort general_purpose_bits = readUShort(header.h.general_purpose_bits);
   int compressed_size = readUInt(header.h.compressed_size);
   int uncompressed_size = readUInt(header.h.uncompressed_size);
   int start = readUInt(header.h.offset_local_header);
   //qDebug("uncompressing file %d: local header at %d", i, start);

   d->device->seek(start);
   LocalFileHeader lh;
   d->device->read((char *)&lh, sizeof(LocalFileHeader));

   uint skip = readUShort(lh.file_name_length) + readUShort(lh.extra_field_length);
   d->device->seek(d->device->pos() + skip);

   int compression_method = readUShort(lh.compression_method);

   if ((general_purpose_bits & Encrypted) != 0) {
      qWarning("QZip: Unsupported encryption method is needed to extract the data.");
      return QByteArray();
   }

   QByteArray compressed = d->device->read(compressed_size);
   if (compression_method == CompressionMethodStored) {
      // no compression
      compressed.truncate(uncompressed_size);
      return compressed;
   } else if (compression_method == CompressionMethodDeflated) {
      // Deflate
      //qDebug("compressed=%d", compressed.size());
      compressed.truncate(compressed_size);
      QByteArray baunzip;
      ulong len = qMax(uncompressed_size,  1);
      int res;
      do {
         baunzip.resize(len);
         res = inflate((uchar *)baunzip.data(), &len,
               (const uchar *)compressed.constData(), compressed_size);

         switch (res) {
            case Z_OK:
               if ((int)len != baunzip.size()) {
                  baunzip.resize(len);
               }
               break;
            case Z_MEM_ERROR:
               qWarning("QZip: Z_MEM_ERROR: Not enough memory");
               break;
            case Z_BUF_ERROR:
               len *= 2;
               break;
            case Z_DATA_ERROR:
               qWarning("QZip: Z_DATA_ERROR: Input data is corrupted");
               break;
         }
      } while (res == Z_BUF_ERROR);
      return baunzip;
   }

   qWarning("QZip: Unsupported compression method %d is needed to extract the data.", compression_method);
   return QByteArray();
}

/*!
    Extracts the full contents of the zip file into \a destinationDir on
    the local filesystem.
    In case writing or linking a file fails, the extraction will be aborted.
*/
bool QZipReader::extractAll(const QString &destinationDir) const
{
   QDir baseDir(destinationDir);

   // create directories first
   const QVector<FileInfo> allFiles = fileInfoList();

   for (const FileInfo &fi : allFiles) {
      const QString absPath = destinationDir + QDir::separator() + fi.filePath;
      if (fi.isDir) {
         if (!baseDir.mkpath(fi.filePath)) {
            return false;
         }
         if (!QFile::setPermissions(absPath, fi.permissions)) {
            return false;
         }
      }
   }

   // set up symlinks
   for (const FileInfo &fi : allFiles) {
      const QString absPath = destinationDir + QDir::separator() + fi.filePath;
      if (fi.isSymLink) {
         QString destination = QFile::decodeName(fileData(fi.filePath));
         if (destination.isEmpty()) {
            return false;
         }
         QFileInfo linkFi(absPath);
         if (!QFile::exists(linkFi.absolutePath())) {
            QDir::root().mkpath(linkFi.absolutePath());
         }
         if (!QFile::link(destination, absPath)) {
            return false;
         }
         /* cannot change permission of links
         if (!QFile::setPermissions(absPath, fi.permissions))
             return false;
         */
      }
   }

   for (const FileInfo &fi : allFiles) {
      const QString absPath = destinationDir + QDir::separator() + fi.filePath;
      if (fi.isFile) {
         QFile f(absPath);
         if (!f.open(QIODevice::WriteOnly)) {
            return false;
         }
         f.write(fileData(fi.filePath));
         f.setPermissions(fi.permissions);
         f.close();
      }
   }

   return true;
}

QZipReader::Status QZipReader::status() const
{
   return d->status;
}


void QZipReader::close()
{
   d->device->close();
}


QZipWriter::QZipWriter(const QString &fileName, QIODevice::OpenMode mode)
{
   QScopedPointer<QFile> f(new QFile(fileName));

   QZipWriter::Status status;

   if (f->open(mode) && f->error() == QFile::NoError) {
      status = QZipWriter::NoError;
   } else {
      if (f->error() == QFile::WriteError) {
         status = QZipWriter::FileWriteError;
      } else if (f->error() == QFile::OpenError) {
         status = QZipWriter::FileOpenError;
      } else if (f->error() == QFile::PermissionsError) {
         status = QZipWriter::FilePermissionsError;
      } else {
         status = QZipWriter::FileError;
      }
   }

   d = new QZipWriterPrivate(f.data(), /*ownDevice=*/true);
   f.take();
   d->status = status;
}

/*!
    Create a new zip archive that operates on the archive found in \a device.
    You have to open the device previous to calling the constructor and
    only a device that is readable will be scanned for zip filecontent.
 */
QZipWriter::QZipWriter(QIODevice *device)
   : d(new QZipWriterPrivate(device, /*ownDevice=*/false))
{
   Q_ASSERT(device);
}

QZipWriter::~QZipWriter()
{
   close();
   delete d;
}

/*!
    Returns device used for writing zip archive.
*/
QIODevice *QZipWriter::device() const
{
   return d->device;
}

/*!
    Returns true if the user can write to the archive; otherwise returns false.
*/
bool QZipWriter::isWritable() const
{
   return d->device->isWritable();
}

/*!
    Returns true if the file exists; otherwise returns false.
*/
bool QZipWriter::exists() const
{
   QFile *f = qobject_cast<QFile *> (d->device);
   if (f == 0) {
      return true;
   }
   return f->exists();
}


QZipWriter::Status QZipWriter::status() const
{
   return d->status;
}


void QZipWriter::setCompressionPolicy(CompressionPolicy policy)
{
   d->compressionPolicy = policy;
}


QZipWriter::CompressionPolicy QZipWriter::compressionPolicy() const
{
   return d->compressionPolicy;
}

void QZipWriter::setCreationPermissions(QFile::Permissions permissions)
{
   d->permissions = permissions;
}


QFile::Permissions QZipWriter::creationPermissions() const
{
   return d->permissions;
}

/*!
    Add a file to the archive with \a data as the file contents.
    The file will be stored in the archive using the \a fileName which
    includes the full path in the archive.

    The new file will get the file permissions based on the current
    creationPermissions and it will be compressed using the zip compression
    based on the current compression policy.

    \sa setCreationPermissions()
    \sa setCompressionPolicy()
*/
void QZipWriter::addFile(const QString &fileName, const QByteArray &data)
{
   d->addEntry(QZipWriterPrivate::File, QDir::fromNativeSeparators(fileName), data);
}

/*!
    Add a file to the archive with \a device as the source of the contents.
    The contents returned from QIODevice::readAll() will be used as the
    filedata.
    The file will be stored in the archive using the \a fileName which
    includes the full path in the archive.
*/
void QZipWriter::addFile(const QString &fileName, QIODevice *device)
{
   Q_ASSERT(device);
   QIODevice::OpenMode mode = device->openMode();
   bool opened = false;
   if ((mode & QIODevice::ReadOnly) == 0) {
      opened = true;
      if (! device->open(QIODevice::ReadOnly)) {
         d->status = FileOpenError;
         return;
      }
   }
   d->addEntry(QZipWriterPrivate::File, QDir::fromNativeSeparators(fileName), device->readAll());
   if (opened) {
      device->close();
   }
}

/*!
    Create a new directory in the archive with the specified \a dirName and
    the \a permissions;
*/
void QZipWriter::addDirectory(const QString &dirName)
{
   QString name(QDir::fromNativeSeparators(dirName));
   // separator is mandatory
   if (!name.endsWith(QLatin1Char('/'))) {
      name.append(QLatin1Char('/'));
   }
   d->addEntry(QZipWriterPrivate::Directory, name, QByteArray());
}

/*!
    Create a new symbolic link in the archive with the specified \a dirName
    and the \a permissions;
    A symbolic link contains the destination (relative) path and name.
*/
void QZipWriter::addSymLink(const QString &fileName, const QString &destination)
{
   d->addEntry(QZipWriterPrivate::Symlink, QDir::fromNativeSeparators(fileName), QFile::encodeName(destination));
}

/*!
   Closes the zip file.
*/
void QZipWriter::close()
{
   if (!(d->device->openMode() & QIODevice::WriteOnly)) {
      d->device->close();
      return;
   }

   //qDebug("QZip::close writing directory, %d entries", d->fileHeaders.size());
   d->device->seek(d->start_of_directory);
   // write new directory
   for (int i = 0; i < d->fileHeaders.size(); ++i) {
      const FileHeader &header = d->fileHeaders.at(i);
      d->device->write((const char *)&header.h, sizeof(CentralFileHeader));
      d->device->write(header.file_name);
      d->device->write(header.extra_field);
      d->device->write(header.file_comment);
   }
   int dir_size = d->device->pos() - d->start_of_directory;
   // write end of directory
   EndOfDirectory eod;
   memset(&eod, 0, sizeof(EndOfDirectory));
   writeUInt(eod.signature, 0x06054b50);
   //uchar this_disk[2];
   //uchar start_of_directory_disk[2];
   writeUShort(eod.num_dir_entries_this_disk, d->fileHeaders.size());
   writeUShort(eod.num_dir_entries, d->fileHeaders.size());
   writeUInt(eod.directory_size, dir_size);
   writeUInt(eod.dir_start_offset, d->start_of_directory);
   writeUShort(eod.comment_length, d->comment.length());

   d->device->write((const char *)&eod, sizeof(EndOfDirectory));
   d->device->write(d->comment);
   d->device->close();
}

#endif // QT_NO_TEXTODFWRITER
