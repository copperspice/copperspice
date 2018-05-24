/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef FrameLoaderClientQt_h
#define FrameLoaderClientQt_h


#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"
#include "KURL.h"
#include <wtf/OwnPtr.h>
#include "PluginView.h"
#include "RefCounted.h"
#include "ResourceError.h"
#include "ResourceResponse.h"
#include <QUrl>
#include <qobject.h>
#include <wtf/Forward.h>

QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

class QWebFrame;

namespace WebCore {

class AuthenticationChallenge;
class DocumentLoader;
class Element;
class FormState;
class NavigationAction;
class FrameNetworkingContext;
class ResourceLoader;

struct LoadErrorResetToken;

class FrameLoaderClientQt : public QObject, public FrameLoaderClient {
    WEB_CS_OBJECT(FrameLoaderClientQt)

    friend class ::QWebFrame;
    void callPolicyFunction(FramePolicyFunction function, PolicyAction action);
    bool callErrorPageExtension(const ResourceError&);

public:
    WEB_CS_SIGNAL_1(Public, void loadProgress(int d))
    WEB_CS_SIGNAL_2(loadProgress,d)
    WEB_CS_SIGNAL_1(Public, void titleChanged(const QString & title))
    WEB_CS_SIGNAL_2(titleChanged,title)
    WEB_CS_SIGNAL_1(Public, void unsupportedContent(QNetworkReply * un_named_arg1))
    WEB_CS_SIGNAL_2(unsupportedContent,un_named_arg1)

public:
    FrameLoaderClientQt();
    ~FrameLoaderClientQt();
    void frameLoaderDestroyed() override;

    void setFrame(QWebFrame* webFrame, Frame* frame);

    bool hasWebView() const override; // mainly for assertions

    void makeRepresentation(DocumentLoader*) override;
    void forceLayout() override;
    void forceLayoutForNonHTML() override;

    void setCopiesOnScroll() override;

    void detachedFromParent2() override;
    void detachedFromParent3() override;

    void assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&) override;

    void dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long, WebCore::ResourceRequest&, const WebCore::ResourceResponse&) override;
    bool shouldUseCredentialStorage(DocumentLoader*, unsigned long identifier) override;
    void dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, unsigned long identifier, const AuthenticationChallenge&) override;
    void dispatchDidCancelAuthenticationChallenge(DocumentLoader*, unsigned long identifier, const AuthenticationChallenge&) override;
    void dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long, const WebCore::ResourceResponse&) override;
    void dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long, int) override;
    void dispatchDidFinishLoading(WebCore::DocumentLoader*, unsigned long) override;
    void dispatchDidFailLoading(WebCore::DocumentLoader*, unsigned long, const WebCore::ResourceError&) override;
    bool dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int) override;

    void dispatchDidHandleOnloadEvents() override;
    void dispatchDidReceiveServerRedirectForProvisionalLoad() override;
    void dispatchDidCancelClientRedirect() override;
    void dispatchWillPerformClientRedirect(const KURL&, double interval, double fireDate) override;
    void dispatchDidChangeLocationWithinPage() override;
    void dispatchDidPushStateWithinPage() override;
    void dispatchDidReplaceStateWithinPage() override;
    void dispatchDidPopStateWithinPage()  override;
    void dispatchWillClose() override;
    void dispatchDidReceiveIcon() override;
    void dispatchDidStartProvisionalLoad() override;
    void dispatchDidReceiveTitle(const StringWithDirection&) override;
    void dispatchDidChangeIcons(WebCore::IconType) override;
    void dispatchDidCommitLoad() override;
    void dispatchDidFailProvisionalLoad(const ResourceError&) override;
    void dispatchDidFailLoad(const WebCore::ResourceError&) override;
    void dispatchDidFinishDocumentLoad() override;
    void dispatchDidFinishLoad() override;
    void dispatchDidFirstLayout() override;
    void dispatchDidFirstVisuallyNonEmptyLayout() override;

    WebCore::Frame* dispatchCreatePage(const WebCore::NavigationAction&) override;
    void dispatchShow() override;

    void dispatchDecidePolicyForResponse(FramePolicyFunction function, const WebCore::ResourceResponse&, const WebCore::ResourceRequest&)  override;
    void dispatchDecidePolicyForNewWindowAction(FramePolicyFunction function, const WebCore::NavigationAction&,
                  const WebCore::ResourceRequest&, PassRefPtr<FormState>, const WTF::String&) override;

    void dispatchDecidePolicyForNavigationAction(FramePolicyFunction function, const WebCore::NavigationAction&,
                  const WebCore::ResourceRequest&, PassRefPtr<FormState>) override;
    void cancelPolicyCheck() override;

    void dispatchUnableToImplementPolicy(const WebCore::ResourceError&) override;

    void dispatchWillSendSubmitEvent(HTMLFormElement*) override { }
    void dispatchWillSubmitForm(FramePolicyFunction, PassRefPtr<FormState>) override;

    void dispatchDidLoadMainResource(DocumentLoader*) override;
    void revertToProvisionalState(DocumentLoader*) override;
    void setMainDocumentError(DocumentLoader*, const ResourceError&) override;

    void postProgressStartedNotification() override;
    void postProgressEstimateChangedNotification() override;
    void postProgressFinishedNotification() override;

    void setMainFrameDocumentReady(bool) override;

    void startDownload(const WebCore::ResourceRequest&) override;

    void willChangeTitle(DocumentLoader*) override;
    void didChangeTitle(DocumentLoader*) override;

    void committedLoad(WebCore::DocumentLoader*, const char*, int) override;
    void finishedLoading(DocumentLoader*) override;

    void updateGlobalHistory() override;
    void updateGlobalHistoryRedirectLinks() override;
    bool shouldGoToHistoryItem(HistoryItem*) const override;
    bool shouldStopLoadingForHistoryItem(HistoryItem*) const override;
    void dispatchDidAddBackForwardItem(HistoryItem*) const override;
    void dispatchDidRemoveBackForwardItem(HistoryItem*) const override;
    void dispatchDidChangeBackForwardIndex() const override;
    void didDisplayInsecureContent() override;
    void didRunInsecureContent(SecurityOrigin*, const KURL&) override;

    ResourceError cancelledError(const ResourceRequest&) override;
    ResourceError blockedError(const ResourceRequest&) override;
    ResourceError cannotShowURLError(const ResourceRequest&) override;
    ResourceError interruptForPolicyChangeError(const ResourceRequest&) override;

    ResourceError cannotShowMIMETypeError(const ResourceResponse&) override;
    ResourceError fileDoesNotExistError(const ResourceResponse&) override;
    ResourceError pluginWillHandleLoadError(const ResourceResponse&) override;

    bool shouldFallBack(const ResourceError&) override;

    bool canHandleRequest(const WebCore::ResourceRequest&) const override;
    bool canShowMIMEType(const String& MIMEType) const override;
    bool canShowMIMETypeAsHTML(const String& MIMEType) const override;
    bool representationExistsForURLScheme(const String& URLScheme) const override;
    String generatedMIMETypeForURLScheme(const String& URLScheme) const override;

    void frameLoadCompleted() override;
    void saveViewStateToItem(WebCore::HistoryItem*) override;
    void restoreViewState() override;
    void provisionalLoadStarted() override;
    void didFinishLoad() override;
    void prepareForDataSourceReplacement() override;

    WTF::PassRefPtr<WebCore::DocumentLoader> createDocumentLoader(const WebCore::ResourceRequest&, const WebCore::SubstituteData&) override;
    void setTitle(const StringWithDirection&, const KURL&) override;

    String userAgent(const WebCore::KURL&) override;

    void savePlatformDataToCachedFrame(WebCore::CachedFrame*) override;
    void transitionToCommittedFromCachedFrame(WebCore::CachedFrame*) override;
    void transitionToCommittedForNewPage() override;

    void didSaveToPageCache() override;
    void didRestoreFromPageCache() override;

    void dispatchDidBecomeFrameset(bool) override;

    bool canCachePage() const override;
    void download(WebCore::ResourceHandle*, const WebCore::ResourceRequest&, const WebCore::ResourceRequest&,
                  const WebCore::ResourceResponse&) override;

    PassRefPtr<Frame> createFrame(const KURL& url, const String &name, HTMLFrameOwnerElement *ownerElement,
                  const String &referrer, bool allowsScrolling, int marginWidth, int marginHeight) override;

    void didTransferChildFrameToNewDocument(WebCore::Page*) override;
    void transferLoadingResourceFromPage(unsigned long, WebCore::DocumentLoader*, const WebCore::ResourceRequest&, WebCore::Page*) override;

    PassRefPtr<Widget> createPlugin(const IntSize&, HTMLPlugInElement*, const KURL&, const Vector<String>&, const Vector<String>&,
                  const String&, bool) override;

    void redirectDataToPlugin(Widget* pluginWidget) override;

    virtual PassRefPtr<Widget> createJavaAppletWidget(const IntSize&, HTMLAppletElement*, const KURL& baseURL, const Vector<String>& paramNames,
                  const Vector<String>& paramValues) override;

    ObjectContentType objectContentType(const KURL&, const String& mimeTypeIn, bool shouldPreferPlugInsForImages) override;
    String overrideMediaType() const override;

    void dispatchDidClearWindowObjectInWorld(DOMWrapperWorld*) override;
    void documentElementAvailable() override;
    void didPerformFirstNavigation() const override;

#if USE(V8)
    // A frame's V8 context was created or destroyed.
    virtual void didCreateScriptContextForFrame();
    virtual void didDestroyScriptContextForFrame();

    // A context untied to a frame was created (through evaluateInIsolatedWorld).
    // This context is not tied to the lifetime of its frame, and is destroyed
    // in garbage collection.
    virtual void didCreateIsolatedScriptContext();

    // Returns true if we should allow the given V8 extension to be added to
    // the script context at the currently loading page and given extension group.
    virtual bool allowScriptExtension(const String& extensionName, int extensionGroup) { return false; }
#endif

    void registerForIconNotification(bool) override;

    QString chooseFile(const QString& oldFile);

    PassRefPtr<FrameNetworkingContext> createNetworkingContext() override;

    const KURL& lastRequestedUrl() const { return m_lastRequestedUrl; }

    static bool dumpFrameLoaderCallbacks;
    static bool dumpProgressFinishedCallback;
    static bool dumpUserGestureInFrameLoaderCallbacks;
    static bool dumpResourceLoadCallbacks;
    static bool dumpResourceResponseMIMETypes;
    static QString dumpResourceLoadCallbacksPath;
    static bool sendRequestReturnsNullOnRedirect;
    static bool sendRequestReturnsNull;
    static QStringList sendRequestClearHeaders;
    static bool policyDelegateEnabled;
    static bool policyDelegatePermissive;
    static bool deferMainResourceDataLoad;
    static bool dumpHistoryCallbacks;
    static QMap<QString, QString> URLsToRedirect;

private:
    WEB_CS_SLOT_1(Private, void onIconLoadedForPageURL(const QString & un_named_arg1))
    WEB_CS_SLOT_2(onIconLoadedForPageURL)

    void emitLoadStarted();
    void emitLoadFinished(bool ok);

    Frame *m_frame;
    QWebFrame *m_webFrame;
    ResourceResponse m_response;

    // Plugin view to redirect data to
    WebCore::PluginView* m_pluginView;
    bool m_hasSentResponseToPlugin;

    // True if makeRepresentation was called.  We don't actually have a concept
    // of a "representation", but we need to know when we're expected to have one.
    // See finishedLoading().
    bool m_hasRepresentation;

    KURL m_lastRequestedUrl;
    bool m_isOriginatingLoad;
};

}

#endif
