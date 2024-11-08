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

#ifndef QReportContext_P_H
#define QReportContext_P_H

#include <QSharedData>
#include <QAbstractUriResolver>
#include <QSourceLocation>
#include <qnamepool_p.h>
#include <qxmlname.h>
#include <qstringfwd.h>

class QAbstractMessageHandler;
class QSourceLocation;

namespace QPatternist {
class SourceLocationReflection;

class ReportContext : public QSharedData
{
 public:
   typedef QHash<const SourceLocationReflection *, QSourceLocation> LocationHash;
   typedef QExplicitlySharedDataPointer<ReportContext> Ptr;

   ReportContext()
   { }

   virtual ~ReportContext();

   enum ErrorCode {
      XSDError,
      XPST0001,
      XPDY0002,
      XPST0003,
      XPTY0004,
      XPST0005,

      XPTY0006,         // not currently used
      XPTY0007,         // not currently used

      XPST0008,
      XQST0009,
      XPST0010,
      XQST0012,
      XQST0013,

      XQST0014,         // not currently used
      XQST0015,         // not currently used

      XQST0016,
      XPST0017,
      XPTY0018,
      XPTY0019,
      XPTY0020,

      XPDY0021,         // not currently used

      XQST0022,

      XQTY0023,         // not currently used

      XQTY0024,
      XQDY0025,
      XQDY0026,
      XQDY0027,

      XQTY0028,         // not currently used
      XQDY0029,         // not currently used

      XQTY0030,
      XQST0031,
      XQST0032,
      XQST0033,
      XQST0034,
      XQST0035,
      XQST0036,

      XQST0037,         // not currently used

      XQST0038,
      XQST0039,
      XQST0040,
      XQDY0041,

      XQST0042,         // not currently used
      XQST0043,         // not currently used

      XQDY0044,
      XQST0045,
      XQST0046,
      XQST0047,
      XQST0048,
      XQST0049,
      XPDY0050,
      XPST0051,

      XQDY0052,         // not currently used
      XQST0053,         // not currently used

      XQST0054,
      XQST0055,

      XQST0056,         // not currently used

      XQST0057,
      XQST0058,
      XQST0059,
      XQST0060,
      XQDY0061,

      XQDY0062,         // not currently used
      XQST0063,         // not currently used

      XQDY0064,
      XQST0065,
      XQST0066,
      XQST0067,
      XQST0068,
      XQST0069,
      XQST0070,
      XQST0071,
      XQDY0072,
      XQST0073,
      XQDY0074,
      XQST0075,
      XQST0076,

      XQST0077,         // not currently used
      XQST0078,         // not currently used

      XQST0079,
      XPST0080,
      XPST0081,

      XQST0082,         // not currently used
      XPST0083,         // not currently used

      XQDY0084,
      XQST0085,
      XQTY0086,
      XQST0087,
      XQST0088,
      XQST0089,
      XQST0090,
      XQDY0091,
      XQDY0092,
      XQST0093,

      FOER0000,
      FOAR0001,
      FOAR0002,
      FOCA0001,
      FOCA0002,
      FOCA0003,
      FOCA0005,
      FOCA0006,
      FOCH0001,
      FOCH0002,
      FOCH0003,
      FOCH0004,
      FODC0001,
      FODC0002,
      FODC0003,
      FODC0004,
      FODC0005,
      FODT0001,
      FODT0002,
      FODT0003,
      FONS0004,
      FONS0005,
      FORG0001,
      FORG0002,
      FORG0003,
      FORG0004,
      FORG0005,
      FORG0006,
      FORG0008,
      FORG0009,
      FORX0001,
      FORX0002,
      FORX0003,
      FORX0004,
      FOTY0012,

      SENR0001,
      SERE0003,
      SEPM0004,
      SERE0005,
      SERE0006,
      SESU0007,
      SERE0008,
      SEPM0009,
      SEPM0010,
      SESU0011,
      SERE0012,
      SESU0013,
      SERE0014,
      SERE0015,
      SEPM0016,

      XTSE0010,
      XTSE0020,
      XTSE0080,
      XTSE0090,
      XTSE0110,
      XTSE0120,
      XTSE0125,
      XTSE0130,
      XTSE0150,
      XTSE0165,
      XTSE0170,
      XTSE0180,
      XTSE0190,
      XTSE0200,
      XTSE0210,
      XTSE0215,
      XTSE0220,
      XTSE0260,
      XTSE0265,
      XTSE0280,
      XTSE0340,
      XTSE0350,
      XTSE0370,
      XTSE0500,
      XTSE0530,
      XTSE0550,
      XTSE0580,
      XTSE0620,
      XTSE0630,
      XTSE0650,
      XTSE0660,
      XTSE0670,
      XTSE0680,
      XTSE0690,
      XTSE0710,
      XTSE0720,
      XTSE0740,
      XTSE0760,
      XTSE0770,
      XTSE0805,
      XTSE0808,
      XTSE0809,
      XTSE0810,
      XTSE0812,
      XTSE0840,
      XTSE0870,
      XTSE0880,
      XTSE0910,
      XTSE0940,
      XTTE0950,
      XTSE0975,
      XTSE1015,
      XTSE1017,
      XTSE1040,
      XTSE1060,
      XTSE1070,
      XTSE1080,
      XTSE1090,
      XTSE1130,
      XTSE1205,
      XTSE1210,
      XTSE1220,
      XTSE1290,
      XTSE1295,
      XTSE1300,
      XTSE1430,
      XTSE1505,
      XTSE1520,
      XTSE1530,
      XTSE1560,
      XTSE1570,
      XTSE1580,
      XTSE1590,
      XTSE1600,
      XTSE1650,
      XTSE1660,
      XTTE0505,
      XTTE0510,
      XTTE0520,
      XTTE0570,
      XTTE0590,
      XTTE0600,
      XTTE0780,
      XTTE0790,
      XTTE0990,
      XTTE1000,
      XTTE1020,
      XTTE1100,
      XTTE1120,
      XTTE1510,
      XTTE1512,
      XTTE1515,
      XTTE1540,
      XTTE1545,
      XTTE1550,
      XTTE1555,
      XTDE0030,
      XTDE0040,
      XTDE0045,
      XTDE0047,
      XTDE0050,
      XTDE0060,
      XTDE0160,
      XTRE0270,
      XTDE0290,
      XTDE0410,
      XTDE0420,
      XTDE0430,
      XTDE0440,
      XTDE0485,
      XTRE0540,
      XTDE0560,
      XTDE0610,
      XTDE0640,
      XTDE0700,
      XTRE0795,
      XTDE0820,
      XTDE0830,
      XTDE0835,
      XTDE0850,
      XTDE0855,
      XTDE0860,
      XTDE0865,
      XTDE0890,
      XTDE0905,
      XTDE0920,
      XTDE0925,
      XTDE0930,
      XTDE0980,
      XTDE1030,
      XTDE1035,
      XTDE1110,
      XTDE1140,
      XTDE1145,
      XTDE1150,
      XTRE1160,
      XTDE1170,
      XTDE1190,
      XTDE1200,
      XTDE1260,
      XTDE1270,
      XTDE1280,
      XTDE1310,
      XTDE1340,
      XTDE1350,
      XTDE1360,
      XTDE1370,
      XTDE1380,
      XTDE1390,
      XTMM9000,
      XTDE1400,
      XTDE1420,
      XTDE1425,
      XTDE1428,
      XTDE1440,
      XTDE1450,
      XTDE1460,
      XTDE1480,
      XTDE1490,
      XTRE1495,
      XTRE1500,
      XTRE1620,
      XTRE1630,
      XTDE1665
   };

   void warning(const QString &message, const QSourceLocation &sourceLocation = QSourceLocation());
   void error(const QString &message, const ReportContext::ErrorCode errorCode, const QSourceLocation &sourceLocation);
   void error(const QString &message, const ReportContext::ErrorCode errorCode, const SourceLocationReflection *const reflection);
   void error(const QString &message, const QXmlName qName, const SourceLocationReflection *const r);

   virtual QAbstractMessageHandler *messageHandler() const = 0;
   virtual NamePool::Ptr namePool() const = 0;

   static QString codeToString(const ReportContext::ErrorCode errorCode);
   static QString codeFromURI(const QString &typeURI, QString &uri);

   virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const = 0;

   QUrl resolveURI(const QUrl &relative, const QUrl &baseURI) const;

   virtual const QAbstractUriResolver *uriResolver() const = 0;

 private:
   void createError(const QString &description, const QtMsgType type, const QUrl &id,
      const QSourceLocation &sourceLocation) const;

   static inline QString finalizeDescription(const QString &desc);
   QSourceLocation lookupSourceLocation(const SourceLocationReflection *const ref) const;

   ReportContext(const ReportContext &) = delete;
   ReportContext &operator=(const ReportContext &) = delete;
};

typedef bool Exception;
}

#endif
