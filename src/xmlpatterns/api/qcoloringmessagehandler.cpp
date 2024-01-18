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

#include <qxmlstreamreader.h>

#include <qcoloringmessagehandler_p.h>
#include <qxmlpatternistcli_p.h>

using namespace QPatternist;

ColoringMessageHandler::ColoringMessageHandler(QObject *parent) : QAbstractMessageHandler(parent)
{
   m_classToColor.insert(QLatin1String("XQuery-data"), Data);
   m_classToColor.insert(QLatin1String("XQuery-expression"), Keyword);
   m_classToColor.insert(QLatin1String("XQuery-function"), Keyword);
   m_classToColor.insert(QLatin1String("XQuery-keyword"), Keyword);
   m_classToColor.insert(QLatin1String("XQuery-type"), Keyword);
   m_classToColor.insert(QLatin1String("XQuery-uri"), Data);
   m_classToColor.insert(QLatin1String("XQuery-filepath"), Data);

   /* If you're tuning the colors, take it easy laddie. Take into account:
    *
    * - Get over your own taste, there's others too on this planet
    * - Make sure it works well on black & white
    * - Make sure it works well on white & black
    */
   insertMapping(Location, CyanForeground);
   insertMapping(ErrorCode, RedForeground);
   insertMapping(Keyword, BlueForeground);
   insertMapping(Data, BlueForeground);
   insertMapping(RunningText, DefaultColor);
}

void ColoringMessageHandler::handleMessage(QtMsgType type,
      const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
   const bool hasLine = sourceLocation.line() != -1;

   switch (type) {
      case QtWarningMsg: {
         if (hasLine) {
            writeUncolored(QXmlPatternistCLI::tr("Warning in %1, at line %2, column %3: %4")
                  .formatArgs(QString(sourceLocation.uri().toEncoded()), QString::number(sourceLocation.line()),
                  QString::number(sourceLocation.column()), colorifyDescription(description)));

         } else {
            writeUncolored(QXmlPatternistCLI::tr("Warning in %1: %2")
                  .formatArgs(QString(sourceLocation.uri().toEncoded()), colorifyDescription(description)));
         }

         break;
      }

      case QtFatalMsg: {
         const QString errorCode(identifier.fragment());

         Q_ASSERT(! errorCode.isEmpty());

         QUrl uri(identifier);
         uri.setFragment(QString());

         QString location;

         if (sourceLocation.isNull()) {
            location = QXmlPatternistCLI::tr("Unknown location");
         } else {
            location = QString::fromLatin1(sourceLocation.uri().toEncoded());
         }

         QString errorId;
         /* if it is a standard error code, we do not want to output the whole URI. */

         if (uri.toString() == "http://www.w3.org/2005/xqt-errors") {
            errorId = errorCode;
         } else {
            errorId = QString::fromLatin1(identifier.toEncoded());
         }

         if (hasLine) {
            writeUncolored(QXmlPatternistCLI::tr("Error %1 in %2, at line %3, column %4 \n   Description: %5")
                  .formatArgs(colorify(errorId, ErrorCode),
                  colorify(location, Location),
                  colorify(QString::number(sourceLocation.line()), Location),
                  colorify(QString::number(sourceLocation.column()), Location),
                  colorifyDescription(description)));

         } else {
            writeUncolored(QXmlPatternistCLI::tr("Error %1 in \"%2\" \n   Description: %3")
                  .formatArgs(colorify(errorId, ErrorCode), colorify(location, Location),
                  colorifyDescription(description)));
         }

         break;
      }

      case QtCriticalMsg:
      case QtDebugMsg: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "CriticalMsg or DebugMsg not supported.");
         return;
      }
   }
}

QString ColoringMessageHandler::colorifyDescription(const QString &in) const
{
   QXmlStreamReader reader(in);

   QString result;
   ColorType currentColor = RunningText;

   while (! reader.atEnd()) {
      reader.readNext();

      switch (reader.tokenType()) {
         case QXmlStreamReader::StartElement: {

            if (reader.name() == "span") {
               Q_ASSERT(m_classToColor.contains(reader.attributes().value("class").toString()));
               currentColor = m_classToColor.value(reader.attributes().value("class").toString());
            }

            continue;
         }

         case QXmlStreamReader::Characters: {
            result.append(colorify(reader.text().toString(), currentColor));
            continue;
         }

         case QXmlStreamReader::EndElement: {
            currentColor = RunningText;
            continue;
         }

         case QXmlStreamReader::StartDocument:
         case QXmlStreamReader::EndDocument:
            continue;

         default:
            Q_ASSERT_X(false, Q_FUNC_INFO, "Unexpected node.");
      }
   }

   Q_ASSERT_X(! reader.hasError(), Q_FUNC_INFO, "The output from Patternist is invalid.");
   return result;
}

