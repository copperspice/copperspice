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

#ifndef QOPENGLVERTEXARRAYOBJECT_H
#define QOPENGLVERTEXARRAYOBJECT_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qobject.h>
#include <qopengl.h>

class QOpenGLVertexArrayObjectPrivate;

class Q_GUI_EXPORT QOpenGLVertexArrayObject : public QObject
{
    GUI_CS_OBJECT(QOpenGLVertexArrayObject)

public:
    explicit QOpenGLVertexArrayObject(QObject* parent = nullptr);
    ~QOpenGLVertexArrayObject();

    bool create();
    void destroy();
    bool isCreated() const;
    GLuint objectId() const;
    void bind();
    void release();

    class Q_GUI_EXPORT Binder
    {
    public:
        inline Binder(QOpenGLVertexArrayObject *v)
            : vao(v)
        {
            Q_ASSERT(v);
            if (vao->isCreated() || vao->create())
                vao->bind();
        }

        inline ~Binder()
        {
            release();
        }

        inline void release()
        {
            vao->release();
        }

        inline void rebind()
        {
            vao->bind();
        }

    private:
        Q_DISABLE_COPY(Binder)
        QOpenGLVertexArrayObject *vao;
    };

protected:
   QScopedPointer<QOpenGLVertexArrayObjectPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QOpenGLVertexArrayObject)
    Q_DECLARE_PRIVATE(QOpenGLVertexArrayObject)

    GUI_CS_SLOT_1(Private, void _q_contextAboutToBeDestroyed())
    GUI_CS_SLOT_2(_q_contextAboutToBeDestroyed)

    QOpenGLVertexArrayObject(QOpenGLVertexArrayObjectPrivate &dd);
};


#endif  // QT_NO_OPENGL

#endif
