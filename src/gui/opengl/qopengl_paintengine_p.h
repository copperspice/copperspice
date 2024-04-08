/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#ifndef QOPENGLPAINTENGINE_P_H
#define QOPENGLPAINTENGINE_P_H

#include <qdebug.h>
#include <qopengl_paintdevice.h>
#include <qvector.h>

#include <qpaintengineex_p.h>
#include <qfontengine_p.h>
#include <qtriangulatingstroker_p.h>
#include <qopengl_engineshadermanager_p.h>
#include <qopengl_2pexvertexarray_p.h>
#include <qopengl_extensions_p.h>

enum EngineMode {
    ImageDrawingMode,
    TextDrawingMode,
    BrushDrawingMode,
    ImageArrayDrawingMode,
    ImageOpacityArrayDrawingMode
};

#define GL_STENCIL_HIGH_BIT         GLuint(0x80)
#define QT_UNKNOWN_TEXTURE_UNIT     GLuint(-1)
#define QT_DEFAULT_TEXTURE_UNIT     GLuint(0)
#define QT_BRUSH_TEXTURE_UNIT       GLuint(0)
#define QT_IMAGE_TEXTURE_UNIT       GLuint(0) //Can be the same as brush texture unit
#define QT_MASK_TEXTURE_UNIT        GLuint(1)
#define QT_BACKGROUND_TEXTURE_UNIT  GLuint(2)

class QOpenGL2PaintEngineExPrivate;

class QOpenGL2PaintEngineState : public QPainterState
{
public:
    QOpenGL2PaintEngineState(QOpenGL2PaintEngineState &other);
    QOpenGL2PaintEngineState();
    ~QOpenGL2PaintEngineState();

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

class Q_GUI_EXPORT QOpenGL2PaintEngineEx : public QPaintEngineEx
{
 public:
    QOpenGL2PaintEngineEx();

    QOpenGL2PaintEngineEx(const QOpenGL2PaintEngineEx &) = delete;
    QOpenGL2PaintEngineEx &operator=(const QOpenGL2PaintEngineEx &) = delete;

    ~QOpenGL2PaintEngineEx();

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

    void drawPixmap(const QRectF &rect, const QPixmap &pm, const QRectF &srcRect) override;
    void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
               QPainter::PixmapFragmentHints hints) override;
    void drawImage(const QRectF &rect, const QImage &pm, const QRectF &srcRect,
               Qt::ImageConversionFlags flags = Qt::AutoColor) override;
    void drawTextItem(const QPointF &point, const QTextItem &textItem) override;
    void fill(const QVectorPath &path, const QBrush &brush) override;
    void stroke(const QVectorPath &path, const QPen &pen) override;
    void clip(const QVectorPath &path, Qt::ClipOperation op) override;

    void drawStaticTextItem(QStaticTextItem *textItem) override;

    bool drawTexture(const QRectF &rect, GLuint texture_id, const QSize &size, const QRectF &srcRect);

    Type type() const override { return OpenGL2; }

    void setState(QPainterState *s) override;
    QPainterState *createState(QPainterState *orig) const override;

    QOpenGL2PaintEngineState *state() {
        return static_cast<QOpenGL2PaintEngineState *>(QPaintEngineEx::state());
    }

    const QOpenGL2PaintEngineState *state() const {
        return static_cast<const QOpenGL2PaintEngineState *>(QPaintEngineEx::state());
    }

    void beginNativePainting() override;
    void endNativePainting() override;

    void invalidateState();

    void setRenderTextActive(bool);

    bool isNativePaintingActive() const;
    bool requiresPretransformedGlyphPositions(QFontEngine *, const QTransform &) const override { return false; }
    bool shouldDrawCachedGlyphs(QFontEngine *, const QTransform &) const override;

 private:
    Q_DECLARE_PRIVATE(QOpenGL2PaintEngineEx)
    friend class QOpenGLEngineShaderManager;
};

// This probably needs to grow to GL_MAX_VERTEX_ATTRIBS, but 3 is ok for now as that's
// all the GL2 engine uses:
#define QT_GL_VERTEX_ARRAY_TRACKED_COUNT 3

class QOpenGL2PaintEngineExPrivate : public QPaintEngineExPrivate
{
 public:
    enum StencilFillMode {
        OddEvenFillMode,
        WindingFillMode,
        TriStripStrokeFillMode
    };

    QOpenGL2PaintEngineExPrivate(QOpenGL2PaintEngineEx *q_ptr)
      : q(q_ptr), shaderManager(nullptr), width(0), height(0), ctx(nullptr), useSystemClip(true), elementIndicesVBOId(0),
        snapToPixelGrid(false), nativePaintingActive(false), inverseScale(1),
        lastTextureUnitUsed(QT_UNKNOWN_TEXTURE_UNIT)
    {
    }

    ~QOpenGL2PaintEngineExPrivate();

    void updateBrushTexture();
    void updateBrushUniforms();
    void updateMatrix();
    void updateCompositionMode();

    enum TextureUpdateMode { UpdateIfNeeded, ForceUpdate };

    template <typename T>
    void updateTexture(GLenum textureUnit, const T &texture, GLenum wrapMode, GLenum filterMode,
          TextureUpdateMode updateMode = UpdateIfNeeded);

    template <typename T>
    GLuint bindTexture(const T &texture);

    void activateTextureUnit(GLenum textureUnit);

    void resetGLState();

    // fill, stroke, drawTexture, drawPixmaps & drawCachedGlyphs are the main rendering entry-points,
    // however writeClip can also be thought of as en entry point as it does similar things.
    void fill(const QVectorPath &path);
    void stroke(const QVectorPath &path, const QPen &pen);

    void drawTexture(const QOpenGLRect& dest, const QOpenGLRect& src, const QSize &textureSize, bool opaque, bool pattern = false);
    void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
          QPainter::PixmapFragmentHints hints);

    void drawCachedGlyphs(QFontEngine::GlyphFormat glyphFormat, QStaticTextItem *staticTextItem);

    // Calls glVertexAttributePointer if the pointer has changed
    inline void setVertexAttributePointer(unsigned int arrayIndex, const GLfloat *pointer);

    // draws whatever is in the vertex array:
    void drawVertexArrays(const float *data, const int *stops, int stopCount, GLenum primitive);
    void drawVertexArrays(QOpenGL2PEXVertexArray &vertexArray, GLenum primitive) {
        drawVertexArrays((const float *) vertexArray.data(), vertexArray.stops(), vertexArray.stopCount(), primitive);
    }

    // Composites the bounding rect onto dest buffer:
    void composite(const QOpenGLRect& boundingRect);

    // Calls drawVertexArrays to render into stencil buffer:
    void fillStencilWithVertexArray(const float *data, int count, const int *stops, int stopCount, const QOpenGLRect &bounds,
                  StencilFillMode mode);

    void fillStencilWithVertexArray(QOpenGL2PEXVertexArray& vertexArray, bool useWindingFill) {
        fillStencilWithVertexArray((const float *) vertexArray.data(), 0, vertexArray.stops(), vertexArray.stopCount(),
                  vertexArray.boundingRect(), useWindingFill ? WindingFillMode : OddEvenFillMode);
    }

    void setBrush(const QBrush& brush);
    void transferMode(EngineMode newMode);
    bool prepareForDraw(bool srcPixelsAreOpaque); // returns true if the program has changed
    bool prepareForCachedGlyphDraw(const QFontEngineGlyphCache &cache);

    inline void useSimpleShader();

    GLuint location(const QOpenGLEngineShaderManager::Uniform uniform) {
        return shaderManager->getUniformLocation(uniform);
    }

    void clearClip(uint value);
    void writeClip(const QVectorPath &path, uint value);
    void resetClipIfNeeded();

    void updateClipScissorTest();
    void setScissor(const QRect &rect);
    void regenerateClip();
    void systemStateChanged() override;

    void setVertexAttribArrayEnabled(int arrayIndex, bool enabled = true);
    void syncGlState();

    static QOpenGLEngineShaderManager* shaderManagerForEngine(QOpenGL2PaintEngineEx *engine) { return engine->d_func()->shaderManager; }
    static QOpenGL2PaintEngineExPrivate *getData(QOpenGL2PaintEngineEx *engine) { return engine->d_func(); }
    static void cleanupVectorPath(QPaintEngineEx *engine, void *data);

    QOpenGLExtensions funcs;

    QOpenGL2PaintEngineEx* q;
    QOpenGLEngineShaderManager* shaderManager;
    QOpenGLPaintDevice* device;
    int width, height;
    QOpenGLContext *ctx;
    EngineMode mode;
    QFontEngine::GlyphFormat glyphCacheFormat;

    bool vertexAttributeArraysEnabledState[QT_GL_VERTEX_ARRAY_TRACKED_COUNT];

    // dirty flags
    bool matrixDirty; // Implies matrix uniforms are also dirty
    bool compositionModeDirty;
    bool brushTextureDirty;
    bool brushUniformsDirty;
    bool opacityUniformDirty;
    bool matrixUniformDirty;

    bool stencilClean; // Has the stencil not been used for clipping so far?
    bool useSystemClip;
    QRegion dirtyStencilRegion;
    QRect currentScissorBounds;
    uint maxClip;

    QBrush currentBrush; // May not be the state's brush!
    const QBrush noBrush;

    QImage currentBrushImage;

    QOpenGL2PEXVertexArray vertexCoordinateArray;
    QOpenGL2PEXVertexArray textureCoordinateArray;
    QVector<GLushort> elementIndices;
    GLuint elementIndicesVBOId;
    QVector<GLfloat> opacityArray;
    GLfloat staticVertexCoordinateArray[8];
    GLfloat staticTextureCoordinateArray[8];

    bool snapToPixelGrid;
    bool nativePaintingActive;
    GLfloat pmvMatrix[3][3];
    GLfloat inverseScale;

    GLenum lastTextureUnitUsed;
    GLuint lastTextureUsed;

    bool needsSync;
    bool multisamplingAlwaysEnabled;

    QTriangulatingStroker stroker;
    QDashedStrokeProcessor dasher;

    QVector<GLuint> unusedVBOSToClean;
    QVector<GLuint> unusedIBOSToClean;

    const GLfloat *vertexAttribPointers[3];

 private:
   Q_DECLARE_PUBLIC(QOpenGL2PaintEngineEx)
};

void QOpenGL2PaintEngineExPrivate::setVertexAttributePointer(unsigned int arrayIndex, const GLfloat *pointer)
{
    Q_ASSERT(arrayIndex < 3);

    if (pointer == vertexAttribPointers[arrayIndex])
        return;

    vertexAttribPointers[arrayIndex] = pointer;

    if (arrayIndex == QT_OPACITY_ATTR)
        funcs.glVertexAttribPointer(arrayIndex, 1, GL_FLOAT, GL_FALSE, 0, pointer);
    else
        funcs.glVertexAttribPointer(arrayIndex, 2, GL_FLOAT, GL_FALSE, 0, pointer);
}

#endif
