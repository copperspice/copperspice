/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qopenglversionfunctions.h>
#include <qopenglcontext.h>
#include <qdebug.h>

QOpenGLVersionFunctionsBackend *QAbstractOpenGLFunctionsPrivate::functionsBackend(QOpenGLContext *context,
                                                                                  const QOpenGLVersionStatus &v)
{
    Q_ASSERT(context);
    return context->functionsBackend(v);
}

void QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(QOpenGLContext *context,
                                                             const QOpenGLVersionStatus &v,
                                                             QOpenGLVersionFunctionsBackend *backend)
{
    Q_ASSERT(context);
    context->insertFunctionsBackend(v, backend);
}

void QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(QOpenGLContext *context, const QOpenGLVersionStatus &v)
{
    Q_ASSERT(context);
    context->removeFunctionsBackend(v);
}

void QAbstractOpenGLFunctionsPrivate::insertExternalFunctions(QOpenGLContext *context, QAbstractOpenGLFunctions *f)
{
    Q_ASSERT(context);
    context->insertExternalFunctions(f);
}

void QAbstractOpenGLFunctionsPrivate::removeExternalFunctions(QOpenGLContext *context, QAbstractOpenGLFunctions *f)
{
    Q_ASSERT(context);
    context->removeExternalFunctions(f);
}

/*!
    \class QAbstractOpenGLFunctions
    \inmodule QtGui
    \since 5.1
    \brief The QAbstractOpenGLFunctions class is the base class of a family of
           classes that expose all functions for each OpenGL version and
           profile.

    OpenGL implementations on different platforms are able to link to a variable
    number of OpenGL functions depending upon the OpenGL ABI on that platform.
    For example, on Microsoft Windows only functions up to those in OpenGL 1.1
    can be linked to at build time. All other functions must be resolved at
    runtime. The traditional solution to this has been to use either
    QOpenGLContext::getProcAddress() or QOpenGLFunctions. The former is tedious
    and error prone and means dealing directly with function pointers. The
    latter only exposes those functions common to OpenGL ES 2 and desktop
    OpenGL. There is however much new OpenGL functionality that is useful when
    writing real world OpenGL applications.

    Qt now provides a family of classes which all inherit from
    QAbstractOpenGLFunctions which expose every core OpenGL function by way of a
    corresponding member function. There is a class for every valid combination
    of OpenGL version and profile. Each class follows the naming convention:
    \badcode
    QOpenGLFunctions_<MAJOR VERSION>_<MINOR VERSION>[_PROFILE]
    \endcode

    For OpenGL versions 1.0 through to 3.0 there are no profiles, leading to the
    classes:

    \list
        \li QOpenGLFunctions_1_0
        \li QOpenGLFunctions_1_1
        \li QOpenGLFunctions_1_2
        \li QOpenGLFunctions_1_3
        \li QOpenGLFunctions_1_4
        \li QOpenGLFunctions_1_5
        \li QOpenGLFunctions_2_0
        \li QOpenGLFunctions_2_1
        \li QOpenGLFunctions_3_0
    \endlist

    where each class inherits from QAbstractOpenGLFunctions.

    OpenGL version 3.1 removed many deprecated functions leading to a much
    simpler and generic API.

    With OpenGL 3.2 the concept of profiles was introduced. Two profiles are
    currently defined for OpenGL: Core and Compatibility.

    The Core profile does not include any of the functions that were removed
    in OpenGL 3.1. The Compatibility profile contains all functions in the
    Core profile of the same version plus all of the functions that were
    removed in OpenGL 3.1. In this way the Compatibility profile classes allow
    use of newer OpenGL functionality but also allows you to keep using your
    legacy OpenGL code. For new OpenGL code the Core profile should be
    preferred.

    Please note that some vendors, notably Apple, do not implement the
    Compatibility profile. Therefore if you wish to target new OpenGL features
    on \macos then you should ensure that you request a Core profile context via
    QSurfaceFormat::setProfile().

    Qt provides classes for all version and Core and Compatibility profile
    combinations. The classes for OpenGL versions 3.1 through to 4.3 are:

    \list
        \li QOpenGLFunctions_3_1
        \li QOpenGLFunctions_3_2_Core
        \li QOpenGLFunctions_3_2_Compatibility
        \li QOpenGLFunctions_3_3_Core
        \li QOpenGLFunctions_3_3_Compatibility
        \li QOpenGLFunctions_4_0_Core
        \li QOpenGLFunctions_4_0_Compatibility
        \li QOpenGLFunctions_4_1_Core
        \li QOpenGLFunctions_4_1_Compatibility
        \li QOpenGLFunctions_4_2_Core
        \li QOpenGLFunctions_4_2_Compatibility
        \li QOpenGLFunctions_4_3_Core
        \li QOpenGLFunctions_4_3_Compatibility
    \endlist

    where each class inherits from QAbstractOpenGLFunctions.

    A pointer to an object of the class corresponding to the version and
    profile of OpenGL in use can be obtained from
    QOpenGLContext::versionFunctions(). If obtained in this way, note that
    the QOpenGLContext retains ownership of the object. This is so that only
    one instance need be created.

    Before calling any of the exposed OpenGL functions you must ensure that the
    object has resolved the function pointers to the OpenGL functions. This
    only needs to be done once per instance with initializeOpenGLFunctions().
    Once initialized, the object can be used to call any OpenGL function for
    the corresponding version and profile. Note that initializeOpenGLFunctions()
    can fail in some circumstances so check the return value. Situations in
    which initialization can fail are if you have a functions object for a version
    or profile that contains functions that are not part of the context being
    used to resolve the function pointers.

    If you exclusively use function objects then you will get compile time
    errors if you attempt to use a function not included in that version and
    profile. This is obviously a lot easier to debug than undefined behavior
    at run time.

    \sa QOpenGLContext::versionFunctions()
*/
/*!
   Constructs a QAbstractOpenGLFunctions object.
*/
QAbstractOpenGLFunctions::QAbstractOpenGLFunctions()
    : d_ptr(new QAbstractOpenGLFunctionsPrivate)
{
}

/*!
   Destroys a QAbstractOpenGLFunctions object.
*/
QAbstractOpenGLFunctions::~QAbstractOpenGLFunctions()
{
    Q_D(QAbstractOpenGLFunctions);
    if (d->owningContext)
        d->removeExternalFunctions(d->owningContext, this);
    delete d_ptr;
}

/*! \internal
 */
bool QAbstractOpenGLFunctions::initializeOpenGLFunctions()
{
    Q_D(QAbstractOpenGLFunctions);
    d->initialized = true;

    // For a subclass whose instance is not created via
    // QOpenGLContext::versionFunctions() owningContext is not set. Set it now
    // and register such instances to the context as external ones. These are
    // not owned by the context but still need certain cleanup when the context
    // is destroyed.
    if (!d->owningContext) {
        d->owningContext = QOpenGLContext::currentContext();
        if (d->owningContext)
            d->insertExternalFunctions(d->owningContext, this);
    }

    return true;
}

/*! \internal
 */
bool QAbstractOpenGLFunctions::isInitialized() const
{
    Q_D(const QAbstractOpenGLFunctions);
    return d->initialized;
}

/*! \internal
 */
void QAbstractOpenGLFunctions::setOwningContext(const QOpenGLContext *context)
{
    Q_D(QAbstractOpenGLFunctions);
    d->owningContext = const_cast<QOpenGLContext*>(context);
}

/*! \internal
 */
QOpenGLContext *QAbstractOpenGLFunctions::owningContext() const
{
    Q_D(const QAbstractOpenGLFunctions);
    return d->owningContext;
}

#if !defined(QT_OPENGL_ES_2)

QOpenGLFunctions_1_0_CoreBackend::QOpenGLFunctions_1_0_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.0 core functions
#if defined(Q_OS_WIN)
    HMODULE handle = static_cast<HMODULE>(QOpenGLContext::openGLModuleHandle());
    if (!handle)
        handle = GetModuleHandleA("opengl32.dll");
    Viewport = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei )>(GetProcAddress(handle, "glViewport"));
    DepthRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(GetProcAddress(handle, "glDepthRange"));
    IsEnabled = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glIsEnabled"));
    GetTexLevelParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLint *)>(GetProcAddress(handle, "glGetTexLevelParameteriv"));
    GetTexLevelParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLfloat *)>(GetProcAddress(handle, "glGetTexLevelParameterfv"));
    GetTexParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(GetProcAddress(handle, "glGetTexParameteriv"));
    GetTexParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(GetProcAddress(handle, "glGetTexParameterfv"));
    GetTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLenum , GLvoid *)>(GetProcAddress(handle, "glGetTexImage"));
    GetString = reinterpret_cast<const GLubyte * (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glGetString"));
    GetIntegerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint *)>(GetProcAddress(handle, "glGetIntegerv"));
    GetFloatv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat *)>(GetProcAddress(handle, "glGetFloatv"));
    GetError = reinterpret_cast<GLenum (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glGetError"));
    GetDoublev = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble *)>(GetProcAddress(handle, "glGetDoublev"));
    GetBooleanv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLboolean *)>(GetProcAddress(handle, "glGetBooleanv"));
    ReadPixels = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , GLvoid *)>(GetProcAddress(handle, "glReadPixels"));
    ReadBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glReadBuffer"));
    PixelStorei = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(GetProcAddress(handle, "glPixelStorei"));
    PixelStoref = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(GetProcAddress(handle, "glPixelStoref"));
    DepthFunc = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glDepthFunc"));
    StencilOp = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum )>(GetProcAddress(handle, "glStencilOp"));
    StencilFunc = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLuint )>(GetProcAddress(handle, "glStencilFunc"));
    LogicOp = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glLogicOp"));
    BlendFunc = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(GetProcAddress(handle, "glBlendFunc"));
    Flush = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glFlush"));
    Finish = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glFinish"));
    Enable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glEnable"));
    Disable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glDisable"));
    DepthMask = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLboolean )>(GetProcAddress(handle, "glDepthMask"));
    ColorMask = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLboolean , GLboolean , GLboolean , GLboolean )>(GetProcAddress(handle, "glColorMask"));
    StencilMask = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(GetProcAddress(handle, "glStencilMask"));
    ClearDepth = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(GetProcAddress(handle, "glClearDepth"));
    ClearStencil = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(GetProcAddress(handle, "glClearStencil"));
    ClearColor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glClearColor"));
    Clear = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbitfield )>(GetProcAddress(handle, "glClear"));
    DrawBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glDrawBuffer"));
    TexImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLsizei , GLint , GLenum , GLenum , const GLvoid *)>(GetProcAddress(handle, "glTexImage2D"));
    TexImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLint , GLenum , GLenum , const GLvoid *)>(GetProcAddress(handle, "glTexImage1D"));
    TexParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(GetProcAddress(handle, "glTexParameteriv"));
    TexParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(GetProcAddress(handle, "glTexParameteri"));
    TexParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(GetProcAddress(handle, "glTexParameterfv"));
    TexParameterf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(GetProcAddress(handle, "glTexParameterf"));
    Scissor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei )>(GetProcAddress(handle, "glScissor"));
    PolygonMode = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(GetProcAddress(handle, "glPolygonMode"));
    PointSize = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(GetProcAddress(handle, "glPointSize"));
    LineWidth = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(GetProcAddress(handle, "glLineWidth"));
    Hint = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(GetProcAddress(handle, "glHint"));
    FrontFace = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glFrontFace"));
    CullFace = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glCullFace"));
#else
    Viewport = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glViewport"));
    DepthRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(context->getProcAddress("glDepthRange"));
    IsEnabled = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glIsEnabled"));
    GetTexLevelParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLint *)>(context->getProcAddress("glGetTexLevelParameteriv"));
    GetTexLevelParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLfloat *)>(context->getProcAddress("glGetTexLevelParameterfv"));
    GetTexParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetTexParameteriv"));
    GetTexParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetTexParameterfv"));
    GetTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLenum , GLvoid *)>(context->getProcAddress("glGetTexImage"));
    GetString = reinterpret_cast<const GLubyte * (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glGetString"));
    GetIntegerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint *)>(context->getProcAddress("glGetIntegerv"));
    GetFloatv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat *)>(context->getProcAddress("glGetFloatv"));
    GetError = reinterpret_cast<GLenum (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glGetError"));
    GetDoublev = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble *)>(context->getProcAddress("glGetDoublev"));
    GetBooleanv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLboolean *)>(context->getProcAddress("glGetBooleanv"));
    ReadPixels = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , GLvoid *)>(context->getProcAddress("glReadPixels"));
    ReadBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glReadBuffer"));
    PixelStorei = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(context->getProcAddress("glPixelStorei"));
    PixelStoref = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(context->getProcAddress("glPixelStoref"));
    DepthFunc = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glDepthFunc"));
    StencilOp = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum )>(context->getProcAddress("glStencilOp"));
    StencilFunc = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLuint )>(context->getProcAddress("glStencilFunc"));
    LogicOp = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glLogicOp"));
    BlendFunc = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glBlendFunc"));
    Flush = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glFlush"));
    Finish = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glFinish"));
    Enable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glEnable"));
    Disable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glDisable"));
    DepthMask = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLboolean )>(context->getProcAddress("glDepthMask"));
    ColorMask = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLboolean , GLboolean , GLboolean , GLboolean )>(context->getProcAddress("glColorMask"));
    StencilMask = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glStencilMask"));
    ClearDepth = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(context->getProcAddress("glClearDepth"));
    ClearStencil = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(context->getProcAddress("glClearStencil"));
    ClearColor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glClearColor"));
    Clear = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbitfield )>(context->getProcAddress("glClear"));
    DrawBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glDrawBuffer"));
    TexImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLsizei , GLint , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexImage2D"));
    TexImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLint , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexImage1D"));
    TexParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glTexParameteriv"));
    TexParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(context->getProcAddress("glTexParameteri"));
    TexParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(context->getProcAddress("glTexParameterfv"));
    TexParameterf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(context->getProcAddress("glTexParameterf"));
    Scissor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glScissor"));
    PolygonMode = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glPolygonMode"));
    PointSize = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glPointSize"));
    LineWidth = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glLineWidth"));
    Hint = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glHint"));
    FrontFace = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glFrontFace"));
    CullFace = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glCullFace"));
#endif

}

QOpenGLVersionStatus QOpenGLFunctions_1_0_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 0, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_1_1_CoreBackend::QOpenGLFunctions_1_1_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.1 core functions
#if defined(Q_OS_WIN)
    HMODULE handle = static_cast<HMODULE>(QOpenGLContext::openGLModuleHandle());
    if (!handle)
        handle = GetModuleHandleA("opengl32.dll");
    Indexubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(GetProcAddress(handle, "glIndexubv"));
    Indexub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte )>(GetProcAddress(handle, "glIndexub"));
    IsTexture = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(GetProcAddress(handle, "glIsTexture"));
    GenTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(GetProcAddress(handle, "glGenTextures"));
    DeleteTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(GetProcAddress(handle, "glDeleteTextures"));
    BindTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(GetProcAddress(handle, "glBindTexture"));
    TexSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(GetProcAddress(handle, "glTexSubImage2D"));
    TexSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLenum , GLenum , const GLvoid *)>(GetProcAddress(handle, "glTexSubImage1D"));
    CopyTexSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLint , GLsizei , GLsizei )>(GetProcAddress(handle, "glCopyTexSubImage2D"));
    CopyTexSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLsizei )>(GetProcAddress(handle, "glCopyTexSubImage1D"));
    CopyTexImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLint , GLint , GLsizei , GLsizei , GLint )>(GetProcAddress(handle, "glCopyTexImage2D"));
    CopyTexImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLint , GLint , GLsizei , GLint )>(GetProcAddress(handle, "glCopyTexImage1D"));
    PolygonOffset = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(GetProcAddress(handle, "glPolygonOffset"));
    GetPointerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLvoid * *)>(GetProcAddress(handle, "glGetPointerv"));
    DrawElements = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , const GLvoid *)>(GetProcAddress(handle, "glDrawElements"));
    DrawArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLsizei )>(GetProcAddress(handle, "glDrawArrays"));
#else
    Indexubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(context->getProcAddress("glIndexubv"));
    Indexub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte )>(context->getProcAddress("glIndexub"));
    IsTexture = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsTexture"));
    GenTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenTextures"));
    DeleteTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteTextures"));
    BindTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glBindTexture"));
    TexSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexSubImage2D"));
    TexSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexSubImage1D"));
    CopyTexSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glCopyTexSubImage2D"));
    CopyTexSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLsizei )>(context->getProcAddress("glCopyTexSubImage1D"));
    CopyTexImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLint , GLint , GLsizei , GLsizei , GLint )>(context->getProcAddress("glCopyTexImage2D"));
    CopyTexImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLint , GLint , GLsizei , GLint )>(context->getProcAddress("glCopyTexImage1D"));
    PolygonOffset = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(context->getProcAddress("glPolygonOffset"));
    GetPointerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLvoid * *)>(context->getProcAddress("glGetPointerv"));
    DrawElements = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , const GLvoid *)>(context->getProcAddress("glDrawElements"));
    DrawArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLsizei )>(context->getProcAddress("glDrawArrays"));
#endif

}

QOpenGLVersionStatus QOpenGLFunctions_1_1_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 1, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_1_2_CoreBackend::QOpenGLFunctions_1_2_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.2 core functions
    CopyTexSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glCopyTexSubImage3D"));
    TexSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexSubImage3D"));
    TexImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLsizei , GLsizei , GLint , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexImage3D"));
    DrawRangeElements = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLuint , GLsizei , GLenum , const GLvoid *)>(context->getProcAddress("glDrawRangeElements"));
    BlendEquation = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glBlendEquation"));
    BlendColor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glBlendColor"));

}

QOpenGLVersionStatus QOpenGLFunctions_1_2_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 2, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_1_3_CoreBackend::QOpenGLFunctions_1_3_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.3 core functions
    GetCompressedTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLvoid *)>(context->getProcAddress("glGetCompressedTexImage"));
    CompressedTexSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexSubImage1D"));
    CompressedTexSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexSubImage2D"));
    CompressedTexSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexSubImage3D"));
    CompressedTexImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLint , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexImage1D"));
    CompressedTexImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLsizei , GLint , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexImage2D"));
    CompressedTexImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLsizei , GLsizei , GLint , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexImage3D"));
    SampleCoverage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLboolean )>(context->getProcAddress("glSampleCoverage"));
    ActiveTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glActiveTexture"));

}

QOpenGLVersionStatus QOpenGLFunctions_1_3_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 3, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_1_4_CoreBackend::QOpenGLFunctions_1_4_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.4 core functions
    PointParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(context->getProcAddress("glPointParameteriv"));
    PointParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(context->getProcAddress("glPointParameteri"));
    PointParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(context->getProcAddress("glPointParameterfv"));
    PointParameterf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(context->getProcAddress("glPointParameterf"));
    MultiDrawElements = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLsizei *, GLenum , const GLvoid* const *, GLsizei )>(context->getProcAddress("glMultiDrawElements"));
    MultiDrawArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *, const GLsizei *, GLsizei )>(context->getProcAddress("glMultiDrawArrays"));
    BlendFuncSeparate = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLenum )>(context->getProcAddress("glBlendFuncSeparate"));

}

QOpenGLVersionStatus QOpenGLFunctions_1_4_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 4, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_1_5_CoreBackend::QOpenGLFunctions_1_5_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.5 core functions
    GetBufferPointerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLvoid* *)>(context->getProcAddress("glGetBufferPointerv"));
    GetBufferParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetBufferParameteriv"));
    UnmapBuffer = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glUnmapBuffer"));
    MapBuffer = reinterpret_cast<GLvoid* (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glMapBuffer"));
    GetBufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLintptr , GLsizeiptr , GLvoid *)>(context->getProcAddress("glGetBufferSubData"));
    BufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLintptr , GLsizeiptr , const GLvoid *)>(context->getProcAddress("glBufferSubData"));
    BufferData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizeiptr , const GLvoid *, GLenum )>(context->getProcAddress("glBufferData"));
    IsBuffer = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsBuffer"));
    GenBuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenBuffers"));
    DeleteBuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteBuffers"));
    BindBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glBindBuffer"));
    GetQueryObjectuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint *)>(context->getProcAddress("glGetQueryObjectuiv"));
    GetQueryObjectiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetQueryObjectiv"));
    GetQueryiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetQueryiv"));
    EndQuery = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glEndQuery"));
    BeginQuery = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glBeginQuery"));
    IsQuery = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsQuery"));
    DeleteQueries = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteQueries"));
    GenQueries = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenQueries"));

}

QOpenGLVersionStatus QOpenGLFunctions_1_5_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 5, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_2_0_CoreBackend::QOpenGLFunctions_2_0_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 2.0 core functions
    VertexAttribPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLboolean , GLsizei , const GLvoid *)>(context->getProcAddress("glVertexAttribPointer"));
    VertexAttrib4usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLushort *)>(context->getProcAddress("glVertexAttrib4usv"));
    VertexAttrib4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttrib4uiv"));
    VertexAttrib4ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLubyte *)>(context->getProcAddress("glVertexAttrib4ubv"));
    VertexAttrib4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib4sv"));
    VertexAttrib4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLshort , GLshort , GLshort , GLshort )>(context->getProcAddress("glVertexAttrib4s"));
    VertexAttrib4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttrib4iv"));
    VertexAttrib4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glVertexAttrib4fv"));
    VertexAttrib4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glVertexAttrib4f"));
    VertexAttrib4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttrib4dv"));
    VertexAttrib4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttrib4d"));
    VertexAttrib4bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLbyte *)>(context->getProcAddress("glVertexAttrib4bv"));
    VertexAttrib4Nusv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLushort *)>(context->getProcAddress("glVertexAttrib4Nusv"));
    VertexAttrib4Nuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttrib4Nuiv"));
    VertexAttrib4Nubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLubyte *)>(context->getProcAddress("glVertexAttrib4Nubv"));
    VertexAttrib4Nub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLubyte , GLubyte , GLubyte , GLubyte )>(context->getProcAddress("glVertexAttrib4Nub"));
    VertexAttrib4Nsv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib4Nsv"));
    VertexAttrib4Niv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttrib4Niv"));
    VertexAttrib4Nbv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLbyte *)>(context->getProcAddress("glVertexAttrib4Nbv"));
    VertexAttrib3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib3sv"));
    VertexAttrib3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLshort , GLshort , GLshort )>(context->getProcAddress("glVertexAttrib3s"));
    VertexAttrib3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glVertexAttrib3fv"));
    VertexAttrib3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glVertexAttrib3f"));
    VertexAttrib3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttrib3dv"));
    VertexAttrib3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttrib3d"));
    VertexAttrib2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib2sv"));
    VertexAttrib2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLshort , GLshort )>(context->getProcAddress("glVertexAttrib2s"));
    VertexAttrib2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glVertexAttrib2fv"));
    VertexAttrib2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat , GLfloat )>(context->getProcAddress("glVertexAttrib2f"));
    VertexAttrib2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttrib2dv"));
    VertexAttrib2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttrib2d"));
    VertexAttrib1sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib1sv"));
    VertexAttrib1s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLshort )>(context->getProcAddress("glVertexAttrib1s"));
    VertexAttrib1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glVertexAttrib1fv"));
    VertexAttrib1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat )>(context->getProcAddress("glVertexAttrib1f"));
    VertexAttrib1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttrib1dv"));
    VertexAttrib1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble )>(context->getProcAddress("glVertexAttrib1d"));
    ValidateProgram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glValidateProgram"));
    UniformMatrix4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix4fv"));
    UniformMatrix3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix3fv"));
    UniformMatrix2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix2fv"));
    Uniform4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLint *)>(context->getProcAddress("glUniform4iv"));
    Uniform3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLint *)>(context->getProcAddress("glUniform3iv"));
    Uniform2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLint *)>(context->getProcAddress("glUniform2iv"));
    Uniform1iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLint *)>(context->getProcAddress("glUniform1iv"));
    Uniform4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLfloat *)>(context->getProcAddress("glUniform4fv"));
    Uniform3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLfloat *)>(context->getProcAddress("glUniform3fv"));
    Uniform2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLfloat *)>(context->getProcAddress("glUniform2fv"));
    Uniform1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLfloat *)>(context->getProcAddress("glUniform1fv"));
    Uniform4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint , GLint )>(context->getProcAddress("glUniform4i"));
    Uniform3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(context->getProcAddress("glUniform3i"));
    Uniform2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(context->getProcAddress("glUniform2i"));
    Uniform1i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(context->getProcAddress("glUniform1i"));
    Uniform4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glUniform4f"));
    Uniform3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glUniform3f"));
    Uniform2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLfloat , GLfloat )>(context->getProcAddress("glUniform2f"));
    Uniform1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLfloat )>(context->getProcAddress("glUniform1f"));
    UseProgram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glUseProgram"));
    ShaderSource = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLchar* const *, const GLint *)>(context->getProcAddress("glShaderSource"));
    LinkProgram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glLinkProgram"));
    IsShader = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsShader"));
    IsProgram = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsProgram"));
    GetVertexAttribPointerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLvoid* *)>(context->getProcAddress("glGetVertexAttribPointerv"));
    GetVertexAttribiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetVertexAttribiv"));
    GetVertexAttribfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLfloat *)>(context->getProcAddress("glGetVertexAttribfv"));
    GetVertexAttribdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLdouble *)>(context->getProcAddress("glGetVertexAttribdv"));
    GetUniformiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint *)>(context->getProcAddress("glGetUniformiv"));
    GetUniformfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLfloat *)>(context->getProcAddress("glGetUniformfv"));
    GetUniformLocation = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLuint , const GLchar *)>(context->getProcAddress("glGetUniformLocation"));
    GetShaderSource = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetShaderSource"));
    GetShaderInfoLog = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetShaderInfoLog"));
    GetShaderiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetShaderiv"));
    GetProgramInfoLog = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetProgramInfoLog"));
    GetProgramiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetProgramiv"));
    GetAttribLocation = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLuint , const GLchar *)>(context->getProcAddress("glGetAttribLocation"));
    GetAttachedShaders = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLsizei *, GLuint *)>(context->getProcAddress("glGetAttachedShaders"));
    GetActiveUniform = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLsizei , GLsizei *, GLint *, GLenum *, GLchar *)>(context->getProcAddress("glGetActiveUniform"));
    GetActiveAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLsizei , GLsizei *, GLint *, GLenum *, GLchar *)>(context->getProcAddress("glGetActiveAttrib"));
    EnableVertexAttribArray = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glEnableVertexAttribArray"));
    DisableVertexAttribArray = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glDisableVertexAttribArray"));
    DetachShader = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glDetachShader"));
    DeleteShader = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glDeleteShader"));
    DeleteProgram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glDeleteProgram"));
    CreateShader = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glCreateShader"));
    CreateProgram = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glCreateProgram"));
    CompileShader = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glCompileShader"));
    BindAttribLocation = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , const GLchar *)>(context->getProcAddress("glBindAttribLocation"));
    AttachShader = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glAttachShader"));
    StencilMaskSeparate = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glStencilMaskSeparate"));
    StencilFuncSeparate = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint , GLuint )>(context->getProcAddress("glStencilFuncSeparate"));
    StencilOpSeparate = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLenum )>(context->getProcAddress("glStencilOpSeparate"));
    DrawBuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLenum *)>(context->getProcAddress("glDrawBuffers"));
    BlendEquationSeparate = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glBlendEquationSeparate"));

}

QOpenGLVersionStatus QOpenGLFunctions_2_0_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(2, 0, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_2_1_CoreBackend::QOpenGLFunctions_2_1_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 2.1 core functions
    UniformMatrix4x3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix4x3fv"));
    UniformMatrix3x4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix3x4fv"));
    UniformMatrix4x2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix4x2fv"));
    UniformMatrix2x4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix2x4fv"));
    UniformMatrix3x2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix3x2fv"));
    UniformMatrix2x3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glUniformMatrix2x3fv"));

}

QOpenGLVersionStatus QOpenGLFunctions_2_1_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(2, 1, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_3_0_CoreBackend::QOpenGLFunctions_3_0_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 3.0 core functions
    IsVertexArray = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsVertexArray"));
    GenVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenVertexArrays"));
    DeleteVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteVertexArrays"));
    BindVertexArray = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glBindVertexArray"));
    FlushMappedBufferRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLintptr , GLsizeiptr )>(context->getProcAddress("glFlushMappedBufferRange"));
    MapBufferRange = reinterpret_cast<GLvoid* (QOPENGLF_APIENTRYP)(GLenum , GLintptr , GLsizeiptr , GLbitfield )>(context->getProcAddress("glMapBufferRange"));
    FramebufferTextureLayer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint , GLint , GLint )>(context->getProcAddress("glFramebufferTextureLayer"));
    RenderbufferStorageMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei )>(context->getProcAddress("glRenderbufferStorageMultisample"));
    BlitFramebuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint , GLint , GLint , GLint , GLint , GLbitfield , GLenum )>(context->getProcAddress("glBlitFramebuffer"));
    GenerateMipmap = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glGenerateMipmap"));
    GetFramebufferAttachmentParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLint *)>(context->getProcAddress("glGetFramebufferAttachmentParameteriv"));
    FramebufferRenderbuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLuint )>(context->getProcAddress("glFramebufferRenderbuffer"));
    FramebufferTexture3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLuint , GLint , GLint )>(context->getProcAddress("glFramebufferTexture3D"));
    FramebufferTexture2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLuint , GLint )>(context->getProcAddress("glFramebufferTexture2D"));
    FramebufferTexture1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLuint , GLint )>(context->getProcAddress("glFramebufferTexture1D"));
    CheckFramebufferStatus = reinterpret_cast<GLenum (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glCheckFramebufferStatus"));
    GenFramebuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenFramebuffers"));
    DeleteFramebuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteFramebuffers"));
    BindFramebuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glBindFramebuffer"));
    IsFramebuffer = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsFramebuffer"));
    GetRenderbufferParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetRenderbufferParameteriv"));
    RenderbufferStorage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLsizei , GLsizei )>(context->getProcAddress("glRenderbufferStorage"));
    GenRenderbuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenRenderbuffers"));
    DeleteRenderbuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteRenderbuffers"));
    BindRenderbuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glBindRenderbuffer"));
    IsRenderbuffer = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsRenderbuffer"));
    GetStringi = reinterpret_cast<const GLubyte * (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glGetStringi"));
    ClearBufferfi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLfloat , GLint )>(context->getProcAddress("glClearBufferfi"));
    ClearBufferfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , const GLfloat *)>(context->getProcAddress("glClearBufferfv"));
    ClearBufferuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , const GLuint *)>(context->getProcAddress("glClearBufferuiv"));
    ClearBufferiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , const GLint *)>(context->getProcAddress("glClearBufferiv"));
    GetTexParameterIuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint *)>(context->getProcAddress("glGetTexParameterIuiv"));
    GetTexParameterIiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetTexParameterIiv"));
    TexParameterIuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glTexParameterIuiv"));
    TexParameterIiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glTexParameterIiv"));
    Uniform4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLuint *)>(context->getProcAddress("glUniform4uiv"));
    Uniform3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLuint *)>(context->getProcAddress("glUniform3uiv"));
    Uniform2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLuint *)>(context->getProcAddress("glUniform2uiv"));
    Uniform1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLuint *)>(context->getProcAddress("glUniform1uiv"));
    Uniform4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glUniform4ui"));
    Uniform3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLuint , GLuint , GLuint )>(context->getProcAddress("glUniform3ui"));
    Uniform2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLuint , GLuint )>(context->getProcAddress("glUniform2ui"));
    Uniform1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLuint )>(context->getProcAddress("glUniform1ui"));
    GetFragDataLocation = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLuint , const GLchar *)>(context->getProcAddress("glGetFragDataLocation"));
    BindFragDataLocation = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , const GLchar *)>(context->getProcAddress("glBindFragDataLocation"));
    GetUniformuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLuint *)>(context->getProcAddress("glGetUniformuiv"));
    VertexAttribI4usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLushort *)>(context->getProcAddress("glVertexAttribI4usv"));
    VertexAttribI4ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLubyte *)>(context->getProcAddress("glVertexAttribI4ubv"));
    VertexAttribI4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttribI4sv"));
    VertexAttribI4bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLbyte *)>(context->getProcAddress("glVertexAttribI4bv"));
    VertexAttribI4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttribI4uiv"));
    VertexAttribI3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttribI3uiv"));
    VertexAttribI2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttribI2uiv"));
    VertexAttribI1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttribI1uiv"));
    VertexAttribI4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttribI4iv"));
    VertexAttribI3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttribI3iv"));
    VertexAttribI2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttribI2iv"));
    VertexAttribI1iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttribI1iv"));
    VertexAttribI4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glVertexAttribI4ui"));
    VertexAttribI3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glVertexAttribI3ui"));
    VertexAttribI2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glVertexAttribI2ui"));
    VertexAttribI1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glVertexAttribI1ui"));
    VertexAttribI4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint )>(context->getProcAddress("glVertexAttribI4i"));
    VertexAttribI3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint )>(context->getProcAddress("glVertexAttribI3i"));
    VertexAttribI2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint )>(context->getProcAddress("glVertexAttribI2i"));
    VertexAttribI1i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint )>(context->getProcAddress("glVertexAttribI1i"));
    GetVertexAttribIuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint *)>(context->getProcAddress("glGetVertexAttribIuiv"));
    GetVertexAttribIiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetVertexAttribIiv"));
    VertexAttribIPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glVertexAttribIPointer"));
    EndConditionalRender = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glEndConditionalRender"));
    BeginConditionalRender = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(context->getProcAddress("glBeginConditionalRender"));
    ClampColor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glClampColor"));
    GetTransformFeedbackVarying = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLsizei , GLsizei *, GLsizei *, GLenum *, GLchar *)>(context->getProcAddress("glGetTransformFeedbackVarying"));
    TransformFeedbackVaryings = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLchar* const *, GLenum )>(context->getProcAddress("glTransformFeedbackVaryings"));
    BindBufferBase = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLuint )>(context->getProcAddress("glBindBufferBase"));
    BindBufferRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLuint , GLintptr , GLsizeiptr )>(context->getProcAddress("glBindBufferRange"));
    EndTransformFeedback = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glEndTransformFeedback"));
    BeginTransformFeedback = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glBeginTransformFeedback"));
    IsEnabledi = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glIsEnabledi"));
    Disablei = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glDisablei"));
    Enablei = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glEnablei"));
    GetIntegeri_v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLint *)>(context->getProcAddress("glGetIntegeri_v"));
    GetBooleani_v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLboolean *)>(context->getProcAddress("glGetBooleani_v"));
    ColorMaski = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLboolean , GLboolean , GLboolean , GLboolean )>(context->getProcAddress("glColorMaski"));

}

QOpenGLVersionStatus QOpenGLFunctions_3_0_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(3, 0, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_3_1_CoreBackend::QOpenGLFunctions_3_1_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 3.1 core functions
    CopyBufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLintptr , GLintptr , GLsizeiptr )>(context->getProcAddress("glCopyBufferSubData"));
    UniformBlockBinding = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glUniformBlockBinding"));
    GetActiveUniformBlockName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetActiveUniformBlockName"));
    GetActiveUniformBlockiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLenum , GLint *)>(context->getProcAddress("glGetActiveUniformBlockiv"));
    GetUniformBlockIndex = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)(GLuint , const GLchar *)>(context->getProcAddress("glGetUniformBlockIndex"));
    GetActiveUniformName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetActiveUniformName"));
    GetActiveUniformsiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLuint *, GLenum , GLint *)>(context->getProcAddress("glGetActiveUniformsiv"));
    GetUniformIndices = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLchar* const *, GLuint *)>(context->getProcAddress("glGetUniformIndices"));
    PrimitiveRestartIndex = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glPrimitiveRestartIndex"));
    TexBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glTexBuffer"));
    DrawElementsInstanced = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , const GLvoid *, GLsizei )>(context->getProcAddress("glDrawElementsInstanced"));
    DrawArraysInstanced = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLsizei , GLsizei )>(context->getProcAddress("glDrawArraysInstanced"));

}

QOpenGLVersionStatus QOpenGLFunctions_3_1_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(3, 1, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_3_2_CoreBackend::QOpenGLFunctions_3_2_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 3.2 core functions
    SampleMaski = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLbitfield )>(context->getProcAddress("glSampleMaski"));
    GetMultisamplefv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLfloat *)>(context->getProcAddress("glGetMultisamplefv"));
    TexImage3DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTexImage3DMultisample"));
    TexImage2DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTexImage2DMultisample"));
    GetSynciv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsync , GLenum , GLsizei , GLsizei *, GLint *)>(context->getProcAddress("glGetSynciv"));
    GetInteger64v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint64 *)>(context->getProcAddress("glGetInteger64v"));
    WaitSync = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsync , GLbitfield , GLuint64 )>(context->getProcAddress("glWaitSync"));
    ClientWaitSync = reinterpret_cast<GLenum (QOPENGLF_APIENTRYP)(GLsync , GLbitfield , GLuint64 )>(context->getProcAddress("glClientWaitSync"));
    DeleteSync = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsync )>(context->getProcAddress("glDeleteSync"));
    IsSync = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLsync )>(context->getProcAddress("glIsSync"));
    FenceSync = reinterpret_cast<GLsync (QOPENGLF_APIENTRYP)(GLenum , GLbitfield )>(context->getProcAddress("glFenceSync"));
    ProvokingVertex = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glProvokingVertex"));
    MultiDrawElementsBaseVertex = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLsizei *, GLenum , const GLvoid* const *, GLsizei , const GLint *)>(context->getProcAddress("glMultiDrawElementsBaseVertex"));
    DrawElementsInstancedBaseVertex = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , const GLvoid *, GLsizei , GLint )>(context->getProcAddress("glDrawElementsInstancedBaseVertex"));
    DrawRangeElementsBaseVertex = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLuint , GLsizei , GLenum , const GLvoid *, GLint )>(context->getProcAddress("glDrawRangeElementsBaseVertex"));
    DrawElementsBaseVertex = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , const GLvoid *, GLint )>(context->getProcAddress("glDrawElementsBaseVertex"));
    FramebufferTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint , GLint )>(context->getProcAddress("glFramebufferTexture"));
    GetBufferParameteri64v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint64 *)>(context->getProcAddress("glGetBufferParameteri64v"));
    GetInteger64i_v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLint64 *)>(context->getProcAddress("glGetInteger64i_v"));

}

QOpenGLVersionStatus QOpenGLFunctions_3_2_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(3, 2, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_3_3_CoreBackend::QOpenGLFunctions_3_3_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 3.3 core functions
    VertexAttribP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLboolean , const GLuint *)>(context->getProcAddress("glVertexAttribP4uiv"));
    VertexAttribP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLboolean , GLuint )>(context->getProcAddress("glVertexAttribP4ui"));
    VertexAttribP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLboolean , const GLuint *)>(context->getProcAddress("glVertexAttribP3uiv"));
    VertexAttribP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLboolean , GLuint )>(context->getProcAddress("glVertexAttribP3ui"));
    VertexAttribP2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLboolean , const GLuint *)>(context->getProcAddress("glVertexAttribP2uiv"));
    VertexAttribP2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLboolean , GLuint )>(context->getProcAddress("glVertexAttribP2ui"));
    VertexAttribP1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLboolean , const GLuint *)>(context->getProcAddress("glVertexAttribP1uiv"));
    VertexAttribP1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLboolean , GLuint )>(context->getProcAddress("glVertexAttribP1ui"));
    SecondaryColorP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glSecondaryColorP3uiv"));
    SecondaryColorP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glSecondaryColorP3ui"));
    ColorP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glColorP4uiv"));
    ColorP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glColorP4ui"));
    ColorP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glColorP3uiv"));
    ColorP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glColorP3ui"));
    NormalP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glNormalP3uiv"));
    NormalP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glNormalP3ui"));
    MultiTexCoordP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glMultiTexCoordP4uiv"));
    MultiTexCoordP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glMultiTexCoordP4ui"));
    MultiTexCoordP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glMultiTexCoordP3uiv"));
    MultiTexCoordP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glMultiTexCoordP3ui"));
    MultiTexCoordP2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glMultiTexCoordP2uiv"));
    MultiTexCoordP2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glMultiTexCoordP2ui"));
    MultiTexCoordP1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glMultiTexCoordP1uiv"));
    MultiTexCoordP1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glMultiTexCoordP1ui"));
    TexCoordP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glTexCoordP4uiv"));
    TexCoordP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glTexCoordP4ui"));
    TexCoordP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glTexCoordP3uiv"));
    TexCoordP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glTexCoordP3ui"));
    TexCoordP2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glTexCoordP2uiv"));
    TexCoordP2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glTexCoordP2ui"));
    TexCoordP1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glTexCoordP1uiv"));
    TexCoordP1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glTexCoordP1ui"));
    VertexP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glVertexP4uiv"));
    VertexP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glVertexP4ui"));
    VertexP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glVertexP3uiv"));
    VertexP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glVertexP3ui"));
    VertexP2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glVertexP2uiv"));
    VertexP2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glVertexP2ui"));
    GetQueryObjectui64v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint64 *)>(context->getProcAddress("glGetQueryObjectui64v"));
    GetQueryObjecti64v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint64 *)>(context->getProcAddress("glGetQueryObjecti64v"));
    QueryCounter = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(context->getProcAddress("glQueryCounter"));
    GetSamplerParameterIuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint *)>(context->getProcAddress("glGetSamplerParameterIuiv"));
    GetSamplerParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLfloat *)>(context->getProcAddress("glGetSamplerParameterfv"));
    GetSamplerParameterIiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetSamplerParameterIiv"));
    GetSamplerParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetSamplerParameteriv"));
    SamplerParameterIuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLuint *)>(context->getProcAddress("glSamplerParameterIuiv"));
    SamplerParameterIiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLint *)>(context->getProcAddress("glSamplerParameterIiv"));
    SamplerParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLfloat *)>(context->getProcAddress("glSamplerParameterfv"));
    SamplerParameterf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLfloat )>(context->getProcAddress("glSamplerParameterf"));
    SamplerParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLint *)>(context->getProcAddress("glSamplerParameteriv"));
    SamplerParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint )>(context->getProcAddress("glSamplerParameteri"));
    BindSampler = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glBindSampler"));
    IsSampler = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsSampler"));
    DeleteSamplers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteSamplers"));
    GenSamplers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenSamplers"));
    GetFragDataIndex = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLuint , const GLchar *)>(context->getProcAddress("glGetFragDataIndex"));
    BindFragDataLocationIndexed = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , const GLchar *)>(context->getProcAddress("glBindFragDataLocationIndexed"));
    VertexAttribDivisor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glVertexAttribDivisor"));

}

QOpenGLVersionStatus QOpenGLFunctions_3_3_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(3, 3, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_4_0_CoreBackend::QOpenGLFunctions_4_0_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 4.0 core functions
    GetQueryIndexediv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLenum , GLint *)>(context->getProcAddress("glGetQueryIndexediv"));
    EndQueryIndexed = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glEndQueryIndexed"));
    BeginQueryIndexed = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLuint )>(context->getProcAddress("glBeginQueryIndexed"));
    DrawTransformFeedbackStream = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLuint )>(context->getProcAddress("glDrawTransformFeedbackStream"));
    DrawTransformFeedback = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glDrawTransformFeedback"));
    ResumeTransformFeedback = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glResumeTransformFeedback"));
    PauseTransformFeedback = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glPauseTransformFeedback"));
    IsTransformFeedback = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsTransformFeedback"));
    GenTransformFeedbacks = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenTransformFeedbacks"));
    DeleteTransformFeedbacks = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteTransformFeedbacks"));
    BindTransformFeedback = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glBindTransformFeedback"));
    PatchParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(context->getProcAddress("glPatchParameterfv"));
    PatchParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(context->getProcAddress("glPatchParameteri"));
    GetProgramStageiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLenum , GLint *)>(context->getProcAddress("glGetProgramStageiv"));
    GetUniformSubroutineuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLuint *)>(context->getProcAddress("glGetUniformSubroutineuiv"));
    UniformSubroutinesuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLuint *)>(context->getProcAddress("glUniformSubroutinesuiv"));
    GetActiveSubroutineName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetActiveSubroutineName"));
    GetActiveSubroutineUniformName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetActiveSubroutineUniformName"));
    GetActiveSubroutineUniformiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLenum , GLint *)>(context->getProcAddress("glGetActiveSubroutineUniformiv"));
    GetSubroutineIndex = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLchar *)>(context->getProcAddress("glGetSubroutineIndex"));
    GetSubroutineUniformLocation = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLchar *)>(context->getProcAddress("glGetSubroutineUniformLocation"));
    GetUniformdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLdouble *)>(context->getProcAddress("glGetUniformdv"));
    UniformMatrix4x3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix4x3dv"));
    UniformMatrix4x2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix4x2dv"));
    UniformMatrix3x4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix3x4dv"));
    UniformMatrix3x2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix3x2dv"));
    UniformMatrix2x4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix2x4dv"));
    UniformMatrix2x3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix2x3dv"));
    UniformMatrix4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix4dv"));
    UniformMatrix3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix3dv"));
    UniformMatrix2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glUniformMatrix2dv"));
    Uniform4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLdouble *)>(context->getProcAddress("glUniform4dv"));
    Uniform3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLdouble *)>(context->getProcAddress("glUniform3dv"));
    Uniform2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLdouble *)>(context->getProcAddress("glUniform2dv"));
    Uniform1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLsizei , const GLdouble *)>(context->getProcAddress("glUniform1dv"));
    Uniform4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glUniform4d"));
    Uniform3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glUniform3d"));
    Uniform2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLdouble , GLdouble )>(context->getProcAddress("glUniform2d"));
    Uniform1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLdouble )>(context->getProcAddress("glUniform1d"));
    DrawElementsIndirect = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glDrawElementsIndirect"));
    DrawArraysIndirect = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLvoid *)>(context->getProcAddress("glDrawArraysIndirect"));
    BlendFuncSeparatei = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLenum , GLenum , GLenum )>(context->getProcAddress("glBlendFuncSeparatei"));
    BlendFunci = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLenum )>(context->getProcAddress("glBlendFunci"));
    BlendEquationSeparatei = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLenum )>(context->getProcAddress("glBlendEquationSeparatei"));
    BlendEquationi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(context->getProcAddress("glBlendEquationi"));
    MinSampleShading = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glMinSampleShading"));

}

QOpenGLVersionStatus QOpenGLFunctions_4_0_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(4, 0, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_4_1_CoreBackend::QOpenGLFunctions_4_1_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 4.1 core functions
    GetDoublei_v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLdouble *)>(context->getProcAddress("glGetDoublei_v"));
    GetFloati_v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLfloat *)>(context->getProcAddress("glGetFloati_v"));
    DepthRangeIndexed = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble )>(context->getProcAddress("glDepthRangeIndexed"));
    DepthRangeArrayv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLdouble *)>(context->getProcAddress("glDepthRangeArrayv"));
    ScissorIndexedv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glScissorIndexedv"));
    ScissorIndexed = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glScissorIndexed"));
    ScissorArrayv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLint *)>(context->getProcAddress("glScissorArrayv"));
    ViewportIndexedfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glViewportIndexedfv"));
    ViewportIndexedf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glViewportIndexedf"));
    ViewportArrayv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLfloat *)>(context->getProcAddress("glViewportArrayv"));
    GetVertexAttribLdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLdouble *)>(context->getProcAddress("glGetVertexAttribLdv"));
    VertexAttribLPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glVertexAttribLPointer"));
    VertexAttribL4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttribL4dv"));
    VertexAttribL3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttribL3dv"));
    VertexAttribL2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttribL2dv"));
    VertexAttribL1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttribL1dv"));
    VertexAttribL4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttribL4d"));
    VertexAttribL3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttribL3d"));
    VertexAttribL2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttribL2d"));
    VertexAttribL1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble )>(context->getProcAddress("glVertexAttribL1d"));
    GetProgramPipelineInfoLog = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetProgramPipelineInfoLog"));
    ValidateProgramPipeline = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glValidateProgramPipeline"));
    ProgramUniformMatrix4x3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix4x3dv"));
    ProgramUniformMatrix3x4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix3x4dv"));
    ProgramUniformMatrix4x2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix4x2dv"));
    ProgramUniformMatrix2x4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix2x4dv"));
    ProgramUniformMatrix3x2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix3x2dv"));
    ProgramUniformMatrix2x3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix2x3dv"));
    ProgramUniformMatrix4x3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix4x3fv"));
    ProgramUniformMatrix3x4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix3x4fv"));
    ProgramUniformMatrix4x2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix4x2fv"));
    ProgramUniformMatrix2x4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix2x4fv"));
    ProgramUniformMatrix3x2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix3x2fv"));
    ProgramUniformMatrix2x3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix2x3fv"));
    ProgramUniformMatrix4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix4dv"));
    ProgramUniformMatrix3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix3dv"));
    ProgramUniformMatrix2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *)>(context->getProcAddress("glProgramUniformMatrix2dv"));
    ProgramUniformMatrix4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix4fv"));
    ProgramUniformMatrix3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix3fv"));
    ProgramUniformMatrix2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *)>(context->getProcAddress("glProgramUniformMatrix2fv"));
    ProgramUniform4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLuint *)>(context->getProcAddress("glProgramUniform4uiv"));
    ProgramUniform4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glProgramUniform4ui"));
    ProgramUniform4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLdouble *)>(context->getProcAddress("glProgramUniform4dv"));
    ProgramUniform4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glProgramUniform4d"));
    ProgramUniform4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLfloat *)>(context->getProcAddress("glProgramUniform4fv"));
    ProgramUniform4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glProgramUniform4f"));
    ProgramUniform4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLint *)>(context->getProcAddress("glProgramUniform4iv"));
    ProgramUniform4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLint )>(context->getProcAddress("glProgramUniform4i"));
    ProgramUniform3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLuint *)>(context->getProcAddress("glProgramUniform3uiv"));
    ProgramUniform3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLuint , GLuint , GLuint )>(context->getProcAddress("glProgramUniform3ui"));
    ProgramUniform3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLdouble *)>(context->getProcAddress("glProgramUniform3dv"));
    ProgramUniform3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glProgramUniform3d"));
    ProgramUniform3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLfloat *)>(context->getProcAddress("glProgramUniform3fv"));
    ProgramUniform3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glProgramUniform3f"));
    ProgramUniform3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLint *)>(context->getProcAddress("glProgramUniform3iv"));
    ProgramUniform3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint )>(context->getProcAddress("glProgramUniform3i"));
    ProgramUniform2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLuint *)>(context->getProcAddress("glProgramUniform2uiv"));
    ProgramUniform2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLuint , GLuint )>(context->getProcAddress("glProgramUniform2ui"));
    ProgramUniform2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLdouble *)>(context->getProcAddress("glProgramUniform2dv"));
    ProgramUniform2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLdouble , GLdouble )>(context->getProcAddress("glProgramUniform2d"));
    ProgramUniform2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLfloat *)>(context->getProcAddress("glProgramUniform2fv"));
    ProgramUniform2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLfloat , GLfloat )>(context->getProcAddress("glProgramUniform2f"));
    ProgramUniform2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLint *)>(context->getProcAddress("glProgramUniform2iv"));
    ProgramUniform2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint )>(context->getProcAddress("glProgramUniform2i"));
    ProgramUniform1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLuint *)>(context->getProcAddress("glProgramUniform1uiv"));
    ProgramUniform1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLuint )>(context->getProcAddress("glProgramUniform1ui"));
    ProgramUniform1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLdouble *)>(context->getProcAddress("glProgramUniform1dv"));
    ProgramUniform1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLdouble )>(context->getProcAddress("glProgramUniform1d"));
    ProgramUniform1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLfloat *)>(context->getProcAddress("glProgramUniform1fv"));
    ProgramUniform1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLfloat )>(context->getProcAddress("glProgramUniform1f"));
    ProgramUniform1iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , const GLint *)>(context->getProcAddress("glProgramUniform1iv"));
    ProgramUniform1i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint )>(context->getProcAddress("glProgramUniform1i"));
    GetProgramPipelineiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetProgramPipelineiv"));
    IsProgramPipeline = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsProgramPipeline"));
    GenProgramPipelines = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenProgramPipelines"));
    DeleteProgramPipelines = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteProgramPipelines"));
    BindProgramPipeline = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glBindProgramPipeline"));
    CreateShaderProgramv = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLchar* const *)>(context->getProcAddress("glCreateShaderProgramv"));
    ActiveShaderProgram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glActiveShaderProgram"));
    UseProgramStages = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLbitfield , GLuint )>(context->getProcAddress("glUseProgramStages"));
    ProgramParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint )>(context->getProcAddress("glProgramParameteri"));
    ProgramBinary = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLvoid *, GLsizei )>(context->getProcAddress("glProgramBinary"));
    GetProgramBinary = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLsizei *, GLenum *, GLvoid *)>(context->getProcAddress("glGetProgramBinary"));
    ClearDepthf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glClearDepthf"));
    DepthRangef = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(context->getProcAddress("glDepthRangef"));
    GetShaderPrecisionFormat = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *, GLint *)>(context->getProcAddress("glGetShaderPrecisionFormat"));
    ShaderBinary = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *, GLenum , const GLvoid *, GLsizei )>(context->getProcAddress("glShaderBinary"));
    ReleaseShaderCompiler = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glReleaseShaderCompiler"));

}

QOpenGLVersionStatus QOpenGLFunctions_4_1_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(4, 1, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_4_2_CoreBackend::QOpenGLFunctions_4_2_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 4.2 core functions
    TexStorage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLsizei )>(context->getProcAddress("glTexStorage3D"));
    TexStorage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei )>(context->getProcAddress("glTexStorage2D"));
    TexStorage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei )>(context->getProcAddress("glTexStorage1D"));
    MemoryBarrier = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbitfield )>(context->getProcAddress("glMemoryBarrier"));
    BindImageTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLint , GLboolean , GLint , GLenum , GLenum )>(context->getProcAddress("glBindImageTexture"));
    GetActiveAtomicCounterBufferiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLenum , GLint *)>(context->getProcAddress("glGetActiveAtomicCounterBufferiv"));
    GetInternalformativ = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , GLint *)>(context->getProcAddress("glGetInternalformativ"));
    DrawTransformFeedbackStreamInstanced = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLuint , GLsizei )>(context->getProcAddress("glDrawTransformFeedbackStreamInstanced"));
    DrawTransformFeedbackInstanced = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLsizei )>(context->getProcAddress("glDrawTransformFeedbackInstanced"));
    DrawElementsInstancedBaseVertexBaseInstance = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , const void *, GLsizei , GLint , GLuint )>(context->getProcAddress("glDrawElementsInstancedBaseVertexBaseInstance"));
    DrawElementsInstancedBaseInstance = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , const void *, GLsizei , GLuint )>(context->getProcAddress("glDrawElementsInstancedBaseInstance"));
    DrawArraysInstancedBaseInstance = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLsizei , GLsizei , GLuint )>(context->getProcAddress("glDrawArraysInstancedBaseInstance"));

}

QOpenGLVersionStatus QOpenGLFunctions_4_2_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(4, 2, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_4_3_CoreBackend::QOpenGLFunctions_4_3_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 4.3 core functions
    GetObjectPtrLabel = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const void *, GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetObjectPtrLabel"));
    ObjectPtrLabel = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const void *, GLsizei , const GLchar *)>(context->getProcAddress("glObjectPtrLabel"));
    GetObjectLabel = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetObjectLabel"));
    ObjectLabel = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLsizei , const GLchar *)>(context->getProcAddress("glObjectLabel"));
    PopDebugGroup = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glPopDebugGroup"));
    PushDebugGroup = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLsizei , const GLchar *)>(context->getProcAddress("glPushDebugGroup"));
    GetDebugMessageLog = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *)>(context->getProcAddress("glGetDebugMessageLog"));
    DebugMessageCallback = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLDEBUGPROC , const void *)>(context->getProcAddress("glDebugMessageCallback"));
    DebugMessageInsert = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint , GLenum , GLsizei , const GLchar *)>(context->getProcAddress("glDebugMessageInsert"));
    DebugMessageControl = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , const GLuint *, GLboolean )>(context->getProcAddress("glDebugMessageControl"));
    TexStorage3DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTexStorage3DMultisample"));
    TexStorage2DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTexStorage2DMultisample"));
    TexBufferRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint , GLintptr , GLsizeiptr )>(context->getProcAddress("glTexBufferRange"));
    ShaderStorageBlockBinding = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glShaderStorageBlockBinding"));
    GetProgramResourceLocationIndex = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLchar *)>(context->getProcAddress("glGetProgramResourceLocationIndex"));
    GetProgramResourceLocation = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLchar *)>(context->getProcAddress("glGetProgramResourceLocation"));
    GetProgramResourceiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLsizei , const GLenum *, GLsizei , GLsizei *, GLint *)>(context->getProcAddress("glGetProgramResourceiv"));
    GetProgramResourceName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLsizei , GLsizei *, GLchar *)>(context->getProcAddress("glGetProgramResourceName"));
    GetProgramResourceIndex = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLchar *)>(context->getProcAddress("glGetProgramResourceIndex"));
    GetProgramInterfaceiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLenum , GLint *)>(context->getProcAddress("glGetProgramInterfaceiv"));
    MultiDrawElementsIndirect = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const void *, GLsizei , GLsizei )>(context->getProcAddress("glMultiDrawElementsIndirect"));
    MultiDrawArraysIndirect = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const void *, GLsizei , GLsizei )>(context->getProcAddress("glMultiDrawArraysIndirect"));
    InvalidateSubFramebuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLenum *, GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glInvalidateSubFramebuffer"));
    InvalidateFramebuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLenum *)>(context->getProcAddress("glInvalidateFramebuffer"));
    InvalidateBufferData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glInvalidateBufferData"));
    InvalidateBufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLintptr , GLsizeiptr )>(context->getProcAddress("glInvalidateBufferSubData"));
    InvalidateTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint )>(context->getProcAddress("glInvalidateTexImage"));
    InvalidateTexSubImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei )>(context->getProcAddress("glInvalidateTexSubImage"));
    GetInternalformati64v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , GLint64 *)>(context->getProcAddress("glGetInternalformati64v"));
    GetFramebufferParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetFramebufferParameteriv"));
    FramebufferParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(context->getProcAddress("glFramebufferParameteri"));
    VertexBindingDivisor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glVertexBindingDivisor"));
    VertexAttribBinding = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glVertexAttribBinding"));
    VertexAttribLFormat = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLuint )>(context->getProcAddress("glVertexAttribLFormat"));
    VertexAttribIFormat = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLuint )>(context->getProcAddress("glVertexAttribIFormat"));
    VertexAttribFormat = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLboolean , GLuint )>(context->getProcAddress("glVertexAttribFormat"));
    BindVertexBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLintptr , GLsizei )>(context->getProcAddress("glBindVertexBuffer"));
    TextureView = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLenum , GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glTextureView"));
    CopyImageSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint , GLint , GLint , GLint , GLuint , GLenum , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei )>(context->getProcAddress("glCopyImageSubData"));
    DispatchComputeIndirect = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLintptr )>(context->getProcAddress("glDispatchComputeIndirect"));
    DispatchCompute = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glDispatchCompute"));
    ClearBufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLintptr , GLsizeiptr , GLenum , GLenum , const void *)>(context->getProcAddress("glClearBufferSubData"));
    ClearBufferData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLenum , const void *)>(context->getProcAddress("glClearBufferData"));

}

QOpenGLVersionStatus QOpenGLFunctions_4_3_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(4, 3, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_4_4_CoreBackend::QOpenGLFunctions_4_4_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 4.4 core functions
    BindVertexBuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLuint *, const GLintptr *, const GLsizei *)>(context->getProcAddress("glBindVertexBuffers"));
    BindImageTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLuint *)>(context->getProcAddress("glBindImageTextures"));
    BindSamplers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLuint *)>(context->getProcAddress("glBindSamplers"));
    BindTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLuint *)>(context->getProcAddress("glBindTextures"));
    BindBuffersRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLsizei , const GLuint *, const GLintptr *, const GLsizeiptr *)>(context->getProcAddress("glBindBuffersRange"));
    BindBuffersBase = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint , GLsizei , const GLuint *)>(context->getProcAddress("glBindBuffersBase"));
    ClearTexSubImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLenum , const void *)>(context->getProcAddress("glClearTexSubImage"));
    ClearTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLenum , const void *)>(context->getProcAddress("glClearTexImage"));
    BufferStorage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizeiptr , const void *, GLbitfield )>(context->getProcAddress("glBufferStorage"));

}

QOpenGLVersionStatus QOpenGLFunctions_4_4_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(4, 4, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_4_5_CoreBackend::QOpenGLFunctions_4_5_CoreBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 4.5 core functions
    TextureBarrier = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glTextureBarrier"));
    ReadnPixels = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , GLsizei , void *)>(context->getProcAddress("glReadnPixels"));
    GetnUniformuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLuint *)>(context->getProcAddress("glGetnUniformuiv"));
    GetnUniformiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLint *)>(context->getProcAddress("glGetnUniformiv"));
    GetnUniformfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLfloat *)>(context->getProcAddress("glGetnUniformfv"));
    GetnUniformdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , GLdouble *)>(context->getProcAddress("glGetnUniformdv"));
    GetnTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLenum , GLsizei , void *)>(context->getProcAddress("glGetnTexImage"));
    GetnCompressedTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLsizei , void *)>(context->getProcAddress("glGetnCompressedTexImage"));
    GetGraphicsResetStatus = reinterpret_cast<GLenum (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glGetGraphicsResetStatus"));
    GetCompressedTextureSubImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLsizei , void *)>(context->getProcAddress("glGetCompressedTextureSubImage"));
    GetTextureSubImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLenum , GLsizei , void *)>(context->getProcAddress("glGetTextureSubImage"));
    MemoryBarrierByRegion = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbitfield )>(context->getProcAddress("glMemoryBarrierByRegion"));
    CreateQueries = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLuint *)>(context->getProcAddress("glCreateQueries"));
    CreateProgramPipelines = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glCreateProgramPipelines"));
    CreateSamplers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glCreateSamplers"));
    GetVertexArrayIndexed64iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLenum , GLint64 *)>(context->getProcAddress("glGetVertexArrayIndexed64iv"));
    GetVertexArrayIndexediv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLenum , GLint *)>(context->getProcAddress("glGetVertexArrayIndexediv"));
    GetVertexArrayiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetVertexArrayiv"));
    VertexArrayBindingDivisor = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glVertexArrayBindingDivisor"));
    VertexArrayAttribLFormat = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLint , GLenum , GLuint )>(context->getProcAddress("glVertexArrayAttribLFormat"));
    VertexArrayAttribIFormat = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLint , GLenum , GLuint )>(context->getProcAddress("glVertexArrayAttribIFormat"));
    VertexArrayAttribFormat = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLint , GLenum , GLboolean , GLuint )>(context->getProcAddress("glVertexArrayAttribFormat"));
    VertexArrayAttribBinding = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glVertexArrayAttribBinding"));
    VertexArrayVertexBuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLsizei , const GLuint *, const GLintptr *, const GLsizei *)>(context->getProcAddress("glVertexArrayVertexBuffers"));
    VertexArrayVertexBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , GLintptr , GLsizei )>(context->getProcAddress("glVertexArrayVertexBuffer"));
    VertexArrayElementBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glVertexArrayElementBuffer"));
    EnableVertexArrayAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glEnableVertexArrayAttrib"));
    DisableVertexArrayAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glDisableVertexArrayAttrib"));
    CreateVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glCreateVertexArrays"));
    GetTextureParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetTextureParameteriv"));
    GetTextureParameterIuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint *)>(context->getProcAddress("glGetTextureParameterIuiv"));
    GetTextureParameterIiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetTextureParameterIiv"));
    GetTextureParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLfloat *)>(context->getProcAddress("glGetTextureParameterfv"));
    GetTextureLevelParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLint *)>(context->getProcAddress("glGetTextureLevelParameteriv"));
    GetTextureLevelParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLfloat *)>(context->getProcAddress("glGetTextureLevelParameterfv"));
    GetCompressedTextureImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLsizei , void *)>(context->getProcAddress("glGetCompressedTextureImage"));
    GetTextureImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLenum , GLenum , GLsizei , void *)>(context->getProcAddress("glGetTextureImage"));
    BindTextureUnit = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glBindTextureUnit"));
    GenerateTextureMipmap = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glGenerateTextureMipmap"));
    TextureParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLint *)>(context->getProcAddress("glTextureParameteriv"));
    TextureParameterIuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLuint *)>(context->getProcAddress("glTextureParameterIuiv"));
    TextureParameterIiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLint *)>(context->getProcAddress("glTextureParameterIiv"));
    TextureParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint )>(context->getProcAddress("glTextureParameteri"));
    TextureParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , const GLfloat *)>(context->getProcAddress("glTextureParameterfv"));
    TextureParameterf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLfloat )>(context->getProcAddress("glTextureParameterf"));
    CopyTextureSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glCopyTextureSubImage3D"));
    CopyTextureSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glCopyTextureSubImage2D"));
    CopyTextureSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei )>(context->getProcAddress("glCopyTextureSubImage1D"));
    CompressedTextureSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLsizei , const void *)>(context->getProcAddress("glCompressedTextureSubImage3D"));
    CompressedTextureSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLsizei , const void *)>(context->getProcAddress("glCompressedTextureSubImage2D"));
    CompressedTextureSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLsizei , GLenum , GLsizei , const void *)>(context->getProcAddress("glCompressedTextureSubImage1D"));
    TextureSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLenum , const void *)>(context->getProcAddress("glTextureSubImage3D"));
    TextureSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , const void *)>(context->getProcAddress("glTextureSubImage2D"));
    TextureSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLsizei , GLenum , GLenum , const void *)>(context->getProcAddress("glTextureSubImage1D"));
    TextureStorage3DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTextureStorage3DMultisample"));
    TextureStorage2DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTextureStorage2DMultisample"));
    TextureStorage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei , GLsizei )>(context->getProcAddress("glTextureStorage3D"));
    TextureStorage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei )>(context->getProcAddress("glTextureStorage2D"));
    TextureStorage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei )>(context->getProcAddress("glTextureStorage1D"));
    TextureBufferRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLintptr , GLsizei )>(context->getProcAddress("glTextureBufferRange"));
    TextureBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint )>(context->getProcAddress("glTextureBuffer"));
    CreateTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLuint *)>(context->getProcAddress("glCreateTextures"));
    GetNamedRenderbufferParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetNamedRenderbufferParameteriv"));
    NamedRenderbufferStorageMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei )>(context->getProcAddress("glNamedRenderbufferStorageMultisample"));
    NamedRenderbufferStorage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLsizei , GLsizei )>(context->getProcAddress("glNamedRenderbufferStorage"));
    CreateRenderbuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glCreateRenderbuffers"));
    GetNamedFramebufferAttachmentParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLenum , GLint *)>(context->getProcAddress("glGetNamedFramebufferAttachmentParameteriv"));
    GetNamedFramebufferParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetNamedFramebufferParameteriv"));
    CheckNamedFramebufferStatus = reinterpret_cast<GLenum (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(context->getProcAddress("glCheckNamedFramebufferStatus"));
    BlitNamedFramebuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLint , GLint , GLint , GLint , GLint , GLint , GLint , GLint , GLbitfield , GLenum )>(context->getProcAddress("glBlitNamedFramebuffer"));
    ClearNamedFramebufferfi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLfloat , GLint )>(context->getProcAddress("glClearNamedFramebufferfi"));
    ClearNamedFramebufferfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint , const GLfloat *)>(context->getProcAddress("glClearNamedFramebufferfv"));
    ClearNamedFramebufferuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint , const GLuint *)>(context->getProcAddress("glClearNamedFramebufferuiv"));
    ClearNamedFramebufferiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint , const GLint *)>(context->getProcAddress("glClearNamedFramebufferiv"));
    InvalidateNamedFramebufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLenum *, GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glInvalidateNamedFramebufferSubData"));
    InvalidateNamedFramebufferData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLenum *)>(context->getProcAddress("glInvalidateNamedFramebufferData"));
    NamedFramebufferReadBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(context->getProcAddress("glNamedFramebufferReadBuffer"));
    NamedFramebufferDrawBuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const GLenum *)>(context->getProcAddress("glNamedFramebufferDrawBuffers"));
    NamedFramebufferDrawBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(context->getProcAddress("glNamedFramebufferDrawBuffer"));
    NamedFramebufferTextureLayer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLint , GLint )>(context->getProcAddress("glNamedFramebufferTextureLayer"));
    NamedFramebufferTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLint )>(context->getProcAddress("glNamedFramebufferTexture"));
    NamedFramebufferParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint )>(context->getProcAddress("glNamedFramebufferParameteri"));
    NamedFramebufferRenderbuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLenum , GLuint )>(context->getProcAddress("glNamedFramebufferRenderbuffer"));
    CreateFramebuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glCreateFramebuffers"));
    GetNamedBufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLintptr , GLsizei , void *)>(context->getProcAddress("glGetNamedBufferSubData"));
    GetNamedBufferPointerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , void * *)>(context->getProcAddress("glGetNamedBufferPointerv"));
    GetNamedBufferParameteri64v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint64 *)>(context->getProcAddress("glGetNamedBufferParameteri64v"));
    GetNamedBufferParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetNamedBufferParameteriv"));
    FlushMappedNamedBufferRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLintptr , GLsizei )>(context->getProcAddress("glFlushMappedNamedBufferRange"));
    UnmapNamedBuffer = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glUnmapNamedBuffer"));
    MapNamedBufferRange = reinterpret_cast<void * (QOPENGLF_APIENTRYP)(GLuint , GLintptr , GLsizei , GLbitfield )>(context->getProcAddress("glMapNamedBufferRange"));
    MapNamedBuffer = reinterpret_cast<void * (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(context->getProcAddress("glMapNamedBuffer"));
    ClearNamedBufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLintptr , GLsizei , GLenum , GLenum , const void *)>(context->getProcAddress("glClearNamedBufferSubData"));
    ClearNamedBufferData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLenum , GLenum , const void *)>(context->getProcAddress("glClearNamedBufferData"));
    CopyNamedBufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLintptr , GLintptr , GLsizei )>(context->getProcAddress("glCopyNamedBufferSubData"));
    NamedBufferSubData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLintptr , GLsizei , const void *)>(context->getProcAddress("glNamedBufferSubData"));
    NamedBufferData = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const void *, GLenum )>(context->getProcAddress("glNamedBufferData"));
    NamedBufferStorage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei , const void *, GLbitfield )>(context->getProcAddress("glNamedBufferStorage"));
    CreateBuffers = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glCreateBuffers"));
    GetTransformFeedbacki64_v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLint64 *)>(context->getProcAddress("glGetTransformFeedbacki64_v"));
    GetTransformFeedbacki_v = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLint *)>(context->getProcAddress("glGetTransformFeedbacki_v"));
    GetTransformFeedbackiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint *)>(context->getProcAddress("glGetTransformFeedbackiv"));
    TransformFeedbackBufferRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , GLintptr , GLsizei )>(context->getProcAddress("glTransformFeedbackBufferRange"));
    TransformFeedbackBufferBase = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glTransformFeedbackBufferBase"));
    CreateTransformFeedbacks = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glCreateTransformFeedbacks"));
    ClipControl = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glClipControl"));

}

QOpenGLVersionStatus QOpenGLFunctions_4_5_CoreBackend::versionStatus()
{
    return QOpenGLVersionStatus(4, 5, QOpenGLVersionStatus::CoreStatus);
}

QOpenGLFunctions_1_0_DeprecatedBackend::QOpenGLFunctions_1_0_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.0 deprecated functions
#if defined(Q_OS_WIN)
    HMODULE handle = static_cast<HMODULE>(QOpenGLContext::openGLModuleHandle());
    if (!handle)
        handle = GetModuleHandleA("opengl32.dll");
    Translatef = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glTranslatef"));
    Translated = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glTranslated"));
    Scalef = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glScalef"));
    Scaled = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glScaled"));
    Rotatef = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glRotatef"));
    Rotated = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glRotated"));
    PushMatrix = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glPushMatrix"));
    PopMatrix = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glPopMatrix"));
    Ortho = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glOrtho"));
    MultMatrixd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glMultMatrixd"));
    MultMatrixf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glMultMatrixf"));
    MatrixMode = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glMatrixMode"));
    LoadMatrixd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glLoadMatrixd"));
    LoadMatrixf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glLoadMatrixf"));
    LoadIdentity = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glLoadIdentity"));
    Frustum = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glFrustum"));
    IsList = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(GetProcAddress(handle, "glIsList"));
    GetTexGeniv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(GetProcAddress(handle, "glGetTexGeniv"));
    GetTexGenfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(GetProcAddress(handle, "glGetTexGenfv"));
    GetTexGendv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLdouble *)>(GetProcAddress(handle, "glGetTexGendv"));
    GetTexEnviv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(GetProcAddress(handle, "glGetTexEnviv"));
    GetTexEnvfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(GetProcAddress(handle, "glGetTexEnvfv"));
    GetPolygonStipple = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte *)>(GetProcAddress(handle, "glGetPolygonStipple"));
    GetPixelMapusv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLushort *)>(GetProcAddress(handle, "glGetPixelMapusv"));
    GetPixelMapuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint *)>(GetProcAddress(handle, "glGetPixelMapuiv"));
    GetPixelMapfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat *)>(GetProcAddress(handle, "glGetPixelMapfv"));
    GetMaterialiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(GetProcAddress(handle, "glGetMaterialiv"));
    GetMaterialfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(GetProcAddress(handle, "glGetMaterialfv"));
    GetMapiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(GetProcAddress(handle, "glGetMapiv"));
    GetMapfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(GetProcAddress(handle, "glGetMapfv"));
    GetMapdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLdouble *)>(GetProcAddress(handle, "glGetMapdv"));
    GetLightiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(GetProcAddress(handle, "glGetLightiv"));
    GetLightfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(GetProcAddress(handle, "glGetLightfv"));
    GetClipPlane = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble *)>(GetProcAddress(handle, "glGetClipPlane"));
    DrawPixels = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(GetProcAddress(handle, "glDrawPixels"));
    CopyPixels = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei , GLenum )>(GetProcAddress(handle, "glCopyPixels"));
    PixelMapusv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLushort *)>(GetProcAddress(handle, "glPixelMapusv"));
    PixelMapuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLuint *)>(GetProcAddress(handle, "glPixelMapuiv"));
    PixelMapfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLfloat *)>(GetProcAddress(handle, "glPixelMapfv"));
    PixelTransferi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(GetProcAddress(handle, "glPixelTransferi"));
    PixelTransferf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(GetProcAddress(handle, "glPixelTransferf"));
    PixelZoom = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(GetProcAddress(handle, "glPixelZoom"));
    AlphaFunc = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(GetProcAddress(handle, "glAlphaFunc"));
    EvalPoint2 = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(GetProcAddress(handle, "glEvalPoint2"));
    EvalMesh2 = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint )>(GetProcAddress(handle, "glEvalMesh2"));
    EvalPoint1 = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(GetProcAddress(handle, "glEvalPoint1"));
    EvalMesh1 = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint )>(GetProcAddress(handle, "glEvalMesh1"));
    EvalCoord2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glEvalCoord2fv"));
    EvalCoord2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(GetProcAddress(handle, "glEvalCoord2f"));
    EvalCoord2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glEvalCoord2dv"));
    EvalCoord2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(GetProcAddress(handle, "glEvalCoord2d"));
    EvalCoord1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glEvalCoord1fv"));
    EvalCoord1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(GetProcAddress(handle, "glEvalCoord1f"));
    EvalCoord1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glEvalCoord1dv"));
    EvalCoord1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(GetProcAddress(handle, "glEvalCoord1d"));
    MapGrid2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLfloat , GLfloat , GLint , GLfloat , GLfloat )>(GetProcAddress(handle, "glMapGrid2f"));
    MapGrid2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLdouble , GLdouble , GLint , GLdouble , GLdouble )>(GetProcAddress(handle, "glMapGrid2d"));
    MapGrid1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLfloat , GLfloat )>(GetProcAddress(handle, "glMapGrid1f"));
    MapGrid1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLdouble , GLdouble )>(GetProcAddress(handle, "glMapGrid1d"));
    Map2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat , GLfloat , GLint , GLint , GLfloat , GLfloat , GLint , GLint , const GLfloat *)>(GetProcAddress(handle, "glMap2f"));
    Map2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble , GLdouble , GLint , GLint , GLdouble , GLdouble , GLint , GLint , const GLdouble *)>(GetProcAddress(handle, "glMap2d"));
    Map1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat , GLfloat , GLint , GLint , const GLfloat *)>(GetProcAddress(handle, "glMap1f"));
    Map1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble , GLdouble , GLint , GLint , const GLdouble *)>(GetProcAddress(handle, "glMap1d"));
    PushAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbitfield )>(GetProcAddress(handle, "glPushAttrib"));
    PopAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glPopAttrib"));
    Accum = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(GetProcAddress(handle, "glAccum"));
    IndexMask = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(GetProcAddress(handle, "glIndexMask"));
    ClearIndex = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(GetProcAddress(handle, "glClearIndex"));
    ClearAccum = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glClearAccum"));
    PushName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(GetProcAddress(handle, "glPushName"));
    PopName = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glPopName"));
    PassThrough = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(GetProcAddress(handle, "glPassThrough"));
    LoadName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(GetProcAddress(handle, "glLoadName"));
    InitNames = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glInitNames"));
    RenderMode = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glRenderMode"));
    SelectBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(GetProcAddress(handle, "glSelectBuffer"));
    FeedbackBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLenum , GLfloat *)>(GetProcAddress(handle, "glFeedbackBuffer"));
    TexGeniv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(GetProcAddress(handle, "glTexGeniv"));
    TexGeni = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(GetProcAddress(handle, "glTexGeni"));
    TexGenfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(GetProcAddress(handle, "glTexGenfv"));
    TexGenf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(GetProcAddress(handle, "glTexGenf"));
    TexGendv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLdouble *)>(GetProcAddress(handle, "glTexGendv"));
    TexGend = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLdouble )>(GetProcAddress(handle, "glTexGend"));
    TexEnviv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(GetProcAddress(handle, "glTexEnviv"));
    TexEnvi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(GetProcAddress(handle, "glTexEnvi"));
    TexEnvfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(GetProcAddress(handle, "glTexEnvfv"));
    TexEnvf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(GetProcAddress(handle, "glTexEnvf"));
    ShadeModel = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glShadeModel"));
    PolygonStipple = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(GetProcAddress(handle, "glPolygonStipple"));
    Materialiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(GetProcAddress(handle, "glMaterialiv"));
    Materiali = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(GetProcAddress(handle, "glMateriali"));
    Materialfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(GetProcAddress(handle, "glMaterialfv"));
    Materialf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(GetProcAddress(handle, "glMaterialf"));
    LineStipple = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLushort )>(GetProcAddress(handle, "glLineStipple"));
    LightModeliv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(GetProcAddress(handle, "glLightModeliv"));
    LightModeli = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(GetProcAddress(handle, "glLightModeli"));
    LightModelfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(GetProcAddress(handle, "glLightModelfv"));
    LightModelf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(GetProcAddress(handle, "glLightModelf"));
    Lightiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(GetProcAddress(handle, "glLightiv"));
    Lighti = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(GetProcAddress(handle, "glLighti"));
    Lightfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(GetProcAddress(handle, "glLightfv"));
    Lightf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(GetProcAddress(handle, "glLightf"));
    Fogiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(GetProcAddress(handle, "glFogiv"));
    Fogi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(GetProcAddress(handle, "glFogi"));
    Fogfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(GetProcAddress(handle, "glFogfv"));
    Fogf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(GetProcAddress(handle, "glFogf"));
    ColorMaterial = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(GetProcAddress(handle, "glColorMaterial"));
    ClipPlane = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLdouble *)>(GetProcAddress(handle, "glClipPlane"));
    Vertex4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glVertex4sv"));
    Vertex4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glVertex4s"));
    Vertex4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glVertex4iv"));
    Vertex4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(GetProcAddress(handle, "glVertex4i"));
    Vertex4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glVertex4fv"));
    Vertex4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glVertex4f"));
    Vertex4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glVertex4dv"));
    Vertex4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glVertex4d"));
    Vertex3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glVertex3sv"));
    Vertex3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glVertex3s"));
    Vertex3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glVertex3iv"));
    Vertex3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(GetProcAddress(handle, "glVertex3i"));
    Vertex3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glVertex3fv"));
    Vertex3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glVertex3f"));
    Vertex3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glVertex3dv"));
    Vertex3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glVertex3d"));
    Vertex2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glVertex2sv"));
    Vertex2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort )>(GetProcAddress(handle, "glVertex2s"));
    Vertex2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glVertex2iv"));
    Vertex2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(GetProcAddress(handle, "glVertex2i"));
    Vertex2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glVertex2fv"));
    Vertex2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(GetProcAddress(handle, "glVertex2f"));
    Vertex2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glVertex2dv"));
    Vertex2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(GetProcAddress(handle, "glVertex2d"));
    TexCoord4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glTexCoord4sv"));
    TexCoord4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glTexCoord4s"));
    TexCoord4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glTexCoord4iv"));
    TexCoord4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(GetProcAddress(handle, "glTexCoord4i"));
    TexCoord4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glTexCoord4fv"));
    TexCoord4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glTexCoord4f"));
    TexCoord4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glTexCoord4dv"));
    TexCoord4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glTexCoord4d"));
    TexCoord3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glTexCoord3sv"));
    TexCoord3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glTexCoord3s"));
    TexCoord3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glTexCoord3iv"));
    TexCoord3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(GetProcAddress(handle, "glTexCoord3i"));
    TexCoord3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glTexCoord3fv"));
    TexCoord3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glTexCoord3f"));
    TexCoord3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glTexCoord3dv"));
    TexCoord3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glTexCoord3d"));
    TexCoord2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glTexCoord2sv"));
    TexCoord2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort )>(GetProcAddress(handle, "glTexCoord2s"));
    TexCoord2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glTexCoord2iv"));
    TexCoord2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(GetProcAddress(handle, "glTexCoord2i"));
    TexCoord2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glTexCoord2fv"));
    TexCoord2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(GetProcAddress(handle, "glTexCoord2f"));
    TexCoord2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glTexCoord2dv"));
    TexCoord2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(GetProcAddress(handle, "glTexCoord2d"));
    TexCoord1sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glTexCoord1sv"));
    TexCoord1s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort )>(GetProcAddress(handle, "glTexCoord1s"));
    TexCoord1iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glTexCoord1iv"));
    TexCoord1i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(GetProcAddress(handle, "glTexCoord1i"));
    TexCoord1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glTexCoord1fv"));
    TexCoord1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(GetProcAddress(handle, "glTexCoord1f"));
    TexCoord1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glTexCoord1dv"));
    TexCoord1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(GetProcAddress(handle, "glTexCoord1d"));
    Rectsv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *, const GLshort *)>(GetProcAddress(handle, "glRectsv"));
    Rects = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glRects"));
    Rectiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *, const GLint *)>(GetProcAddress(handle, "glRectiv"));
    Recti = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(GetProcAddress(handle, "glRecti"));
    Rectfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *, const GLfloat *)>(GetProcAddress(handle, "glRectfv"));
    Rectf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glRectf"));
    Rectdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *, const GLdouble *)>(GetProcAddress(handle, "glRectdv"));
    Rectd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glRectd"));
    RasterPos4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glRasterPos4sv"));
    RasterPos4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glRasterPos4s"));
    RasterPos4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glRasterPos4iv"));
    RasterPos4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(GetProcAddress(handle, "glRasterPos4i"));
    RasterPos4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glRasterPos4fv"));
    RasterPos4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glRasterPos4f"));
    RasterPos4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glRasterPos4dv"));
    RasterPos4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glRasterPos4d"));
    RasterPos3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glRasterPos3sv"));
    RasterPos3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glRasterPos3s"));
    RasterPos3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glRasterPos3iv"));
    RasterPos3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(GetProcAddress(handle, "glRasterPos3i"));
    RasterPos3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glRasterPos3fv"));
    RasterPos3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glRasterPos3f"));
    RasterPos3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glRasterPos3dv"));
    RasterPos3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glRasterPos3d"));
    RasterPos2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glRasterPos2sv"));
    RasterPos2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort )>(GetProcAddress(handle, "glRasterPos2s"));
    RasterPos2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glRasterPos2iv"));
    RasterPos2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(GetProcAddress(handle, "glRasterPos2i"));
    RasterPos2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glRasterPos2fv"));
    RasterPos2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(GetProcAddress(handle, "glRasterPos2f"));
    RasterPos2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glRasterPos2dv"));
    RasterPos2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(GetProcAddress(handle, "glRasterPos2d"));
    Normal3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glNormal3sv"));
    Normal3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glNormal3s"));
    Normal3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glNormal3iv"));
    Normal3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(GetProcAddress(handle, "glNormal3i"));
    Normal3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glNormal3fv"));
    Normal3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glNormal3f"));
    Normal3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glNormal3dv"));
    Normal3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glNormal3d"));
    Normal3bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLbyte *)>(GetProcAddress(handle, "glNormal3bv"));
    Normal3b = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbyte , GLbyte , GLbyte )>(GetProcAddress(handle, "glNormal3b"));
    Indexsv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glIndexsv"));
    Indexs = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort )>(GetProcAddress(handle, "glIndexs"));
    Indexiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glIndexiv"));
    Indexi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(GetProcAddress(handle, "glIndexi"));
    Indexfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glIndexfv"));
    Indexf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(GetProcAddress(handle, "glIndexf"));
    Indexdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glIndexdv"));
    Indexd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(GetProcAddress(handle, "glIndexd"));
    End = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glEnd"));
    EdgeFlagv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLboolean *)>(GetProcAddress(handle, "glEdgeFlagv"));
    EdgeFlag = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLboolean )>(GetProcAddress(handle, "glEdgeFlag"));
    Color4usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLushort *)>(GetProcAddress(handle, "glColor4usv"));
    Color4us = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLushort , GLushort , GLushort , GLushort )>(GetProcAddress(handle, "glColor4us"));
    Color4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLuint *)>(GetProcAddress(handle, "glColor4uiv"));
    Color4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , GLuint )>(GetProcAddress(handle, "glColor4ui"));
    Color4ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(GetProcAddress(handle, "glColor4ubv"));
    Color4ub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte , GLubyte , GLubyte , GLubyte )>(GetProcAddress(handle, "glColor4ub"));
    Color4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glColor4sv"));
    Color4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glColor4s"));
    Color4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glColor4iv"));
    Color4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(GetProcAddress(handle, "glColor4i"));
    Color4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glColor4fv"));
    Color4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glColor4f"));
    Color4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glColor4dv"));
    Color4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glColor4d"));
    Color4bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLbyte *)>(GetProcAddress(handle, "glColor4bv"));
    Color4b = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbyte , GLbyte , GLbyte , GLbyte )>(GetProcAddress(handle, "glColor4b"));
    Color3usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLushort *)>(GetProcAddress(handle, "glColor3usv"));
    Color3us = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLushort , GLushort , GLushort )>(GetProcAddress(handle, "glColor3us"));
    Color3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLuint *)>(GetProcAddress(handle, "glColor3uiv"));
    Color3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(GetProcAddress(handle, "glColor3ui"));
    Color3ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(GetProcAddress(handle, "glColor3ubv"));
    Color3ub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte , GLubyte , GLubyte )>(GetProcAddress(handle, "glColor3ub"));
    Color3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(GetProcAddress(handle, "glColor3sv"));
    Color3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(GetProcAddress(handle, "glColor3s"));
    Color3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(GetProcAddress(handle, "glColor3iv"));
    Color3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(GetProcAddress(handle, "glColor3i"));
    Color3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(GetProcAddress(handle, "glColor3fv"));
    Color3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(GetProcAddress(handle, "glColor3f"));
    Color3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(GetProcAddress(handle, "glColor3dv"));
    Color3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(GetProcAddress(handle, "glColor3d"));
    Color3bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLbyte *)>(GetProcAddress(handle, "glColor3bv"));
    Color3b = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbyte , GLbyte , GLbyte )>(GetProcAddress(handle, "glColor3b"));
    Bitmap = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLsizei , GLfloat , GLfloat , GLfloat , GLfloat , const GLubyte *)>(GetProcAddress(handle, "glBitmap"));
    Begin = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glBegin"));
    ListBase = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(GetProcAddress(handle, "glListBase"));
    GenLists = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)(GLsizei )>(GetProcAddress(handle, "glGenLists"));
    DeleteLists = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei )>(GetProcAddress(handle, "glDeleteLists"));
    CallLists = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLenum , const GLvoid *)>(GetProcAddress(handle, "glCallLists"));
    CallList = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(GetProcAddress(handle, "glCallList"));
    EndList = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glEndList"));
    NewList = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(GetProcAddress(handle, "glNewList"));
#else
    Translatef = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glTranslatef"));
    Translated = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glTranslated"));
    Scalef = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glScalef"));
    Scaled = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glScaled"));
    Rotatef = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glRotatef"));
    Rotated = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glRotated"));
    PushMatrix = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glPushMatrix"));
    PopMatrix = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glPopMatrix"));
    Ortho = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glOrtho"));
    MultMatrixd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glMultMatrixd"));
    MultMatrixf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glMultMatrixf"));
    MatrixMode = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glMatrixMode"));
    LoadMatrixd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glLoadMatrixd"));
    LoadMatrixf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glLoadMatrixf"));
    LoadIdentity = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glLoadIdentity"));
    Frustum = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glFrustum"));
    IsList = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIsList"));
    GetTexGeniv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetTexGeniv"));
    GetTexGenfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetTexGenfv"));
    GetTexGendv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLdouble *)>(context->getProcAddress("glGetTexGendv"));
    GetTexEnviv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetTexEnviv"));
    GetTexEnvfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetTexEnvfv"));
    GetPolygonStipple = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte *)>(context->getProcAddress("glGetPolygonStipple"));
    GetPixelMapusv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLushort *)>(context->getProcAddress("glGetPixelMapusv"));
    GetPixelMapuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint *)>(context->getProcAddress("glGetPixelMapuiv"));
    GetPixelMapfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat *)>(context->getProcAddress("glGetPixelMapfv"));
    GetMaterialiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetMaterialiv"));
    GetMaterialfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetMaterialfv"));
    GetMapiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetMapiv"));
    GetMapfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetMapfv"));
    GetMapdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLdouble *)>(context->getProcAddress("glGetMapdv"));
    GetLightiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetLightiv"));
    GetLightfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetLightfv"));
    GetClipPlane = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble *)>(context->getProcAddress("glGetClipPlane"));
    DrawPixels = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glDrawPixels"));
    CopyPixels = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLsizei , GLsizei , GLenum )>(context->getProcAddress("glCopyPixels"));
    PixelMapusv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLushort *)>(context->getProcAddress("glPixelMapusv"));
    PixelMapuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLuint *)>(context->getProcAddress("glPixelMapuiv"));
    PixelMapfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLfloat *)>(context->getProcAddress("glPixelMapfv"));
    PixelTransferi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(context->getProcAddress("glPixelTransferi"));
    PixelTransferf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(context->getProcAddress("glPixelTransferf"));
    PixelZoom = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(context->getProcAddress("glPixelZoom"));
    AlphaFunc = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(context->getProcAddress("glAlphaFunc"));
    EvalPoint2 = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(context->getProcAddress("glEvalPoint2"));
    EvalMesh2 = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint )>(context->getProcAddress("glEvalMesh2"));
    EvalPoint1 = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(context->getProcAddress("glEvalPoint1"));
    EvalMesh1 = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint )>(context->getProcAddress("glEvalMesh1"));
    EvalCoord2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glEvalCoord2fv"));
    EvalCoord2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(context->getProcAddress("glEvalCoord2f"));
    EvalCoord2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glEvalCoord2dv"));
    EvalCoord2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(context->getProcAddress("glEvalCoord2d"));
    EvalCoord1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glEvalCoord1fv"));
    EvalCoord1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glEvalCoord1f"));
    EvalCoord1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glEvalCoord1dv"));
    EvalCoord1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(context->getProcAddress("glEvalCoord1d"));
    MapGrid2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLfloat , GLfloat , GLint , GLfloat , GLfloat )>(context->getProcAddress("glMapGrid2f"));
    MapGrid2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLdouble , GLdouble , GLint , GLdouble , GLdouble )>(context->getProcAddress("glMapGrid2d"));
    MapGrid1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLfloat , GLfloat )>(context->getProcAddress("glMapGrid1f"));
    MapGrid1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLdouble , GLdouble )>(context->getProcAddress("glMapGrid1d"));
    Map2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat , GLfloat , GLint , GLint , GLfloat , GLfloat , GLint , GLint , const GLfloat *)>(context->getProcAddress("glMap2f"));
    Map2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble , GLdouble , GLint , GLint , GLdouble , GLdouble , GLint , GLint , const GLdouble *)>(context->getProcAddress("glMap2d"));
    Map1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat , GLfloat , GLint , GLint , const GLfloat *)>(context->getProcAddress("glMap1f"));
    Map1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble , GLdouble , GLint , GLint , const GLdouble *)>(context->getProcAddress("glMap1d"));
    PushAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbitfield )>(context->getProcAddress("glPushAttrib"));
    PopAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glPopAttrib"));
    Accum = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(context->getProcAddress("glAccum"));
    IndexMask = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glIndexMask"));
    ClearIndex = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glClearIndex"));
    ClearAccum = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glClearAccum"));
    PushName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glPushName"));
    PopName = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glPopName"));
    PassThrough = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glPassThrough"));
    LoadName = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glLoadName"));
    InitNames = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glInitNames"));
    RenderMode = reinterpret_cast<GLint (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glRenderMode"));
    SelectBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glSelectBuffer"));
    FeedbackBuffer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLenum , GLfloat *)>(context->getProcAddress("glFeedbackBuffer"));
    TexGeniv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glTexGeniv"));
    TexGeni = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(context->getProcAddress("glTexGeni"));
    TexGenfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(context->getProcAddress("glTexGenfv"));
    TexGenf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(context->getProcAddress("glTexGenf"));
    TexGendv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLdouble *)>(context->getProcAddress("glTexGendv"));
    TexGend = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLdouble )>(context->getProcAddress("glTexGend"));
    TexEnviv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glTexEnviv"));
    TexEnvi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(context->getProcAddress("glTexEnvi"));
    TexEnvfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(context->getProcAddress("glTexEnvfv"));
    TexEnvf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(context->getProcAddress("glTexEnvf"));
    ShadeModel = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glShadeModel"));
    PolygonStipple = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(context->getProcAddress("glPolygonStipple"));
    Materialiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glMaterialiv"));
    Materiali = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(context->getProcAddress("glMateriali"));
    Materialfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(context->getProcAddress("glMaterialfv"));
    Materialf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(context->getProcAddress("glMaterialf"));
    LineStipple = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLushort )>(context->getProcAddress("glLineStipple"));
    LightModeliv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(context->getProcAddress("glLightModeliv"));
    LightModeli = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(context->getProcAddress("glLightModeli"));
    LightModelfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(context->getProcAddress("glLightModelfv"));
    LightModelf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(context->getProcAddress("glLightModelf"));
    Lightiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glLightiv"));
    Lighti = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(context->getProcAddress("glLighti"));
    Lightfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(context->getProcAddress("glLightfv"));
    Lightf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(context->getProcAddress("glLightf"));
    Fogiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(context->getProcAddress("glFogiv"));
    Fogi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(context->getProcAddress("glFogi"));
    Fogfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(context->getProcAddress("glFogfv"));
    Fogf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(context->getProcAddress("glFogf"));
    ColorMaterial = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glColorMaterial"));
    ClipPlane = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLdouble *)>(context->getProcAddress("glClipPlane"));
    Vertex4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glVertex4sv"));
    Vertex4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(context->getProcAddress("glVertex4s"));
    Vertex4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glVertex4iv"));
    Vertex4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(context->getProcAddress("glVertex4i"));
    Vertex4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glVertex4fv"));
    Vertex4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glVertex4f"));
    Vertex4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glVertex4dv"));
    Vertex4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glVertex4d"));
    Vertex3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glVertex3sv"));
    Vertex3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(context->getProcAddress("glVertex3s"));
    Vertex3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glVertex3iv"));
    Vertex3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(context->getProcAddress("glVertex3i"));
    Vertex3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glVertex3fv"));
    Vertex3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glVertex3f"));
    Vertex3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glVertex3dv"));
    Vertex3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glVertex3d"));
    Vertex2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glVertex2sv"));
    Vertex2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort )>(context->getProcAddress("glVertex2s"));
    Vertex2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glVertex2iv"));
    Vertex2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(context->getProcAddress("glVertex2i"));
    Vertex2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glVertex2fv"));
    Vertex2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(context->getProcAddress("glVertex2f"));
    Vertex2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glVertex2dv"));
    Vertex2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(context->getProcAddress("glVertex2d"));
    TexCoord4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glTexCoord4sv"));
    TexCoord4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(context->getProcAddress("glTexCoord4s"));
    TexCoord4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glTexCoord4iv"));
    TexCoord4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(context->getProcAddress("glTexCoord4i"));
    TexCoord4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glTexCoord4fv"));
    TexCoord4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glTexCoord4f"));
    TexCoord4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glTexCoord4dv"));
    TexCoord4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glTexCoord4d"));
    TexCoord3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glTexCoord3sv"));
    TexCoord3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(context->getProcAddress("glTexCoord3s"));
    TexCoord3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glTexCoord3iv"));
    TexCoord3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(context->getProcAddress("glTexCoord3i"));
    TexCoord3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glTexCoord3fv"));
    TexCoord3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glTexCoord3f"));
    TexCoord3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glTexCoord3dv"));
    TexCoord3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glTexCoord3d"));
    TexCoord2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glTexCoord2sv"));
    TexCoord2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort )>(context->getProcAddress("glTexCoord2s"));
    TexCoord2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glTexCoord2iv"));
    TexCoord2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(context->getProcAddress("glTexCoord2i"));
    TexCoord2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glTexCoord2fv"));
    TexCoord2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(context->getProcAddress("glTexCoord2f"));
    TexCoord2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glTexCoord2dv"));
    TexCoord2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(context->getProcAddress("glTexCoord2d"));
    TexCoord1sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glTexCoord1sv"));
    TexCoord1s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort )>(context->getProcAddress("glTexCoord1s"));
    TexCoord1iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glTexCoord1iv"));
    TexCoord1i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(context->getProcAddress("glTexCoord1i"));
    TexCoord1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glTexCoord1fv"));
    TexCoord1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glTexCoord1f"));
    TexCoord1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glTexCoord1dv"));
    TexCoord1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(context->getProcAddress("glTexCoord1d"));
    Rectsv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *, const GLshort *)>(context->getProcAddress("glRectsv"));
    Rects = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(context->getProcAddress("glRects"));
    Rectiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *, const GLint *)>(context->getProcAddress("glRectiv"));
    Recti = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(context->getProcAddress("glRecti"));
    Rectfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *, const GLfloat *)>(context->getProcAddress("glRectfv"));
    Rectf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glRectf"));
    Rectdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *, const GLdouble *)>(context->getProcAddress("glRectdv"));
    Rectd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glRectd"));
    RasterPos4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glRasterPos4sv"));
    RasterPos4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(context->getProcAddress("glRasterPos4s"));
    RasterPos4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glRasterPos4iv"));
    RasterPos4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(context->getProcAddress("glRasterPos4i"));
    RasterPos4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glRasterPos4fv"));
    RasterPos4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glRasterPos4f"));
    RasterPos4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glRasterPos4dv"));
    RasterPos4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glRasterPos4d"));
    RasterPos3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glRasterPos3sv"));
    RasterPos3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(context->getProcAddress("glRasterPos3s"));
    RasterPos3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glRasterPos3iv"));
    RasterPos3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(context->getProcAddress("glRasterPos3i"));
    RasterPos3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glRasterPos3fv"));
    RasterPos3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glRasterPos3f"));
    RasterPos3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glRasterPos3dv"));
    RasterPos3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glRasterPos3d"));
    RasterPos2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glRasterPos2sv"));
    RasterPos2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort )>(context->getProcAddress("glRasterPos2s"));
    RasterPos2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glRasterPos2iv"));
    RasterPos2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(context->getProcAddress("glRasterPos2i"));
    RasterPos2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glRasterPos2fv"));
    RasterPos2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(context->getProcAddress("glRasterPos2f"));
    RasterPos2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glRasterPos2dv"));
    RasterPos2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(context->getProcAddress("glRasterPos2d"));
    Normal3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glNormal3sv"));
    Normal3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(context->getProcAddress("glNormal3s"));
    Normal3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glNormal3iv"));
    Normal3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(context->getProcAddress("glNormal3i"));
    Normal3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glNormal3fv"));
    Normal3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glNormal3f"));
    Normal3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glNormal3dv"));
    Normal3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glNormal3d"));
    Normal3bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLbyte *)>(context->getProcAddress("glNormal3bv"));
    Normal3b = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbyte , GLbyte , GLbyte )>(context->getProcAddress("glNormal3b"));
    Indexsv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glIndexsv"));
    Indexs = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort )>(context->getProcAddress("glIndexs"));
    Indexiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glIndexiv"));
    Indexi = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(context->getProcAddress("glIndexi"));
    Indexfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glIndexfv"));
    Indexf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glIndexf"));
    Indexdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glIndexdv"));
    Indexd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(context->getProcAddress("glIndexd"));
    End = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glEnd"));
    EdgeFlagv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLboolean *)>(context->getProcAddress("glEdgeFlagv"));
    EdgeFlag = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLboolean )>(context->getProcAddress("glEdgeFlag"));
    Color4usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLushort *)>(context->getProcAddress("glColor4usv"));
    Color4us = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLushort , GLushort , GLushort , GLushort )>(context->getProcAddress("glColor4us"));
    Color4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLuint *)>(context->getProcAddress("glColor4uiv"));
    Color4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glColor4ui"));
    Color4ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(context->getProcAddress("glColor4ubv"));
    Color4ub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte , GLubyte , GLubyte , GLubyte )>(context->getProcAddress("glColor4ub"));
    Color4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glColor4sv"));
    Color4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort , GLshort )>(context->getProcAddress("glColor4s"));
    Color4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glColor4iv"));
    Color4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint , GLint )>(context->getProcAddress("glColor4i"));
    Color4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glColor4fv"));
    Color4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glColor4f"));
    Color4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glColor4dv"));
    Color4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glColor4d"));
    Color4bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLbyte *)>(context->getProcAddress("glColor4bv"));
    Color4b = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbyte , GLbyte , GLbyte , GLbyte )>(context->getProcAddress("glColor4b"));
    Color3usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLushort *)>(context->getProcAddress("glColor3usv"));
    Color3us = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLushort , GLushort , GLushort )>(context->getProcAddress("glColor3us"));
    Color3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLuint *)>(context->getProcAddress("glColor3uiv"));
    Color3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glColor3ui"));
    Color3ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(context->getProcAddress("glColor3ubv"));
    Color3ub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte , GLubyte , GLubyte )>(context->getProcAddress("glColor3ub"));
    Color3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glColor3sv"));
    Color3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(context->getProcAddress("glColor3s"));
    Color3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glColor3iv"));
    Color3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(context->getProcAddress("glColor3i"));
    Color3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glColor3fv"));
    Color3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glColor3f"));
    Color3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glColor3dv"));
    Color3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glColor3d"));
    Color3bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLbyte *)>(context->getProcAddress("glColor3bv"));
    Color3b = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbyte , GLbyte , GLbyte )>(context->getProcAddress("glColor3b"));
    Bitmap = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLsizei , GLfloat , GLfloat , GLfloat , GLfloat , const GLubyte *)>(context->getProcAddress("glBitmap"));
    Begin = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glBegin"));
    ListBase = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glListBase"));
    GenLists = reinterpret_cast<GLuint (QOPENGLF_APIENTRYP)(GLsizei )>(context->getProcAddress("glGenLists"));
    DeleteLists = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLsizei )>(context->getProcAddress("glDeleteLists"));
    CallLists = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLenum , const GLvoid *)>(context->getProcAddress("glCallLists"));
    CallList = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glCallList"));
    EndList = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glEndList"));
    NewList = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum )>(context->getProcAddress("glNewList"));
#endif

}

QOpenGLVersionStatus QOpenGLFunctions_1_0_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 0, QOpenGLVersionStatus::DeprecatedStatus);
}

QOpenGLFunctions_1_1_DeprecatedBackend::QOpenGLFunctions_1_1_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.1 deprecated functions
#if defined(Q_OS_WIN)
    HMODULE handle = static_cast<HMODULE>(QOpenGLContext::openGLModuleHandle());
    if (!handle)
        handle = GetModuleHandleA("opengl32.dll");
    PushClientAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbitfield )>(GetProcAddress(handle, "glPushClientAttrib"));
    PopClientAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(GetProcAddress(handle, "glPopClientAttrib"));
    Indexubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(GetProcAddress(handle, "glIndexubv"));
    Indexub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte )>(GetProcAddress(handle, "glIndexub"));
    PrioritizeTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *, const GLfloat *)>(GetProcAddress(handle, "glPrioritizeTextures"));
    AreTexturesResident = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *, GLboolean *)>(GetProcAddress(handle, "glAreTexturesResident"));
    VertexPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLenum , GLsizei , const GLvoid *)>(GetProcAddress(handle, "glVertexPointer"));
    TexCoordPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLenum , GLsizei , const GLvoid *)>(GetProcAddress(handle, "glTexCoordPointer"));
    NormalPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLvoid *)>(GetProcAddress(handle, "glNormalPointer"));
    InterleavedArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLvoid *)>(GetProcAddress(handle, "glInterleavedArrays"));
    GetPointerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLvoid * *)>(GetProcAddress(handle, "glGetPointerv"));
    IndexPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLvoid *)>(GetProcAddress(handle, "glIndexPointer"));
    EnableClientState = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glEnableClientState"));
    EdgeFlagPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLvoid *)>(GetProcAddress(handle, "glEdgeFlagPointer"));
    DisableClientState = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(GetProcAddress(handle, "glDisableClientState"));
    ColorPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLenum , GLsizei , const GLvoid *)>(GetProcAddress(handle, "glColorPointer"));
    ArrayElement = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(GetProcAddress(handle, "glArrayElement"));
#else
    PushClientAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbitfield )>(context->getProcAddress("glPushClientAttrib"));
    PopClientAttrib = reinterpret_cast<void (QOPENGLF_APIENTRYP)()>(context->getProcAddress("glPopClientAttrib"));
    Indexubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(context->getProcAddress("glIndexubv"));
    Indexub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte )>(context->getProcAddress("glIndexub"));
    PrioritizeTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *, const GLfloat *)>(context->getProcAddress("glPrioritizeTextures"));
    AreTexturesResident = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *, GLboolean *)>(context->getProcAddress("glAreTexturesResident"));
    VertexPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glVertexPointer"));
    TexCoordPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glTexCoordPointer"));
    NormalPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glNormalPointer"));
    InterleavedArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glInterleavedArrays"));
    GetPointerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLvoid * *)>(context->getProcAddress("glGetPointerv"));
    IndexPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glIndexPointer"));
    EnableClientState = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glEnableClientState"));
    EdgeFlagPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLvoid *)>(context->getProcAddress("glEdgeFlagPointer"));
    DisableClientState = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glDisableClientState"));
    ColorPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glColorPointer"));
    ArrayElement = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint )>(context->getProcAddress("glArrayElement"));
#endif

}

QOpenGLVersionStatus QOpenGLFunctions_1_1_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 1, QOpenGLVersionStatus::DeprecatedStatus);
}

QOpenGLFunctions_1_2_DeprecatedBackend::QOpenGLFunctions_1_2_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.2 deprecated functions
    ResetMinmax = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glResetMinmax"));
    ResetHistogram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glResetHistogram"));
    Minmax = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLboolean )>(context->getProcAddress("glMinmax"));
    Histogram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLboolean )>(context->getProcAddress("glHistogram"));
    GetMinmaxParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetMinmaxParameteriv"));
    GetMinmaxParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetMinmaxParameterfv"));
    GetMinmax = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLboolean , GLenum , GLenum , GLvoid *)>(context->getProcAddress("glGetMinmax"));
    GetHistogramParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetHistogramParameteriv"));
    GetHistogramParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetHistogramParameterfv"));
    GetHistogram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLboolean , GLenum , GLenum , GLvoid *)>(context->getProcAddress("glGetHistogram"));
    SeparableFilter2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *, const GLvoid *)>(context->getProcAddress("glSeparableFilter2D"));
    GetSeparableFilter = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLvoid *, GLvoid *, GLvoid *)>(context->getProcAddress("glGetSeparableFilter"));
    GetConvolutionParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetConvolutionParameteriv"));
    GetConvolutionParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetConvolutionParameterfv"));
    GetConvolutionFilter = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLvoid *)>(context->getProcAddress("glGetConvolutionFilter"));
    CopyConvolutionFilter2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint , GLint , GLsizei , GLsizei )>(context->getProcAddress("glCopyConvolutionFilter2D"));
    CopyConvolutionFilter1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint , GLint , GLsizei )>(context->getProcAddress("glCopyConvolutionFilter1D"));
    ConvolutionParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glConvolutionParameteriv"));
    ConvolutionParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(context->getProcAddress("glConvolutionParameteri"));
    ConvolutionParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(context->getProcAddress("glConvolutionParameterfv"));
    ConvolutionParameterf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(context->getProcAddress("glConvolutionParameterf"));
    ConvolutionFilter2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glConvolutionFilter2D"));
    ConvolutionFilter1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glConvolutionFilter1D"));
    CopyColorSubTable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLint , GLint , GLsizei )>(context->getProcAddress("glCopyColorSubTable"));
    ColorSubTable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glColorSubTable"));
    GetColorTableParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetColorTableParameteriv"));
    GetColorTableParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetColorTableParameterfv"));
    GetColorTable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLvoid *)>(context->getProcAddress("glGetColorTable"));
    CopyColorTable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint , GLint , GLsizei )>(context->getProcAddress("glCopyColorTable"));
    ColorTableParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glColorTableParameteriv"));
    ColorTableParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(context->getProcAddress("glColorTableParameterfv"));
    ColorTable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glColorTable"));

}

QOpenGLVersionStatus QOpenGLFunctions_1_2_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 2, QOpenGLVersionStatus::DeprecatedStatus);
}

QOpenGLFunctions_1_3_DeprecatedBackend::QOpenGLFunctions_1_3_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.3 deprecated functions
    MultTransposeMatrixd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glMultTransposeMatrixd"));
    MultTransposeMatrixf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glMultTransposeMatrixf"));
    LoadTransposeMatrixd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glLoadTransposeMatrixd"));
    LoadTransposeMatrixf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glLoadTransposeMatrixf"));
    MultiTexCoord4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLshort *)>(context->getProcAddress("glMultiTexCoord4sv"));
    MultiTexCoord4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLshort , GLshort , GLshort , GLshort )>(context->getProcAddress("glMultiTexCoord4s"));
    MultiTexCoord4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(context->getProcAddress("glMultiTexCoord4iv"));
    MultiTexCoord4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint )>(context->getProcAddress("glMultiTexCoord4i"));
    MultiTexCoord4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(context->getProcAddress("glMultiTexCoord4fv"));
    MultiTexCoord4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glMultiTexCoord4f"));
    MultiTexCoord4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLdouble *)>(context->getProcAddress("glMultiTexCoord4dv"));
    MultiTexCoord4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glMultiTexCoord4d"));
    MultiTexCoord3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLshort *)>(context->getProcAddress("glMultiTexCoord3sv"));
    MultiTexCoord3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLshort , GLshort , GLshort )>(context->getProcAddress("glMultiTexCoord3s"));
    MultiTexCoord3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(context->getProcAddress("glMultiTexCoord3iv"));
    MultiTexCoord3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint )>(context->getProcAddress("glMultiTexCoord3i"));
    MultiTexCoord3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(context->getProcAddress("glMultiTexCoord3fv"));
    MultiTexCoord3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glMultiTexCoord3f"));
    MultiTexCoord3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLdouble *)>(context->getProcAddress("glMultiTexCoord3dv"));
    MultiTexCoord3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glMultiTexCoord3d"));
    MultiTexCoord2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLshort *)>(context->getProcAddress("glMultiTexCoord2sv"));
    MultiTexCoord2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLshort , GLshort )>(context->getProcAddress("glMultiTexCoord2s"));
    MultiTexCoord2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(context->getProcAddress("glMultiTexCoord2iv"));
    MultiTexCoord2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint )>(context->getProcAddress("glMultiTexCoord2i"));
    MultiTexCoord2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(context->getProcAddress("glMultiTexCoord2fv"));
    MultiTexCoord2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat , GLfloat )>(context->getProcAddress("glMultiTexCoord2f"));
    MultiTexCoord2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLdouble *)>(context->getProcAddress("glMultiTexCoord2dv"));
    MultiTexCoord2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble , GLdouble )>(context->getProcAddress("glMultiTexCoord2d"));
    MultiTexCoord1sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLshort *)>(context->getProcAddress("glMultiTexCoord1sv"));
    MultiTexCoord1s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLshort )>(context->getProcAddress("glMultiTexCoord1s"));
    MultiTexCoord1iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLint *)>(context->getProcAddress("glMultiTexCoord1iv"));
    MultiTexCoord1i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(context->getProcAddress("glMultiTexCoord1i"));
    MultiTexCoord1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLfloat *)>(context->getProcAddress("glMultiTexCoord1fv"));
    MultiTexCoord1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLfloat )>(context->getProcAddress("glMultiTexCoord1f"));
    MultiTexCoord1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLdouble *)>(context->getProcAddress("glMultiTexCoord1dv"));
    MultiTexCoord1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLdouble )>(context->getProcAddress("glMultiTexCoord1d"));
    ClientActiveTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glClientActiveTexture"));

}

QOpenGLVersionStatus QOpenGLFunctions_1_3_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 3, QOpenGLVersionStatus::DeprecatedStatus);
}

QOpenGLFunctions_1_4_DeprecatedBackend::QOpenGLFunctions_1_4_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 1.4 deprecated functions
    WindowPos3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glWindowPos3sv"));
    WindowPos3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(context->getProcAddress("glWindowPos3s"));
    WindowPos3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glWindowPos3iv"));
    WindowPos3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(context->getProcAddress("glWindowPos3i"));
    WindowPos3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glWindowPos3fv"));
    WindowPos3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glWindowPos3f"));
    WindowPos3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glWindowPos3dv"));
    WindowPos3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glWindowPos3d"));
    WindowPos2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glWindowPos2sv"));
    WindowPos2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort )>(context->getProcAddress("glWindowPos2s"));
    WindowPos2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glWindowPos2iv"));
    WindowPos2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint )>(context->getProcAddress("glWindowPos2i"));
    WindowPos2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glWindowPos2fv"));
    WindowPos2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat )>(context->getProcAddress("glWindowPos2f"));
    WindowPos2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glWindowPos2dv"));
    WindowPos2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble )>(context->getProcAddress("glWindowPos2d"));
    SecondaryColorPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glSecondaryColorPointer"));
    SecondaryColor3usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLushort *)>(context->getProcAddress("glSecondaryColor3usv"));
    SecondaryColor3us = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLushort , GLushort , GLushort )>(context->getProcAddress("glSecondaryColor3us"));
    SecondaryColor3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLuint *)>(context->getProcAddress("glSecondaryColor3uiv"));
    SecondaryColor3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glSecondaryColor3ui"));
    SecondaryColor3ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLubyte *)>(context->getProcAddress("glSecondaryColor3ubv"));
    SecondaryColor3ub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLubyte , GLubyte , GLubyte )>(context->getProcAddress("glSecondaryColor3ub"));
    SecondaryColor3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLshort *)>(context->getProcAddress("glSecondaryColor3sv"));
    SecondaryColor3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLshort , GLshort , GLshort )>(context->getProcAddress("glSecondaryColor3s"));
    SecondaryColor3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLint *)>(context->getProcAddress("glSecondaryColor3iv"));
    SecondaryColor3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLint , GLint , GLint )>(context->getProcAddress("glSecondaryColor3i"));
    SecondaryColor3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glSecondaryColor3fv"));
    SecondaryColor3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glSecondaryColor3f"));
    SecondaryColor3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glSecondaryColor3dv"));
    SecondaryColor3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glSecondaryColor3d"));
    SecondaryColor3bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLbyte *)>(context->getProcAddress("glSecondaryColor3bv"));
    SecondaryColor3b = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLbyte , GLbyte , GLbyte )>(context->getProcAddress("glSecondaryColor3b"));
    FogCoordPointer = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glFogCoordPointer"));
    FogCoorddv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLdouble *)>(context->getProcAddress("glFogCoorddv"));
    FogCoordd = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLdouble )>(context->getProcAddress("glFogCoordd"));
    FogCoordfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(const GLfloat *)>(context->getProcAddress("glFogCoordfv"));
    FogCoordf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLfloat )>(context->getProcAddress("glFogCoordf"));

}

QOpenGLVersionStatus QOpenGLFunctions_1_4_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(1, 4, QOpenGLVersionStatus::DeprecatedStatus);
}

QOpenGLFunctions_2_0_DeprecatedBackend::QOpenGLFunctions_2_0_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 2.0 deprecated functions
    VertexAttrib4usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLushort *)>(context->getProcAddress("glVertexAttrib4usv"));
    VertexAttrib4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttrib4uiv"));
    VertexAttrib4ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLubyte *)>(context->getProcAddress("glVertexAttrib4ubv"));
    VertexAttrib4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib4sv"));
    VertexAttrib4s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLshort , GLshort , GLshort , GLshort )>(context->getProcAddress("glVertexAttrib4s"));
    VertexAttrib4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttrib4iv"));
    VertexAttrib4fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glVertexAttrib4fv"));
    VertexAttrib4f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glVertexAttrib4f"));
    VertexAttrib4dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttrib4dv"));
    VertexAttrib4d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttrib4d"));
    VertexAttrib4bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLbyte *)>(context->getProcAddress("glVertexAttrib4bv"));
    VertexAttrib4Nusv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLushort *)>(context->getProcAddress("glVertexAttrib4Nusv"));
    VertexAttrib4Nuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttrib4Nuiv"));
    VertexAttrib4Nubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLubyte *)>(context->getProcAddress("glVertexAttrib4Nubv"));
    VertexAttrib4Nub = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLubyte , GLubyte , GLubyte , GLubyte )>(context->getProcAddress("glVertexAttrib4Nub"));
    VertexAttrib4Nsv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib4Nsv"));
    VertexAttrib4Niv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttrib4Niv"));
    VertexAttrib4Nbv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLbyte *)>(context->getProcAddress("glVertexAttrib4Nbv"));
    VertexAttrib3sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib3sv"));
    VertexAttrib3s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLshort , GLshort , GLshort )>(context->getProcAddress("glVertexAttrib3s"));
    VertexAttrib3fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glVertexAttrib3fv"));
    VertexAttrib3f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat , GLfloat , GLfloat )>(context->getProcAddress("glVertexAttrib3f"));
    VertexAttrib3dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttrib3dv"));
    VertexAttrib3d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttrib3d"));
    VertexAttrib2sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib2sv"));
    VertexAttrib2s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLshort , GLshort )>(context->getProcAddress("glVertexAttrib2s"));
    VertexAttrib2fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glVertexAttrib2fv"));
    VertexAttrib2f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat , GLfloat )>(context->getProcAddress("glVertexAttrib2f"));
    VertexAttrib2dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttrib2dv"));
    VertexAttrib2d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble , GLdouble )>(context->getProcAddress("glVertexAttrib2d"));
    VertexAttrib1sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttrib1sv"));
    VertexAttrib1s = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLshort )>(context->getProcAddress("glVertexAttrib1s"));
    VertexAttrib1fv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLfloat *)>(context->getProcAddress("glVertexAttrib1fv"));
    VertexAttrib1f = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLfloat )>(context->getProcAddress("glVertexAttrib1f"));
    VertexAttrib1dv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLdouble *)>(context->getProcAddress("glVertexAttrib1dv"));
    VertexAttrib1d = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLdouble )>(context->getProcAddress("glVertexAttrib1d"));

}

QOpenGLVersionStatus QOpenGLFunctions_2_0_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(2, 0, QOpenGLVersionStatus::DeprecatedStatus);
}

QOpenGLFunctions_3_0_DeprecatedBackend::QOpenGLFunctions_3_0_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 3.0 deprecated functions
    VertexAttribI4usv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLushort *)>(context->getProcAddress("glVertexAttribI4usv"));
    VertexAttribI4ubv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLubyte *)>(context->getProcAddress("glVertexAttribI4ubv"));
    VertexAttribI4sv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLshort *)>(context->getProcAddress("glVertexAttribI4sv"));
    VertexAttribI4bv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLbyte *)>(context->getProcAddress("glVertexAttribI4bv"));
    VertexAttribI4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttribI4uiv"));
    VertexAttribI3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttribI3uiv"));
    VertexAttribI2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttribI2uiv"));
    VertexAttribI1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLuint *)>(context->getProcAddress("glVertexAttribI1uiv"));
    VertexAttribI4iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttribI4iv"));
    VertexAttribI3iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttribI3iv"));
    VertexAttribI2iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttribI2iv"));
    VertexAttribI1iv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , const GLint *)>(context->getProcAddress("glVertexAttribI1iv"));
    VertexAttribI4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glVertexAttribI4ui"));
    VertexAttribI3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glVertexAttribI3ui"));
    VertexAttribI2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint , GLuint )>(context->getProcAddress("glVertexAttribI2ui"));
    VertexAttribI1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLuint )>(context->getProcAddress("glVertexAttribI1ui"));
    VertexAttribI4i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint , GLint )>(context->getProcAddress("glVertexAttribI4i"));
    VertexAttribI3i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint , GLint )>(context->getProcAddress("glVertexAttribI3i"));
    VertexAttribI2i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint , GLint )>(context->getProcAddress("glVertexAttribI2i"));
    VertexAttribI1i = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLint )>(context->getProcAddress("glVertexAttribI1i"));

}

QOpenGLVersionStatus QOpenGLFunctions_3_0_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(3, 0, QOpenGLVersionStatus::DeprecatedStatus);
}

QOpenGLFunctions_3_3_DeprecatedBackend::QOpenGLFunctions_3_3_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 3.3 deprecated functions
    SecondaryColorP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glSecondaryColorP3uiv"));
    SecondaryColorP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glSecondaryColorP3ui"));
    ColorP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glColorP4uiv"));
    ColorP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glColorP4ui"));
    ColorP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glColorP3uiv"));
    ColorP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glColorP3ui"));
    NormalP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glNormalP3uiv"));
    NormalP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glNormalP3ui"));
    MultiTexCoordP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glMultiTexCoordP4uiv"));
    MultiTexCoordP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glMultiTexCoordP4ui"));
    MultiTexCoordP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glMultiTexCoordP3uiv"));
    MultiTexCoordP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glMultiTexCoordP3ui"));
    MultiTexCoordP2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glMultiTexCoordP2uiv"));
    MultiTexCoordP2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glMultiTexCoordP2ui"));
    MultiTexCoordP1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLuint *)>(context->getProcAddress("glMultiTexCoordP1uiv"));
    MultiTexCoordP1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint )>(context->getProcAddress("glMultiTexCoordP1ui"));
    TexCoordP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glTexCoordP4uiv"));
    TexCoordP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glTexCoordP4ui"));
    TexCoordP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glTexCoordP3uiv"));
    TexCoordP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glTexCoordP3ui"));
    TexCoordP2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glTexCoordP2uiv"));
    TexCoordP2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glTexCoordP2ui"));
    TexCoordP1uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glTexCoordP1uiv"));
    TexCoordP1ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glTexCoordP1ui"));
    VertexP4uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glVertexP4uiv"));
    VertexP4ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glVertexP4ui"));
    VertexP3uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glVertexP3uiv"));
    VertexP3ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glVertexP3ui"));
    VertexP2uiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , const GLuint *)>(context->getProcAddress("glVertexP2uiv"));
    VertexP2ui = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glVertexP2ui"));

}

QOpenGLVersionStatus QOpenGLFunctions_3_3_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(3, 3, QOpenGLVersionStatus::DeprecatedStatus);
}

QOpenGLFunctions_4_5_DeprecatedBackend::QOpenGLFunctions_4_5_DeprecatedBackend(QOpenGLContext *context)
    : QOpenGLVersionFunctionsBackend(context)
{
    // OpenGL 4.5 deprecated functions
    GetnMinmax = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLboolean , GLenum , GLenum , GLsizei , void *)>(context->getProcAddress("glGetnMinmax"));
    GetnHistogram = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLboolean , GLenum , GLenum , GLsizei , void *)>(context->getProcAddress("glGetnHistogram"));
    GetnSeparableFilter = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , void *, GLsizei , void *, void *)>(context->getProcAddress("glGetnSeparableFilter"));
    GetnConvolutionFilter = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , void *)>(context->getProcAddress("glGetnConvolutionFilter"));
    GetnColorTable = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , void *)>(context->getProcAddress("glGetnColorTable"));
    GetnPolygonStipple = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLubyte *)>(context->getProcAddress("glGetnPolygonStipple"));
    GetnPixelMapusv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLushort *)>(context->getProcAddress("glGetnPixelMapusv"));
    GetnPixelMapuiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLuint *)>(context->getProcAddress("glGetnPixelMapuiv"));
    GetnPixelMapfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLfloat *)>(context->getProcAddress("glGetnPixelMapfv"));
    GetnMapiv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLsizei , GLint *)>(context->getProcAddress("glGetnMapiv"));
    GetnMapfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLsizei , GLfloat *)>(context->getProcAddress("glGetnMapfv"));
    GetnMapdv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLsizei , GLdouble *)>(context->getProcAddress("glGetnMapdv"));

}

QOpenGLVersionStatus QOpenGLFunctions_4_5_DeprecatedBackend::versionStatus()
{
    return QOpenGLVersionStatus(4, 5, QOpenGLVersionStatus::DeprecatedStatus);
}

#else

// No backends for OpenGL ES 2

#endif // !QT_OPENGL_ES_2


