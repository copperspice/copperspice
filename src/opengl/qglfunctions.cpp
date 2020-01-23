/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qglfunctions.h>

#include <qgl_p.h>
#include <qopenglcontext_p.h>
#include <qopengl_p.h>


// Hidden private fields for additional extension data.
struct QGLFunctionsPrivateEx : public QGLFunctionsPrivate, public QOpenGLSharedResource {
   QGLFunctionsPrivateEx(QOpenGLContext *context)
      : QGLFunctionsPrivate(QGLContext::fromOpenGLContext(context))
      , QOpenGLSharedResource(context->shareGroup())
      , m_features(-1) {
      funcs = new QOpenGLFunctions(context);
      funcs->initializeOpenGLFunctions();
   }

   ~QGLFunctionsPrivateEx() {
      delete funcs;
   }

   void invalidateResource() override {
      m_features = -1;
   }

   void freeResource(QOpenGLContext *) override {
      // no gl resources to free
   }

   int m_features;
};

Q_GLOBAL_STATIC(QOpenGLMultiGroupSharedResource, qt_gl_functions_resource)

static QGLFunctionsPrivateEx *qt_gl_functions(const QGLContext *context = 0)
{
   if (!context) {
      context = QGLContext::currentContext();
   }
   Q_ASSERT(context);
   QGLFunctionsPrivateEx *funcs =
      reinterpret_cast<QGLFunctionsPrivateEx *>
      (qt_gl_functions_resource()->value<QGLFunctionsPrivateEx>(context->contextHandle()));
   return funcs;
}

/*!
    Constructs a default function resolver.  The resolver cannot
    be used until initializeGLFunctions() is called to specify
    the context.

    \sa initializeGLFunctions()
*/
QGLFunctions::QGLFunctions()
   : d_ptr(0)
{
}

/*!
    Constructs a function resolver for \a context.  If \a context
    is null, then the resolver will be created for the current QGLContext.

    An object constructed in this way can only be used with \a context
    and other contexts that share with it.  Use initializeGLFunctions()
    to change the object's context association.

    \sa initializeGLFunctions()
*/
QGLFunctions::QGLFunctions(const QGLContext *context)
   : d_ptr(qt_gl_functions(context))
{
}

/*!
    \fn QGLFunctions::~QGLFunctions()

    Destroys this function resolver.
*/

static int qt_gl_resolve_features()
{
   QOpenGLContext *ctx = QOpenGLContext::currentContext();
   if (ctx->isOpenGLES()) {
      // OpenGL ES 2
      int features = QGLFunctions::Multitexture |
         QGLFunctions::Shaders |
         QGLFunctions::Buffers |
         QGLFunctions::Framebuffers |
         QGLFunctions::BlendColor |
         QGLFunctions::BlendEquation |
         QGLFunctions::BlendEquationSeparate |
         QGLFunctions::BlendFuncSeparate |
         QGLFunctions::BlendSubtract |
         QGLFunctions::CompressedTextures |
         QGLFunctions::Multisample |
         QGLFunctions::StencilSeparate;
      QOpenGLExtensionMatcher extensions;
      if (extensions.match("GL_OES_texture_npot")) {
         features |= QGLFunctions::NPOTTextures;
      }
      if (extensions.match("GL_IMG_texture_npot")) {
         features |= QGLFunctions::NPOTTextures;
      }
      return features;
   } else {
      // OpenGL
      int features = 0;
      QGLFormat::OpenGLVersionFlags versions = QGLFormat::openGLVersionFlags();
      QOpenGLExtensionMatcher extensions;

      // Recognize features by extension name.
      if (extensions.match("GL_ARB_multitexture")) {
         features |= QGLFunctions::Multitexture;
      }
      if (extensions.match("GL_ARB_shader_objects")) {
         features |= QGLFunctions::Shaders;
      }
      if (extensions.match("GL_EXT_framebuffer_object") ||
         extensions.match("GL_ARB_framebuffer_object")) {
         features |= QGLFunctions::Framebuffers;
      }
      if (extensions.match("GL_EXT_blend_color")) {
         features |= QGLFunctions::BlendColor;
      }
      if (extensions.match("GL_EXT_blend_equation_separate")) {
         features |= QGLFunctions::BlendEquationSeparate;
      }
      if (extensions.match("GL_EXT_blend_func_separate")) {
         features |= QGLFunctions::BlendFuncSeparate;
      }
      if (extensions.match("GL_EXT_blend_subtract")) {
         features |= QGLFunctions::BlendSubtract;
      }
      if (extensions.match("GL_ARB_texture_compression")) {
         features |= QGLFunctions::CompressedTextures;
      }
      if (extensions.match("GL_ARB_multisample")) {
         features |= QGLFunctions::Multisample;
      }
      if (extensions.match("GL_ARB_texture_non_power_of_two")) {
         features |= QGLFunctions::NPOTTextures;
      }

      // Recognize features by minimum OpenGL version.
      if (versions & QGLFormat::OpenGL_Version_1_2) {
         features |= QGLFunctions::BlendColor |
            QGLFunctions::BlendEquation;
      }
      if (versions & QGLFormat::OpenGL_Version_1_3) {
         features |= QGLFunctions::Multitexture |
            QGLFunctions::CompressedTextures |
            QGLFunctions::Multisample;
      }
      if (versions & QGLFormat::OpenGL_Version_1_4) {
         features |= QGLFunctions::BlendFuncSeparate;
      }
      if (versions & QGLFormat::OpenGL_Version_1_5) {
         features |= QGLFunctions::Buffers;
      }
      if (versions & QGLFormat::OpenGL_Version_2_0) {
         features |= QGLFunctions::Shaders |
            QGLFunctions::StencilSeparate |
            QGLFunctions::BlendEquationSeparate |
            QGLFunctions::NPOTTextures;
      }
      return features;
   }
}

/*!
    Returns the set of features that are present on this system's
    OpenGL implementation.

    It is assumed that the QGLContext associated with this function
    resolver is current.

    \sa hasOpenGLFeature()
*/
QGLFunctions::OpenGLFeatures QGLFunctions::openGLFeatures() const
{
   QGLFunctionsPrivateEx *d = static_cast<QGLFunctionsPrivateEx *>(d_ptr);
   if (!d) {
      return 0;
   }
   if (d->m_features == -1) {
      d->m_features = qt_gl_resolve_features();
   }
   return QGLFunctions::OpenGLFeatures(d->m_features);
}

/*!
    Returns \c true if \a feature is present on this system's OpenGL
    implementation; false otherwise.

    It is assumed that the QGLContext associated with this function
    resolver is current.

    \sa openGLFeatures()
*/
bool QGLFunctions::hasOpenGLFeature(QGLFunctions::OpenGLFeature feature) const
{
   QGLFunctionsPrivateEx *d = static_cast<QGLFunctionsPrivateEx *>(d_ptr);
   if (!d) {
      return false;
   }
   if (d->m_features == -1) {
      d->m_features = qt_gl_resolve_features();
   }
   return (d->m_features & int(feature)) != 0;
}

/*!
    Initializes GL function resolution for \a context.  If \a context
    is null, then the current QGLContext will be used.

    After calling this function, the QGLFunctions object can only be
    used with \a context and other contexts that share with it.
    Call initializeGLFunctions() again to change the object's context
    association.
*/
void QGLFunctions::initializeGLFunctions(const QGLContext *context)
{
   d_ptr = qt_gl_functions(context);
}

QGLFunctionsPrivate::QGLFunctionsPrivate(const QGLContext *)
   : funcs(0)
{
}


