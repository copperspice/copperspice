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

#include <qglengineshadermanager_p.h>

#include <qalgorithms.h>
#include <qmetaenum.h>

#include <qglengineshadersource_p.h>
#include <qglshadercache_p.h>
#include <qopenglcontext_p.h>
#include <qpaintengineex_opengl2_p.h>

class QGLEngineSharedShadersResource : public QOpenGLSharedResource
{
public:
    QGLEngineSharedShadersResource(QOpenGLContext *ctx)
      : QOpenGLSharedResource(ctx->shareGroup()),
        m_shaders(new QGLEngineSharedShaders(QGLContext::fromOpenGLContext(ctx)))
    {
    }

    ~QGLEngineSharedShadersResource()
    {
        delete m_shaders;
    }

    void invalidateResource() override
    {
        delete m_shaders;
        m_shaders = nullptr;
    }

    void freeResource(QOpenGLContext *) override
    {
    }

    QGLEngineSharedShaders *shaders() const { return m_shaders; }

private:
    QGLEngineSharedShaders *m_shaders;
};

class QGLShaderStorage
{
 public:
   QGLEngineSharedShaders *shadersForThread(const QGLContext *context) {
      QOpenGLMultiGroupSharedResource *&shaders = m_storage.localData();

        if (! shaders) {
            shaders = new QOpenGLMultiGroupSharedResource;
        }

        QGLEngineSharedShadersResource *resource = shaders->value<QGLEngineSharedShadersResource>(context->contextHandle());

        return resource ? resource->shaders() : nullptr;
    }

 private:
    QThreadStorage<QOpenGLMultiGroupSharedResource *> m_storage;
};

static QGLShaderStorage *qt_shader_storage()
{
   static QGLShaderStorage retval;
   return &retval;
}

QGLEngineSharedShaders *QGLEngineSharedShaders::shadersForContext(const QGLContext *context)
{
   return qt_shader_storage()->shadersForThread(context);
}

QStringList QGLEngineSharedShaders::qShaderSnippets;

QGLEngineSharedShaders::QGLEngineSharedShaders(const QGLContext *context)
    : blitShaderProg(nullptr), simpleShaderProg(nullptr)
{
   /*
       Rather than having the shader source array statically initialised, it is initialised
       here instead. This is to allow new shader names to be inserted or existing names moved
       around without having to change the order of the glsl strings. It is hoped this will
       make future hard-to-find runtime bugs more obvious and generally give more solid code.
   */

   static bool snippetsPopulated = false;

   if (! snippetsPopulated) {
      qShaderSnippets.resize(TotalSnippetCount);

      qShaderSnippets[MainVertexShader]                                   = qglslMainVertexShader;
      qShaderSnippets[MainWithTexCoordsVertexShader]                      = qglslMainWithTexCoordsVertexShader;
      qShaderSnippets[MainWithTexCoordsAndOpacityVertexShader]            = qglslMainWithTexCoordsAndOpacityVertexShader;

      qShaderSnippets[UntransformedPositionVertexShader]                  = qglslUntransformedPositionVertexShader;
      qShaderSnippets[PositionOnlyVertexShader]                           = qglslPositionOnlyVertexShader;
      qShaderSnippets[ComplexGeometryPositionOnlyVertexShader]            = qglslComplexGeometryPositionOnlyVertexShader;
      qShaderSnippets[PositionWithPatternBrushVertexShader]               = qglslPositionWithPatternBrushVertexShader;
      qShaderSnippets[PositionWithLinearGradientBrushVertexShader]        = qglslPositionWithLinearGradientBrushVertexShader;
      qShaderSnippets[PositionWithConicalGradientBrushVertexShader]       = qglslPositionWithConicalGradientBrushVertexShader;
      qShaderSnippets[PositionWithRadialGradientBrushVertexShader]        = qglslPositionWithRadialGradientBrushVertexShader;
      qShaderSnippets[PositionWithTextureBrushVertexShader]               = qglslPositionWithTextureBrushVertexShader;
      qShaderSnippets[AffinePositionWithPatternBrushVertexShader]         = qglslAffinePositionWithPatternBrushVertexShader;
      qShaderSnippets[AffinePositionWithLinearGradientBrushVertexShader]  = qglslAffinePositionWithLinearGradientBrushVertexShader;
      qShaderSnippets[AffinePositionWithConicalGradientBrushVertexShader] = qglslAffinePositionWithConicalGradientBrushVertexShader;
      qShaderSnippets[AffinePositionWithRadialGradientBrushVertexShader]  = qglslAffinePositionWithRadialGradientBrushVertexShader;
      qShaderSnippets[AffinePositionWithTextureBrushVertexShader]         = qglslAffinePositionWithTextureBrushVertexShader;

      qShaderSnippets[MainFragmentShader_CMO]                   = qglslMainFragmentShader_CMO;
      qShaderSnippets[MainFragmentShader_CM]                    = qglslMainFragmentShader_CM;
      qShaderSnippets[MainFragmentShader_MO]                    = qglslMainFragmentShader_MO;
      qShaderSnippets[MainFragmentShader_M]                     = qglslMainFragmentShader_M;
      qShaderSnippets[MainFragmentShader_CO]                    = qglslMainFragmentShader_CO;
      qShaderSnippets[MainFragmentShader_C]                     = qglslMainFragmentShader_C;
      qShaderSnippets[MainFragmentShader_O]                     = qglslMainFragmentShader_O;
      qShaderSnippets[MainFragmentShader]                       = qglslMainFragmentShader;
      qShaderSnippets[MainFragmentShader_ImageArrays]           = qglslMainFragmentShader_ImageArrays;

      qShaderSnippets[ImageSrcFragmentShader]                   = qglslImageSrcFragmentShader;
      qShaderSnippets[ImageSrcWithPatternFragmentShader]        = qglslImageSrcWithPatternFragmentShader;
      qShaderSnippets[NonPremultipliedImageSrcFragmentShader]   = qglslNonPremultipliedImageSrcFragmentShader;
      qShaderSnippets[CustomImageSrcFragmentShader]             = qglslCustomSrcFragmentShader; // Calls "customShader", which must be appended
      qShaderSnippets[SolidBrushSrcFragmentShader]              = qglslSolidBrushSrcFragmentShader;

      if (!context->contextHandle()->isOpenGLES())
       qShaderSnippets[TextureBrushSrcFragmentShader]           = qglslTextureBrushSrcFragmentShader_desktop;
      else
       qShaderSnippets[TextureBrushSrcFragmentShader]           = qglslTextureBrushSrcFragmentShader_ES;

      qShaderSnippets[TextureBrushSrcWithPatternFragmentShader] = qglslTextureBrushSrcWithPatternFragmentShader;
      qShaderSnippets[PatternBrushSrcFragmentShader]            = qglslPatternBrushSrcFragmentShader;
      qShaderSnippets[LinearGradientBrushSrcFragmentShader]     = qglslLinearGradientBrushSrcFragmentShader;
      qShaderSnippets[RadialGradientBrushSrcFragmentShader]     = qglslRadialGradientBrushSrcFragmentShader;
      qShaderSnippets[ConicalGradientBrushSrcFragmentShader]    = qglslConicalGradientBrushSrcFragmentShader;
      qShaderSnippets[ShockingPinkSrcFragmentShader]            = qglslShockingPinkSrcFragmentShader;

      qShaderSnippets[NoMaskFragmentShader]                      = "";
      qShaderSnippets[MaskFragmentShader]                        = qglslMaskFragmentShader;
      qShaderSnippets[RgbMaskFragmentShaderPass1]                = qglslRgbMaskFragmentShaderPass1;
      qShaderSnippets[RgbMaskFragmentShaderPass2]                = qglslRgbMaskFragmentShaderPass2;
      qShaderSnippets[RgbMaskWithGammaFragmentShader]            = ""; //###

      qShaderSnippets[NoCompositionModeFragmentShader]           = "";
      qShaderSnippets[MultiplyCompositionModeFragmentShader]     = ""; //###
      qShaderSnippets[ScreenCompositionModeFragmentShader]       = ""; //###
      qShaderSnippets[OverlayCompositionModeFragmentShader]      = ""; //###
      qShaderSnippets[DarkenCompositionModeFragmentShader]       = ""; //###
      qShaderSnippets[LightenCompositionModeFragmentShader]      = ""; //###
      qShaderSnippets[ColorDodgeCompositionModeFragmentShader]   = ""; //###
      qShaderSnippets[ColorBurnCompositionModeFragmentShader]    = ""; //###
      qShaderSnippets[HardLightCompositionModeFragmentShader]    = ""; //###
      qShaderSnippets[SoftLightCompositionModeFragmentShader]    = ""; //###
      qShaderSnippets[DifferenceCompositionModeFragmentShader]   = ""; //###
      qShaderSnippets[ExclusionCompositionModeFragmentShader]    = ""; //###

      snippetsPopulated = true;
   }

   QGLShader *fragShader;
   QGLShader *vertexShader;
   QString vertexSource;
   QString fragSource;

   // Compile up the simple shader:
   vertexSource.append(qShaderSnippets[MainVertexShader]);
   vertexSource.append(qShaderSnippets[PositionOnlyVertexShader]);

   fragSource.append(qShaderSnippets[MainFragmentShader]);
   fragSource.append(qShaderSnippets[ShockingPinkSrcFragmentShader]);

   simpleShaderProg = new QGLShaderProgram(context, nullptr);

   CachedShader simpleShaderCache(fragSource, vertexSource);

   bool inCache = simpleShaderCache.load(simpleShaderProg, context);

   if (! inCache) {
      vertexShader = new QGLShader(QGLShader::Vertex, context, nullptr);
      shaders.append(vertexShader);

      if (! vertexShader->compileSourceCode(vertexSource)) {
         qWarning("Vertex shader for simpleShaderProg (MainVertexShader & PositionOnlyVertexShader) failed to compile");
      }

      fragShader = new QGLShader(QGLShader::Fragment, context, nullptr);
      shaders.append(fragShader);

      if (! fragShader->compileSourceCode(fragSource)) {
         qWarning("Fragment shader for simpleShaderProg (MainFragmentShader & ShockingPinkSrcFragmentShader) failed to compile");
      }

      simpleShaderProg->addShader(vertexShader);
      simpleShaderProg->addShader(fragShader);

      simpleShaderProg->bindAttributeLocation("vertexCoordsArray", QT_VERTEX_COORDS_ATTR);
      simpleShaderProg->bindAttributeLocation("pmvMatrix1", QT_PMV_MATRIX_1_ATTR);
      simpleShaderProg->bindAttributeLocation("pmvMatrix2", QT_PMV_MATRIX_2_ATTR);
      simpleShaderProg->bindAttributeLocation("pmvMatrix3", QT_PMV_MATRIX_3_ATTR);
   }

   simpleShaderProg->link();

   if (simpleShaderProg->isLinked()) {
      if (!inCache) {
         simpleShaderCache.store(simpleShaderProg, context);
      }

   } else {
      qCritical("Errors linking simple shader: %s", csPrintable(simpleShaderProg->log()));
   }

   // Compile the blit shader:
   vertexSource.clear();
   vertexSource.append(qShaderSnippets[MainWithTexCoordsVertexShader]);
   vertexSource.append(qShaderSnippets[UntransformedPositionVertexShader]);

   fragSource.clear();
   fragSource.append(qShaderSnippets[MainFragmentShader]);
   fragSource.append(qShaderSnippets[ImageSrcFragmentShader]);

   blitShaderProg = new QGLShaderProgram(context, nullptr);

   CachedShader blitShaderCache(fragSource, vertexSource);

   inCache = blitShaderCache.load(blitShaderProg, context);

   if (! inCache) {
      vertexShader = new QGLShader(QGLShader::Vertex, context, nullptr);
      shaders.append(vertexShader);
      if (!vertexShader->compileSourceCode(vertexSource)) {
         qWarning("Vertex shader for blitShaderProg (MainWithTexCoordsVertexShader & UntransformedPositionVertexShader) failed to compile");
      }

      fragShader = new QGLShader(QGLShader::Fragment, context, nullptr);
      shaders.append(fragShader);
      if (!fragShader->compileSourceCode(fragSource)) {
         qWarning("Fragment shader for blitShaderProg (MainFragmentShader & ImageSrcFragmentShader) failed to compile");
      }

      blitShaderProg->addShader(vertexShader);
      blitShaderProg->addShader(fragShader);

      blitShaderProg->bindAttributeLocation("textureCoordArray", QT_TEXTURE_COORDS_ATTR);
      blitShaderProg->bindAttributeLocation("vertexCoordsArray", QT_VERTEX_COORDS_ATTR);
   }

   blitShaderProg->link();

   if (blitShaderProg->isLinked()) {
      if (! inCache) {
         blitShaderCache.store(blitShaderProg, context);
      }
   } else {
        qCritical("Errors linking blit shader: %s", csPrintable(blitShaderProg->log()));
   }

#if defined(CS_SHOW_DEBUG_OPENGL)
   qDebug("QGLEngineSharedShaders() %p for thread %p", this, QThread::currentThread());
#endif
}

QGLEngineSharedShaders::~QGLEngineSharedShaders()
{
#if defined(CS_SHOW_DEBUG_OPENGL)
   qDebug("~QGLEngineSharedShaders() %p for thread %p", this, QThread::currentThread());
#endif

   qDeleteAll(shaders);
   shaders.clear();

   qDeleteAll(cachedPrograms);
   cachedPrograms.clear();

   if (blitShaderProg) {
      delete blitShaderProg;
      blitShaderProg = nullptr;
   }

   if (simpleShaderProg) {
      delete simpleShaderProg;
      simpleShaderProg = nullptr;
   }
}

#if defined(CS_SHOW_DEBUG_OPENGL)
QString QGLEngineSharedShaders::snippetNameStr(SnippetName name)
{
   QMetaEnum m = staticMetaObject().enumerator(staticMetaObject().indexOfEnumerator("SnippetName"));
   return m.valueToKey(name);
}
#endif

// The address returned here will only be valid until next time this function is called.
// The program is return bound.
QGLEngineShaderProg *QGLEngineSharedShaders::findProgramInCache(const QGLEngineShaderProg &prog)
{
   for (int i = 0; i < cachedPrograms.size(); ++i) {
      QGLEngineShaderProg *cachedProg = cachedPrograms[i];

      if (*cachedProg == prog) {
         // Move the program to the top of the list as a poor-man's cache algo
         cachedPrograms.move(i, 0);
         cachedProg->program->bind();
         return cachedProg;
      }
   }

   QScopedPointer<QGLEngineShaderProg> newProg;

   do {
      QString fragSource;

      // Insert the custom stage before the srcPixel shader to work around an ATI driver bug
      // where you cannot forward declare a function that takes a sampler as argument.

      if (prog.srcPixelFragShader == CustomImageSrcFragmentShader) {
         fragSource.append(prog.customStageSource);
      }

      fragSource.append(qShaderSnippets[prog.mainFragShader]);
      fragSource.append(qShaderSnippets[prog.srcPixelFragShader]);

      if (prog.compositionFragShader) {
         fragSource.append(qShaderSnippets[prog.compositionFragShader]);
      }

      if (prog.maskFragShader) {
         fragSource.append(qShaderSnippets[prog.maskFragShader]);
      }

      QString vertexSource;
      vertexSource.append(qShaderSnippets[prog.mainVertexShader]);
      vertexSource.append(qShaderSnippets[prog.positionVertexShader]);

      QScopedPointer<QGLShaderProgram> shaderProgram(new QGLShaderProgram);

      CachedShader shaderCache(fragSource, vertexSource);
      bool inCache = shaderCache.load(shaderProgram.data(), QGLContext::currentContext());

      if (!inCache) {

         QScopedPointer<QGLShader> fragShader(new QGLShader(QGLShader::Fragment));
         QString description;

#if defined(CS_SHOW_DEBUG_OPENGL)
         // Name the shader for easier debugging
         description.append("Fragment shader: main=");
         description.append(snippetNameStr(prog.mainFragShader));
         description.append(", srcPixel=");
         description.append(snippetNameStr(prog.srcPixelFragShader));

         if (prog.compositionFragShader) {
            description.append(", composition=");
            description.append(snippetNameStr(prog.compositionFragShader));
         }

         if (prog.maskFragShader) {
            description.append(", mask=");
            description.append(snippetNameStr(prog.maskFragShader));
         }

         fragShader->setObjectName(description);
#endif

         if (! fragShader->compileSourceCode(fragSource)) {
            qWarning() << "Warning:" << description << "failed to compile!";
            break;
         }

         QScopedPointer<QGLShader> vertexShader(new QGLShader(QGLShader::Vertex));

#if defined(CS_SHOW_DEBUG_OPENGL)
         // Name the shader for easier debugging
         description.clear();
         description.append("Vertex shader: main=");
         description.append(snippetNameStr(prog.mainVertexShader));
         description.append(", position=");
         description.append(snippetNameStr(prog.positionVertexShader));
         vertexShader->setObjectName(description);
#endif

         if (! vertexShader->compileSourceCode(vertexSource)) {
            qWarning() << "Warning:" << description << "failed to compile!";
            break;
         }

         shaders.append(vertexShader.data());
         shaders.append(fragShader.data());
         shaderProgram->addShader(vertexShader.take());
         shaderProgram->addShader(fragShader.take());

         // We have to bind the vertex attribute names before the program is linked:
         shaderProgram->bindAttributeLocation("vertexCoordsArray", QT_VERTEX_COORDS_ATTR);
         if (prog.useTextureCoords) {
            shaderProgram->bindAttributeLocation("textureCoordArray", QT_TEXTURE_COORDS_ATTR);
         }

         if (prog.useOpacityAttribute) {
            shaderProgram->bindAttributeLocation("opacityArray", QT_OPACITY_ATTR);
         }

         if (prog.usePmvMatrixAttribute) {
            shaderProgram->bindAttributeLocation("pmvMatrix1", QT_PMV_MATRIX_1_ATTR);
            shaderProgram->bindAttributeLocation("pmvMatrix2", QT_PMV_MATRIX_2_ATTR);
            shaderProgram->bindAttributeLocation("pmvMatrix3", QT_PMV_MATRIX_3_ATTR);
         }
      }

      newProg.reset(new QGLEngineShaderProg(prog));
      newProg->program = shaderProgram.take();

      newProg->program->link();
      if (newProg->program->isLinked()) {
         if (! inCache) {
            shaderCache.store(newProg->program, QGLContext::currentContext());
         }

      } else {
         QString error("Shader program failed to link,");

#if defined(CS_SHOW_DEBUG_OPENGL)
         error += "\n  Shaders Used:\n";

         for (int i = 0; i < newProg->program->shaders().count(); ++i) {
            QGLShader *shader = newProg->program->shaders().at(i);

            error += QLatin1String("    ") + shader->objectName() + QLatin1String(": \n")
                     + QLatin1String(shader->sourceCode()) + "\n";
         }
#endif
         error += "  Error Log:\n    " + newProg->program->log();
         qWarning() << error;
         break;
      }

      newProg->program->bind();

      if (newProg->maskFragShader != QGLEngineSharedShaders::NoMaskFragmentShader) {
         GLuint location = newProg->program->uniformLocation("maskTexture");
         newProg->program->setUniformValue(location, QT_MASK_TEXTURE_UNIT);
      }

      if (cachedPrograms.count() > 30) {
         // The cache is full, so delete the last 5 programs in the list.
         // These programs will be least used, as a program us bumped to
         // the top of the list when it's used.
         for (int i = 0; i < 5; ++i) {
            delete cachedPrograms.last();
            cachedPrograms.removeLast();
         }
      }

      cachedPrograms.insert(0, newProg.data());

   } while (false);

   return newProg.take();
}

void QGLEngineSharedShaders::cleanupCustomStage(QGLCustomShaderStage *stage)
{
   // Remove any shader programs which has this as the custom shader src:
   for (int i = 0; i < cachedPrograms.size(); ++i) {
      QGLEngineShaderProg *cachedProg = cachedPrograms[i];

      if (cachedProg->customStageSource == stage->source()) {
         delete cachedProg;
         cachedPrograms.removeAt(i);
         i--;
      }
   }
}


QGLEngineShaderManager::QGLEngineShaderManager(QGLContext *context)
   : ctx(context), shaderProgNeedsChanging(true), complexGeometry(false),
     srcPixelType(Qt::NoBrush), opacityMode(NoOpacity), maskType(NoMask),
     compositionMode(QPainter::CompositionMode_SourceOver), customSrcStage(nullptr), currentShaderProg(nullptr)
{
   sharedShaders = QGLEngineSharedShaders::shadersForContext(context);
}

QGLEngineShaderManager::~QGLEngineShaderManager()
{
   removeCustomStage();
}

GLuint QGLEngineShaderManager::getUniformLocation(Uniform id)
{
   if (! currentShaderProg) {
      return 0;
   }

   QVector<uint> &uniformLocations = currentShaderProg->uniformLocations;
   if (uniformLocations.isEmpty()) {
      uniformLocations.fill(GLuint(-1), NumUniforms);
   }

   static const char *const uniformNames[] = {
      "imageTexture",
      "patternColor",
      "globalOpacity",
      "depth",
      "maskTexture",
      "fragmentColor",
      "linearData",
      "angle",
      "halfViewportSize",
      "fmp",
      "fmp2_m_radius2",
      "inverse_2_fmp2_m_radius2",
      "sqrfr",
      "bradius",
      "invertedTextureSize",
      "brushTransform",
      "brushTexture",
      "matrix",
      "translateZ"
   };

   if (uniformLocations.at(id) == GLuint(-1)) {
      uniformLocations[id] = currentShaderProg->program->uniformLocation(uniformNames[id]);
   }

   return uniformLocations.at(id);
}


void QGLEngineShaderManager::optimiseForBrushTransform(QTransform::TransformationType transformType)
{
   (void) transformType;
}

void QGLEngineShaderManager::setDirty()
{
   shaderProgNeedsChanging = true;
}

void QGLEngineShaderManager::setSrcPixelType(Qt::BrushStyle style)
{
   Q_ASSERT(style != Qt::NoBrush);
   if (srcPixelType == PixelSrcType(style)) {
      return;
   }

   srcPixelType = style;
   shaderProgNeedsChanging = true; //###
}

void QGLEngineShaderManager::setSrcPixelType(PixelSrcType type)
{
   if (srcPixelType == type) {
      return;
   }

   srcPixelType = type;
   shaderProgNeedsChanging = true; //###
}

void QGLEngineShaderManager::setOpacityMode(OpacityMode mode)
{
   if (opacityMode == mode) {
      return;
   }

   opacityMode = mode;
   shaderProgNeedsChanging = true; //###
}

void QGLEngineShaderManager::setMaskType(MaskType type)
{
   if (maskType == type) {
      return;
   }

   maskType = type;
   shaderProgNeedsChanging = true; //###
}

void QGLEngineShaderManager::setCompositionMode(QPainter::CompositionMode mode)
{
   if (compositionMode == mode) {
      return;
   }

   compositionMode = mode;
   shaderProgNeedsChanging = true; //###
}

void QGLEngineShaderManager::setCustomStage(QGLCustomShaderStage *stage)
{
   if (customSrcStage) {
      removeCustomStage();
   }
   customSrcStage = stage;
   shaderProgNeedsChanging = true;
}

void QGLEngineShaderManager::removeCustomStage()
{
   if (customSrcStage) {
      customSrcStage->setInactive();
   }

   customSrcStage = nullptr;
   shaderProgNeedsChanging = true;
}

QGLShaderProgram *QGLEngineShaderManager::currentProgram()
{
   if (currentShaderProg) {
      return currentShaderProg->program;
   } else {
      return sharedShaders->simpleProgram();
   }
}

void QGLEngineShaderManager::useSimpleProgram()
{
   sharedShaders->simpleProgram()->bind();
   QGLContextPrivate *ctx_d = ctx->d_func();
   ctx_d->setVertexAttribArrayEnabled(QT_VERTEX_COORDS_ATTR, true);
   ctx_d->setVertexAttribArrayEnabled(QT_TEXTURE_COORDS_ATTR, false);
   ctx_d->setVertexAttribArrayEnabled(QT_OPACITY_ATTR, false);
   shaderProgNeedsChanging = true;
}

void QGLEngineShaderManager::useBlitProgram()
{
   sharedShaders->blitProgram()->bind();
   QGLContextPrivate *ctx_d = ctx->d_func();
   ctx_d->setVertexAttribArrayEnabled(QT_VERTEX_COORDS_ATTR, true);
   ctx_d->setVertexAttribArrayEnabled(QT_TEXTURE_COORDS_ATTR, true);
   ctx_d->setVertexAttribArrayEnabled(QT_OPACITY_ATTR, false);
   shaderProgNeedsChanging = true;
}

QGLShaderProgram *QGLEngineShaderManager::simpleProgram()
{
   return sharedShaders->simpleProgram();
}

QGLShaderProgram *QGLEngineShaderManager::blitProgram()
{
   return sharedShaders->blitProgram();
}

// Select & use the correct shader program using the current state.
// Returns true if program needed changing.
bool QGLEngineShaderManager::useCorrectShaderProg()
{
   if (!shaderProgNeedsChanging) {
      return false;
   }

   bool useCustomSrc = (customSrcStage != nullptr);

   if (useCustomSrc && srcPixelType != QGLEngineShaderManager::ImageSrc && srcPixelType != Qt::TexturePattern) {
      useCustomSrc = false;
      qWarning("QGLEngineShaderManager - Ignoring custom shader stage for non image src");
   }

   QGLEngineShaderProg requiredProgram;

   bool texCoords = false;

   // Choose vertex shader shader position function (which typically also sets
   // varyings) and the source pixel (srcPixel) fragment shader function:
   requiredProgram.positionVertexShader = QGLEngineSharedShaders::InvalidSnippetName;
   requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::InvalidSnippetName;
   bool isAffine = brushTransform.isAffine();
   if ( (srcPixelType >= Qt::Dense1Pattern) && (srcPixelType <= Qt::DiagCrossPattern) ) {
      if (isAffine) {
         requiredProgram.positionVertexShader = QGLEngineSharedShaders::AffinePositionWithPatternBrushVertexShader;
      } else {
         requiredProgram.positionVertexShader = QGLEngineSharedShaders::PositionWithPatternBrushVertexShader;
      }

      requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::PatternBrushSrcFragmentShader;
   } else switch (srcPixelType) {
         default:
         case Qt::NoBrush:
            qFatal("QGLEngineShaderManager::useCorrectShaderProg() - Qt::NoBrush style is set");
            break;
         case QGLEngineShaderManager::ImageSrc:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::ImageSrcFragmentShader;
            requiredProgram.positionVertexShader = QGLEngineSharedShaders::PositionOnlyVertexShader;
            texCoords = true;
            break;
         case QGLEngineShaderManager::NonPremultipliedImageSrc:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::NonPremultipliedImageSrcFragmentShader;
            requiredProgram.positionVertexShader = QGLEngineSharedShaders::PositionOnlyVertexShader;
            texCoords = true;
            break;
         case QGLEngineShaderManager::PatternSrc:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::ImageSrcWithPatternFragmentShader;
            requiredProgram.positionVertexShader = QGLEngineSharedShaders::PositionOnlyVertexShader;
            texCoords = true;
            break;

         case QGLEngineShaderManager::TextureSrcWithPattern:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::TextureBrushSrcWithPatternFragmentShader;
            requiredProgram.positionVertexShader = isAffine ? QGLEngineSharedShaders::AffinePositionWithTextureBrushVertexShader
                                                   : QGLEngineSharedShaders::PositionWithTextureBrushVertexShader;
            break;
         case Qt::SolidPattern:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::SolidBrushSrcFragmentShader;
            requiredProgram.positionVertexShader = QGLEngineSharedShaders::PositionOnlyVertexShader;
            break;

         case Qt::LinearGradientPattern:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::LinearGradientBrushSrcFragmentShader;
            requiredProgram.positionVertexShader = isAffine ?
                                                   QGLEngineSharedShaders::AffinePositionWithLinearGradientBrushVertexShader
                                                   : QGLEngineSharedShaders::PositionWithLinearGradientBrushVertexShader;
            break;
         case Qt::ConicalGradientPattern:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::ConicalGradientBrushSrcFragmentShader;
            requiredProgram.positionVertexShader = isAffine ?
                                                   QGLEngineSharedShaders::AffinePositionWithConicalGradientBrushVertexShader
                                                   : QGLEngineSharedShaders::PositionWithConicalGradientBrushVertexShader;
            break;
         case Qt::RadialGradientPattern:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::RadialGradientBrushSrcFragmentShader;
            requiredProgram.positionVertexShader = isAffine ?
                                                   QGLEngineSharedShaders::AffinePositionWithRadialGradientBrushVertexShader
                                                   : QGLEngineSharedShaders::PositionWithRadialGradientBrushVertexShader;
            break;
         case Qt::TexturePattern:
            requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::TextureBrushSrcFragmentShader;
            requiredProgram.positionVertexShader = isAffine ? QGLEngineSharedShaders::AffinePositionWithTextureBrushVertexShader
                                                   : QGLEngineSharedShaders::PositionWithTextureBrushVertexShader;
            break;
      };

   if (useCustomSrc) {
      requiredProgram.srcPixelFragShader = QGLEngineSharedShaders::CustomImageSrcFragmentShader;
      requiredProgram.customStageSource  = customSrcStage->source();
   }

   const bool hasCompose = compositionMode > QPainter::CompositionMode_Plus;
   const bool hasMask    = maskType != QGLEngineShaderManager::NoMask;

   // Choose fragment shader main function:
   if (opacityMode == AttributeOpacity) {
      Q_ASSERT(!hasCompose && !hasMask);
      requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader_ImageArrays;

   } else {
      bool useGlobalOpacity = (opacityMode == UniformOpacity);
      if (hasCompose && hasMask && useGlobalOpacity) {
         requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader_CMO;
      }
      if (hasCompose && hasMask && !useGlobalOpacity) {
         requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader_CM;
      }
      if (!hasCompose && hasMask && useGlobalOpacity) {
         requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader_MO;
      }
      if (!hasCompose && hasMask && !useGlobalOpacity) {
         requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader_M;
      }
      if (hasCompose && !hasMask && useGlobalOpacity) {
         requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader_CO;
      }
      if (hasCompose && !hasMask && !useGlobalOpacity) {
         requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader_C;
      }
      if (!hasCompose && !hasMask && useGlobalOpacity) {
         requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader_O;
      }
      if (!hasCompose && !hasMask && !useGlobalOpacity) {
         requiredProgram.mainFragShader = QGLEngineSharedShaders::MainFragmentShader;
      }
   }

   if (hasMask) {
      if (maskType == PixelMask) {
         requiredProgram.maskFragShader = QGLEngineSharedShaders::MaskFragmentShader;
         texCoords = true;
      } else if (maskType == SubPixelMaskPass1) {
         requiredProgram.maskFragShader = QGLEngineSharedShaders::RgbMaskFragmentShaderPass1;
         texCoords = true;
      } else if (maskType == SubPixelMaskPass2) {
         requiredProgram.maskFragShader = QGLEngineSharedShaders::RgbMaskFragmentShaderPass2;
         texCoords = true;
      } else if (maskType == SubPixelWithGammaMask) {
         requiredProgram.maskFragShader = QGLEngineSharedShaders::RgbMaskWithGammaFragmentShader;
         texCoords = true;
      } else {
         qCritical("QGLEngineShaderManager::useCorrectShaderProg() - Unknown mask type");
      }
   } else {
      requiredProgram.maskFragShader = QGLEngineSharedShaders::NoMaskFragmentShader;
   }

   if (hasCompose) {
      switch (compositionMode) {
         case QPainter::CompositionMode_Multiply:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::MultiplyCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_Screen:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::ScreenCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_Overlay:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::OverlayCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_Darken:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::DarkenCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_Lighten:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::LightenCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_ColorDodge:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::ColorDodgeCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_ColorBurn:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::ColorBurnCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_HardLight:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::HardLightCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_SoftLight:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::SoftLightCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_Difference:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::DifferenceCompositionModeFragmentShader;
            break;
         case QPainter::CompositionMode_Exclusion:
            requiredProgram.compositionFragShader = QGLEngineSharedShaders::ExclusionCompositionModeFragmentShader;
            break;
         default:
            qWarning("QGLEngineShaderManager::useCorrectShaderProg() - Unsupported composition mode");
      }
   } else {
      requiredProgram.compositionFragShader = QGLEngineSharedShaders::NoCompositionModeFragmentShader;
   }

   // Choose vertex shader main function
   if (opacityMode == AttributeOpacity) {
      Q_ASSERT(texCoords);
      requiredProgram.mainVertexShader = QGLEngineSharedShaders::MainWithTexCoordsAndOpacityVertexShader;
   } else if (texCoords) {
      requiredProgram.mainVertexShader = QGLEngineSharedShaders::MainWithTexCoordsVertexShader;
   } else {
      requiredProgram.mainVertexShader = QGLEngineSharedShaders::MainVertexShader;
   }

   requiredProgram.useTextureCoords = texCoords;
   requiredProgram.useOpacityAttribute = (opacityMode == AttributeOpacity);

   if (complexGeometry && srcPixelType == Qt::SolidPattern) {
      requiredProgram.positionVertexShader = QGLEngineSharedShaders::ComplexGeometryPositionOnlyVertexShader;
      requiredProgram.usePmvMatrixAttribute = false;
   } else {
      requiredProgram.usePmvMatrixAttribute = true;

      // Force complexGeometry off, since we currently don't support that mode for
      // non-solid brushes
      complexGeometry = false;
   }

   // At this point, requiredProgram is fully populated so try to find the program in the cache
   currentShaderProg = sharedShaders->findProgramInCache(requiredProgram);

   if (currentShaderProg && useCustomSrc) {
      customSrcStage->setUniforms(currentShaderProg->program);
   }

   // Make sure all the vertex attribute arrays the program uses are enabled (and the ones it
   // doesn't use are disabled)
   QGLContextPrivate *ctx_d = ctx->d_func();
   ctx_d->setVertexAttribArrayEnabled(QT_VERTEX_COORDS_ATTR, true);
   ctx_d->setVertexAttribArrayEnabled(QT_TEXTURE_COORDS_ATTR, currentShaderProg && currentShaderProg->useTextureCoords);
   ctx_d->setVertexAttribArrayEnabled(QT_OPACITY_ATTR, currentShaderProg && currentShaderProg->useOpacityAttribute);

   shaderProgNeedsChanging = false;
   return true;
}

