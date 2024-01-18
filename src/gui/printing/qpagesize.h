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

#ifndef QPAGESIZE_H
#define QPAGESIZE_H

#include <qsharedpointer.h>
#include <qstring.h>
#include <qrectf.h>

#if defined(B0)
#undef B0             // Terminal hang-up, assume you do not want that
#endif

class QPageSizePrivate;
class QSize;
class QSizeF;

class Q_GUI_EXPORT QPageSize
{
 public:

   enum PageSizeId {
      A4,
      B5,
      Letter,
      Legal,
      Executive,
      A0,
      A1,
      A2,
      A3,
      A5,
      A6,
      A7,
      A8,
      A9,
      B0,
      B1,
      B10,
      B2,
      B3,
      B4,
      B6,
      B7,
      B8,
      B9,
      C5E,
      Comm10E,
      DLE,
      Folio,
      Ledger,
      Tabloid,
      Custom,

      // New values derived from PPD standard
      A10,
      A3Extra,
      A4Extra,
      A4Plus,
      A4Small,
      A5Extra,
      B5Extra,

      JisB0,
      JisB1,
      JisB2,
      JisB3,
      JisB4,
      JisB5,
      JisB6,
      JisB7,
      JisB8,
      JisB9,
      JisB10,

      // AnsiA = Letter,
      // AnsiB = Ledger,
      AnsiC,
      AnsiD,
      AnsiE,
      LegalExtra,
      LetterExtra,
      LetterPlus,
      LetterSmall,
      TabloidExtra,

      ArchA,
      ArchB,
      ArchC,
      ArchD,
      ArchE,

      Imperial7x9,
      Imperial8x10,
      Imperial9x11,
      Imperial9x12,
      Imperial10x11,
      Imperial10x13,
      Imperial10x14,
      Imperial12x11,
      Imperial15x11,

      ExecutiveStandard,
      Note,
      Quarto,
      Statement,
      SuperA,
      SuperB,
      Postcard,
      DoublePostcard,
      Prc16K,
      Prc32K,
      Prc32KBig,

      FanFoldUS,
      FanFoldGerman,
      FanFoldGermanLegal,

      EnvelopeB4,
      EnvelopeB5,
      EnvelopeB6,
      EnvelopeC0,
      EnvelopeC1,
      EnvelopeC2,
      EnvelopeC3,
      EnvelopeC4,
      // EnvelopeC5 = C5E,
      EnvelopeC6,
      EnvelopeC65,
      EnvelopeC7,
      // EnvelopeDL = DLE,

      Envelope9,
      // Envelope10 = Comm10E,
      Envelope11,
      Envelope12,
      Envelope14,
      EnvelopeMonarch,
      EnvelopePersonal,

      EnvelopeChou3,
      EnvelopeChou4,
      EnvelopeInvite,
      EnvelopeItalian,
      EnvelopeKaku2,
      EnvelopeKaku3,
      EnvelopePrc1,
      EnvelopePrc2,
      EnvelopePrc3,
      EnvelopePrc4,
      EnvelopePrc5,
      EnvelopePrc6,
      EnvelopePrc7,
      EnvelopePrc8,
      EnvelopePrc9,
      EnvelopePrc10,
      EnvelopeYou4,

      // Last item, with commonly used synynoms from QPagedPrintEngine / QPrinter
      LastPageSize = EnvelopeYou4,
      NPageSize = LastPageSize,
      NPaperSize = LastPageSize,

      // Convenience overloads for naming consistency
      AnsiA = Letter,
      AnsiB = Ledger,
      EnvelopeC5 = C5E,
      EnvelopeDL = DLE,
      Envelope10 = Comm10E
   };

   enum Unit {
      Millimeter,
      Point,
      Inch,
      Pica,
      Didot,
      Cicero,
      DevicePixel
   };

   enum SizeMatchPolicy {
      FuzzyMatch,
      FuzzyOrientationMatch,
      ExactMatch
   };

   QPageSize();
   explicit QPageSize(PageSizeId sizeId);
   explicit QPageSize(const QSize &size, const QString &name = QString(), SizeMatchPolicy matchPolicy = FuzzyMatch);
   explicit QPageSize(const QSizeF &size, Unit units, const QString &name = QString(), SizeMatchPolicy matchPolicy = FuzzyMatch);

   QPageSize(const QPageSize &other);

   ~QPageSize();

   QPageSize &operator=(QPageSize &&other)  {
      swap(other);
      return *this;
   }
   QPageSize &operator=(const QPageSize &other);

   void swap(QPageSize &other) {
      qSwap(d, other.d);
   }

   friend Q_GUI_EXPORT bool operator==(const QPageSize &lhs, const QPageSize &rhs);
   bool isEquivalentTo(const QPageSize &other) const;

   bool isValid() const;

   QString key() const;
   QString name() const;

   PageSizeId id() const;

   int windowsId() const;

   QSizeF definitionSize() const;
   Unit definitionUnits() const;

   QSizeF size(Unit units) const;
   QSize sizePoints() const;
   QSize sizePixels(int resolution) const;

   QRectF rect(Unit units) const;
   QRect rectPoints() const;
   QRect rectPixels(int resolution) const;

   static QString key(PageSizeId pageSizeId);
   static QString name(PageSizeId pageSizeId);

   static PageSizeId id(const QSize &pointSize, SizeMatchPolicy matchPolicy = FuzzyMatch);
   static PageSizeId id(const QSizeF &size, Unit units, SizeMatchPolicy matchPolicy = FuzzyMatch);

   static PageSizeId id(int windowsId);
   static int windowsId(PageSizeId pageSizeId);

   static QSizeF definitionSize(PageSizeId pageSizeId);
   static Unit definitionUnits(PageSizeId pageSizeId);

   static QSizeF size(PageSizeId pageSizeId, Unit units);
   static QSize sizePoints(PageSizeId pageSizeId);
   static QSize sizePixels(PageSizeId pageSizeId, int resolution);

 private:
   friend class QPageSizePrivate;
   friend class QPlatformPrintDevice;

   QPageSize(const QString &key, const QSize &pointSize, const QString &name);
   QPageSize(int windowsId, const QSize &pointSize, const QString &name);
   QPageSize(QPageSizePrivate &dd);
   QSharedDataPointer<QPageSizePrivate> d;
};

Q_GUI_EXPORT bool operator==(const QPageSize &lhs, const QPageSize &rhs);

inline bool operator!=(const QPageSize &lhs, const QPageSize &rhs)
{
   return !operator==(lhs, rhs);
}

Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QPageSize &pageSize);

CS_DECLARE_METATYPE(QPageSize)
CS_DECLARE_METATYPE(QPageSize::Unit)

#endif
