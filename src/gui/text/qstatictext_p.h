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

#ifndef QSTATICTEXT_P_H
#define QSTATICTEXT_P_H

#include <qstatictext.h>
#include <qtextureglyphcache_p.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QStaticText;

class QStaticTextUserData
{
 public:
   enum Type {
      NoUserData,
      OpenGLUserData
   };

   QStaticTextUserData(Type t) : type(t) {
      ref = 0;
   }
   virtual ~QStaticTextUserData() {}

   QAtomicInt ref;
   Type type;
};

class Q_GUI_EXPORT QStaticTextItem
{
 public:
   QStaticTextItem() : chars(0), numChars(0), useBackendOptimizations(false),
      userDataNeedsUpdate(0), m_fontEngine(0), m_userData(0) {}

   QStaticTextItem(const QStaticTextItem &other) {
      operator=(other);
   }

   void operator=(const QStaticTextItem &other) {
      glyphPositions = other.glyphPositions;
      glyphs = other.glyphs;
      chars = other.chars;
      numGlyphs = other.numGlyphs;
      numChars = other.numChars;
      font = other.font;
      color = other.color;
      useBackendOptimizations = other.useBackendOptimizations;
      userDataNeedsUpdate = other.userDataNeedsUpdate;

      m_fontEngine = 0;
      m_userData = 0;
      setUserData(other.userData());
      setFontEngine(other.fontEngine());
   }

   ~QStaticTextItem();

   void setUserData(QStaticTextUserData *newUserData) {
      if (m_userData == newUserData) {
         return;
      }

      if (m_userData != 0 && !m_userData->ref.deref()) {
         delete m_userData;
      }

      m_userData = newUserData;
      if (m_userData != 0) {
         m_userData->ref.ref();
      }
   }
   QStaticTextUserData *userData() const {
      return m_userData;
   }

   void setFontEngine(QFontEngine *fe);
   QFontEngine *fontEngine() const {
      return m_fontEngine;
   }

   union {
      QFixedPoint *glyphPositions;             // 8 bytes per glyph
      int positionOffset;
   };
   union {
      glyph_t *glyphs;                         // 4 bytes per glyph
      int glyphOffset;
   };
   union {
      QChar *chars;                            // 2 bytes per glyph
      int charOffset;
   };
   // =================
   // 14 bytes per glyph

   // 12 bytes for pointers
   int numGlyphs;                               // 4 bytes per item
   int numChars;                                // 4 bytes per item
   QFont font;                                  // 8 bytes per item
   QColor color;                                // 10 bytes per item
   char useBackendOptimizations : 1;            // 1 byte per item
   char userDataNeedsUpdate : 1;                //
   // ================
   // 51 bytes per item

 private: // Needs special handling in setters, so private to avoid abuse
   QFontEngine *m_fontEngine;                     // 4 bytes per item
   QStaticTextUserData *m_userData;               // 8 bytes per item

};

class QStaticTextPrivate
{
 public:
   QStaticTextPrivate();
   QStaticTextPrivate(const QStaticTextPrivate &other);
   ~QStaticTextPrivate();

   void init();
   void paintText(const QPointF &pos, QPainter *p);

   void invalidate() {
      needsRelayout = true;
   }

   QAtomicInt ref;                      // 4 bytes per text

   QString text;                        // 4 bytes per text
   QFont font;                          // 8 bytes per text
   qreal textWidth;                     // 8 bytes per text
   QSizeF actualSize;                   // 16 bytes per text
   QPointF position;                    // 16 bytes per text

   QTransform matrix;                   // 80 bytes per text
   QStaticTextItem *items;              // 4 bytes per text
   int itemCount;                       // 4 bytes per text

   glyph_t *glyphPool;                  // 4 bytes per text
   QFixedPoint *positionPool;           // 4 bytes per text
   QChar *charPool;                     // 4 bytes per text

   QTextOption textOption;              // 28 bytes per text

   unsigned char needsRelayout            : 1; // 1 byte per text
   unsigned char useBackendOptimizations  : 1;
   unsigned char textFormat               : 2;
   unsigned char untransformedCoordinates : 1;
   // ================
   // 195 bytes per text

   static QStaticTextPrivate *get(const QStaticText *q);
};

QT_END_NAMESPACE

#endif // QSTATICTEXT_P_H
