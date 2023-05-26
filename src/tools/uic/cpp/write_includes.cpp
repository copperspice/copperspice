/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qstring.h>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <stdio.h>

constexpr int debugWriteIncludes   = 0;
constexpr int warnHeaderGeneration = 0;

struct ClassInfoEntry {
   const char *klass;
   const char *module;
   const char *header;
};

static const ClassInfoEntry qclass_lib_map[] = {

#define QT_CLASS_LIB(klass, module, header) { #klass, #module, #header },

#include <qclass_lib_map.h>

#undef QT_CLASS_LIB
};

// Format a module header as 'QtCore/QObject'
static inline QString moduleHeader(const QString &module, const QString &header)
{
   QString rc = module;
   rc += '/';
   rc += header;

   return rc;
}

namespace CPP {

WriteIncludes::WriteIncludes(Uic *uic)
   : m_uic(uic), m_output(uic->output()), m_scriptsActivated(false), m_laidOut(false)
{
   // When possible (no namespace) use the "QtModule/QClass" convention
   // and create a re-mapping of the old header "qclass.h" to it.

   const QString namespaceDelimiter = "::";
   const ClassInfoEntry *classLibEnd = qclass_lib_map + sizeof(qclass_lib_map) / sizeof(ClassInfoEntry);

   for (const ClassInfoEntry *it = qclass_lib_map; it < classLibEnd;  ++it) {

      const QString klass  = QString::fromLatin1(it->klass);
      const QString module = QString::fromLatin1(it->module);
      QString header       = QString::fromLatin1(it->header);

      if (klass.contains(namespaceDelimiter)) {
         m_classToHeader.insert(klass, moduleHeader(module, header));

      } else {
         const QString newHeader = moduleHeader(module, klass);
         m_classToHeader.insert(klass, newHeader);
         m_oldHeaderToNewHeader.insert(header, newHeader);
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
   if (debugWriteIncludes) {
      fprintf(stderr, "%s '%s'\n", Q_FUNC_INFO, csPrintable(node->attributeClass()));
   }

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
   if (debugWriteIncludes) {
      fprintf(stderr, "%s %s '%s' %d\n", Q_FUNC_INFO, csPrintable(className), csPrintable(header), global);
   }

   do {
      if (!header.isEmpty()) {
         break;
      }

      // Known class
      const StringMap::const_iterator it = m_classToHeader.constFind(className);
      if (it != m_classToHeader.constEnd()) {
         header = it.value();
         global =  true;
         break;
      }

      // Quick check by class name to detect includehints provided for custom widgets.
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
      if (!m_uic->option().implicitIncludes) {
         break;
      }

      header =  lowerClassName;
      header += ".h";

      if (warnHeaderGeneration) {
         qWarning("%s: Warning: generated header '%s' for class '%s'.",
            csPrintable(m_uic->option().messagePrefix()), csPrintable(header), csPrintable(className));
      }

      global = true;

   } while (false);

   if (!header.isEmpty()) {
      insertInclude(header, global);
   }
}

void WriteIncludes::add(const QString &className, bool determineHeader, const QString &header, bool global)
{
   if (debugWriteIncludes) {
      fprintf(stderr, "%s %s '%s' %d\n", Q_FUNC_INFO, csPrintable(className), csPrintable(header), global);
   }

   if (className.isEmpty() || m_knownClasses.contains(className)) {
      return;
   }

   m_knownClasses.insert(className);

   if (! m_laidOut && m_uic->customWidgetsInfo()->extends(className, "QToolBox")) {
      add("QLayout");   // spacing property of QToolBox)
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
      if (!domScript->text().isEmpty()) {
         activateScripts();
      }

   if (!node->elementHeader() || node->elementHeader()->text().isEmpty()) {
      add(className, false); // no header specified

   } else {
      // custom header unless it is a built-in qt class
      QString header;
      bool global = false;

      if (!m_classToHeader.contains(className)) {
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
   if (debugWriteIncludes) {
      fprintf(stderr, "%s %s %d\n", Q_FUNC_INFO, csPrintable(header), global);
   }

   OrderedSet &includes = global ?  m_globalIncludes : m_localIncludes;
   if (includes.contains(header)) {
      return;
   }

   // Insert. Also remember base name for quick check of suspicious custom plugins
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

