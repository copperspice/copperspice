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

#include "aboutdialog.h"

#include "helpviewer.h"
#include "tracer.h"

#include <QtCore/QBuffer>

#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QDesktopServices>

QT_BEGIN_NAMESPACE

AboutLabel::AboutLabel(QWidget *parent)
    : QTextBrowser(parent)
{
    TRACE_OBJ
    setFrameStyle(QFrame::NoFrame);
    QPalette p;
    p.setColor(QPalette::Base, p.color(QPalette::Background));
    setPalette(p);
}

void AboutLabel::setText(const QString &text, const QByteArray &resources)
{
    TRACE_OBJ
    QDataStream in(resources);
    in >> m_resourceMap;

    QTextBrowser::setText(text);
}

QSize AboutLabel::minimumSizeHint() const
{
    TRACE_OBJ
    QTextDocument *doc = document();
    doc->adjustSize();
    return QSize(int(doc->size().width()), int(doc->size().height()));
}

QVariant AboutLabel::loadResource(int type, const QUrl &name)
{
    TRACE_OBJ
    if (type == 2 || type == 3) {
        if (m_resourceMap.contains(name.toString())) {
            return m_resourceMap.value(name.toString());
        }
    }
    return QVariant();
}

void AboutLabel::setSource(const QUrl &url)
{
    TRACE_OBJ
    if (url.isValid() && (!HelpViewer::isLocalUrl(url)
    || !HelpViewer::canOpenPage(url.path()))) {
        if (!QDesktopServices::openUrl(url)) {
            QMessageBox::warning(this, tr("Warning"),
                tr("Unable to launch external application.\n"), tr("OK"));
        }
    }
}

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint |
        Qt::WindowTitleHint|Qt::WindowSystemMenuHint)
{
    TRACE_OBJ
    m_pixmapLabel = 0;
    m_aboutLabel = new AboutLabel();

    m_closeButton = new QPushButton();
    m_closeButton->setText(tr("&Close"));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(close()));

    m_layout = new QGridLayout(this);
    m_layout->addWidget(m_aboutLabel, 1, 0, 1, -1);
    m_layout->addItem(new QSpacerItem(20, 10, QSizePolicy::Minimum,
        QSizePolicy::Fixed), 2, 1, 1, 1);
    m_layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding), 3, 0, 1, 1);
    m_layout->addWidget(m_closeButton, 3, 1, 1, 1);
    m_layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding), 3, 2, 1, 1);
}

void AboutDialog::setText(const QString &text, const QByteArray &resources)
{
    TRACE_OBJ
    m_aboutLabel->setText(text, resources);
    updateSize();
}

void AboutDialog::setPixmap(const QPixmap &pixmap)
{
    TRACE_OBJ
    if (!m_pixmapLabel) {
        m_pixmapLabel = new QLabel();
        m_layout->addWidget(m_pixmapLabel, 0, 0, 1, -1, Qt::AlignCenter);
    }
    m_pixmapLabel->setPixmap(pixmap);
    updateSize();
}

QString AboutDialog::documentTitle() const
{
    TRACE_OBJ
    return m_aboutLabel->documentTitle();
}

void AboutDialog::updateSize()
{
    TRACE_OBJ
    QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos())
        .size();
    int limit = qMin(screenSize.width()/2, 500);

#ifdef Q_WS_MAC
    limit = qMin(screenSize.width()/2, 420);
#endif

    layout()->activate();
    int width = layout()->totalMinimumSize().width();

    if (width > limit)
        width = limit;

    QFontMetrics fm(qApp->font("QWorkspaceTitleBar"));
    int windowTitleWidth = qMin(fm.width(windowTitle()) + 50, limit);
    if (windowTitleWidth > width)
        width = windowTitleWidth;

    layout()->activate();
    int height = (layout()->hasHeightForWidth())
        ? layout()->totalHeightForWidth(width)
        : layout()->totalMinimumSize().height();
    setFixedSize(width, height);
    QCoreApplication::removePostedEvents(this, QEvent::LayoutRequest);
}

QT_END_NAMESPACE
