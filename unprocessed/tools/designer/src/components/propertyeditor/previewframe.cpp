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

#include "previewframe.h"
#include "previewwidget.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QPaintEvent>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

    class PreviewMdiArea: public QMdiArea {
    public:
        PreviewMdiArea(QWidget *parent = 0) : QMdiArea(parent) {}
    protected:
        bool viewportEvent ( QEvent * event );
    };

    bool PreviewMdiArea::viewportEvent (QEvent * event) {
        if (event->type() != QEvent::Paint)
            return QMdiArea::viewportEvent (event);
        QWidget *paintWidget = viewport();
        QPainter p(paintWidget);
        p.fillRect(rect(), paintWidget->palette().color(backgroundRole()).dark());
        p.setPen(QPen(Qt::white));
        //: Palette editor background
        p.drawText(0, height() / 2,  width(), height(), Qt::AlignHCenter,
                   QCoreApplication::translate("qdesigner_internal::PreviewMdiArea", "The moose in the noose\nate the goose who was loose."));
        return true;
    }

PreviewFrame::PreviewFrame(QWidget *parent) :
    QFrame(parent),
    m_mdiArea(new PreviewMdiArea(this))
{
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    vbox->addWidget(m_mdiArea);

    setMinimumSize(ensureMdiSubWindow()->minimumSizeHint());
}

void PreviewFrame::setPreviewPalette(const QPalette &pal)
{
    ensureMdiSubWindow()->widget()->setPalette(pal);
}

void PreviewFrame::setSubWindowActive(bool active)
{
    m_mdiArea->setActiveSubWindow (active ? ensureMdiSubWindow() : static_cast<QMdiSubWindow *>(0));
}

QMdiSubWindow *PreviewFrame::ensureMdiSubWindow()
{
    if (!m_mdiSubWindow) {
        PreviewWidget *previewWidget = new PreviewWidget(m_mdiArea);
        m_mdiSubWindow = m_mdiArea->addSubWindow(previewWidget, Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
        m_mdiSubWindow->move(10,10);
        m_mdiSubWindow->showMaximized();
    }

    const Qt::WindowStates state = m_mdiSubWindow->windowState();
    if (state & Qt::WindowMinimized)
        m_mdiSubWindow->setWindowState(state & ~Qt::WindowMinimized);

    return m_mdiSubWindow;
}
}

QT_END_NAMESPACE
