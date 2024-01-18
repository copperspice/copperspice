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

#ifndef SHADEREFFECTITEM_H
#define SHADEREFFECTITEM_H

#include <QDeclarativeItem>
#include <QtOpenGL>
#include "shadereffectsource.h"
#include "scenegraph/qsggeometry.h"

QT_BEGIN_NAMESPACE

class ShaderEffectItem : public QDeclarativeItem
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_PROPERTY(QString fragmentShader READ fragmentShader WRITE setFragmentShader NOTIFY fragmentShaderChanged)
    Q_PROPERTY(QString vertexShader READ vertexShader WRITE setVertexShader NOTIFY vertexShaderChanged)
    Q_PROPERTY(bool blending READ blending WRITE setBlending NOTIFY blendingChanged)
    Q_PROPERTY(QSize meshResolution READ meshResolution WRITE setMeshResolution NOTIFY meshResolutionChanged)

public:
    ShaderEffectItem(QDeclarativeItem* parent = 0);
    ~ShaderEffectItem();

    virtual void componentComplete();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    QString fragmentShader() const { return m_fragment_code; }
    void setFragmentShader(const QString &code);

    QString vertexShader() const { return m_vertex_code; }
    void setVertexShader(const QString &code);

    bool blending() const { return m_blending; }
    void setBlending(bool enable);

    QSize meshResolution() const { return m_meshResolution; }
    void setMeshResolution(const QSize &size);

    void preprocess();

Q_SIGNALS:
    void fragmentShaderChanged();
    void vertexShaderChanged();
    void blendingChanged();
    void activeChanged();
    void meshResolutionChanged();

protected:
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

private Q_SLOTS:
    void changeSource(int index);
    void handleVisibilityChange();
    void markDirty();

private:
    void checkViewportUpdateMode();
    void renderEffect(QPainter *painter, const QMatrix4x4 &matrix);
    void updateEffectState(const QMatrix4x4 &matrix);
    void updateGeometry();
    void bindGeometry();
    void setSource(const QVariant &var, int index);
    void disconnectPropertySignals();
    void connectPropertySignals();
    void reset();
    void updateProperties();
    void updateShaderProgram();
    void lookThroughShaderCode(const QString &code);
    bool active() const { return m_active; }
    void setActive(bool enable);

private:
    QString m_fragment_code;
    QString m_vertex_code;
    QGLShaderProgram* m_program;
    QVector<const char *> m_attributeNames;
    QSet<QByteArray> m_uniformNames;
    QSize m_meshResolution;
    QSGGeometry m_geometry;

    struct SourceData
    {
        QSignalMapper *mapper;
        QPointer<ShaderEffectSource> source;
        QPointer<QDeclarativeItem> item;
        QByteArray name;
    };

    QVector<SourceData> m_sources;

    bool m_changed : 1;
    bool m_blending : 1;
    bool m_program_dirty : 1;
    bool m_active : 1;
    bool m_respectsMatrix : 1;
    bool m_respectsOpacity : 1;
    bool m_checkedViewportUpdateMode : 1;
    bool m_checkedOpenGL : 1;
    bool m_checkedShaderPrograms : 1;
    bool m_hasShaderPrograms : 1;
    bool m_mirrored : 1;
    bool m_defaultVertexShader : 1;
};

QT_END_NAMESPACE

#endif // SHADEREFFECTITEM_H
