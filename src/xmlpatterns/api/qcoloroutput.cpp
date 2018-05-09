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

#include <QFile>
#include <QHash>
#include <QTextCodec>

#include "qcoloroutput_p.h"

// TODO: rename insertMapping() to insertColorMapping()
// TODO: Use a smart pointer for managing ColorOutputPrivate *d;
// TODO: break out the C++ example into a snippet file

/* This include must appear here, because if it appears at the beginning of the file for
 * instance, it breaks build -- "qglobal.h:628: error: template with
 * C linkage" -- on Mac OS X 10.4. */

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

QT_BEGIN_NAMESPACE

using namespace QPatternist;

namespace QPatternist {
class ColorOutputPrivate
{
 public:
   ColorOutputPrivate() : currentColorID(-1)

   {
      /* - QIODevice::Unbuffered because we want it to appear when the user actually calls, performance
       *   is considered of lower priority.
       */
      m_out.open(stderr, QIODevice::WriteOnly | QIODevice::Unbuffered);

      coloringEnabled = isColoringPossible();
   }

   ColorOutput::ColorMapping  colorMapping;
   int                        currentColorID;
   bool                       coloringEnabled;

   static const char *const foregrounds[];
   static const char *const backgrounds[];

   inline void write(const QString &msg) {
      m_out.write(msg.toUtf8());
   }

   static QString escapeCode(const QString &in) {
      QString result;
      result.append(QChar(0x1B));
      result.append(QLatin1Char('['));
      result.append(in);
      result.append(QLatin1Char('m'));
      return result;
   }

 private:
   QFile m_out;

   /*!
    Returns true if it's suitable to send colored output to \c stderr.
    */
   inline bool isColoringPossible() const {
#           if defined(Q_OS_WIN32)
      /* Windows doesn't at all support ANSI escape codes, unless
       * the user install a "device driver". See the Wikipedia links in the
       * class documentation for details. */
      return false;
#           else
      /* We use QFile::handle() to get the file descriptor. It's a bit unsure
       * whether it's 2 on all platforms and in all cases, so hopefully this layer
       * of abstraction helps handle such cases. */
      return isatty(m_out.handle());
#           endif
   }
};
}

const char *const ColorOutputPrivate::foregrounds[] = {
   "0;30",
   "0;34",
   "0;32",
   "0;36",
   "0;31",
   "0;35",
   "0;33",
   "0;37",
   "1;30",
   "1;34",
   "1;32",
   "1;36",
   "1;31",
   "1;35",
   "1;33",
   "1;37"
};

const char *const ColorOutputPrivate::backgrounds[] = {
   "0;40",
   "0;44",
   "0;42",
   "0;46",
   "0;41",
   "0;45",
   "0;43"
};

void ColorOutput::setColorMapping(const ColorMapping &cMapping)
{
   d->colorMapping = cMapping;
}

ColorOutput::ColorMapping ColorOutput::colorMapping() const
{
   return d->colorMapping;
}

ColorOutput::ColorOutput() : d(new ColorOutputPrivate())
{
}

ColorOutput::~ColorOutput()
{
   delete d;
}

void ColorOutput::write(const QString &message, int colorID)
{
   d->write(colorify(message, colorID));
}

void ColorOutput::writeUncolored(const QString &message)
{
   d->write(message + '\n');
}

QString ColorOutput::colorify(const QString &message, int colorID) const
{
   Q_ASSERT_X(colorID == -1 || d->colorMapping.contains(colorID), Q_FUNC_INFO,
              qPrintable(QString("There is no color registered by id %1").formatArg(colorID)));

   Q_ASSERT_X(! message.isEmpty(), Q_FUNC_INFO, "Can not print an empty string.");

   if (colorID != -1) {
      d->currentColorID = colorID;
   }

   if (d->coloringEnabled && colorID != -1) {
      const int color(d->colorMapping.value(colorID));

      /* If DefaultColor is set, we don't want to color it. */
      if (color & DefaultColor) {
         return message;
      }

      const int foregroundCode = (int(color) & ForegroundMask) >> ForegroundShift;
      const int backgroundCode = (int(color) & BackgroundMask) >> BackgroundShift;
      QString finalMessage;
      bool closureNeeded = false;

      if (foregroundCode) {
         finalMessage.append(ColorOutputPrivate::escapeCode(QString::fromLatin1(ColorOutputPrivate::foregrounds[foregroundCode - 1])));
         closureNeeded = true;
      }

      if (backgroundCode) {
         finalMessage.append(ColorOutputPrivate::escapeCode(QString::fromLatin1(ColorOutputPrivate::backgrounds[backgroundCode - 1])));
         closureNeeded = true;
      }

      finalMessage.append(message);

      if (closureNeeded) {
         finalMessage.append(QChar(0x1B));
         finalMessage.append("[0m");
      }

      return finalMessage;
   } else {
      return message;
   }
}

/*!
  Adds a color mapping from \a colorID to \a colorCode, for this ColorOutput instance.

  This is a convenience function for creating a ColorOutput::ColorMapping instance and
  calling setColorMapping().

  \sa colorMapping(), setColorMapping()
 */
void ColorOutput::insertMapping(int colorID, const ColorCode colorCode)
{
   d->colorMapping.insert(colorID, colorCode);
}

QT_END_NAMESPACE
