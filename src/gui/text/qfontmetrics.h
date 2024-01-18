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

#ifndef QFONTMETRICS_H
#define QFONTMETRICS_H

#include <qfont.h>
#include <qsharedpointer.h>
#include <qrect.h>

class QTextCodec;
class QRect;

class Q_GUI_EXPORT QFontMetrics
{
 public:
   explicit QFontMetrics(const QFont &font);
   QFontMetrics(const QFont &font, QPaintDevice *paintDevice);
   QFontMetrics(const QFontMetrics &other);

   ~QFontMetrics();

   QFontMetrics &operator=(const QFontMetrics &other);

   QFontMetrics &operator=(QFontMetrics &&other) {
      qSwap(d, other.d);
      return *this;
   }

   void swap(QFontMetrics &other) {
      qSwap(d, other.d);
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

   bool inFont(QChar ch) const;
   bool inFontUcs4(char32_t ch) const;

   int leftBearing(QChar ch) const;
   int rightBearing(QChar ch) const;
   int width(const QString &text, int len = -1) const;
   int width(const QString &text, int len, int flags) const;

   int width(QChar ch) const;

   QRect boundingRect(QChar ch) const;

   QRect boundingRect(const QString &text) const;
   QRect boundingRect(const QRect &rect, int flags, const QString &text, int tabStops = 0,
         int *tabArray = nullptr) const;

   QRect boundingRect(int x, int y, int width, int height, int flags, const QString &text, int tabStops = 0,
         int *tabArray = nullptr) const
   {
      return boundingRect(QRect(x, y, width, height), flags, text, tabStops, tabArray);
   }

   QSize size(int flags, const QString &text, int tabStops = 0, int *tabArray = nullptr) const;

   QRect tightBoundingRect(const QString &text) const;

   QString elidedText(const QString &text, Qt::TextElideMode mode, int width, int flags = 0) const;

   int underlinePos() const;
   int overlinePos() const;
   int strikeOutPos() const;
   int lineWidth() const;

   bool operator==(const QFontMetrics &other) const;

   bool operator !=(const QFontMetrics &other) const {
      return !operator==(other);
   }

 private:
   friend class QFontMetricsF;
   friend class QStackTextEngine;

   QExplicitlySharedDataPointer<QFontPrivate> d;
};

class Q_GUI_EXPORT QFontMetricsF
{
 public:
   QFontMetricsF(const QFont &font);
   QFontMetricsF(const QFont &font, QPaintDevice *paintDevice);
   QFontMetricsF(const QFontMetrics &fontMetrics);
   QFontMetricsF(const QFontMetricsF &other);

   ~QFontMetricsF();

   QFontMetricsF &operator=(const QFontMetrics &fontMetrics);
   QFontMetricsF &operator=(const QFontMetricsF &other);

   QFontMetricsF &operator=(QFontMetricsF &&other) {
      qSwap(d, other.d);
      return *this;
   }

   void swap(QFontMetricsF &other) {
      qSwap(d, other.d);
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

   bool inFont(QChar ch) const;
   bool inFontUcs4(char32_t ch) const;

   qreal leftBearing(QChar ch) const;
   qreal rightBearing(QChar ch) const;
   qreal width(const QString &text) const;

   qreal width(QChar ch) const;

   QRectF boundingRect(const QString &text) const;
   QRectF boundingRect(QChar ch) const;
   QRectF boundingRect(const QRectF &rect, int flags, const QString &text, int tabStops = 0, int *tabArray = nullptr) const;
   QSizeF size(int flags, const QString &text, int tabStops = 0, int *tabArray = nullptr) const;

   QRectF tightBoundingRect(const QString &text) const;

   QString elidedText(const QString &text, Qt::TextElideMode mode, qreal width, int flags = 0) const;

   qreal underlinePos() const;
   qreal overlinePos() const;
   qreal strikeOutPos() const;
   qreal lineWidth() const;

   bool operator==(const QFontMetricsF &other) const;

   bool operator !=(const QFontMetricsF &other) const {
      return !operator==(other);
   }

 private:
   QExplicitlySharedDataPointer<QFontPrivate> d;
};

#endif // QFONTMETRICS_H
