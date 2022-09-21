/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBVIEW_H
#define QWEBVIEW_H

#include <qwebkitglobal.h>
#include <qwebpage.h>
#include <qwidget.h>
#include <qicon.h>
#include <qpainter.h>
#include <qurl.h>
#include <qnetaccess_manager.h>

class QNetworkRequest;
class QPrinter;
class QWebPage;
class QWebViewPrivate;
class QWebNetworkRequest;

class QWEBKIT_EXPORT QWebView : public QWidget {
    WEB_CS_OBJECT(QWebView)

    WEB_CS_PROPERTY_READ(title, title)
    WEB_CS_PROPERTY_READ(url, url)
    WEB_CS_PROPERTY_WRITE(url, setUrl)
    WEB_CS_PROPERTY_READ(icon, icon)
    WEB_CS_PROPERTY_READ(selectedText, selectedText)
    WEB_CS_PROPERTY_READ(selectedHtml, selectedHtml)
    WEB_CS_PROPERTY_READ(hasSelection, hasSelection)
    WEB_CS_PROPERTY_READ(modified, isModified)

    //Q_PROPERTY(Qt::TextInteractionFlags textInteractionFlags READ textInteractionFlags WRITE setTextInteractionFlags)
    WEB_CS_PROPERTY_READ(textSizeMultiplier, textSizeMultiplier)
    WEB_CS_PROPERTY_WRITE(textSizeMultiplier, setTextSizeMultiplier)
    WEB_CS_PROPERTY_DESIGNABLE(textSizeMultiplier, false)
    WEB_CS_PROPERTY_READ(zoomFactor, zoomFactor)
    WEB_CS_PROPERTY_WRITE(zoomFactor, setZoomFactor)

    WEB_CS_PROPERTY_READ(renderHints, renderHints)
    WEB_CS_PROPERTY_WRITE(renderHints, setRenderHints)

public:
    explicit QWebView(QWidget* parent = 0);
    virtual ~QWebView();

    QWebPage* page() const;
    void setPage(QWebPage* page);

    void load(const QUrl& url);
    void load(const QNetworkRequest& request,
              QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation,
              const QByteArray &body = QByteArray());
    void setHtml(const QString& html, const QUrl& baseUrl = QUrl());
    void setContent(const QByteArray& data, const QString& mimeType = QString(), const QUrl& baseUrl = QUrl());

    QWebHistory* history() const;
    QWebSettings* settings() const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QIcon icon() const;

    bool hasSelection() const;
    QString selectedText() const;
    QString selectedHtml() const;

#ifndef QT_NO_ACTION
    QAction* pageAction(QWebPage::WebAction action) const;
#endif
    void triggerPageAction(QWebPage::WebAction action, bool checked = false);

    bool isModified() const;

    /*
    Qt::TextInteractionFlags textInteractionFlags() const;
    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    void setTextInteractionFlag(Qt::TextInteractionFlag flag);
    */

    QVariant inputMethodQuery(Qt::InputMethodQuery property) const override;

    QSize sizeHint() const override;

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    void setTextSizeMultiplier(qreal factor);
    qreal textSizeMultiplier() const;

    QPainter::RenderHints renderHints() const;
    void setRenderHints(QPainter::RenderHints hints);
    void setRenderHint(QPainter::RenderHint hint, bool enabled = true);

    bool findText(const QString& subString, QWebPage::FindFlags options = 0);

    bool event(QEvent *event) override;

    WEB_CS_SLOT_1(Public, void stop())
    WEB_CS_SLOT_2(stop)

    WEB_CS_SLOT_1(Public, void back())
    WEB_CS_SLOT_2(back)

    WEB_CS_SLOT_1(Public, void forward())
    WEB_CS_SLOT_2(forward)

    WEB_CS_SLOT_1(Public, void reload())
    WEB_CS_SLOT_2(reload)

    WEB_CS_SLOT_1(Public, void print(QPrinter * printer))
    WEB_CS_SLOT_2(print)

    WEB_CS_SIGNAL_1(Public, void loadStarted())
    WEB_CS_SIGNAL_2(loadStarted)

    WEB_CS_SIGNAL_1(Public, void loadProgress(int progress))
    WEB_CS_SIGNAL_2(loadProgress,progress)

    WEB_CS_SIGNAL_1(Public, void loadFinished(bool ok))
    WEB_CS_SIGNAL_2(loadFinished,ok)

    WEB_CS_SIGNAL_1(Public, void titleChanged(const QString & title))
    WEB_CS_SIGNAL_2(titleChanged,title)

    WEB_CS_SIGNAL_1(Public, void statusBarMessage(const QString & text))
    WEB_CS_SIGNAL_2(statusBarMessage,text)

    WEB_CS_SIGNAL_1(Public, void linkClicked(const QUrl & url))
    WEB_CS_SIGNAL_2(linkClicked, url)

    WEB_CS_SIGNAL_1(Public, void selectionChanged())
    WEB_CS_SIGNAL_2(selectionChanged)

    WEB_CS_SIGNAL_1(Public, void iconChanged())
    WEB_CS_SIGNAL_2(iconChanged)

    WEB_CS_SIGNAL_1(Public, void urlChanged(const QUrl & url))
    WEB_CS_SIGNAL_2(urlChanged, url)

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    virtual QWebView *createWindow(QWebPage::WebWindowType type);

    void changeEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif

#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *event) override;
#endif

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    bool focusNextPrevChild(bool next) override;

private:
    friend class QWebPage;
    QWebViewPrivate* d;

    WEB_CS_SLOT_1(Private, void _q_pageDestroyed())
    WEB_CS_SLOT_2(_q_pageDestroyed)
};

#endif
