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

#ifndef QPaintEngineEx_OPENGL2_P_H
#define QPaintEngineEx_OPENGL2_P_H

#include <qdebug.h>
#include <qvector.h>

#include <qpaintengineex_p.h>
#include <qglengineshadermanager_p.h>
#include <qgl2pexvertexarray_p.h>
#include <qglpaintdevice_p.h>
#include <qfontengine_p.h>
#include <qtriangulatingstroker_p.h>
#include <qopengl_extensions_p.h>

enum EngineMode {
   ImageDrawingMode,
   TextDrawingMode,
   BrushDrawingMode,
   ImageArrayDrawingMode,

};

#define GL_STENCIL_HIGH_BIT         GLuint(0x80)
#define QT_BRUSH_TEXTURE_UNIT       GLuint(0)
#define QT_IMAGE_TEXTURE_UNIT       GLuint(0) //Can be the same as brush texture unit
#define QT_MASK_TEXTURE_UNIT        GLuint(1)
#define QT_BACKGROUND_TEXTURE_UNIT  GLuint(2)

class QGL2PaintEngineExPrivate;


class QGL2PaintEngineState : public QPainterState
{
 public:
   QGL2PaintEngineState(QGL2PaintEngineState &other);
   QGL2PaintEngineState();
   ~QGL2PaintEngineState();

   uint isNew : 1;
   uint needsClipBufferClear : 1;
   uint clipTestEnabled : 1;
   uint canRestoreClip : 1;
   uint matrixChanged : 1;
   uint compositionModeChanged : 1;
   uint opacityChanged : 1;
   uint renderHintsChanged : 1;
   uint clipChanged : 1;
   uint currentClip : 8;

   QRect rectangleClip;
};

class Q_OPENGL_EXPORT QGL2PaintEngineEx : public QPaintEngineEx
{
   Q_DECLARE_PRIVATE(QGL2PaintEngineEx)

 public:
   QGL2PaintEngineEx();

   QGL2PaintEngineEx(const QGL2PaintEngineEx &) = delete;
   QGL2PaintEngineEx &operator=(const QGL2PaintEngineEx &) = delete;

   ~QGL2PaintEngineEx();

   bool begin(QPaintDevice *device) override;
   void ensureActive();
   bool end() override;

   void clipEnabledChanged() override;
   void penChanged() override;
   void brushChanged() override;
   void brushOriginChanged() override;
   void opacityChanged() override;
   void compositionModeChanged() override;
   void renderHintsChanged() override;
   void transformChanged() override;

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
                  QPainter::PixmapFragmentHints hints) override;

   void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                  Qt::ImageConversionFlags flags = Qt::AutoColor) override;

   void drawTextItem(const QPointF &p, const QTextItem &textItem) override;
   void fill(const QVectorPath &path, const QBrush &brush) override;
   void stroke(const QVectorPath &path, const QPen &pen) override;
   void clip(const QVectorPath &path, Qt::ClipOperation op) override;

   void drawStaticTextItem(QStaticTextItem *textItem) override;

   bool drawTexture(const QRectF &r, GLuint texture_id, const QSize &size, const QRectF &sr);

   Type type() const override {
      return OpenGL2;
   }

   void setState(QPainterState *s) override;
   QPainterState *createState(QPainterState *orig) const override;

   QGL2PaintEngineState *state() {
      return static_cast<QGL2PaintEngineState *>(QPaintEngineEx::state());
   }

   const QGL2PaintEngineState *state() const {
      return static_cast<const QGL2PaintEngineState *>(QPaintEngineEx::state());
   }

   void beginNativePainting() override;
   void endNativePainting() override;

   void invalidateState();


   void setRenderTextActive(bool);

   bool isNativePaintingActive() const;
   bool requiresPretransformedGlyphPositions(QFontEngine *, const QTransform &) const override { return false; }
   bool shouldDrawCachedGlyphs(QFontEngine *, const QTransform &) const override;

   void setTranslateZ(GLfloat z);
};

class QGL2PaintEngineExPrivate : public QPaintEngineExPrivate, protected QOpenGLExtensions
{
   Q_DECLARE_PUBLIC(QGL2PaintEngineEx)

 public:
   enum StencilFillMode {
      OddEvenFillMode,
      WindingFillMode,
      TriStripStrokeFillMode
   };

   QGL2PaintEngineExPrivate(QGL2PaintEngineEx *q_ptr)
      : q(q_ptr), shaderManager(nullptr), width(0), height(0), ctx(nullptr), useSystemClip(true),
        elementIndicesVBOId(0), snapToPixelGrid(false), nativePaintingActive(false),
        inverseScale(1), lastMaskTextureUsed(0), translateZ(0)
    { }

   ~QGL2PaintEngineExPrivate();

   void updateBrushTexture();
   void updateBrushUniforms();
   void updateMatrix();
   void updateCompositionMode();
   void updateTextureFilter(GLenum target, GLenum wrapMode, bool smoothPixmapTransform, GLuint texture_id = GLuint(-1));

   void resetGLState();
   bool resetOpenGLContextActiveEngine();

   // fill, stroke, drawTexture, drawPixmaps & drawCachedGlyphs are the main rendering entry-points,
   // however writeClip can also be thought of as en entry point as it does similar things.
   void fill(const QVectorPath &path);
   void stroke(const QVectorPath &path, const QPen &pen);
   void drawTexture(const QGLRect &dest, const QGLRect &src, const QSize &textureSize, bool opaque, bool pattern = false);
   void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
                             QPainter::PixmapFragmentHints hints);

   void drawCachedGlyphs(QFontEngine::GlyphFormat glyphFormat, QStaticTextItem *staticTextItem);

   // Calls glVertexAttributePointer if the pointer has changed
   inline void setVertexAttributePointer(unsigned int arrayIndex, const GLfloat *pointer);

   // draws whatever is in the vertex array:
   void drawVertexArrays(const float *data, const int *stops, int stopCount, GLenum primitive);

   void drawVertexArrays(QGL2PEXVertexArray &vertexArray, GLenum primitive) {
      drawVertexArrays((const float *) vertexArray.data(), vertexArray.stops(), vertexArray.stopCount(), primitive);
   }

   // Composites the bounding rect onto dest buffer:
   void composite(const QGLRect &boundingRect);

   // Calls drawVertexArrays to render into stencil buffer:
   void fillStencilWithVertexArray(const float *data, int count, const int *stops, int stopCount, const QGLRect &bounds,
                                   StencilFillMode mode);

   void fillStencilWithVertexArray(QGL2PEXVertexArray &vertexArray, bool useWindingFill) {
      fillStencilWithVertexArray((const float *) vertexArray.data(), 0, vertexArray.stops(), vertexArray.stopCount(),
                                 vertexArray.boundingRect(),
                                 useWindingFill ? WindingFillMode : OddEvenFillMode);
   }

   void setBrush(const QBrush &brush);
   void transferMode(EngineMode newMode);
   bool prepareForDraw(bool srcPixelsAreOpaque); // returns true if the program has changed
   bool prepareForCachedGlyphDraw(const QFontEngineGlyphCache &cache);

   inline void useSimpleShader();
   inline GLuint location(const QGLEngineShaderManager::Uniform uniform) {
      return shaderManager->getUniformLocation(uniform);
   }

   void clearClip(uint value);
   void writeClip(const QVectorPath &path, uint value);
   void resetClipIfNeeded();

   void updateClipScissorTest();
   void setScissor(const QRect &rect);
   void regenerateClip();
   void systemStateChanged() override;

   static QGLEngineShaderManager* shaderManagerForEngine(QGL2PaintEngineEx *engine) { return engine->d_func()->shaderManager; }
   static QGL2PaintEngineExPrivate *getData(QGL2PaintEngineEx *engine) { return engine->d_func(); }

   static void cleanupVectorPath(QPaintEngineEx *engine, void *data);

   QGL2PaintEngineEx *q;
   QGLEngineShaderManager *shaderManager;
   QGLPaintDevice *device;
   int width, height;
   QGLContext *ctx;
   EngineMode mode;

   QFontEngine::GlyphFormat glyphCacheFormat;

   // Dirty flags
   bool matrixDirty; // Implies matrix uniforms are also dirty
   bool compositionModeDirty;
   bool brushTextureDirty;
   bool brushUniformsDirty;
   bool opacityUniformDirty;
   bool matrixUniformDirty;
   bool translateZUniformDirty;

   bool stencilClean; // Has the stencil not been used for clipping so far?
   bool useSystemClip;
   QRegion dirtyStencilRegion;
   QRect currentScissorBounds;
   uint maxClip;

   QBrush currentBrush; // May not be the state's brush!
   const QBrush noBrush;

   QPixmap currentBrushPixmap;

   QGL2PEXVertexArray vertexCoordinateArray;
   QGL2PEXVertexArray textureCoordinateArray;
   QVector<GLushort> elementIndices;
   GLuint elementIndicesVBOId;
   QVector<GLfloat> opacityArray;
   GLfloat staticVertexCoordinateArray[8];
   GLfloat staticTextureCoordinateArray[8];

   bool snapToPixelGrid;
   bool nativePaintingActive;
   GLfloat pmvMatrix[3][3];
   GLfloat inverseScale;

   GLuint lastTextureUsed;
   GLuint lastMaskTextureUsed;

   bool needsSync;
   bool multisamplingAlwaysEnabled;

   GLfloat depthRange[2];

   float textureInvertedY;

   QTriangulatingStroker stroker;
   QDashedStrokeProcessor dasher;

   QSet<QVectorPath::CacheEntry *> pathCaches;
   QVector<GLuint> unusedVBOSToClean;
   QVector<GLuint> unusedIBOSToClean;

   const GLfloat *vertexAttribPointers[3];

   GLfloat translateZ;
};

void QGL2PaintEngineExPrivate::setVertexAttributePointer(unsigned int arrayIndex, const GLfloat *pointer)
{
   Q_ASSERT(arrayIndex < 3);
   if (pointer == vertexAttribPointers[arrayIndex]) {
      return;
   }

   vertexAttribPointers[arrayIndex] = pointer;
   if (arrayIndex == QT_OPACITY_ATTR) {
      glVertexAttribPointer(arrayIndex, 1, GL_FLOAT, GL_FALSE, 0, pointer);
   } else {
      glVertexAttribPointer(arrayIndex, 2, GL_FLOAT, GL_FALSE, 0, pointer);
   }
}

#endif
