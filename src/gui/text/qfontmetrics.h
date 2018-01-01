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

#ifndef QFONTMETRICS_H
#define QFONTMETRICS_H

#include <QtGui/qfont.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

#ifdef Q_WS_QWS
class QFontEngine;
#endif

class QTextCodec;
class QRect;

class Q_GUI_EXPORT QFontMetrics
{

 public:
   QFontMetrics(const QFont &);
   QFontMetrics(const QFont &, QPaintDevice *pd);
   QFontMetrics(const QFontMetrics &);
   ~QFontMetrics();

   QFontMetrics &operator=(const QFontMetrics &);

   inline QFontMetrics &operator=(QFontMetrics && other) {
      qSwap(d, other.d);
      return *this;
   }

   int ascent() const;
   int descent() const;
   int height() const;
   int leading() const;
   int lineSpacing() const;
   int minLeftBearing() const;
   int minRightBearing() const;
   int maxWidth() const;

   int xHeight() const;
   int averageCharWidth() const;

   bool inFont(QChar) const;
   bool inFontUcs4(uint ucs4) const;

   int leftBearing(QChar) const;
   int rightBearing(QChar) const;
   int width(const QString &, int len = -1) const;
   int width(const QString &, int len, int flags) const;

   int width(QChar) const;
   int charWidth(const QString &str, int pos) const;

   QRect boundingRect(QChar) const;

   QRect boundingRect(const QString &text) const;
   QRect boundingRect(const QRect &r, int flags, const QString &text, int tabstops = 0, int *tabarray = 0) const;

   inline QRect boundingRect(int x, int y, int w, int h, int flags, const QString &text, int tabstops = 0, 
            int *tabarray = 0) const {
      return boundingRect(QRect(x, y, w, h), flags, text, tabstops, tabarray);
   }

   QSize size(int flags, const QString &str, int tabstops = 0, int *tabarray = 0) const;

   QRect tightBoundingRect(const QString &text) const;

   QString elidedText(const QString &text, Qt::TextElideMode mode, int width, int flags = 0) const;

   int underlinePos() const;
   int overlinePos() const;
   int strikeOutPos() const;
   int lineWidth() const;

   bool operator==(const QFontMetrics &other); // 5.0 - remove me
   bool operator==(const QFontMetrics &other) const;

   inline bool operator !=(const QFontMetrics &other) {
      return !operator==(other);   // 5.0 - remove me
   }

   inline bool operator !=(const QFontMetrics &other) const {
      return !operator==(other);
   }

 private:

#if defined(Q_OS_MAC)
   friend class QFontPrivate;
#endif

   friend class QFontMetricsF;
   friend class QStackTextEngine;

   QExplicitlySharedDataPointer<QFontPrivate> d;
};


class Q_GUI_EXPORT QFontMetricsF
{
 public:
   QFontMetricsF(const QFont &);
   QFontMetricsF(const QFont &, QPaintDevice *pd);
   QFontMetricsF(const QFontMetrics &);
   QFontMetricsF(const QFontMetricsF &);
   ~QFontMetricsF();

   QFontMetricsF &operator=(const QFontMetricsF &);
   QFontMetricsF &operator=(const QFontMetrics &);

   inline QFontMetricsF &operator=(QFontMetricsF && other) {
      qSwap(d, other.d);
      return *this;
   }

   qreal ascent() const;
   qreal descent() const;
   qreal height() const;
   qreal leading() const;
   qreal lineSpacing() const;
   qreal minLeftBearing() const;
   qreal minRightBearing() const;
   qreal maxWidth() const;

   qreal xHeight() const;
   qreal averageCharWidth() const;

   bool inFont(QChar) const;
   bool inFontUcs4(uint ucs4) const;

   qreal leftBearing(QChar) const;
   qreal rightBearing(QChar) const;
   qreal width(const QString &string) const;

   qreal width(QChar) const;

   QRectF boundingRect(const QString &string) const;
   QRectF boundingRect(QChar) const;
   QRectF boundingRect(const QRectF &r, int flags, const QString &string, int tabstops = 0, int *tabarray = 0) const;
   QSizeF size(int flags, const QString &str, int tabstops = 0, int *tabarray = 0) const;

   QRectF tightBoundingRect(const QString &text) const;

   QString elidedText(const QString &text, Qt::TextElideMode mode, qreal width, int flags = 0) const;

   qreal underlinePos() const;
   qreal overlinePos() const;
   qreal strikeOutPos() const;
   qreal lineWidth() const;

   bool operator==(const QFontMetricsF &other); // 5.0 - remove me
   bool operator==(const QFontMetricsF &other) const;
   inline bool operator !=(const QFontMetricsF &other) {
      return !operator==(other);   // 5.0 - remove me
   }
   inline bool operator !=(const QFontMetricsF &other) const {
      return !operator==(other);
   }

 private:
   QExplicitlySharedDataPointer<QFontPrivate> d;
};

QT_END_NAMESPACE

#endif // QFONTMETRICS_H
