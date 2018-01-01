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

#ifndef QWAYLANDCLIPBOARD_H
#define QWAYLANDCLIPBOARD_H

#include <QtGui/QPlatformClipboard>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

class QWaylandDisplay;
class QWaylandSelection;
class QWaylandMimeData;
struct wl_selection_offer;

class QWaylandClipboardSignalEmitter : public QObject
{
    Q_OBJECT
public slots:
    void emitChanged();
};

class QWaylandClipboard : public QPlatformClipboard
{
public:
    QWaylandClipboard(QWaylandDisplay *display);
    ~QWaylandClipboard();

    QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard);
    void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard);
    bool supportsMode(QClipboard::Mode mode) const;

    void unregisterSelection(QWaylandSelection *selection);

    void createSelectionOffer(uint32_t id);

    QVariant retrieveData(const QString &mimeType, QVariant::Type type) const;

private:
    static void offer(void *data,
                      struct wl_selection_offer *selection_offer,
                      const char *type);
    static void keyboardFocus(void *data,
                              struct wl_selection_offer *selection_offer,
                              struct wl_input_device *input_device);
    static const struct wl_selection_offer_listener selectionOfferListener;

    static void syncCallback(void *data);
    static void forceRoundtrip(struct wl_display *display);

    QWaylandDisplay *mDisplay;
    QWaylandMimeData *mMimeDataIn;
    QList<QWaylandSelection *> mSelections;
    QStringList mOfferedMimeTypes;
    struct wl_selection_offer *mOffer;
    QWaylandClipboardSignalEmitter mEmitter;
};

#endif // QWAYLANDCLIPBOARD_H
