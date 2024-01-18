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

#include <qlcdnumber.h>

#ifndef QT_NO_LCDNUMBER

#include <qbitarray.h>
#include <qpainter.h>
#include <qframe_p.h>

class QLCDNumberPrivate : public QFramePrivate
{
   Q_DECLARE_PUBLIC(QLCDNumber)

 public:
   void init();
   void internalSetString(const QString &s);
   void drawString(const QString &s, QPainter &, QBitArray * = nullptr, bool = true);
   //void drawString(const QString &, QPainter &, QBitArray * = 0) const;
   void drawDigit(const QPoint &, QPainter &, int, char, char = ' ');
   void drawSegment(const QPoint &, char, QPainter &, int, bool = false);

   int ndigits;
   double val;
   uint base : 2;
   uint smallPoint : 1;
   uint fill : 1;
   uint shadow : 1;
   QString digitStr;
   QBitArray points;
};

static QString int2string(int num, int base, int ndigits, bool *oflow)
{
   QString s;
   bool negative;

   if (num < 0) {
      negative = true;
      num      = -num;
   } else {
      negative = false;
   }

   switch (base) {
      case QLCDNumber::Hex:
         s = QString("%1").formatArg(num, ndigits, 16);
         break;

      case QLCDNumber::Dec:
         s = QString("%1").formatArg(num, ndigits, 10);
         break;

      case QLCDNumber::Oct:
         s = QString("%1").formatArg(num, ndigits, 8);
         break;

      case QLCDNumber::Bin: {
         char buf[42];
         char *p = &buf[41];
         uint n = num;
         int len = 0;
         *p = '\0';

         do {
            *--p = (char)((n & 1) + '0');
            n >>= 1;
            len++;
         } while (n != 0);

         len = ndigits - len;
         if (len > 0) {
            s.fill(QLatin1Char(' '), len);
         }

         s += QString::fromLatin1(p);
      }
      break;
   }

   if (negative) {
      for (QString::size_type i = 0; i < s.length(); i++) {

         if (s[i] != ' ') {

            if (i != 0) {
               s.replace(i - 1, 1, '-');

            } else {
               s.insert(0, '-');
            }

            break;
         }
      }
   }

   if (oflow) {
      *oflow = (int)s.length() > ndigits;
   }

   return s;
}

static QString double2string(double num, int base, int ndigits, bool *oflow)
{
   QString s;

   if (base != QLCDNumber::Dec) {
      bool of = num >= 2147483648.0 || num < -2147483648.0;

      if (of) {                             // oops, integer overflow
         if (oflow) {
            *oflow = true;
         }
         return s;
      }
      s = int2string((int)num, base, ndigits, nullptr);

   } else {                                    // decimal base
      int nd = ndigits;

      do {
         s = QString("%1").formatArg(num, ndigits, 'g', nd);
         int i = s.indexOf('e');

         if (i > 0 && s[i + 1] == '+') {
            s.replace(i, 2, " e");

         }

      } while (nd-- && (int)s.length() > ndigits);
   }

   if (oflow) {
      *oflow = (int)s.length() > ndigits;
   }

   return s;
}

static const char *getSegments(char ch)               // gets list of segments for ch
{
   static const char segments[30][8] = {
      { 0, 1, 2, 4, 5, 6, 99, 0},            // 0    0 / O
      { 2, 5, 99, 0, 0, 0, 0, 0},            // 1    1
      { 0, 2, 3, 4, 6, 99, 0, 0},            // 2    2
      { 0, 2, 3, 5, 6, 99, 0, 0},            // 3    3
      { 1, 2, 3, 5, 99, 0, 0, 0},            // 4    4
      { 0, 1, 3, 5, 6, 99, 0, 0},            // 5    5 / S
      { 0, 1, 3, 4, 5, 6, 99, 0},            // 6    6
      { 0, 2, 5, 99, 0, 0, 0, 0},            // 7    7
      { 0, 1, 2, 3, 4, 5, 6, 99},            // 8    8
      { 0, 1, 2, 3, 5, 6, 99, 0},            // 9    9 / g
      { 3, 99, 0, 0, 0, 0, 0, 0},            // 10   -
      { 7, 99, 0, 0, 0, 0, 0, 0},            // 11   .
      { 0, 1, 2, 3, 4, 5, 99, 0},            // 12   A
      { 1, 3, 4, 5, 6, 99, 0, 0},            // 13   B
      { 0, 1, 4, 6, 99, 0, 0, 0},            // 14   C
      { 2, 3, 4, 5, 6, 99, 0, 0},            // 15   D
      { 0, 1, 3, 4, 6, 99, 0, 0},            // 16   E
      { 0, 1, 3, 4, 99, 0, 0, 0},            // 17   F
      { 1, 3, 4, 5, 99, 0, 0, 0},            // 18   h
      { 1, 2, 3, 4, 5, 99, 0, 0},            // 19   H
      { 1, 4, 6, 99, 0, 0, 0, 0},            // 20   L
      { 3, 4, 5, 6, 99, 0, 0, 0},            // 21   o
      { 0, 1, 2, 3, 4, 99, 0, 0},            // 22   P
      { 3, 4, 99, 0, 0, 0, 0, 0},            // 23   r
      { 4, 5, 6, 99, 0, 0, 0, 0},            // 24   u
      { 1, 2, 4, 5, 6, 99, 0, 0},            // 25   U
      { 1, 2, 3, 5, 6, 99, 0, 0},            // 26   Y
      { 8, 9, 99, 0, 0, 0, 0, 0},            // 27   :
      { 0, 1, 2, 3, 99, 0, 0, 0},            // 28   '
      {99, 0, 0, 0, 0, 0, 0, 0}
   };           // 29   empty

   if (ch >= '0' && ch <= '9') {
      return segments[ch - '0'];
   }
   if (ch >= 'A' && ch <= 'F') {
      return segments[ch - 'A' + 12];
   }
   if (ch >= 'a' && ch <= 'f') {
      return segments[ch - 'a' + 12];
   }

   int n;
   switch (ch) {
      case '-':
         n = 10;
         break;

      case 'O':
         n = 0;
         break;

      case 'g':
         n = 9;
         break;

      case '.':
         n = 11;
         break;

      case 'h':
         n = 18;
         break;

      case 'H':
         n = 19;
         break;
      case 'l':
      case 'L':
         n = 20;
         break;

      case 'o':
         n = 21;
         break;

      case 'p':
      case 'P':
         n = 22;
         break;

      case 'r':
      case 'R':
         n = 23;
         break;

      case 's':
      case 'S':
         n = 5;
         break;

      case 'u':
         n = 24;
         break;

      case 'U':
         n = 25;
         break;

      case 'y':
      case 'Y':
         n = 26;
         break;

      case ':':
         n = 27;
         break;

      case '\'':
         n = 28;
         break;

      default:
         n = 29;
         break;
   }

   return segments[n];
}

QLCDNumber::QLCDNumber(QWidget *parent)
   : QFrame(*new QLCDNumberPrivate, parent)
{
   Q_D(QLCDNumber);
   d->ndigits = 5;
   d->init();
}

QLCDNumber::QLCDNumber(uint numDigits, QWidget *parent)
   : QFrame(*new QLCDNumberPrivate, parent)
{
   Q_D(QLCDNumber);
   d->ndigits = numDigits;
   d->init();
}

void QLCDNumberPrivate::init()
{
   Q_Q(QLCDNumber);

   q->setFrameStyle(QFrame::Box | QFrame::Raised);
   val        = 0;
   base       = QLCDNumber::Dec;
   smallPoint = false;
   q->setDigitCount(ndigits);
   q->setSegmentStyle(QLCDNumber::Filled);
   q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
}

QLCDNumber::~QLCDNumber()
{
}

void QLCDNumber::setDigitCount(int numDigits)
{
   Q_D(QLCDNumber);

   if (numDigits > 99) {
      qWarning("QLCDNumber::setNumDigits() Max 99 digits allowed (%s)", csPrintable(objectName()));
      numDigits = 99;
   }

   if (numDigits < 0) {
      qWarning("QLCDNumber::setNumDigits() Min 0 digits allowed (%s)", csPrintable(objectName()));
      numDigits = 0;
   }

   if (d->digitStr.isEmpty()) {                 // from constructor
      d->ndigits = numDigits;
      d->digitStr.fill(QLatin1Char(' '), d->ndigits);
      d->points.fill(0, d->ndigits);

      // "0" is the default number
      d->digitStr.replace(d->ndigits - 1, 1, '0');


   } else {
      bool doDisplay = d->ndigits == 0;
      if (numDigits == d->ndigits) {           // no change
         return;
      }

      int i;
      int dif;

      if (numDigits > d->ndigits) {            // expand
         dif = numDigits - d->ndigits;
         QString buf;
         buf.fill(QLatin1Char(' '), dif);
         d->digitStr.insert(0, buf);
         d->points.resize(numDigits);

         for (i = numDigits - 1; i >= dif; i--) {
            d->points.setBit(i, d->points.testBit(i - dif));
         }
         for (i = 0; i < dif; i++) {
            d->points.clearBit(i);
         }

      } else {                                        // shrink
         dif = d->ndigits - numDigits;
         d->digitStr = d->digitStr.right(numDigits);
         QBitArray tmpPoints = d->points;
         d->points.resize(numDigits);
         for (i = 0; i < (int)numDigits; i++) {
            d->points.setBit(i, tmpPoints.testBit(i + dif));
         }
      }

      d->ndigits = numDigits;
      if (doDisplay) {
         display(value());
      }
      update();
   }
}

int QLCDNumber::digitCount() const
{
   Q_D(const QLCDNumber);
   return d->ndigits;
}

bool QLCDNumber::checkOverflow(int num) const
{
   Q_D(const QLCDNumber);
   bool of;
   int2string(num, d->base, d->ndigits, &of);
   return of;
}

bool QLCDNumber::checkOverflow(double num) const
{
   Q_D(const QLCDNumber);
   bool of;
   double2string(num, d->base, d->ndigits, &of);

   return of;
}

QLCDNumber::Mode QLCDNumber::mode() const
{
   Q_D(const QLCDNumber);
   return (QLCDNumber::Mode) d->base;
}

void QLCDNumber::setMode(Mode m)
{
   Q_D(QLCDNumber);
   d->base = m;
   display(d->val);
}

double QLCDNumber::value() const
{
   Q_D(const QLCDNumber);
   return d->val;
}

void QLCDNumber::display(double num)
{
   Q_D(QLCDNumber);

   d->val = num;

   bool of;
   QString s = double2string(d->val, d->base, d->ndigits, &of);

   if (of) {
      emit overflow();
   } else {
      d->internalSetString(s);
   }
}

int QLCDNumber::intValue() const
{
   Q_D(const QLCDNumber);
   return qRound(d->val);
}

void QLCDNumber::display(int num)
{
   Q_D(QLCDNumber);

   d->val = (double)num;

   bool of;
   QString s = int2string(num, d->base, d->ndigits, &of);

   if (of) {
      emit overflow();
   } else {
      d->internalSetString(s);
   }
}

void QLCDNumber::display(const QString &s)
{
   Q_D(QLCDNumber);

   d->val = 0;

   bool ok = false;
   double v = s.toDouble(&ok);

   if (ok) {
      d->val = v;
   }

   d->internalSetString(s);
}

void QLCDNumber::setHexMode()
{
   setMode(Hex);
}

void QLCDNumber::setDecMode()
{
   setMode(Dec);
}

void QLCDNumber::setOctMode()
{
   setMode(Oct);
}

void QLCDNumber::setBinMode()
{
   setMode(Bin);
}

void QLCDNumber::setSmallDecimalPoint(bool b)
{
   Q_D(QLCDNumber);
   d->smallPoint = b;
   update();
}

bool QLCDNumber::smallDecimalPoint() const
{
   Q_D(const QLCDNumber);
   return d->smallPoint;
}

void QLCDNumber::paintEvent(QPaintEvent *)
{
   Q_D(QLCDNumber);

   QPainter p(this);
   drawFrame(&p);

   p.setRenderHint(QPainter::Antialiasing);

   if (d->shadow) {
      p.translate(0.5, 0.5);
   }

   if (d->smallPoint) {
      d->drawString(d->digitStr, p, &d->points, false);
   } else {
      d->drawString(d->digitStr, p, nullptr, false);
   }
}

void QLCDNumberPrivate::internalSetString(const QString &s)
{
   Q_Q(QLCDNumber);

   QString buffer;
   int i;
   int len = s.length();

   QBitArray newPoints(ndigits);

   if (!smallPoint) {
      if (len == ndigits) {
         buffer = s;
      } else {
         buffer = s.right(ndigits).rightJustified(ndigits, QLatin1Char(' '));
      }

   } else {
      int  index = -1;
      bool lastWasPoint = true;
      newPoints.clearBit(0);

      for (i = 0; i < len; i++) {
         if (s[i] == QLatin1Char('.')) {

            if (lastWasPoint) {
               // point already set for digit?

               if (index == ndigits - 1) {
                  // no more digits
                  break;
               }

               index++;
               buffer.replace(index, 1, ' ');    // 2 points in a row, add space
            }

            newPoints.setBit(index);             // set decimal point
            lastWasPoint = true;

         } else {
            if (index == ndigits - 1) {
               break;
            }

            index++;
            buffer.replace(index, 1, s[i]);

            newPoints.clearBit(index);           // decimal point default off
            lastWasPoint = false;
         }
      }

      if (index < ((int) ndigits) - 1) {
         for (i = index; i >= 0; i--) {

            buffer.replace(ndigits - 1 - index + i, 1, buffer[i]);
            newPoints.setBit(ndigits - 1 - index + i, newPoints.testBit(i));
         }

         for (i = 0; i < ndigits - index - 1; i++) {
            buffer.replace(i, 1, ' ');
            newPoints.clearBit(i);
         }
      }
   }

   if (buffer == digitStr) {
      return;
   }

   digitStr = buffer;
   if (smallPoint) {
      points = newPoints;
   }
   q->update();
}

void QLCDNumberPrivate::drawString(const QString &s, QPainter &p, QBitArray *newPoints, bool newString)
{
   Q_Q(QLCDNumber);
   QPoint  pos;

   int digitSpace = smallPoint ? 2 : 1;
   int xSegLen    = q->width() * 5 / (ndigits * (5 + digitSpace) + digitSpace);
   int ySegLen    = q->height() * 5 / 12;
   int segLen     = ySegLen > xSegLen ? xSegLen : ySegLen;
   int xAdvance   = segLen * (5 + digitSpace) / 5;
   int xOffset    = (q->width() - ndigits * xAdvance + segLen / 5) / 2;
   int yOffset    = (q->height() - segLen * 2) / 2;

   for (int i = 0;  i < ndigits; i++) {
      pos = QPoint(xOffset + xAdvance * i, yOffset);

      if (newString) {
         drawDigit(pos, p, segLen, s[i].toLatin1(), digitStr[i].toLatin1());
      } else {
         drawDigit(pos, p, segLen, s[i].toLatin1());
      }

      if (newPoints) {
         char newPoint = newPoints->testBit(i) ? '.' : ' ';
         if (newString) {
            char oldPoint = points.testBit(i) ? '.' : ' ';
            drawDigit(pos, p, segLen, newPoint, oldPoint);
         } else {
            drawDigit(pos, p, segLen, newPoint);
         }
      }
   }

   if (newString) {
      digitStr = s;
      digitStr.truncate(ndigits);
      if (newPoints) {
         points = *newPoints;
      }
   }
}

// internal
void QLCDNumberPrivate::drawDigit(const QPoint &pos, QPainter &p, int segLen,
   char newCh, char oldCh)
{
   // Draws and/or erases segments to change display of a single digit
   // from oldCh to newCh

   char updates[18][2];        // can hold 2 times number of segments, only
   // first 9 used if segment table is correct
   int  nErases;
   int  nUpdates;
   const char *segs;
   int  i, j;

   const char erase      = 0;
   const char draw       = 1;
   const char leaveAlone = 2;

   segs = getSegments(oldCh);
   for (nErases = 0; segs[nErases] != 99; nErases++) {
      updates[nErases][0] = erase;            // get segments to erase to
      updates[nErases][1] = segs[nErases];    // remove old char
   }

   nUpdates = nErases;
   segs = getSegments(newCh);

   for (i = 0 ; segs[i] != 99 ; i++) {
      for (j = 0;  j < nErases; j++)
         if (segs[i] == updates[j][1]) {   // same segment ?
            updates[j][0] = leaveAlone;     // yes, already on screen
            break;
         }
      if (j == nErases) {                   // if not already on screen
         updates[nUpdates][0] = draw;
         updates[nUpdates][1] = segs[i];
         nUpdates++;
      }
   }

   for (i = 0; i < nUpdates; i++) {
      if (updates[i][0] == draw) {
         drawSegment(pos, updates[i][1], p, segLen);
      }
      if (updates[i][0] == erase) {
         drawSegment(pos, updates[i][1], p, segLen, true);
      }
   }
}

static void addPoint(QPolygon &a, const QPoint &p)
{
   uint n = a.size();
   a.resize(n + 1);
   a.setPoint(n, p);
}

// internal
void QLCDNumberPrivate::drawSegment(const QPoint &pos, char segmentNo, QPainter &p,
   int segLen, bool erase)
{
   Q_Q(QLCDNumber);
   QPoint ppt;
   QPoint pt = pos;
   int width = segLen / 5;

   const QPalette &pal = q->palette();
   QColor lightColor, darkColor, fgColor;
   if (erase) {
      lightColor = pal.color(q->backgroundRole());
      darkColor  = lightColor;
      fgColor    = lightColor;
   } else {
      lightColor = pal.light().color();
      darkColor  = pal.dark().color();
      fgColor    = pal.color(q->foregroundRole());
   }


#define LINETO(X,Y) addPoint(a, QPoint(pt.x() + (X),pt.y() + (Y)))
#define LIGHT
#define DARK

   if (fill) {
      QPolygon a(0);
      //The following is an exact copy of the switch below.
      //don't make any changes here
      switch (segmentNo) {
         case 0 :
            ppt = pt;
            LIGHT;
            LINETO(segLen - 1, 0);
            DARK;
            LINETO(segLen - width - 1, width);
            LINETO(width, width);
            LINETO(0, 0);
            break;

         case 1 :
            pt += QPoint(0, 1);
            ppt = pt;
            LIGHT;
            LINETO(width, width);
            DARK;
            LINETO(width, segLen - width / 2 - 2);
            LINETO(0, segLen - 2);
            LIGHT;
            LINETO(0, 0);
            break;

         case 2 :
            pt += QPoint(segLen - 1, 1);
            ppt = pt;
            DARK;
            LINETO(0, segLen - 2);
            LINETO(-width, segLen - width / 2 - 2);
            LIGHT;
            LINETO(-width, width);
            LINETO(0, 0);
            break;

         case 3 :
            pt += QPoint(0, segLen);
            ppt = pt;
            LIGHT;
            LINETO(width, -width / 2);
            LINETO(segLen - width - 1, -width / 2);
            LINETO(segLen - 1, 0);
            DARK;
            if (width & 1) {            // adjust for integer division error
               LINETO(segLen - width - 3, width / 2 + 1);
               LINETO(width + 2, width / 2 + 1);
            } else {
               LINETO(segLen - width - 1, width / 2);
               LINETO(width, width / 2);
            }
            LINETO(0, 0);
            break;

         case 4 :
            pt += QPoint(0, segLen + 1);
            ppt = pt;
            LIGHT;
            LINETO(width, width / 2);
            DARK;
            LINETO(width, segLen - width - 2);
            LINETO(0, segLen - 2);
            LIGHT;
            LINETO(0, 0);
            break;

         case 5 :
            pt += QPoint(segLen - 1, segLen + 1);
            ppt = pt;
            DARK;
            LINETO(0, segLen - 2);
            LINETO(-width, segLen - width - 2);
            LIGHT;
            LINETO(-width, width / 2);
            LINETO(0, 0);
            break;
         case 6 :
            pt += QPoint(0, segLen * 2);
            ppt = pt;
            LIGHT;
            LINETO(width, -width);
            LINETO(segLen - width - 1, -width);
            LINETO(segLen - 1, 0);
            DARK;
            LINETO(0, 0);
            break;

         case 7 :
            if (smallPoint) {
               // if smallpoint place'.' between other digits
               pt += QPoint(segLen + width / 2, segLen * 2);
            } else {
               pt += QPoint(segLen / 2, segLen * 2);
            }

            ppt = pt;
            DARK;
            LINETO(width, 0);
            LINETO(width, -width);
            LIGHT;
            LINETO(0, -width);
            LINETO(0, 0);
            break;

         case 8 :
            pt += QPoint(segLen / 2 - width / 2 + 1, segLen / 2 + width);
            ppt = pt;
            DARK;
            LINETO(width, 0);
            LINETO(width, -width);
            LIGHT;
            LINETO(0, -width);
            LINETO(0, 0);
            break;

         case 9 :
            pt += QPoint(segLen / 2 - width / 2 + 1, 3 * segLen / 2 + width);
            ppt = pt;
            DARK;
            LINETO(width, 0);
            LINETO(width, -width);
            LIGHT;
            LINETO(0, -width);
            LINETO(0, 0);
            break;

         default :
            qWarning("QLCDNumber::drawSegment() Illegal segment id %d (%s)", segmentNo, csPrintable(q->objectName()));
      }

      // End exact copy
      p.setPen(Qt::NoPen);
      p.setBrush(fgColor);
      p.drawPolygon(a);
      p.setBrush(Qt::NoBrush);

      pt = pos;
   }
#undef LINETO
#undef LIGHT
#undef DARK

#define LINETO(X,Y) p.drawLine(ppt.x(), ppt.y(), pt.x()+(X), pt.y()+(Y)); \
                    ppt = QPoint(pt.x()+(X), pt.y()+(Y))

#define LIGHT p.setPen(lightColor)

#define DARK  p.setPen(darkColor)
   if (shadow)
      switch (segmentNo) {
         case 0 :
            ppt = pt;
            LIGHT;
            LINETO(segLen - 1, 0);
            DARK;
            LINETO(segLen - width - 1, width);
            LINETO(width, width);
            LINETO(0, 0);
            break;

         case 1 :
            pt += QPoint(0, 1);
            ppt = pt;
            LIGHT;
            LINETO(width, width);
            DARK;
            LINETO(width, segLen - width / 2 - 2);
            LINETO(0, segLen - 2);
            LIGHT;
            LINETO(0, 0);
            break;

         case 2 :
            pt += QPoint(segLen - 1, 1);
            ppt = pt;
            DARK;
            LINETO(0, segLen - 2);
            LINETO(-width, segLen - width / 2 - 2);
            LIGHT;
            LINETO(-width, width);
            LINETO(0, 0);
            break;

         case 3 :
            pt += QPoint(0, segLen);
            ppt = pt;
            LIGHT;
            LINETO(width, -width / 2);
            LINETO(segLen - width - 1, -width / 2);
            LINETO(segLen - 1, 0);
            DARK;
            if (width & 1) {            // adjust for integer division error
               LINETO(segLen - width - 3, width / 2 + 1);
               LINETO(width + 2, width / 2 + 1);
            } else {
               LINETO(segLen - width - 1, width / 2);
               LINETO(width, width / 2);
            }
            LINETO(0, 0);
            break;

         case 4 :
            pt += QPoint(0, segLen + 1);
            ppt = pt;
            LIGHT;
            LINETO(width, width / 2);
            DARK;
            LINETO(width, segLen - width - 2);
            LINETO(0, segLen - 2);
            LIGHT;
            LINETO(0, 0);
            break;

         case 5 :
            pt += QPoint(segLen - 1, segLen + 1);
            ppt = pt;
            DARK;
            LINETO(0, segLen - 2);
            LINETO(-width, segLen - width - 2);
            LIGHT;
            LINETO(-width, width / 2);
            LINETO(0, 0);
            break;

         case 6 :
            pt += QPoint(0, segLen * 2);
            ppt = pt;
            LIGHT;
            LINETO(width, -width);
            LINETO(segLen - width - 1, -width);
            LINETO(segLen - 1, 0);
            DARK;
            LINETO(0, 0);
            break;

         case 7 :
            if (smallPoint) { // if smallpoint place'.' between other digits
               pt += QPoint(segLen + width / 2, segLen * 2);
            } else {
               pt += QPoint(segLen / 2, segLen * 2);
            }
            ppt = pt;
            DARK;
            LINETO(width, 0);
            LINETO(width, -width);
            LIGHT;
            LINETO(0, -width);
            LINETO(0, 0);
            break;

         case 8 :
            pt += QPoint(segLen / 2 - width / 2 + 1, segLen / 2 + width);
            ppt = pt;
            DARK;
            LINETO(width, 0);
            LINETO(width, -width);
            LIGHT;
            LINETO(0, -width);
            LINETO(0, 0);
            break;

         case 9 :
            pt += QPoint(segLen / 2 - width / 2 + 1, 3 * segLen / 2 + width);
            ppt = pt;
            DARK;
            LINETO(width, 0);
            LINETO(width, -width);
            LIGHT;
            LINETO(0, -width);
            LINETO(0, 0);
            break;

         default:
            qWarning("QLCDNumber::drawSegment() Illegal segment id %d (%s)", segmentNo, csPrintable(q->objectName()));
      }

#undef LINETO
#undef LIGHT
#undef DARK
}

void QLCDNumber::setSegmentStyle(SegmentStyle s)
{
   Q_D(QLCDNumber);
   d->fill = (s == Flat || s == Filled);
   d->shadow = (s == Outline || s == Filled);
   update();
}

QLCDNumber::SegmentStyle QLCDNumber::segmentStyle() const
{
   Q_D(const QLCDNumber);
   Q_ASSERT(d->fill || d->shadow);

   if (!d->fill && d->shadow) {
      return Outline;
   }

   if (d->fill && d->shadow) {
      return Filled;
   }

   return Flat;
}

QSize QLCDNumber::sizeHint() const
{
   return QSize(10 + 9 * (digitCount() + (smallDecimalPoint() ? 0 : 1)), 23);
}

bool QLCDNumber::event(QEvent *e)
{
   return QFrame::event(e);
}

#endif // QT_NO_LCDNUMBER
