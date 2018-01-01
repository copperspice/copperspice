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

#include "qdesigner_widget_p.h"
#include "formwindowbase_p.h"
#include "grid_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

/* QDesignerDialog / QDesignerWidget are used to paint a grid on QDialog and QWidget main containers
 * and container extension pages.
 * The paint routines work as follows:
 * We need to clean the background here (to make the parent grid disappear in case we are a container page
 * and to make palette background settings take effect),
 * which would normally break style sheets with background settings.
 * So, we manually make the style paint after cleaning. On top comes the grid
 * In addition, this code works around
 * the QStyleSheetStyle setting Qt::WA_StyledBackground to false for subclasses of QWidget.
 */

QDesignerDialog::QDesignerDialog(QDesignerFormWindowInterface *fw, QWidget *parent) :
    QDialog(parent),
    m_formWindow(qobject_cast<qdesigner_internal::FormWindowBase*>(fw))
{
}

void QDesignerDialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QStyleOption opt;
    opt.initFrom(this);
    p.fillRect(e->rect(), palette().brush(backgroundRole()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    if (m_formWindow && m_formWindow->gridVisible())
        m_formWindow->designerGrid().paint(p, this, e);
}

QDesignerWidget::QDesignerWidget(QDesignerFormWindowInterface* formWindow, QWidget *parent)  :
    QWidget(parent),
    m_formWindow(qobject_cast<qdesigner_internal::FormWindowBase*>(formWindow))
{
}

QDesignerWidget::~QDesignerWidget()
{
}

QDesignerFormWindowInterface* QDesignerWidget::formWindow() const
{
    return m_formWindow;
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QStyleOption opt;
    opt.initFrom(this);
    p.fillRect(e->rect(), palette().brush(backgroundRole()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    if (m_formWindow && m_formWindow->gridVisible())
        m_formWindow->designerGrid().paint(p, this, e);
}

QT_END_NAMESPACE
