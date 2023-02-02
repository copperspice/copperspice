/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qopengl_vertexarrayobject.h>

#include <qoffscreensurface.h>
#include <qopenglcontext.h>
#include <qopenglfunctions_3_0.h>
#include <qopenglfunctions_3_2_core.h>

#include <qopengl_extensions_p.h>
#include <qopengl_vertexarrayobject_p.h>

class QOpenGLFunctions_3_0;
class QOpenGLFunctions_3_2_Core;

void qtInitializeVertexArrayObjectHelper(QOpenGLVertexArrayObjectHelper *helper, QOpenGLContext *context)
{
    Q_ASSERT(helper);
    Q_ASSERT(context);

    bool tryARB = true;

    if (context->isOpenGLES()) {
        if (context->format().majorVersion() >= 3) {
            QOpenGLES3Helper *es3 = static_cast<QOpenGLExtensions *>(context->functions())->gles3Helper();
            helper->GenVertexArrays = es3->GenVertexArrays;
            helper->DeleteVertexArrays = es3->DeleteVertexArrays;
            helper->BindVertexArray = es3->BindVertexArray;
            helper->IsVertexArray = es3->IsVertexArray;
            tryARB = false;
        } else if (context->hasExtension(QByteArrayLiteral("GL_OES_vertex_array_object"))) {
            helper->GenVertexArrays = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_GenVertexArrays_t>(context->getProcAddress(QByteArrayLiteral("glGenVertexArraysOES")));
            helper->DeleteVertexArrays = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_DeleteVertexArrays_t>(context->getProcAddress(QByteArrayLiteral("glDeleteVertexArraysOES")));
            helper->BindVertexArray = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_BindVertexArray_t>(context->getProcAddress(QByteArrayLiteral("glBindVertexArrayOES")));
            helper->IsVertexArray = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_IsVertexArray_t>(context->getProcAddress(QByteArrayLiteral("glIsVertexArrayOES")));
            tryARB = false;
        }
    } else if (context->hasExtension(QByteArrayLiteral("GL_APPLE_vertex_array_object")) &&
               !context->hasExtension(QByteArrayLiteral("GL_ARB_vertex_array_object"))) {
        helper->GenVertexArrays = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_GenVertexArrays_t>(context->getProcAddress(QByteArrayLiteral("glGenVertexArraysAPPLE")));
        helper->DeleteVertexArrays = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_DeleteVertexArrays_t>(context->getProcAddress(QByteArrayLiteral("glDeleteVertexArraysAPPLE")));
        helper->BindVertexArray = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_BindVertexArray_t>(context->getProcAddress(QByteArrayLiteral("glBindVertexArrayAPPLE")));
        helper->IsVertexArray = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_IsVertexArray_t>(context->getProcAddress(QByteArrayLiteral("glIsVertexArrayAPPLE")));
        tryARB = false;
    }

    if (tryARB && context->hasExtension(QByteArrayLiteral("GL_ARB_vertex_array_object"))) {
        helper->GenVertexArrays = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_GenVertexArrays_t>(context->getProcAddress(QByteArrayLiteral("glGenVertexArrays")));
        helper->DeleteVertexArrays = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_DeleteVertexArrays_t>(context->getProcAddress(QByteArrayLiteral("glDeleteVertexArrays")));
        helper->BindVertexArray = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_BindVertexArray_t>(context->getProcAddress(QByteArrayLiteral("glBindVertexArray")));
        helper->IsVertexArray = reinterpret_cast<QOpenGLVertexArrayObjectHelper::qt_IsVertexArray_t>(context->getProcAddress(QByteArrayLiteral("glIsVertexArray")));
    }
}

class QOpenGLVertexArrayObjectPrivate
{
public:
    QOpenGLVertexArrayObjectPrivate()
        : vao(0), vaoFuncsType(NotSupported), context(nullptr)
    {
    }

    ~QOpenGLVertexArrayObjectPrivate()
    {
        if (vaoFuncsType == ARB || vaoFuncsType == APPLE || vaoFuncsType == OES)
            delete vaoFuncs.helper;
    }

    bool create();
    void destroy();
    void bind();
    void release();
    void _q_contextAboutToBeDestroyed();

    Q_DECLARE_PUBLIC(QOpenGLVertexArrayObject)

    GLuint vao;

    union {
        QOpenGLFunctions_3_0 *core_3_0;
        QOpenGLFunctions_3_2_Core *core_3_2;
        QOpenGLVertexArrayObjectHelper *helper;
    } vaoFuncs;
    enum {
        NotSupported,
        Core_3_0,
        Core_3_2,
        ARB,
        APPLE,
        OES
    } vaoFuncsType;

    QOpenGLContext *context;

 protected:
    QOpenGLVertexArrayObject *q_ptr;
};

bool QOpenGLVertexArrayObjectPrivate::create()
{
    if (vao) {
        qWarning("QOpenGLVertexArrayObject::create() VAO is already created");
        return false;
    }

    Q_Q(QOpenGLVertexArrayObject);

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning("QOpenGLVertexArrayObject::create() requires a valid current OpenGL context");
        return false;
    }

    //Fail early, if context is the same as ctx, it means we have tried to initialize for this context and failed
    if (ctx == context)
        return false;

    context = ctx;
    QObject::connect(context, SIGNAL(aboutToBeDestroyed()), q, SLOT(_q_contextAboutToBeDestroyed()));

    if (ctx->isOpenGLES()) {
        if (ctx->format().majorVersion() >= 3 || ctx->hasExtension(QByteArrayLiteral("GL_OES_vertex_array_object"))) {
            vaoFuncs.helper = new QOpenGLVertexArrayObjectHelper(ctx);
            vaoFuncsType = OES;
            vaoFuncs.helper->glGenVertexArrays(1, &vao);
        }

    } else {
        vaoFuncs.core_3_0 = nullptr;
        vaoFuncsType = NotSupported;

        QSurfaceFormat format = ctx->format();
#ifndef QT_OPENGL_ES_2
        if (format.version() >= qMakePair<int, int>(3,2)) {
            vaoFuncs.core_3_2 = ctx->versionFunctions<QOpenGLFunctions_3_2_Core>();
            vaoFuncsType = Core_3_2;
            vaoFuncs.core_3_2->glGenVertexArrays(1, &vao);
        } else if (format.majorVersion() >= 3) {
            vaoFuncs.core_3_0 = ctx->versionFunctions<QOpenGLFunctions_3_0>();
            vaoFuncsType = Core_3_0;
            vaoFuncs.core_3_0->glGenVertexArrays(1, &vao);
        } else
#endif
        if (ctx->hasExtension(QByteArrayLiteral("GL_ARB_vertex_array_object"))) {
            vaoFuncs.helper = new QOpenGLVertexArrayObjectHelper(ctx);
            vaoFuncsType = ARB;
            vaoFuncs.helper->glGenVertexArrays(1, &vao);
        } else if (ctx->hasExtension(QByteArrayLiteral("GL_APPLE_vertex_array_object"))) {
            vaoFuncs.helper = new QOpenGLVertexArrayObjectHelper(ctx);
            vaoFuncsType = APPLE;
            vaoFuncs.helper->glGenVertexArrays(1, &vao);
        }
    }

    return (vao != 0);
}

void QOpenGLVertexArrayObjectPrivate::destroy()
{
    Q_Q(QOpenGLVertexArrayObject);

    if (context) {
        QObject::disconnect(context, SIGNAL(aboutToBeDestroyed()), q, SLOT(_q_contextAboutToBeDestroyed()));
        context = nullptr;
    }

    if (!vao)
        return;

    switch (vaoFuncsType) {
#ifndef QT_OPENGL_ES_2
    case Core_3_2:
        vaoFuncs.core_3_2->glDeleteVertexArrays(1, &vao);
        break;
    case Core_3_0:
        vaoFuncs.core_3_0->glDeleteVertexArrays(1, &vao);
        break;
#endif
    case ARB:
    case APPLE:
    case OES:
        vaoFuncs.helper->glDeleteVertexArrays(1, &vao);
        break;
    default:
        break;
    }

    vao = 0;
}

/*!
    \internal
*/
void QOpenGLVertexArrayObjectPrivate::_q_contextAboutToBeDestroyed()
{
    destroy();
}

void QOpenGLVertexArrayObjectPrivate::bind()
{
    switch (vaoFuncsType) {
#ifndef QT_OPENGL_ES_2
    case Core_3_2:
        vaoFuncs.core_3_2->glBindVertexArray(vao);
        break;
    case Core_3_0:
        vaoFuncs.core_3_0->glBindVertexArray(vao);
        break;
#endif
    case ARB:
    case APPLE:
    case OES:
        vaoFuncs.helper->glBindVertexArray(vao);
        break;
    default:
        break;
    }
}

void QOpenGLVertexArrayObjectPrivate::release()
{
    switch (vaoFuncsType) {
#ifndef QT_OPENGL_ES_2
    case Core_3_2:
        vaoFuncs.core_3_2->glBindVertexArray(0);
        break;
    case Core_3_0:
        vaoFuncs.core_3_0->glBindVertexArray(0);
        break;
#endif
    case ARB:
    case APPLE:
    case OES:
        vaoFuncs.helper->glBindVertexArray(0);
        break;
    default:
        break;
    }
}


/*!
    \class QOpenGLVertexArrayObject
    \brief The QOpenGLVertexArrayObject class wraps an OpenGL Vertex Array Object.
    \inmodule QtGui
    \since 5.1
    \ingroup painting-3D

    A Vertex Array Object (VAO) is an OpenGL container object that encapsulates
    the state needed to specify per-vertex attribute data to the OpenGL pipeline.
    To put it another way, a VAO remembers the states of buffer objects (see
    QOpenGLBuffer) and their associated state (e.g. vertex attribute divisors).
    This allows a very easy and efficient method of switching between OpenGL buffer
    states for rendering different "objects" in a scene. The QOpenGLVertexArrayObject
    class is a thin wrapper around an OpenGL VAO.

    For the desktop, VAOs are supported as a core feature in OpenGL 3.0 or newer and by the
    GL_ARB_vertex_array_object for older versions. On OpenGL ES 2, VAOs are provided by
    the optional GL_OES_vertex_array_object extension. You can check the version of
    OpenGL with QOpenGLContext::surfaceFormat() and check for the presence of extensions
    with QOpenGLContext::hasExtension().

    As with the other Qt OpenGL classes, QOpenGLVertexArrayObject has a create()
    function to create the underlying OpenGL object. This is to allow the developer to
    ensure that there is a valid current OpenGL context at the time.

    Once you have successfully created a VAO the typical usage pattern is:

    \list
        \li In scene initialization function, for each visual object:
        \list
            \li Bind the VAO
            \li Set vertex data state for this visual object (vertices, normals, texture coordinates etc.)
            \li Unbind (release()) the VAO
        \endlist
        \li In render function, for each visual object:
        \list
            \li Bind the VAO (and shader program if needed)
            \li Call a glDraw*() function
            \li Unbind (release()) the VAO
        \endlist
    \endlist

    The act of binding the VAO in the render function has the effect of restoring
    all of the vertex data state setup in the initialization phase. In this way we can
    set a great deal of state when setting up a VAO and efficiently switch between
    state sets of objects to be rendered. Using VAOs also allows the OpenGL driver
    to amortise the validation checks of the vertex data.

    \note Vertex Array Objects, like all other OpenGL container objects, are specific
    to the context for which they were created and cannot be shared amongst a
    context group.

    \sa QOpenGLVertexArrayObject::Binder, QOpenGLBuffer
*/

/*!
    Creates a QOpenGLVertexArrayObject with the given \a parent. You must call create()
    with a valid OpenGL context before using.
*/
QOpenGLVertexArrayObject::QOpenGLVertexArrayObject(QObject* parent)
    : QObject(parent), d_ptr(new QOpenGLVertexArrayObjectPrivate)
{
   d_ptr->q_ptr = this;
}

/*!
    \internal
*/
QOpenGLVertexArrayObject::QOpenGLVertexArrayObject(QOpenGLVertexArrayObjectPrivate &dd)
    : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

/*!
    Destroys the QOpenGLVertexArrayObject and the underlying OpenGL resource.
*/
QOpenGLVertexArrayObject::~QOpenGLVertexArrayObject()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();

    Q_D(QOpenGLVertexArrayObject);
    QOpenGLContext *oldContext  = nullptr;
    QSurface *oldContextSurface = nullptr;
    QScopedPointer<QOffscreenSurface> offscreenSurface;

    if (d->context && ctx && d->context != ctx) {
        oldContext = ctx;
        oldContextSurface = ctx->surface();
        // Cannot just make the current surface current again with another context.
        // The format may be incompatible and some platforms (iOS) may impose
        // restrictions on using a window with different contexts. Create an
        // offscreen surface (a pbuffer or a hidden window) instead to be safe.
        offscreenSurface.reset(new QOffscreenSurface);
        offscreenSurface->setFormat(d->context->format());
        offscreenSurface->create();
        if (d->context->makeCurrent(offscreenSurface.data())) {
            ctx = d->context;
        } else {
            qWarning("QOpenGLVertexArrayObject::~QOpenGLVertexArrayObject() failed to make VAO's context current");
            ctx = nullptr;
        }
    }

    if (ctx)
        destroy();

    if (oldContext) {
        if (!oldContext->makeCurrent(oldContextSurface))
            qWarning("QOpenGLVertexArrayObject::~QOpenGLVertexArrayObject() failed to restore current context");
    }
}

/*!
    Creates the underlying OpenGL vertex array object. There must be a valid OpenGL context
    that supports vertex array objects current for this function to succeed.

    Returns \c true if the OpenGL vertex array object was successfully created.

    When the return value is \c false, vertex array object support is not available. This
    is not an error: on systems with OpenGL 2.x or OpenGL ES 2.0 vertex array objects may
    not be supported. The application is free to continue execution in this case, but it
    then has to be prepared to operate in a VAO-less manner too. This means that instead
    of merely calling bind(), the value of isCreated() must be checked and the vertex
    arrays has to be initialized in the traditional way when there is no vertex array
    object present.

    \sa isCreated()
*/
bool QOpenGLVertexArrayObject::create()
{
    Q_D(QOpenGLVertexArrayObject);
    return d->create();
}

/*!
    Destroys the underlying OpenGL vertex array object. There must be a valid OpenGL context
    that supports vertex array objects current for this function to succeed.
*/
void QOpenGLVertexArrayObject::destroy()
{
    Q_D(QOpenGLVertexArrayObject);
    d->destroy();
}

/*!
    Returns \c true is the underlying OpenGL vertex array object has been created. If this
    returns \c true and the associated OpenGL context is current, then you are able to bind()
    this object.
*/
bool QOpenGLVertexArrayObject::isCreated() const
{
    Q_D(const QOpenGLVertexArrayObject);
    return (d->vao != 0);
}

/*!
    Returns the id of the underlying OpenGL vertex array object.
*/
GLuint QOpenGLVertexArrayObject::objectId() const
{
    Q_D(const QOpenGLVertexArrayObject);
    return d->vao;
}

/*!
    Binds this vertex array object to the OpenGL binding point. From this point on
    and until release() is called or another vertex array object is bound, any
    modifications made to vertex data state are stored inside this vertex array object.

    If another vertex array object is then bound you can later restore the set of
    state associated with this object by calling bind() on this object once again.
    This allows efficient changes between vertex data states in rendering functions.
*/
void QOpenGLVertexArrayObject::bind()
{
    Q_D(QOpenGLVertexArrayObject);
    d->bind();
}

/*!
    Unbinds this vertex array object by binding the default vertex array object (id = 0).
*/
void QOpenGLVertexArrayObject::release()
{
    Q_D(QOpenGLVertexArrayObject);
    d->release();
}

void QOpenGLVertexArrayObject::_q_contextAboutToBeDestroyed()
{
   Q_D(QOpenGLVertexArrayObject);
   d->_q_contextAboutToBeDestroyed();
}