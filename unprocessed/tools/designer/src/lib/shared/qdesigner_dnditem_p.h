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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_DNDITEM_H
#define QDESIGNER_DNDITEM_H

#include "shared_global_p.h"
#include <QtDesigner/abstractdnditem.h>

#include <QtCore/QPoint>
#include <QtCore/QList>
#include <QtCore/QMimeData>

QT_BEGIN_NAMESPACE

class QDrag;
class QImage;
class QDropEvent;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT QDesignerDnDItem: public QDesignerDnDItemInterface
{
public:
    explicit QDesignerDnDItem(DropType type, QWidget *source = 0);
    virtual ~QDesignerDnDItem();

    virtual DomUI *domUi() const;
    virtual QWidget *decoration() const;
    virtual QWidget *widget() const;
    virtual QPoint hotSpot() const;
    virtual QWidget *source() const;

    virtual DropType type() const;

protected:
    void setDomUi(DomUI *dom_ui);
    void init(DomUI *ui, QWidget *widget, QWidget *decoration, const QPoint &global_mouse_pos);

private:
    QWidget *m_source;
    const DropType m_type;
    const QPoint m_globalStartPos;
    DomUI *m_dom_ui;
    QWidget *m_widget;
    QWidget *m_decoration;
    QPoint m_hot_spot;

    Q_DISABLE_COPY(QDesignerDnDItem)
};

// Mime data for use with designer drag and drop operations.

class  QDESIGNER_SHARED_EXPORT QDesignerMimeData : public QMimeData {
    Q_OBJECT

public:
    typedef QList<QDesignerDnDItemInterface *> QDesignerDnDItems;

    virtual ~QDesignerMimeData();

    const QDesignerDnDItems &items() const { return m_items; }

    // Execute a drag and drop operation.
    static Qt::DropAction execDrag(const QDesignerDnDItems &items, QWidget * dragSource);

    QPoint hotSpot() const { return m_hotSpot; }

    // Move the decoration. Required for drops over form windows as the position
    // is derived from the decoration position.
    void moveDecoration(const QPoint &globalPos) const;

    // For a move operation, create the undo command sequence to remove
    // the widgets from the source form.
    static void removeMovedWidgetsFromSourceForm(const QDesignerDnDItems &items);

    // Accept an event with the proper action.
    void acceptEvent(QDropEvent *e) const;

    // Helper to accept an event with the desired action.
    static void acceptEventWithAction(Qt::DropAction desiredAction, QDropEvent *e);

private:
    QDesignerMimeData(const QDesignerDnDItems &items, QDrag *drag);
    Qt::DropAction proposedDropAction() const;

    static void setImageTransparency(QImage &image, int alpha);

    const QDesignerDnDItems m_items;
    QPoint m_globalStartPos;
    QPoint m_hotSpot;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_DNDITEM_H
