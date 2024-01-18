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

#ifndef QOPENGL_CUSTOM_SHADER_STAGE_H
#define QOPENGL_CUSTOM_SHADER_STAGE_H

#include <qopengl_shaderprogram.h>
#include <qglobal.h>

class QPainter;
class QOpenGLCustomShaderStagePrivate;

class Q_GUI_EXPORT QOpenGLCustomShaderStage
{
    Q_DECLARE_PRIVATE(QOpenGLCustomShaderStage)

 public:
    QOpenGLCustomShaderStage();

    QOpenGLCustomShaderStage(const QOpenGLCustomShaderStage &) = delete;
    QOpenGLCustomShaderStage &operator=(const QOpenGLCustomShaderStage &) = delete;

    virtual ~QOpenGLCustomShaderStage();
    virtual void setUniforms(QOpenGLShaderProgram*)
    { }

    void setUniformsDirty();

    bool setOnPainter(QPainter*);
    void removeFromPainter(QPainter*);
    QByteArray source() const;

    void setInactive();

 protected:
    void setSource(const QByteArray&);

 private:
    QOpenGLCustomShaderStagePrivate* d_ptr;
};

#endif
