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

#include "abstractformwindow.h"
#include "inplace_widget_helper.h"

#include <QtGui/QResizeEvent>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QShortcut>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    InPlaceWidgetHelper::InPlaceWidgetHelper(QWidget *editorWidget, QWidget *parentWidget, QDesignerFormWindowInterface *fw)
        : QObject(0),
    m_editorWidget(editorWidget),
    m_parentWidget(parentWidget),
    m_noChildEvent(m_parentWidget->testAttribute(Qt::WA_NoChildEventsForParent))
    {
        m_editorWidget->setAttribute(Qt::WA_DeleteOnClose);
        m_editorWidget->setParent(m_parentWidget->window());
        m_parentWidget->installEventFilter(this);
        m_editorWidget->installEventFilter(this);
        connect(m_editorWidget, SIGNAL(destroyed()), fw->mainContainer(), SLOT(setFocus()));
    }

    InPlaceWidgetHelper::~InPlaceWidgetHelper()
    {
        if (m_parentWidget)
            m_parentWidget->setAttribute(Qt::WA_NoChildEventsForParent, m_noChildEvent);
    }

    Qt::Alignment InPlaceWidgetHelper::alignment() const {
         if (m_parentWidget->metaObject()->indexOfProperty("alignment") != -1)
             return Qt::Alignment(m_parentWidget->property("alignment").toInt());

         if (qobject_cast<const QPushButton *>(m_parentWidget)
             || qobject_cast<const QToolButton *>(m_parentWidget) /* tool needs to be more complex */)
             return Qt::AlignHCenter;

         return Qt::AlignJustify;
     }


    bool InPlaceWidgetHelper::eventFilter(QObject *object, QEvent *e)
    {
        if (object == m_parentWidget) {
            if (e->type() == QEvent::Resize) {
                const QResizeEvent *event = static_cast<const QResizeEvent*>(e);
                const QPoint localPos = m_parentWidget->geometry().topLeft();
                const QPoint globalPos = m_parentWidget->parentWidget() ? m_parentWidget->parentWidget()->mapToGlobal(localPos) : localPos;
                const QPoint newPos = (m_editorWidget->parentWidget() ? m_editorWidget->parentWidget()->mapFromGlobal(globalPos) : globalPos)
                    + m_posOffset;
                const QSize newSize = event->size() + m_sizeOffset;
                m_editorWidget->setGeometry(QRect(newPos, newSize));
            }
        } else if (object == m_editorWidget) {
            if (e->type() == QEvent::ShortcutOverride) {
                if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape) {
                    e->accept();
                    return false;
                }
            } else if (e->type() == QEvent::KeyPress) {
                if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape) {
                    e->accept();
                    m_editorWidget->close();
                    return true;
                }
            } else if (e->type() == QEvent::Show) {
                const QPoint localPos = m_parentWidget->geometry().topLeft();
                const QPoint globalPos = m_parentWidget->parentWidget() ? m_parentWidget->parentWidget()->mapToGlobal(localPos) : localPos;
                const QPoint newPos = m_editorWidget->parentWidget() ? m_editorWidget->parentWidget()->mapFromGlobal(globalPos) : globalPos;
                m_posOffset = m_editorWidget->geometry().topLeft() - newPos;
                m_sizeOffset = m_editorWidget->size() - m_parentWidget->size();
            }
        }

        return QObject::eventFilter(object, e);
    }
}

QT_END_NAMESPACE
