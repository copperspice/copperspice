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

#include <qwidget.h>
#include <qpainter.h>
#include <qfont_p.h>
#include <qunicodetables_p.h>
#include <qfontdatabase.h>
#include <qtextcodec.h>
#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmap.h>

// #include <qmemorymanager_qws.h>

#include <qtextengine_p.h>
#include <qfontengine_p.h>

#if !defined(QT_NO_FREETYPE)
#include <qfontengine_ft_p.h>
#endif

QT_BEGIN_NAMESPACE

void QFont::initialize()
{ }

void QFont::cleanup()
{
   QFontCache::cleanup();
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

Qt::HANDLE QFont::handle() const
{
#ifndef QT_NO_FREETYPE
   return freetypeFace();
#endif
   return 0;
}

FT_Face QFont::freetypeFace() const
{
#ifndef QT_NO_FREETYPE
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   if (engine->type() == QFontEngine::Multi) {
      engine = static_cast<QFontEngineMulti *>(engine)->engine(0);
   }
   if (engine->type() == QFontEngine::Freetype) {
      const QFontEngineFT *ft = static_cast<const QFontEngineFT *>(engine);
      return ft->non_locked_face();
   }
#endif
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
   switch (d->request.styleHint) {
      case QFont::Times:
         return QString::fromLatin1("times");
      case QFont::Courier:
      case QFont::Monospace:
         return QString::fromLatin1("courier");
      case QFont::Decorative:
         return QString::fromLatin1("old english");
      case QFont::Helvetica:
      case QFont::System:
      default:
         return QString::fromLatin1("helvetica");
   }
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
