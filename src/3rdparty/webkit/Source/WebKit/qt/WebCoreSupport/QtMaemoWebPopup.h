/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef QtMaemoWebPopup_h
#define QtMaemoWebPopup_h

#include "qwebkitplatformplugin.h"

#include <QDialog>

class QListWidgetItem;
class QListWidget;


namespace WebCore {

class Maemo5Popup : public QDialog {
    CS_OBJECT(Maemo5Popup)
public:
    Maemo5Popup(const QWebSelectData& data) : m_data(data) {}

    WEB_CS_SIGNAL_1(Public, void itemClicked(int idx))
    WEB_CS_SIGNAL_2(itemClicked,idx) 

protected :
    WEB_CS_SLOT_1(Protected, void onItemSelected(QListWidgetItem * item))
    WEB_CS_SLOT_2(onItemSelected) 

protected:
    void populateList();

    const QWebSelectData& m_data;
    QListWidget* m_list;
};


class QtMaemoWebPopup : public QWebSelectMethod {
    CS_OBJECT(QtMaemoWebPopup)
public:
    QtMaemoWebPopup();
    ~QtMaemoWebPopup();

    virtual void show(const QWebSelectData& data);
    virtual void hide();

private :
    WEB_CS_SLOT_1(Private, void popupClosed())
    WEB_CS_SLOT_2(popupClosed) 
    WEB_CS_SLOT_1(Private, void itemClicked(int idx))
    WEB_CS_SLOT_2(itemClicked) 

private:
    Maemo5Popup* m_popup;

    Maemo5Popup* createPopup(const QWebSelectData& data);
    Maemo5Popup* createSingleSelectionPopup(const QWebSelectData& data);
    Maemo5Popup* createMultipleSelectionPopup(const QWebSelectData& data);
};


class Maemo5SingleSelectionPopup : public Maemo5Popup {
    CS_OBJECT(Maemo5SingleSelectionPopup)

public:
    Maemo5SingleSelectionPopup(const QWebSelectData& data);
};


class Maemo5MultipleSelectionPopup : public Maemo5Popup {
    CS_OBJECT(Maemo5MultipleSelectionPopup)

public:
    Maemo5MultipleSelectionPopup(const QWebSelectData& data);
};

}

#endif // QtMaemoWebPopup_h
