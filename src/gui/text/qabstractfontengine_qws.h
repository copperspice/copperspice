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

#ifndef QABSTRACTFONTENGINE_QWS_H
#define QABSTRACTFONTENGINE_QWS_H

#include <QtCore/qobject.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>
#include <QtCore/qfactoryinterface.h>
#include <QtGui/qpaintengine.h>
#include <QtGui/qfontdatabase.h>

QT_BEGIN_NAMESPACE

class QFontEngineInfoPrivate;
class QFontEnginePluginPrivate;

class Q_GUI_EXPORT QFontEngineInfo
{

 public:
   QFontEngineInfo();
   explicit QFontEngineInfo(const QString &family);
   QFontEngineInfo(const QFontEngineInfo &other);
   QFontEngineInfo &operator=(const QFontEngineInfo &other);
   ~QFontEngineInfo();

   void setFamily(const QString &name);
   QString family() const;

   void setPixelSize(qreal size);
   qreal pixelSize() const;

   void setWeight(int weight);
   int weight() const;

   void setStyle(QFont::Style style);
   QFont::Style style() const;

   QList<QFontDatabase::WritingSystem> writingSystems() const;
   void setWritingSystems(const QList<QFontDatabase::WritingSystem> &writingSystems);

 private:
   QFontEngineInfoPrivate *d;
};

class QAbstractFontEngine;

struct Q_GUI_EXPORT QFontEngineFactoryInterface : public QFactoryInterface {
   virtual QAbstractFontEngine *create(const QFontEngineInfo &info) = 0;
   virtual QList<QFontEngineInfo> availableFontEngines() const = 0;
};

#define QFontEngineFactoryInterface_iid "com.copperspice.QFontEngineFactoryInterface"
CS_DECLARE_INTERFACE(QFontEngineFactoryInterface, QFontEngineFactoryInterface_iid)

class Q_GUI_EXPORT QFontEnginePlugin : public QObject, public QFontEngineFactoryInterface
{
   GUI_CS_OBJECT_MULTIPLE(QFontEnginePlugin, QObject)
   CS_INTERFACES(QFontEngineFactoryInterface, QFactoryInterface)

 public:
   QFontEnginePlugin(const QString &foundry, QObject *parent = nullptr);
   ~QFontEnginePlugin();

   virtual QStringList keys() const;

   virtual QAbstractFontEngine *create(const QFontEngineInfo &info) = 0;
   virtual QList<QFontEngineInfo> availableFontEngines() const = 0;

 private:
   Q_DECLARE_PRIVATE(QFontEnginePlugin)
   Q_DISABLE_COPY(QFontEnginePlugin)
};

class QAbstractFontEnginePrivate;

class Q_GUI_EXPORT QAbstractFontEngine : public QObject
{
   GUI_CS_OBJECT(QAbstractFontEngine)

 public:
   enum Capability {
      CanOutlineGlyphs = 1,
      CanRenderGlyphs_Mono = 2,
      CanRenderGlyphs_Gray = 4,
      CanRenderGlyphs = CanRenderGlyphs_Mono | CanRenderGlyphs_Gray
   };
   using Capabilities = QFlags<Capability>;

   explicit QAbstractFontEngine(QObject *parent = nullptr);
   ~QAbstractFontEngine();

   typedef int Fixed; // 26.6

   struct FixedPoint {
      Fixed x;
      Fixed y;
   };

   struct GlyphMetrics {
      inline GlyphMetrics()
         : x(0), y(0), width(0), height(0),
           advance(0) {}
      Fixed x;
      Fixed y;
      Fixed width;
      Fixed height;
      Fixed advance;
   };

   enum FontProperty {
      Ascent,
      Descent,
      Leading,
      XHeight,
      AverageCharWidth,
      LineThickness,
      UnderlinePosition,
      MaxCharWidth,
      MinLeftBearing,
      MinRightBearing,
      GlyphCount,

      // hints
      CacheGlyphsHint,
      OutlineGlyphsHint
   };

   // keep in sync with QTextEngine::ShaperFlag!!
   enum TextShapingFlag {
      RightToLeft         = 0x0001,
      ReturnDesignMetrics = 0x0002
   };
   using TextShapingFlags = QFlags<TextShapingFlag>;

   virtual Capabilities capabilities() const = 0;
   virtual QVariant fontProperty(FontProperty property) const = 0;

   virtual bool convertStringToGlyphIndices(const QChar *string, int length, uint *glyphs, int *numGlyphs,
         TextShapingFlags flags) const = 0;

   virtual void getGlyphAdvances(const uint *glyphs, int numGlyphs, Fixed *advances, TextShapingFlags flags) const = 0;

   virtual GlyphMetrics glyphMetrics(uint glyph) const = 0;

   virtual bool renderGlyph(uint glyph, int depth, int bytesPerLine, int height, uchar *buffer);

   virtual void addGlyphOutlinesToPath(uint *glyphs, int numGlyphs, FixedPoint *positions, QPainterPath *path);

   /*
   enum Extension {
       GetTrueTypeTable
   };

   virtual bool supportsExtension(Extension extension) const;
   virtual QVariant extension(Extension extension, const QVariant &argument = QVariant());
   */

 private:
   Q_DECLARE_PRIVATE(QAbstractFontEngine)
   Q_DISABLE_COPY(QAbstractFontEngine)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractFontEngine::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractFontEngine::TextShapingFlags)

QT_END_NAMESPACE

#endif
