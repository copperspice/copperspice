/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QWAYLAND_DECORATIONS_BLITTER_H
#define QWAYLAND_DECORATIONS_BLITTER_H

#include <qdebug.h>
#include <qopengl_shaderprogram.h>
#include <qopenglfunctions.h>
#include <qwayland_egl_window.h>

#include <qopengl_texturecache_p.h>
#include <qwayland_abstract_decoration_p.h>

namespace QtWaylandClient {

class QWaylandGLContext;

class DecorationsBlitter : public QOpenGLFunctions
{
 public:
   DecorationsBlitter(QWaylandGLContext *context)
      : m_glContext(context)
   {
      initializeOpenGLFunctions();
      m_blitterProgram = new QOpenGLShaderProgram();

      m_blitterProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, "attribute vec4 position;\n\
            attribute vec4 texCoords;\n\
            varying vec2 outTexCoords;\n\
            void main()\n\
            {\n\
               gl_Position = position;\n\
               outTexCoords = texCoords.xy;\n\
            }");

      m_blitterProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, "varying highp vec2 outTexCoords;\n\
            uniform sampler2D texture;\n\
            void main()\n\
            {\n\
               gl_FragColor = texture2D(texture, outTexCoords);\n\
            }");

      m_blitterProgram->bindAttributeLocation("position",  0);
      m_blitterProgram->bindAttributeLocation("texCoords", 1);

      if (! m_blitterProgram->link()) {
#if defined(CS_SHOW_DEBUG_PLATFORM)
         qDebug() << "Shader Program link failed.";
         qDebug() << m_blitterProgram->log();
#endif
      }
   }

   ~DecorationsBlitter() {
      delete m_blitterProgram;
   }

   void blit(QWaylandEglWindow *window) {
      QOpenGLTextureCache *cache = QOpenGLTextureCache::cacheForContext(m_glContext->context());

      QRect windowRect = window->window()->frameGeometry();
      int scale = window->scale();

      glViewport(0, 0, windowRect.width() * scale, windowRect.height() * scale);

      glDisable(GL_DEPTH_TEST);
      glDisable(GL_BLEND);
      glDisable(GL_CULL_FACE);
      glDisable(GL_SCISSOR_TEST);
      glDepthMask(GL_FALSE);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      m_glContext->setNativeDefaultFbo(true);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      m_glContext->setNativeDefaultFbo(false);

      static const GLfloat squareVertices[] = {
         -1.f, -1.f,
         1.0f, -1.f,
         -1.f,  1.0f,
         1.0f,  1.0f
      };

      static const GLfloat inverseSquareVertices[] = {
         -1.f,  1.f,
          1.f,  1.f,
         -1.f, -1.f,
          1.f, -1.f
      };

      static const GLfloat textureVertices[] = {
         0.0f,  0.0f,
         1.0f,  0.0f,
         0.0f,  1.0f,
         1.0f,  1.0f,
      };

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      m_blitterProgram->bind();

      m_blitterProgram->enableAttributeArray(0);
      m_blitterProgram->enableAttributeArray(1);
      m_blitterProgram->setAttributeArray(1, textureVertices, 2);

      glActiveTexture(GL_TEXTURE0);

      // draw Decoration
      m_blitterProgram->setAttributeArray(0, inverseSquareVertices, 2);

      QImage decorationImage = window->decoration()->contentImage();
      cache->bindTexture(m_glContext->context(), decorationImage);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      if (m_glContext->context()->functions()->hasOpenGLFeature(QOpenGLFunctions::NPOTTextureRepeat)) {
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      } else {
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      // draw content
      m_blitterProgram->setAttributeArray(0, squareVertices, 2);
      glBindTexture(GL_TEXTURE_2D, window->contentTexture());

      QRect r = window->contentsRect();
      glViewport(r.x() * scale, r.y() * scale, r.width() * scale, r.height() * scale);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      // cleanup
      m_blitterProgram->disableAttributeArray(0);
      m_blitterProgram->disableAttributeArray(1);
   }

 private:
   QOpenGLShaderProgram *m_blitterProgram;
   QWaylandGLContext    *m_glContext;
};

}

#endif
