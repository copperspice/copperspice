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

#ifndef QOPENGLVERTEXARRAYOBJECT_P_H
#define QOPENGLVERTEXARRAYOBJECT_P_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qopengl.h>

class QOpenGLVertexArrayObjectHelper;
class QOpenGLContext;

void Q_GUI_EXPORT qtInitializeVertexArrayObjectHelper(QOpenGLVertexArrayObjectHelper *helper, QOpenGLContext *context);

class QOpenGLVertexArrayObjectHelper
{
 public:
    explicit inline QOpenGLVertexArrayObjectHelper(QOpenGLContext *context)
        : GenVertexArrays(nullptr), DeleteVertexArrays(nullptr),
          BindVertexArray(nullptr), IsVertexArray(nullptr)
    {
        qtInitializeVertexArrayObjectHelper(this, context);
    }

   QOpenGLVertexArrayObjectHelper(const QOpenGLVertexArrayObjectHelper &) = delete;
   QOpenGLVertexArrayObjectHelper &operator=(const QOpenGLVertexArrayObjectHelper &) = delete;

    bool isValid() const {
        return GenVertexArrays && DeleteVertexArrays && BindVertexArray && IsVertexArray;
    }

    void glGenVertexArrays(GLsizei n, GLuint *arrays) const {
        GenVertexArrays(n, arrays);
    }

    void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) const {
        DeleteVertexArrays(n, arrays);
    }

    inline void glBindVertexArray(GLuint array) const
    {
        BindVertexArray(array);
    }

    GLboolean glIsVertexArray(GLuint array) const {
        return IsVertexArray(array);
    }

private:
    friend void Q_GUI_EXPORT qtInitializeVertexArrayObjectHelper(QOpenGLVertexArrayObjectHelper *helper, QOpenGLContext *context);

    // Function signatures are equivalent between desktop core, ARB, APPLE, ES 3 and ES 2 extensions
    typedef void (QOPENGLF_APIENTRYP qt_GenVertexArrays_t)(GLsizei n, GLuint *arrays);
    typedef void (QOPENGLF_APIENTRYP qt_DeleteVertexArrays_t)(GLsizei n, const GLuint *arrays);
    typedef void (QOPENGLF_APIENTRYP qt_BindVertexArray_t)(GLuint array);
    typedef GLboolean (QOPENGLF_APIENTRYP qt_IsVertexArray_t)(GLuint array);

    qt_GenVertexArrays_t GenVertexArrays;
    qt_DeleteVertexArrays_t DeleteVertexArrays;
    qt_BindVertexArray_t BindVertexArray;
    qt_IsVertexArray_t IsVertexArray;
};

#endif // QT_NO_OPENGL

#endif
