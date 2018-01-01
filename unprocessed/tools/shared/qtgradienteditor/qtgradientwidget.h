/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QTGRADIENTWIDGET_H
#define QTGRADIENTWIDGET_H

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QtGradientWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool backgroundCheckered READ isBackgroundCheckered WRITE setBackgroundCheckered)
public:
    QtGradientWidget(QWidget *parent = 0);
    ~QtGradientWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    int heightForWidth(int w) const;

    bool isBackgroundCheckered() const;
    void setBackgroundCheckered(bool checkered);

    QGradientStops gradientStops() const;

    void setGradientType(QGradient::Type type);
    QGradient::Type gradientType() const;

    void setGradientSpread(QGradient::Spread spread);
    QGradient::Spread gradientSpread() const;

    void setStartLinear(const QPointF &point);
    QPointF startLinear() const;

    void setEndLinear(const QPointF &point);
    QPointF endLinear() const;

    void setCentralRadial(const QPointF &point);
    QPointF centralRadial() const;

    void setFocalRadial(const QPointF &point);
    QPointF focalRadial() const;

    void setRadiusRadial(qreal radius);
    qreal radiusRadial() const;

    void setCentralConical(const QPointF &point);
    QPointF centralConical() const;

    void setAngleConical(qreal angle);
    qreal angleConical() const;

public slots:
    void setGradientStops(const QGradientStops &stops);
signals:

    void startLinearChanged(const QPointF &point);
    void endLinearChanged(const QPointF &point);
    void centralRadialChanged(const QPointF &point);
    void focalRadialChanged(const QPointF &point);
    void radiusRadialChanged(qreal radius);
    void centralConicalChanged(const QPointF &point);
    void angleConicalChanged(qreal angle);

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

private:
    QScopedPointer<class QtGradientWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGradientWidget)
    Q_DISABLE_COPY(QtGradientWidget)
};

QT_END_NAMESPACE

#endif
