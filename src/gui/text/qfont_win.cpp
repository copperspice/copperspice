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

#include <qfont.h>
#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qtextengine_p.h>
#include <qfontmetrics.h>
#include <qfontinfo.h>
#include <qwidget.h>
#include <qpainter.h>
#include <limits.h>
#include <qt_windows.h>
#include <qapplication_p.h>
#include <qapplication.h>
#include <qunicodetables_p.h>
#include <qfontdatabase.h>

QT_BEGIN_NAMESPACE

extern HDC   shared_dc();                // common dc for all fonts
extern QFont::Weight weightFromInteger(int weight); // qfontdatabase.cpp

// ### maybe move to qapplication_win
QFont qt_LOGFONTtoQFont(LOGFONT &lf, bool /*scale*/)
{
   QString family = QString::fromWCharArray(lf.lfFaceName);
   QFont qf(family);
   qf.setItalic(lf.lfItalic);
   if (lf.lfWeight != FW_DONTCARE) {
      qf.setWeight(weightFromInteger(lf.lfWeight));
   }
   int lfh = qAbs(lf.lfHeight);
   qf.setPointSizeF(lfh * 72.0 / GetDeviceCaps(shared_dc(), LOGPIXELSY));
   qf.setUnderline(false);
   qf.setOverline(false);
   qf.setStrikeOut(false);
   return qf;
}


static inline float pixelSize(const QFontDef &request, int dpi)
{
   float pSize;
   if (request.pointSize != -1) {
      pSize = request.pointSize * dpi / 72.;
   } else {
      pSize = request.pixelSize;
   }
   return pSize;
}

static inline float pointSize(const QFontDef &fd, int dpi)
{
   float pSize;
   if (fd.pointSize < 0) {
      pSize = fd.pixelSize * 72. / ((float)dpi);
   } else {
      pSize = fd.pointSize;
   }
   return pSize;
}

/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::initialize()
{
}

void QFont::cleanup()
{
   QFontCache::cleanup();
}

HFONT QFont::handle() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
   if (engine->type() == QFontEngine::Multi) {
      engine = static_cast<QFontEngineMulti *>(engine)->engine(0);
   }
   if (engine->type() == QFontEngine::Win) {
      return static_cast<QFontEngineWin *>(engine)->hfont;
   }
   return 0;
}

QString QFont::rawName() const
{
   return family();
}

void QFont::setRawName(const QString &name)
{
   setFamily(name);
}

QString QFont::defaultFamily() const
{
   switch (d->request.styleHint) {
      case QFont::Times:
         return QString::fromLatin1("Times New Roman");
      case QFont::Courier:
      case QFont::Monospace:
         return QString::fromLatin1("Courier New");
      case QFont::Decorative:
         return QString::fromLatin1("Bookman Old Style");
      case QFont::Cursive:
         return QString::fromLatin1("Comic Sans MS");
      case QFont::Fantasy:
         return QString::fromLatin1("Impact");
      case QFont::Helvetica:
         return QString::fromLatin1("Arial");
      case QFont::System:
      default:
         return QString::fromLatin1("MS Sans Serif");
   }
}

QString QFont::lastResortFamily() const
{
   return QString::fromLatin1("helvetica");
}

QString QFont::lastResortFont() const
{
   return QString::fromLatin1("arial");
}

QT_END_NAMESPACE
