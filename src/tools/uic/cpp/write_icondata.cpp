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

#include <write_icondata.h>

#include <driver.h>
#include <ui4.h>
#include <uic.h>

#include <qtextstream.h>

namespace CPP {

static QByteArray transformImageData(QString data)
{
   int baSize = data.length() / 2;
   uchar *ba = new uchar[baSize];

   for (int i = 0; i < baSize; ++i) {
      char h = data[2 * (i)].toLatin1();
      char l = data[2 * (i) + 1].toLatin1();
      uchar r = 0;

      if (h <= '9') {
         r += h - '0';

      } else {
         r += h - 'a' + 10;
      }

      r = r << 4;

      if (l <= '9') {
         r += l - '0';
      } else {
         r += l - 'a' + 10;
      }

      ba[i] = r;
   }

   QByteArray retval(reinterpret_cast<const char *>(ba), baSize);
   delete [] ba;

   return retval;
}

static QByteArray unzipXPM(QString data, ulong &length)
{
#ifndef QT_NO_COMPRESS
   const int lengthOffset = 4;
   QByteArray ba(lengthOffset, ' ');

   // qUncompress() expects the first 4 bytes to be the expected length of the
   // uncompressed data
   ba[0] = (length & 0xff000000) >> 24;
   ba[1] = (length & 0x00ff0000) >> 16;
   ba[2] = (length & 0x0000ff00) >> 8;
   ba[3] = (length & 0x000000ff);
   ba.append(transformImageData(data));

   QByteArray baunzip = qUncompress(ba);
   return baunzip;

#else
   (void) data;
   (void) length;

   return QByteArray();
#endif

}

WriteIconData::WriteIconData(Uic *uic)
   : driver(uic->driver()), output(uic->output()), option(uic->option())
{
}

void WriteIconData::acceptUI(DomUI *node)
{
   TreeWalker::acceptUI(node);
}

void WriteIconData::acceptImages(DomImages *images)
{
   TreeWalker::acceptImages(images);
}

void WriteIconData::acceptImage(DomImage *image)
{
   // Limit line length when writing code.
   writeImage(output, option.indent, true, image);
}

void WriteIconData::writeImage(QTextStream &output, const QString &indent,
   bool limitXPM_LineLength, const DomImage *image)
{
   QString img  = image->attributeName() + "_data";
   QString data = image->elementData()->text();
   QString fmt  = image->elementData()->attributeFormat();

   int size = image->elementData()->attributeLength();

   if (fmt == "XPM.GZ") {
      ulong length = size;

      QByteArray baunzip = unzipXPM(data, length);
      length = baunzip.size();

      // shouldn't we test the initial 'length' against the
      // resulting 'length' to catch corrupt UIC files?

      int a        = 0;
      int column   = 0;
      bool inQuote = false;

      output << indent << "/* XPM */\n"
             << indent << "static const char* const " << img << "[] = { \n";

      while (baunzip[a] != '\"') {
         a++;
      }

      for (; a < (int) length; a++) {
         output << baunzip[a];

         if (baunzip[a] == '\n') {
            column = 0;
         } else if (baunzip[a] == '"') {
            inQuote = ! inQuote;
         }

         column++;
         if (limitXPM_LineLength && column >= 512 && inQuote) {
            output << "\"\n\""; // be nice with MSVC & Co.
            column = 1;
         }
      }

      if (! baunzip.trimmed ().endsWith ("};")) {
         output << "};";
      }

      output << "\n\n";

   } else {
      output << indent << "static const unsigned char " << img << "[] = { \n";
      output << indent;
      int a ;

      for (a = 0; a < (int) (data.length() / 2) - 1; a++) {
         output << "0x" << QString(data[2 * a]) << QString(data[2 * a + 1]) << ',';

         if (a % 12 == 11) {
            output << '\n' << indent;
         } else {
            output << ' ';
         }
      }

      output << "0x" << QString(data[2 * a]) << QString(data[2 * a + 1]) << '\n';
      output << "};\n\n";
   }
}

void WriteIconData::writeImage(QIODevice &output, DomImage *image)
{
   const QByteArray array = transformImageData(image->elementData()->text());
   output.write(array.constData(), array.size());
}

}
