/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qbytearray.h>
#include <qbuffer.h>
#include <qdebug.h>
#include <qfile.h>
#include <qregularexpression.h>
#include <qstring.h>
#include <qstringlist.h>

static QString utf8Escape(const QString &array)
{
   // convert tranøy.no to tran\xc3\xb8y.no

   QString retval;

   for (int i = 0; i < array.size_storage(); ++i) {
      char c = array.constData()[i];

      // if char is non-ascii then escape it
      if (c < 0x20 || uchar(c) >= 0x7f) {
         retval += "\\x" + QString::number(uchar(c), 16);

      } else {
         // if previous char was escaped, we need to make sure the next char is not
         // interpreted as part of the hex value, e.g. "äc.com" -> "\xabc.com"; this
         // should be "\xab""c.com"

         QRegularExpression hexEscape("\\\\x[a-fA-F0-9][a-fA-F0-9]$");

         bool isHexChar = ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));

         if (retval.contains(hexEscape) && isHexChar) {
            retval += "\"\"";
         }

         retval += c;
      }
   }

   return retval;
}

int main(int argc, char **argv)
{
   QCoreApplication app(argc, argv);

   if (argc < 2) {
      printf("\nUsage: Generate_TLD  InputFile\n\n");
      printf("Input File:  https://publicsuffix.org/list/public_suffix_list.dat \n");
      exit(1);
   }

   QFile file(QString::fromUtf8(argv[1]));
   file.open(QIODevice::ReadOnly);

   QFile outFile("qurltlds_p.h");
   outFile.open(QIODevice::WriteOnly);

   QByteArray outIndicesBufferBA;
   QBuffer outIndicesBuffer(&outIndicesBufferBA);
   outIndicesBuffer.open(QIODevice::WriteOnly);

   QByteArray outDataBufferBA;
   QBuffer outDataBuffer(&outDataBufferBA);
   outDataBuffer.open(QIODevice::WriteOnly);

   // number of valid lines in the file
   int lineCount = 0;

   while (! file.atEnd()) {
      QString nextLine = QString::fromUtf8(file.readLine()).trimmed();

      // skip comments and blank lines
      if (nextLine.startsWith("//") || nextLine.isEmpty()) {
         continue;
      }

      ++lineCount;
   }

   file.reset();

   //
   QVector<QString> strings(lineCount);

   while (! file.atEnd()) {
      QString nextLine = QString::fromUtf8(file.readLine()).trimmed();

      // skip comments and blank lines
      if (nextLine.startsWith("//") || nextLine.isEmpty()) {
         continue;
      }

      int num = qHash(nextLine) % lineCount;

      QString utf8String = utf8Escape(nextLine);

      // for domain 1.com, we could get something like a.com\01.com,
      // which would be interpreted as octal 01,
      // so we need to separate those strings with quotes
      QRegularExpression regexpOctalEscape("^[0-9]");

      if (! strings.at(num).isEmpty() && nextLine.contains(regexpOctalEscape)) {
         strings[num].append("\"\"");
      }

      strings[num].append(utf8String);
      strings[num].append("\\0");
   }

   outIndicesBuffer.write("static constexpr const quint16 tldCount = ");
   outIndicesBuffer.write(QByteArray::number(lineCount));
   outIndicesBuffer.write(";\n\n");

   outIndicesBuffer.write("static const quint32 tldIndices[] = {\n");

   int totalUtf8Size  = 0;
   int chunkSize      = 0;
   int stringUtf8Size = 0;

   QStringList chunks;

   for (int a = 0; a < lineCount; a++) {
      bool lineIsEmpty = strings.at(a).isEmpty();

      if (! lineIsEmpty) {
         strings[a].prepend("\"");
         strings[a].append("\"");
      }

      int zeroCount = strings.at(a).count("\\0");
      int utf8CharsCount = strings.at(a).count("\\x");
      int quoteCount = strings.at(a).count('"');

      stringUtf8Size = strings.at(a).count() - (zeroCount + quoteCount + utf8CharsCount * 3);
      chunkSize += stringUtf8Size;

      if (chunkSize > 65535) {
         static int chunkCount = 0;

         qWarning() << "chunk" << ++chunkCount << "has length" << chunkSize - stringUtf8Size;

         outDataBuffer.write(",\n");
         chunks.append(QByteArray::number(totalUtf8Size));
         chunkSize = 0;
      }

      outDataBuffer.write(strings.at(a).toUtf8());

      if (! lineIsEmpty) {
         outDataBuffer.write("\n");
      }

      outIndicesBuffer.write(QByteArray::number(totalUtf8Size));
      outIndicesBuffer.write(",\n");
      totalUtf8Size += stringUtf8Size;
   }

   chunks.append(QByteArray::number(totalUtf8Size));
   outIndicesBuffer.write(QByteArray::number(totalUtf8Size));
   outIndicesBuffer.write("};\n");
   outIndicesBuffer.close();
   outFile.write(outIndicesBufferBA);

   outDataBuffer.close();

   outFile.write("\nstatic const char *tldData[] = {\n");

   outFile.write(outDataBufferBA);
   outFile.write("};\n\n");

   // write chunk information
   outFile.write("static constexpr const quint16 tldChunkCount = ");
   outFile.write(QByteArray::number(chunks.count()));
   outFile.write(";\n");

   outFile.write("static constexpr const quint32 tldChunks[] = {");
   outFile.write(chunks.join(", ").toLatin1());
   outFile.write("};\n");

   outFile.close();

   printf("Data generated, copy qurltlds_p.h  src/core/io/qurltlds_p.h\n");

   exit(0);
}
