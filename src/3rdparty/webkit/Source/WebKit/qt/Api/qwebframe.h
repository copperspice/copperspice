/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QWEBFRAME_H
#define QWEBFRAME_H

#include <qobject.h>
#include <qurl.h>
#include <qvariant.h>
#include <qicon.h>
#include <qscriptengine.h>
#include <qnetaccess_manager.h>
#include <qwebkitglobal.h>

class QRect;
class QPoint;
class QPainter;
class QPixmap;
class QMouseEvent;
class QWheelEvent;
class QNetworkRequest;
class QRegion;
class QPrinter;

class QWebNetworkRequest;
class QWebFramePrivate;
class QWebPage;
class QWebHitTestResult;
class QWebHistoryItem;
class QWebSecurityOrigin;
class QWebElement;
class QWebElementCollection;
class QWebScriptWorld;

class DumpRenderTreeSupportQt;
namespace WebCore {
    class WidgetPrivate;
    class FrameLoaderClientQt;
    class ChromeClientQt;
    class PlatformLayerProxyQt;
}
class QWebFrameData;
class QWebHitTestResultPrivate;
class QWebFrame;

class QWEBKIT_EXPORT QWebHitTestResult {
public:
    QWebHitTestResult();
    QWebHitTestResult(const QWebHitTestResult &other);
    QWebHitTestResult &operator=(const QWebHitTestResult &other);
    ~QWebHitTestResult();

    bool isNull() const;

    QPoint pos() const;
    QRect boundingRect() const;
    QWebElement enclosingBlockElement() const;
    QString title() const;

    QString linkText() const;
    QUrl linkUrl() const;
    QUrl linkTitle() const;
    QWebFrame *linkTargetFrame() const;
    QWebElement linkElement() const;

    QString alternateText() const; // for img, area, input and applet

    QUrl imageUrl() const;
    QPixmap pixmap() const;

    bool isContentEditable() const;
    bool isContentSelected() const;

    QWebElement element() const;

    QWebFrame *frame() const;

private:
    QWebHitTestResult(QWebHitTestResultPrivate *priv);
    QWebHitTestResultPrivate *d;

    friend class QWebFrame;
    friend class QWebPagePrivate;
    friend class QWebPage;
};

class QWEBKIT_EXPORT QWebFrame : public QObject
{
    WEB_CS_OBJECT(QWebFrame)

    WEB_CS_PROPERTY_READ(textSizeMultiplier, textSizeMultiplier)
    WEB_CS_PROPERTY_WRITE(textSizeMultiplier, setTextSizeMultiplier)
    WEB_CS_PROPERTY_DESIGNABLE(textSizeMultiplier, false)
    WEB_CS_PROPERTY_READ(zoomFactor, zoomFactor)
    WEB_CS_PROPERTY_WRITE(zoomFactor, setZoomFactor)
    WEB_CS_PROPERTY_READ(title, title)
    WEB_CS_PROPERTY_READ(url, url)
    WEB_CS_PROPERTY_WRITE(url, setUrl)
    WEB_CS_PROPERTY_READ(requestedUrl, requestedUrl)
    WEB_CS_PROPERTY_READ(baseUrl, baseUrl)
    WEB_CS_PROPERTY_READ(icon, icon)
    WEB_CS_PROPERTY_READ(contentsSize, contentsSize)
    WEB_CS_PROPERTY_READ(scrollPosition, scrollPosition)
    WEB_CS_PROPERTY_WRITE(scrollPosition, setScrollPosition)
    WEB_CS_PROPERTY_READ(focus, hasFocus)

private:
    QWebFrame(QWebPage *parent, QWebFrameData *frameData);
    QWebFrame(QWebFrame *parent, QWebFrameData *frameData);
    ~QWebFrame();

public:
    QWebPage *page() const;

    void load(const QUrl &url);
    void load(const QNetworkRequest &request,
              QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation,
              const QByteArray &body = QByteArray());

    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    void setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl());

    void addToJavaScriptWindowObject(const QString &name, QObject *object);
    void addToJavaScriptWindowObject(const QString &name, QObject *object, QScriptEngine::ValueOwnership owner);

    QString toHtml() const;
    QString toPlainText() const;
    QString renderTreeDump() const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QUrl requestedUrl() const;
    QUrl baseUrl() const;
    QIcon icon() const;
    QMultiMap<QString, QString> metaData() const;

    QString frameName() const;

    QWebFrame *parentFrame() const;
    QList<QWebFrame*> childFrames() const;

    Qt::ScrollBarPolicy scrollBarPolicy(Qt::Orientation orientation) const;
    void setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy);

    void setScrollBarValue(Qt::Orientation orientation, int value);
    int scrollBarValue(Qt::Orientation orientation) const;
    int scrollBarMinimum(Qt::Orientation orientation) const;
    int scrollBarMaximum(Qt::Orientation orientation) const;
    QRect scrollBarGeometry(Qt::Orientation orientation) const;

    void scroll(int dx, int dy);
    QPoint scrollPosition() const;
    void setScrollPosition(const QPoint &pos);

    void scrollToAnchor(const QString& anchor);

    enum RenderLayer {
        ContentsLayer = 0x10,
        ScrollBarLayer = 0x20,
        PanIconLayer = 0x40,

        AllLayers = 0xff
    };

    void render(QPainter *painter);
    void render(QPainter *painter, const QRegion& clip);
    void render(QPainter *painter, RenderLayer layer, const QRegion& clip = QRegion());

    void setTextSizeMultiplier(qreal factor);
    qreal textSizeMultiplier() const;

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    bool hasFocus() const;
    void setFocus();

    QPoint pos() const;
    QRect geometry() const;
    QSize contentsSize() const;

    QWebElement documentElement() const;
    QWebElementCollection findAllElements(const QString &selectorQuery) const;
    QWebElement findFirstElement(const QString &selectorQuery) const;

    QWebHitTestResult hitTestContent(const QPoint &pos) const;

    bool event(QEvent *event) override;

    QWebSecurityOrigin securityOrigin() const;

public :
    WEB_CS_SLOT_1(Public, QVariant evaluateJavaScript(const QString & scriptSource))
    WEB_CS_SLOT_2(evaluateJavaScript)

#ifndef QT_NO_PRINTER
    WEB_CS_SLOT_1(Public, void print(QPrinter * printer))
    WEB_CS_SLOT_2(print)
#endif

    WEB_CS_SIGNAL_1(Public, void javaScriptWindowObjectCleared())
    WEB_CS_SIGNAL_2(javaScriptWindowObjectCleared)

    WEB_CS_SIGNAL_1(Public, void provisionalLoad())
    WEB_CS_SIGNAL_2(provisionalLoad)

    WEB_CS_SIGNAL_1(Public, void titleChanged(const QString & title))
    WEB_CS_SIGNAL_2(titleChanged,title)

    WEB_CS_SIGNAL_1(Public, void urlChanged(const QUrl & url))
    WEB_CS_SIGNAL_2(urlChanged,url)

    WEB_CS_SIGNAL_1(Public, void initialLayoutCompleted())
    WEB_CS_SIGNAL_2(initialLayoutCompleted)

    WEB_CS_SIGNAL_1(Public, void iconChanged())
    WEB_CS_SIGNAL_2(iconChanged)

    WEB_CS_SIGNAL_1(Public, void contentsSizeChanged(const QSize & size))
    WEB_CS_SIGNAL_2(contentsSizeChanged,size)

    WEB_CS_SIGNAL_1(Public, void loadStarted())
    WEB_CS_SIGNAL_2(loadStarted)

    WEB_CS_SIGNAL_1(Public, void loadFinished(bool ok))
    WEB_CS_SIGNAL_2(loadFinished,ok)

    WEB_CS_SIGNAL_1(Public, void pageChanged())
    WEB_CS_SIGNAL_2(pageChanged)

private:
    friend class QGraphicsWebView;
    friend class QWebPage;
    friend class QWebPagePrivate;
    friend class QWebFramePrivate;
    friend class DumpRenderTreeSupportQt;
    friend class WebCore::WidgetPrivate;
    friend class WebCore::FrameLoaderClientQt;
    friend class WebCore::ChromeClientQt;
    friend class WebCore::PlatformLayerProxyQt;
    QWebFramePrivate *d;

    WEB_CS_SLOT_1(Private, void _q_orientationChanged())
    WEB_CS_SLOT_2(_q_orientationChanged)
};

#endif
