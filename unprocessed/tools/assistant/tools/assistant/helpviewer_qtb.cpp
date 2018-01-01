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

#include "helpviewer.h"

#include "globalactions.h"
#include "helpenginewrapper.h"
#include "helpviewer_p.h"
#include "openpagesmanager.h"
#include "tracer.h"

#include <QtCore/QStringBuilder>

#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>

QT_BEGIN_NAMESPACE

HelpViewer::HelpViewer(qreal zoom, QWidget *parent)
    : QTextBrowser(parent)
    , d(new HelpViewerPrivate(zoom))
{
    TRACE_OBJ
    QPalette p = palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight,
        p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText,
        p.color(QPalette::Active, QPalette::HighlightedText));
    setPalette(p);

    installEventFilter(this);
    document()->setDocumentMargin(8);

    QFont font = viewerFont();
    font.setPointSize(int(font.pointSize() + zoom));
    setViewerFont(font);

    connect(this, SIGNAL(sourceChanged(QUrl)), this, SIGNAL(titleChanged()));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(setLoadFinished(bool)));
}

QFont HelpViewer::viewerFont() const
{
    TRACE_OBJ
    if (HelpEngineWrapper::instance().usesBrowserFont())
        return HelpEngineWrapper::instance().browserFont();
    return qApp->font();
}

void HelpViewer::setViewerFont(const QFont &newFont)
{
    TRACE_OBJ
    if (font() != newFont) {
        d->forceFont = true;
        setFont(newFont);
        d->forceFont = false;
    }
}

void HelpViewer::scaleUp()
{
    TRACE_OBJ
    if (d->zoomCount < 10) {
        d->zoomCount++;
        d->forceFont = true;
        zoomIn();
        d->forceFont = false;
    }
}

void HelpViewer::scaleDown()
{
    TRACE_OBJ
    if (d->zoomCount > -5) {
        d->zoomCount--;
        d->forceFont = true;
        zoomOut();
        d->forceFont = false;
    }
}

void HelpViewer::resetScale()
{
    TRACE_OBJ
    if (d->zoomCount != 0) {
        d->forceFont = true;
        zoomOut(d->zoomCount);
        d->forceFont = false;
    }
    d->zoomCount = 0;
}

qreal HelpViewer::scale() const
{
    TRACE_OBJ
    return d->zoomCount;
}

QString HelpViewer::title() const
{
    TRACE_OBJ
    return documentTitle();
}

void HelpViewer::setTitle(const QString &title)
{
    TRACE_OBJ
    setDocumentTitle(title);
}

QUrl HelpViewer::source() const
{
    TRACE_OBJ
    return QTextBrowser::source();
}

void HelpViewer::setSource(const QUrl &url)
{
    TRACE_OBJ
    if (launchWithExternalApp(url))
        return;

    emit loadStarted();
    QString string = url.toString();
    const HelpEngineWrapper &engine = HelpEngineWrapper::instance();
    const QUrl &resolvedUrl = (string == QLatin1String("help") ? LocalHelpFile :
        engine.findFile(string));
    QTextBrowser::setSource(resolvedUrl);
    if (!url.isValid()) {
        setHtml(string == QLatin1String("about:blank") ? AboutBlank
            : PageNotFoundMessage.arg(url.toString()));
    }
    emit loadFinished(true);
}

QString HelpViewer::selectedText() const
{
    TRACE_OBJ
    return textCursor().selectedText();
}

bool HelpViewer::isForwardAvailable() const
{
    TRACE_OBJ
    return QTextBrowser::isForwardAvailable();
}

bool HelpViewer::isBackwardAvailable() const
{
    TRACE_OBJ
    return QTextBrowser::isBackwardAvailable();
}

bool HelpViewer::findText(const QString &text, FindFlags flags, bool incremental,
    bool fromSearch)
{
    TRACE_OBJ
    QTextDocument *doc = document();
    QTextCursor cursor = textCursor();
    if (!doc || cursor.isNull())
        return false;

    const int position = cursor.selectionStart();
    if (incremental)
        cursor.setPosition(position);

    QTextDocument::FindFlags textDocFlags;
    if (flags & HelpViewer::FindBackward)
        textDocFlags |= QTextDocument::FindBackward;
    if (flags & HelpViewer::FindCaseSensitively)
        textDocFlags |= QTextDocument::FindCaseSensitively;

    QTextCursor found = doc->find(text, cursor, textDocFlags);
    if (found.isNull()) {
        if ((flags & HelpViewer::FindBackward) == 0)
            cursor.movePosition(QTextCursor::Start);
        else
            cursor.movePosition(QTextCursor::End);
        found = doc->find(text, cursor, textDocFlags);
    }

    if (fromSearch) {
        cursor.beginEditBlock();
        viewport()->setUpdatesEnabled(false);

        QTextCharFormat marker;
        marker.setForeground(Qt::red);
        cursor.movePosition(QTextCursor::Start);
        setTextCursor(cursor);

        while (find(text)) {
            QTextCursor hit = textCursor();
            hit.mergeCharFormat(marker);
        }

        viewport()->setUpdatesEnabled(true);
        cursor.endEditBlock();
    }

    bool cursorIsNull = found.isNull();
    if (cursorIsNull) {
        found = textCursor();
        found.setPosition(position);
    }
    setTextCursor(found);
    return !cursorIsNull;
}

// -- public slots

void HelpViewer::copy()
{
    TRACE_OBJ
    QTextBrowser::copy();
}

void HelpViewer::forward()
{
    TRACE_OBJ
    QTextBrowser::forward();
}

void HelpViewer::backward()
{
    TRACE_OBJ
    QTextBrowser::backward();
}

// -- protected

void HelpViewer::keyPressEvent(QKeyEvent *e)
{
    TRACE_OBJ
    if ((e->key() == Qt::Key_Home && e->modifiers() != Qt::NoModifier)
        || (e->key() == Qt::Key_End && e->modifiers() != Qt::NoModifier)) {
        QKeyEvent* event = new QKeyEvent(e->type(), e->key(), Qt::NoModifier,
            e->text(), e->isAutoRepeat(), e->count());
        e = event;
    }
    QTextBrowser::keyPressEvent(e);
}


void HelpViewer::wheelEvent(QWheelEvent *e)
{
    TRACE_OBJ
    if (e->modifiers() == Qt::ControlModifier) {
        e->accept();
        e->delta() > 0 ? scaleUp() : scaleDown();
    } else {
        QTextBrowser::wheelEvent(e);
    }
}

void HelpViewer::mousePressEvent(QMouseEvent *e)
{
    TRACE_OBJ
#ifdef Q_OS_LINUX
    if (handleForwardBackwardMouseButtons(e))
        return;
#endif

    QTextBrowser::mousePressEvent(e);
}

void HelpViewer::mouseReleaseEvent(QMouseEvent *e)
{
    TRACE_OBJ
#ifndef Q_OS_LINUX
    if (handleForwardBackwardMouseButtons(e))
        return;
#endif

    bool controlPressed = e->modifiers() & Qt::ControlModifier;
    if ((controlPressed && d->hasAnchorAt(this, e->pos())) ||
        (e->button() == Qt::MidButton && d->hasAnchorAt(this, e->pos()))) {
        d->openLinkInNewPage();
        return;
    }

    QTextBrowser::mouseReleaseEvent(e);
}

// -- private slots

void HelpViewer::actionChanged()
{
    // stub
    TRACE_OBJ
}

// -- private

bool HelpViewer::eventFilter(QObject *obj, QEvent *event)
{
    TRACE_OBJ
    if (event->type() == QEvent::FontChange && !d->forceFont)
        return true;
    return QTextBrowser::eventFilter(obj, event);
}

void HelpViewer::contextMenuEvent(QContextMenuEvent *event)
{
    TRACE_OBJ

    QMenu menu(QString(), 0);
    QUrl link;
    QAction *copyAnchorAction = 0;
    if (d->hasAnchorAt(this, event->pos())) {
        link = anchorAt(event->pos());
        if (link.isRelative())
            link = source().resolved(link);
        menu.addAction(tr("Open Link"), d, SLOT(openLink()));
        menu.addAction(tr("Open Link in New Tab\tCtrl+LMB"), d, SLOT(openLinkInNewPage()));

        if (!link.isEmpty() && link.isValid())
            copyAnchorAction = menu.addAction(tr("Copy &Link Location"));
    } else if (!selectedText().isEmpty()) {
        menu.addAction(tr("Copy"), this, SLOT(copy()));
    } else {
        menu.addAction(tr("Reload"), this, SLOT(reload()));
    }

    if (copyAnchorAction == menu.exec(event->globalPos()))
        QApplication::clipboard()->setText(link.toString());
}

QVariant HelpViewer::loadResource(int type, const QUrl &name)
{
    TRACE_OBJ
    QByteArray ba;
    if (type < 4) {
        ba = HelpEngineWrapper::instance().fileData(name);
        if (name.toString().endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
            QImage image;
            image.loadFromData(ba, "svg");
            if (!image.isNull())
                return image;
        }
    }
    return ba;
}

QT_END_NAMESPACE
