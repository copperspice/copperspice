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

#ifndef QBMPHANDLER_P_H
#define QBMPHANDLER_P_H

#include <qimageiohandler.h>

#ifndef QT_NO_IMAGEFORMAT_BMP


struct BMP_FILEHDR {                    // BMP file header
   char   bfType[2];                    // "BM"
   qint32  bfSize;                      // size of file
   qint16  bfReserved1;
   qint16  bfReserved2;
   qint32  bfOffBits;                   // pointer to the pixmap bits
};

struct BMP_INFOHDR {                    // BMP information header
   qint32  biSize;                      // size of this struct
   qint32  biWidth;                     // pixmap width
   qint32  biHeight;                    // pixmap height
   qint16  biPlanes;                    // should be 1
   qint16  biBitCount;                  // number of bits per pixel
   qint32  biCompression;               // compression method
   qint32  biSizeImage;                 // size of image
   qint32  biXPelsPerMeter;             // horizontal resolution
   qint32  biYPelsPerMeter;             // vertical resolution
   qint32  biClrUsed;                   // number of colors used
   qint32  biClrImportant;              // number of important colors
};

class QBmpHandler : public QImageIOHandler
{
 public:
   enum InternalFormat {
      DibFormat,
      BmpFormat
   };

   explicit QBmpHandler(InternalFormat fmt = BmpFormat);

   bool canRead() override;
   bool read(QImage *image) override;
   bool write(const QImage &image) override;

   QString name() const override;

   static bool canRead(QIODevice *device);

   QVariant option(ImageOption option) override;
   void setOption(ImageOption option, const QVariant &value) override;
   bool supportsOption(ImageOption option) const override;

 private:
   bool readHeader();
   inline QString formatName() const;

   enum State {
      Ready,
      ReadHeader,
      Error
   };
   const InternalFormat m_format;
   State state;

   BMP_FILEHDR fileHeader;
   BMP_INFOHDR infoHeader;
   int startpos;
};

#endif // QT_NO_IMAGEFORMAT_BMP

#endif
