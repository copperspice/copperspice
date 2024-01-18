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

#ifndef QOPENGLWIDGET_H
#define QOPENGLWIDGET_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qwidget.h>
#include <qsurfaceformat.h>
#include <qopengl.h>

class QOpenGLWidgetPrivate;

class Q_GUI_EXPORT QOpenGLWidget : public QWidget
{
    GUI_CS_OBJECT(QOpenGLWidget)

 public:
    enum UpdateBehavior {
        NoPartialUpdate,
        PartialUpdate
    };

    explicit QOpenGLWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

    QOpenGLWidget(const QOpenGLWidget &) = delete;
    QOpenGLWidget &operator=(const QOpenGLWidget &) = delete;

    ~QOpenGLWidget();

    void setUpdateBehavior(UpdateBehavior updateBehavior);
    UpdateBehavior updateBehavior() const;

    void setFormat(const QSurfaceFormat &format);
    QSurfaceFormat format() const;

    bool isValid() const;

    void makeCurrent();
    void doneCurrent();

    QOpenGLContext *context() const;
    GLuint defaultFramebufferObject() const;

    QImage grabFramebuffer();

    GUI_CS_SIGNAL_1(Public, void aboutToCompose())
    GUI_CS_SIGNAL_2(aboutToCompose)

    GUI_CS_SIGNAL_1(Public, void frameSwapped())
    GUI_CS_SIGNAL_2(frameSwapped)

    GUI_CS_SIGNAL_1(Public, void aboutToResize())
    GUI_CS_SIGNAL_2(aboutToResize)

    GUI_CS_SIGNAL_1(Public, void resized())
    GUI_CS_SIGNAL_2(resized)

 protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;

    int metric(QPaintDevice::PaintDeviceMetric metric) const override;
    QPaintDevice *redirected(QPoint *point) const override;
    QPaintEngine *paintEngine() const override;

 private:
    Q_DECLARE_PRIVATE(QOpenGLWidget)
};

#endif // QT_NO_OPENGL

#endif
