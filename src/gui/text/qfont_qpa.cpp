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

#include <qapplication_p.h>
#include <QtGui/QPlatformFontDatabase>

QT_BEGIN_NAMESPACE

void QFont::initialize()
{
   QApplicationPrivate::platformIntegration()->fontDatabase()->populateFontDatabase();
}

void QFont::cleanup()
{
   QFontCache::cleanup();
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

Qt::HANDLE QFont::handle() const
{
   return 0;
}

QString QFont::rawName() const
{
   return QLatin1String("unknown");
}

void QFont::setRawName(const QString &)
{
}

QString QFont::defaultFamily() const
{
   QString familyName;
   switch (d->request.styleHint) {
      case QFont::Times:
         familyName = QString::fromLatin1("times");
      case QFont::Courier:
      case QFont::Monospace:
         familyName = QString::fromLatin1("monospace");
      case QFont::Decorative:
         familyName = QString::fromLatin1("old english");
      case QFont::Helvetica:
      case QFont::System:
      default:
         familyName = QString::fromLatin1("helvetica");
   }

   QStringList list = QApplicationPrivate::platformIntegration()->fontDatabase()->fallbacksForFamily(familyName,
                      QFont::StyleNormal, QFont::StyleHint(d->request.styleHint), QChar::Script_Common);
   if (list.size()) {
      familyName = list.at(0);
   }
   return familyName;
}

QString QFont::lastResortFamily() const
{
   return QString::fromLatin1("helvetica");
}

QString QFont::lastResortFont() const
{
   qFatal("QFont::lastResortFont: Cannot find any reasonable font");
   // Shut compiler up
   return QString();
}


QT_END_NAMESPACE

