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

#include <write_includes.h>

#include <databaseinfo.h>
#include <driver.h>
#include <ui4.h>
#include <uic.h>

#include <qfileinfo.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdio.h>

struct ClassInfoEntry {
   const char *m_name;
   const char *m_library;
   const char *m_header;
};

static const ClassInfoEntry qclass_lib_map[] = {

#define QT_CLASS_LIB(Name, Library, Header) { #Name, #Library, #Header },
#include <qclass_lib_map.h>
#undef QT_CLASS_LIB

};

// Format a module header as 'QtCore/QObject'
static inline QString modifyHeader(const QString &name, const QString &header)
{
   QString retval = name + '/' + header;
   return retval;
}

namespace CPP {

WriteIncludes::WriteIncludes(Uic *uic)
   : m_uic(uic), m_output(uic->output()), m_scriptsActivated(false), m_laidOut(false)
{
   const QString namespaceDelimiter = "::";

   for (auto item : qclass_lib_map) {

      const QString className   = QString::fromLatin1(item.m_name);
      const QString libraryName = QString::fromLatin1(item.m_library);
      const QString headerFName = QString::fromLatin1(item.m_header);

      if (className.contains(namespaceDelimiter)) {
         m_classToHeader.insert(className, modifyHeader(libraryName, headerFName));

      } else {
         const QString newHeader = modifyHeader(libraryName, className);

         m_classToHeader.insert(className, newHeader);
         m_oldHeaderToNewHeader.insert(headerFName, newHeader);
      }
   }
}

void WriteIncludes::acceptUI(DomUI *node)
{
   m_scriptsActivated = false;
   m_laidOut = false;

   m_localIncludes.clear();
   m_globalIncludes.clear();
   m_knownClasses.clear();
   m_includeBaseNames.clear();

   if (node->elementIncludes()) {
      acceptIncludes(node->elementIncludes());
   }

   if (node->elementCustomWidgets()) {
      TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());
   }

   add("QApplication");
   add("QVariant");
   add("QAction");
   add("QButtonGroup");
   add("QHeaderView");

   if (m_uic->databaseInfo()->connections().size()) {
      add("QSqlDatabase");
      add("QSqlRecord");
   }

   TreeWalker::acceptUI(node);

   writeHeaders(m_globalIncludes, true);
   writeHeaders(m_localIncludes, false);

   m_output << '\n';
}

void WriteIncludes::acceptWidget(DomWidget *node)
{
   add(node->attributeClass());
   TreeWalker::acceptWidget(node);
}

void WriteIncludes::acceptLayout(DomLayout *node)
{
   add(node->attributeClass());
   m_laidOut = true;
   TreeWalker::acceptLayout(node);
}

void WriteIncludes::acceptSpacer(DomSpacer *node)
{
   add("QSpacerItem");
   TreeWalker::acceptSpacer(node);
}

void WriteIncludes::acceptProperty(DomProperty *node)
{
   if (node->kind() == DomProperty::Date) {
      add("QDate");
   }

   if (node->kind() == DomProperty::Locale) {
      add("QLocale");
   }

   TreeWalker::acceptProperty(node);
}

void WriteIncludes::insertIncludeForClass(const QString &className, QString header, bool global)
{
   do {
      if (! header.isEmpty()) {
         break;
      }

      // Known class
      const StringMap::const_iterator iter = m_classToHeader.constFind(className);

      if (iter != m_classToHeader.constEnd()) {
         header = iter.value();
         global = true;
         break;
      }

      // check by class name to detect includehints provided for custom widgets.
      // Remove namespaces
      QString lowerClassName = className.toLower();

      static const QString namespaceSeparator = "::";
      const int namespaceIndex = lowerClassName.lastIndexOf(namespaceSeparator);

      if (namespaceIndex != -1) {
         lowerClassName.remove(0, namespaceIndex + namespaceSeparator.size());
      }

      if (m_includeBaseNames.contains(lowerClassName)) {
         header.clear();
         break;
      }

      // Last resort: Create default header
      if (! m_uic->option().implicitIncludes) {
         break;
      }

      header =  lowerClassName;
      header += ".h";

      global = true;

   } while (false);

   if (! header.isEmpty()) {
      insertInclude(header, global);
   }
}

void WriteIncludes::add(const QString &className, bool determineHeader, const QString &header, bool global)
{
   if (className.isEmpty() || m_knownClasses.contains(className)) {
      return;
   }

   m_knownClasses.insert(className);

   if (! m_laidOut && m_uic->customWidgetsInfo()->extends(className, "QToolBox")) {
      // spacing property of QToolBox
      add("QLayout");
   }

   if (className == "Line") {
      add("QFrame");
      return;
   }

   if (determineHeader) {
      insertIncludeForClass(className, header, global);
   }
}

void WriteIncludes::acceptCustomWidget(DomCustomWidget *node)
{
   const QString className = node->elementClass();

   if (className.isEmpty()) {
      return;
   }

   if (const DomScript *domScript = node->elementScript())
      if (! domScript->text().isEmpty()) {
         activateScripts();
      }

   if (! node->elementHeader() || node->elementHeader()->text().isEmpty()) {
      // no header specified
      add(className, false);

   } else {
      // custom header unless it is a built-in class
      QString header;
      bool global = false;

      if (! m_classToHeader.contains(className)) {
         global = node->elementHeader()->attributeLocation().toLower() == "global";
         header = node->elementHeader()->text();
      }

      add(className, true, header, global);
   }
}

void WriteIncludes::acceptCustomWidgets(DomCustomWidgets *node)
{
   (void) node;
}

void WriteIncludes::acceptIncludes(DomIncludes *node)
{
   TreeWalker::acceptIncludes(node);
}

void WriteIncludes::acceptInclude(DomInclude *node)
{
   bool global = true;

   if (node->hasAttributeLocation()) {
      global = node->attributeLocation() == "global";
   }

   insertInclude(node->text(), global);
}

void WriteIncludes::insertInclude(const QString &header, bool global)
{
   OrderedSet &includes = global ?  m_globalIncludes : m_localIncludes;
   if (includes.contains(header)) {
      return;
   }

   includes.insert(header, false);

   const QString lowerBaseName = QFileInfo(header).completeBaseName ().toLower();
   m_includeBaseNames.insert(lowerBaseName);
}

void WriteIncludes::writeHeaders(const OrderedSet &headers, bool global)
{
   const QChar openingQuote = global ? QChar('<') : QChar('"');
   const QChar closingQuote = global ? QChar('>') : QChar('"');

   // Check for the old headers 'qslider.h' and replace by 'QtGui/QSlider'
   const OrderedSet::const_iterator cend = headers.constEnd();

   for (OrderedSet::const_iterator sit = headers.constBegin(); sit != cend; ++sit) {
      const StringMap::const_iterator hit = m_oldHeaderToNewHeader.constFind(sit.key());

      const bool mapped     =  hit != m_oldHeaderToNewHeader.constEnd();
      const  QString header =  mapped ? hit.value() : sit.key();

      if (! header.trimmed().isEmpty()) {
         m_output << "#include " << openingQuote << header << closingQuote << '\n';
      }
   }
}

void WriteIncludes::acceptWidgetScripts(const DomScripts &scripts, DomWidget *, const  DomWidgets &)
{
   if (! scripts.empty()) {
      activateScripts();
   }
}

void WriteIncludes::activateScripts()
{
   if (! m_scriptsActivated) {
      add("QScriptEngine");
      add("QDebug");
      m_scriptsActivated = true;
   }
}

} // namespace CPP
