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

#include "grid_p.h"

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QPainter>
#include <QtGui/QWidget>
#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

static const bool defaultSnap = true;
static const bool defaultVisible = true;
static const int DEFAULT_GRID = 10;
static const char* KEY_VISIBLE = "gridVisible";
static const char* KEY_SNAPX =  "gridSnapX";
static const char* KEY_SNAPY =  "gridSnapY";
static const char* KEY_DELTAX =  "gridDeltaX";
static const char* KEY_DELTAY =  "gridDeltaY";

// Insert a value into the serialization map unless default
template <class T>
    static inline void valueToVariantMap(T value, T defaultValue, const QString &key, QVariantMap &v, bool forceKey) {
        if (forceKey || value != defaultValue)
            v.insert(key, QVariant(value));
    }

// Obtain a value form QVariantMap
template <class T>
    static inline bool valueFromVariantMap(const QVariantMap &v, const QString &key, T &value) {
        const QVariantMap::const_iterator it = v.constFind(key);
        const bool found = it != v.constEnd();
        if (found)
            value = qvariant_cast<T>(it.value());
        return found;
    }

namespace qdesigner_internal
{

Grid::Grid() :
    m_visible(defaultVisible),
    m_snapX(defaultSnap),
    m_snapY(defaultSnap),
    m_deltaX(DEFAULT_GRID),
    m_deltaY(DEFAULT_GRID)
{
}

bool Grid::fromVariantMap(const QVariantMap& vm)
{
    Grid grid;
    bool anyData = valueFromVariantMap(vm, QLatin1String(KEY_VISIBLE), grid.m_visible);
    anyData |= valueFromVariantMap(vm, QLatin1String(KEY_SNAPX), grid.m_snapX);
    anyData |= valueFromVariantMap(vm, QLatin1String(KEY_SNAPY), grid.m_snapY);
    anyData |= valueFromVariantMap(vm, QLatin1String(KEY_DELTAX), grid.m_deltaX);
    anyData |= valueFromVariantMap(vm, QLatin1String(KEY_DELTAY), grid.m_deltaY);
    if (!anyData)
        return false;
    if (grid.m_deltaX == 0 || grid.m_deltaY == 0) {
        qWarning("Attempt to set invalid grid with a spacing of 0.");
        return false;
    }
    *this = grid;
    return true;
}

QVariantMap Grid::toVariantMap(bool forceKeys) const
{
    QVariantMap rc;
    addToVariantMap(rc, forceKeys);
    return rc;
}

void  Grid::addToVariantMap(QVariantMap& vm, bool forceKeys) const
{
    valueToVariantMap(m_visible, defaultVisible, QLatin1String(KEY_VISIBLE), vm, forceKeys);
    valueToVariantMap(m_snapX, defaultSnap, QLatin1String(KEY_SNAPX), vm, forceKeys);
    valueToVariantMap(m_snapY, defaultSnap, QLatin1String(KEY_SNAPY), vm, forceKeys);
    valueToVariantMap(m_deltaX, DEFAULT_GRID, QLatin1String(KEY_DELTAX), vm, forceKeys);
    valueToVariantMap(m_deltaY, DEFAULT_GRID, QLatin1String(KEY_DELTAY), vm, forceKeys);
}

void Grid::paint(QWidget *widget, QPaintEvent *e) const
{
    QPainter p(widget);
    paint(p, widget, e);
}

void Grid::paint(QPainter &p, const QWidget *widget, QPaintEvent *e) const
{
    p.setPen(widget->palette().dark().color());

    if (m_visible) {
        const int xstart = (e->rect().x() / m_deltaX) * m_deltaX;
        const int ystart = (e->rect().y() / m_deltaY) * m_deltaY;

        const int xend = e->rect().right();
        const int yend = e->rect().bottom();

        typedef QVector<QPointF> Points;
        static Points points;
        points.clear();

        for (int x = xstart; x <= xend; x += m_deltaX) {
            points.reserve((yend - ystart) / m_deltaY + 1);
            for (int y = ystart; y <= yend; y += m_deltaY)
                points.push_back(QPointF(x, y));
            p.drawPoints( &(*points.begin()), points.count());
            points.clear();
        }
    }
}

int Grid::snapValue(int value, int grid) const
{
    const int rest = value % grid;
    const int absRest = (rest < 0) ? -rest : rest;
    int offset = 0;
    if (2 * absRest > grid)
        offset = 1;
    if (rest < 0)
        offset *= -1;
    return (value / grid + offset) * grid;
}

QPoint Grid::snapPoint(const QPoint &p) const
{
    const int sx = m_snapX ? snapValue(p.x(), m_deltaX) : p.x();
    const int sy = m_snapY ? snapValue(p.y(), m_deltaY) : p.y();
    return QPoint(sx, sy);
}

int Grid::widgetHandleAdjustX(int x) const
{
    return m_snapX ? (x / m_deltaX) * m_deltaX + 1 : x;
}

int Grid::widgetHandleAdjustY(int y) const
{
    return m_snapY ? (y / m_deltaY) * m_deltaY + 1 : y;
}

bool Grid::equals(const Grid &rhs) const
{
    return m_visible == rhs.m_visible &&
           m_snapX   == rhs.m_snapX &&
           m_snapY   == rhs.m_snapY &&
           m_deltaX  == rhs.m_deltaX &&
           m_deltaY  == rhs.m_deltaY;
}
}

QT_END_NAMESPACE
