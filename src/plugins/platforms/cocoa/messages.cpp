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

#include <messages.h>

#include <QCoreApplication>

// Translatable messages should go into this .cpp file for them to be picked up by lupdate.

QString msgAboutCs()
{
   return QCoreApplication::translate("QCocoaMenuItem", "About CS");
}

static const char *application_menu_strings[] = {
   cs_mark_tr("MAC_APPLICATION_MENU", "Services"),
   cs_mark_tr("MAC_APPLICATION_MENU", "Hide %1"),
   cs_mark_tr("MAC_APPLICATION_MENU", "Hide Others"),
   cs_mark_tr("MAC_APPLICATION_MENU", "Show All"),
   cs_mark_tr("MAC_APPLICATION_MENU", "Preferences..."),
   cs_mark_tr("MAC_APPLICATION_MENU", "Quit %1"),
   cs_mark_tr("MAC_APPLICATION_MENU", "About %1")
};

QString qt_mac_applicationmenu_string(int type)
{
   QString menuString = QString::fromLatin1(application_menu_strings[type]);
   const QString translated = QCoreApplication::translate("QMenuBar", application_menu_strings[type]);

   if (translated != menuString) {
      return translated;
   } else {
      return QCoreApplication::translate("MAC_APPLICATION_MENU", application_menu_strings[type]);
   }
}

QPlatformMenuItem::MenuRole detectMenuRole(const QString &caption)
{
   QString captionNoAmpersand(caption);
   captionNoAmpersand.remove(QChar('&'));
   const QString aboutString = QCoreApplication::translate("QCocoaMenuItem", "About");

   if (captionNoAmpersand.startsWith(aboutString, Qt::CaseInsensitive) || caption.endsWith(aboutString, Qt::CaseInsensitive)) {
      return QPlatformMenuItem::AboutRole;
   }

   if (captionNoAmpersand.startsWith(QCoreApplication::translate("QCocoaMenuItem", "Config"), Qt::CaseInsensitive)
      || captionNoAmpersand.startsWith(QCoreApplication::translate("QCocoaMenuItem", "Preference"), Qt::CaseInsensitive)
      || captionNoAmpersand.startsWith(QCoreApplication::translate("QCocoaMenuItem", "Options"), Qt::CaseInsensitive)
      || captionNoAmpersand.startsWith(QCoreApplication::translate("QCocoaMenuItem", "Setting"), Qt::CaseInsensitive)
      || captionNoAmpersand.startsWith(QCoreApplication::translate("QCocoaMenuItem", "Setup"), Qt::CaseInsensitive)) {
      return QPlatformMenuItem::PreferencesRole;
   }

   if (captionNoAmpersand.startsWith(QCoreApplication::translate("QCocoaMenuItem", "Quit"), Qt::CaseInsensitive)
      || captionNoAmpersand.startsWith(QCoreApplication::translate("QCocoaMenuItem", "Exit"), Qt::CaseInsensitive)) {
      return QPlatformMenuItem::QuitRole;
   }

   if (!captionNoAmpersand.compare(QCoreApplication::translate("QCocoaMenuItem", "Cut"), Qt::CaseInsensitive)) {
      return QPlatformMenuItem::CutRole;
   }
   if (!captionNoAmpersand.compare(QCoreApplication::translate("QCocoaMenuItem", "Copy"), Qt::CaseInsensitive)) {
      return QPlatformMenuItem::CopyRole;
   }
   if (!captionNoAmpersand.compare(QCoreApplication::translate("QCocoaMenuItem", "Paste"), Qt::CaseInsensitive)) {
      return QPlatformMenuItem::PasteRole;
   }
   if (!captionNoAmpersand.compare(QCoreApplication::translate("QCocoaMenuItem", "Select All"), Qt::CaseInsensitive)) {
      return QPlatformMenuItem::SelectAllRole;
   }
   return QPlatformMenuItem::NoRole;
}

QString msgDialogButtonDiscard()
{
   return QCoreApplication::translate("QCocoaTheme", "Do not Save");
}

