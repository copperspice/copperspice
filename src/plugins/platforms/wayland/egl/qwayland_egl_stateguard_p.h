/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QWAYLAND_EGL_STATEGUARD_H
#define QWAYLAND_EGL_STATEGUARD_H

#include <qopenglfunctions.h>

#include <qopenglcontext_p.h>

namespace QtWaylandClient {

static constexpr const int STATE_GUARD_VERTEX_ATTRIB_COUNT = 2;

class EGL_StateGuard
{
 public:
   EGL_StateGuard() {
      QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());

      glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &m_program);
      glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *) &m_activeTextureUnit);
      glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &m_texture);
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &m_fbo);
      glGetIntegerv(GL_VIEWPORT, m_viewport);
      glGetIntegerv(GL_DEPTH_WRITEMASK, &m_depthWriteMask);
      glGetIntegerv(GL_COLOR_WRITEMASK, m_colorWriteMask);

      m_blend   = glIsEnabled(GL_BLEND);
      m_depth   = glIsEnabled(GL_DEPTH_TEST);
      m_cull    = glIsEnabled(GL_CULL_FACE);
      m_scissor = glIsEnabled(GL_SCISSOR_TEST);

      for (int i = 0; i < STATE_GUARD_VERTEX_ATTRIB_COUNT; ++i) {
         glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, (GLint *) &m_vertexAttribs[i].enabled);
         glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, (GLint *) &m_vertexAttribs[i].arrayBuffer);
         glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &m_vertexAttribs[i].size);
         glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &m_vertexAttribs[i].stride);
         glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, (GLint *) &m_vertexAttribs[i].type);
         glFuncs.glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, (GLint *) &m_vertexAttribs[i].normalized);
         glFuncs.glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &m_vertexAttribs[i].pointer);
      }

      glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint *) &m_minFilter);
      glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint *) &m_magFilter);
      glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint *) &m_wrapS);
      glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint *) &m_wrapT);
   }

   ~EGL_StateGuard() {
      QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());

      glFuncs.glUseProgram(m_program);
      glActiveTexture(m_activeTextureUnit);
      glBindTexture(GL_TEXTURE_2D, m_texture);

      glFuncs.glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
      glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
      glDepthMask(m_depthWriteMask);
      glColorMask(m_colorWriteMask[0], m_colorWriteMask[1], m_colorWriteMask[2], m_colorWriteMask[3]);

      if (m_blend) {
         glEnable(GL_BLEND);
      }

      if (m_depth) {
         glEnable(GL_DEPTH_TEST);
      }

      if (m_cull) {
         glEnable(GL_CULL_FACE);
      }

      if (m_scissor) {
         glEnable(GL_SCISSOR_TEST);
      }

      for (int i = 0; i < STATE_GUARD_VERTEX_ATTRIB_COUNT; ++i) {
         if (m_vertexAttribs[i].enabled) {
            glFuncs.glEnableVertexAttribArray(i);
         }

         GLuint prevBuf;
         glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint *) &prevBuf);
         glFuncs.glBindBuffer(GL_ARRAY_BUFFER, m_vertexAttribs[i].arrayBuffer);

         glFuncs.glVertexAttribPointer(i, m_vertexAttribs[i].size, m_vertexAttribs[i].type,
               m_vertexAttribs[i].normalized, m_vertexAttribs[i].stride, m_vertexAttribs[i].pointer);

         glFuncs.glBindBuffer(GL_ARRAY_BUFFER, prevBuf);
      }

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
   }

 private:
   GLuint m_program;
   GLenum m_activeTextureUnit;
   GLuint m_texture;
   GLuint m_fbo;
   GLint m_depthWriteMask;
   GLint m_colorWriteMask[4];
   GLboolean m_blend;
   GLboolean m_depth;
   GLboolean m_cull;
   GLboolean m_scissor;
   GLint m_viewport[4];

   struct VertexAttrib {
      bool enabled;
      GLuint arrayBuffer;
      GLint size;
      GLint stride;
      GLenum type;
      bool normalized;
      void *pointer;
   } m_vertexAttribs[STATE_GUARD_VERTEX_ATTRIB_COUNT];

   GLenum m_minFilter;
   GLenum m_magFilter;
   GLenum m_wrapS;
   GLenum m_wrapT;
};

}

#endif
