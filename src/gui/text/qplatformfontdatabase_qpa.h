/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPLATFORMFONTDATABASE_QPA_H
#define QPLATFORMFONTDATABASE_QPA_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QFontDatabase>
#include <qfont_p.h>

QT_BEGIN_NAMESPACE

class QWritingSystemsPrivate;
class QFontRequestPrivate;

class Q_GUI_EXPORT QSupportedWritingSystems
{

 public:
   QSupportedWritingSystems();
   QSupportedWritingSystems(const QSupportedWritingSystems &other);
   QSupportedWritingSystems &operator=(const QSupportedWritingSystems &other);
   ~QSupportedWritingSystems();

   void setSupported(QFontDatabase::WritingSystem, bool supported = true);
   bool supported(QFontDatabase::WritingSystem) const;

 private:
   void detach();

   QWritingSystemsPrivate *d;

   friend Q_GUI_EXPORT bool operator==(const QSupportedWritingSystems &, const QSupportedWritingSystems &);
   friend Q_GUI_EXPORT bool operator!=(const QSupportedWritingSystems &, const QSupportedWritingSystems &);
};

Q_GUI_EXPORT bool operator==(const QSupportedWritingSystems &, const QSupportedWritingSystems &);
Q_GUI_EXPORT bool operator!=(const QSupportedWritingSystems &, const QSupportedWritingSystems &);

class Q_GUI_EXPORT QPlatformFontDatabase
{

 public:
   virtual void populateFontDatabase();
   virtual QFontEngine *fontEngine(const QFontDef &fontDef, QChar::Script script, void *handle);
   virtual QStringList fallbacksForFamily(const QString family, const QFont::Style &style,
                  const QFont::StyleHint &styleHint, const QChar::Script &script) const;

   virtual QStringList addApplicationFont(const QByteArray &fontData, const QString &fileName);
   virtual void releaseHandle(void *handle);

   virtual QFontEngine *fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference);

   virtual QString fontDir() const;

   //callback
   static void registerQPF2Font(const QByteArray &dataArray, void *handle);
   static void registerFont(const QString &familyname, const QString &foundryname, QFont::Weight weight,
                            QFont::Style style, QFont::Stretch stretch, bool antialiased, bool scalable, int pixelSize,
                            const QSupportedWritingSystems &writingSystems, void *handle);
};

QT_END_NAMESPACE

#endif // QPLATFORMFONTDATABASE_QPA_H
