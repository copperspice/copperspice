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

#ifndef QTGRADIENTSTOPSMODEL_H
#define QTGRADIENTSTOPSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class QColor;

class QtGradientStopsModel;

class QtGradientStop
{
public:
    qreal position() const;
    QColor color() const;
    QtGradientStopsModel *gradientModel() const;

private:
    void setColor(const QColor &color);
    void setPosition(qreal position);
    friend class QtGradientStopsModel;
    QtGradientStop(QtGradientStopsModel *model = 0);
    ~QtGradientStop();
    QScopedPointer<class QtGradientStopPrivate> d_ptr;
};

class QtGradientStopsModel : public QObject
{
    Q_OBJECT
public:
    typedef QMap<qreal, QtGradientStop *> PositionStopMap;

    QtGradientStopsModel(QObject *parent = 0);
    ~QtGradientStopsModel();

    PositionStopMap stops() const;
    QtGradientStop *at(qreal pos) const;
    QColor color(qreal pos) const; // calculated between points
    QList<QtGradientStop *> selectedStops() const;
    QtGradientStop *currentStop() const;
    bool isSelected(QtGradientStop *stop) const;
    QtGradientStop *firstSelected() const;
    QtGradientStop *lastSelected() const;
    QtGradientStopsModel *clone() const;

    QtGradientStop *addStop(qreal pos, const QColor &color);
    void removeStop(QtGradientStop *stop);
    void moveStop(QtGradientStop *stop, qreal newPos);
    void swapStops(QtGradientStop *stop1, QtGradientStop *stop2);
    void changeStop(QtGradientStop *stop, const QColor &newColor);
    void selectStop(QtGradientStop *stop, bool select);
    void setCurrentStop(QtGradientStop *stop);

    void moveStops(double newPosition); // moves current stop to newPos and all selected stops are moved accordingly
    void clear();
    void clearSelection();
    void flipAll();
    void selectAll();
    void deleteStops();

signals:
    void stopAdded(QtGradientStop *stop);
    void stopRemoved(QtGradientStop *stop);
    void stopMoved(QtGradientStop *stop, qreal newPos);
    void stopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2);
    void stopChanged(QtGradientStop *stop, const QColor &newColor);
    void stopSelected(QtGradientStop *stop, bool selected);
    void currentStopChanged(QtGradientStop *stop);

private:
    QScopedPointer<class QtGradientStopsModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGradientStopsModel)
    Q_DISABLE_COPY(QtGradientStopsModel)
};

QT_END_NAMESPACE

#endif
