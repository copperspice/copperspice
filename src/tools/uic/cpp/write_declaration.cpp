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

#include <write_declaration.h>

#include <customwidgetsinfo.h>
#include <databaseinfo.h>
#include <driver.h>
#include <extract_images.h>
#include <ui4.h>
#include <uic.h>
#include <write_icondeclaration.h>
#include <write_iconinitialization.h>
#include <write_initialization.h>

#include <qdebug.h>
#include <qtextstream.h>

namespace {

void openNameSpaces(const QStringList &namespaceList, QTextStream &output)
{
   if (namespaceList.empty()) {
      return;
   }

   const QStringList::const_iterator cend = namespaceList.constEnd();

   for (QStringList::const_iterator it = namespaceList.constBegin(); it != cend; ++it) {
      if (!it->isEmpty()) {
         output << "namespace " << *it << " {\n";
      }
   }
}

void closeNameSpaces(const QStringList &namespaceList, QTextStream &output)
{
   if (namespaceList.empty()) {
      return;
   }

   QListIterator<QString> it(namespaceList);
   it.toBack();

   while (it.hasPrevious()) {
      const QString ns = it.previous();

      if (!ns.isEmpty()) {
         output << "}  // namespace " << ns << "\n";
      }
   }
}

void writeScriptContextClass(const QString &indent, QTextStream &str)
{
   str << indent << "class ScriptContext\n"
      << indent << "{\n"
      << indent << "public:\n"
      << indent << "    void run(const QString &script, QWidget *widget, const QWidgetList &childWidgets)\n"
      << indent << "    {\n"
      << indent << "        QScriptValue widgetObject =  scriptEngine.newQObject(widget);\n"
      << indent << "        QScriptValue childWidgetArray = scriptEngine.newArray (childWidgets.size());\n"
      << indent << "        for (int i = 0; i < childWidgets.size(); i++)\n"
      << indent << "               childWidgetArray.setProperty(i, scriptEngine.newQObject(childWidgets[i]));\n"
      << indent << "        QScriptContext *ctx = scriptEngine.pushContext();\n"
      << indent << "        ctx ->activationObject().setProperty(\"widget\", widgetObject);\n"
      << indent << "        ctx ->activationObject().setProperty(\"childWidgets\", childWidgetArray);\n\n"
      << indent << "        scriptEngine.evaluate(script);\n"
      << indent << "        if (scriptEngine.hasUncaughtException ()) {\n"
      << indent <<
      "            qWarning() << \"An exception occurred at line \" << scriptEngine.uncaughtExceptionLineNumber()\n"
      << indent <<
      "                       << \" of the script for \" << widget->objectName() << \": \" << engineError() << '\\n'\n"
      << indent << "                       << script;\n"
      << indent << "        }\n\n"
      << indent << "        scriptEngine.popContext();\n"
      << indent << "    }\n\n"
      << indent << "private:\n"
      << indent << "    QString engineError()\n"
      << indent << "    {\n"
      << indent << "        QScriptValue error = scriptEngine.evaluate(\"Error\");\n"
      << indent << "        return error.toString();\n"
      << indent << "    }\n\n"
      << indent << "    QScriptEngine scriptEngine;\n"
      << indent << "};\n\n";
}
}

namespace CPP {

WriteDeclaration::WriteDeclaration(Uic *uic, bool activateScripts)  :
   m_uic(uic),
   m_driver(uic->driver()),
   m_output(uic->output()),
   m_option(uic->option()),
   m_activateScripts(activateScripts)
{
}

void WriteDeclaration::acceptUI(DomUI *node)
{
   QString qualifiedClassName = node->elementClass() + m_option.postfix;
   QString className = qualifiedClassName;

   QString varName = m_driver->findOrInsertWidget(node->elementWidget());
   QString widgetClassName = node->elementWidget()->attributeClass();

   QString exportMacro = node->elementExportMacro();
   if (!exportMacro.isEmpty()) {
      exportMacro.append(' ');
   }

   QStringList namespaceList = qualifiedClassName.split("::");

   if (namespaceList.count()) {
      className = namespaceList.last();
      namespaceList.removeLast();
   }

   openNameSpaces(namespaceList, m_output);

   if (namespaceList.count()) {
      m_output << "\n";
   }

   m_output << "class " << exportMacro << m_option.prefix << className << "\n"
            << "{\n"
            << "public:\n";

   const QStringList connections = m_uic->databaseInfo()->connections();
   for (int i = 0; i < connections.size(); ++i) {
      const QString connection = connections.at(i);

      if (connection == "(default)") {
         continue;
      }

      m_output << m_option.indent << "QSqlDatabase " << connection << "Connection;\n";
   }

   TreeWalker::acceptWidget(node->elementWidget());
   if (const DomButtonGroups *domButtonGroups = node->elementButtonGroups()) {
      acceptButtonGroups(domButtonGroups);
   }

   m_output << "\n";

   WriteInitialization(m_uic, m_activateScripts).acceptUI(node);

   if (node->elementImages()) {
      if (m_option.extractImages) {
         ExtractImages(m_uic->option()).acceptUI(node);

      } else {
         m_output << "\n"
            << "protected:\n"
            << m_option.indent << "enum IconID\n"
            << m_option.indent << "{\n";
         WriteIconDeclaration(m_uic).acceptUI(node);

         m_output << m_option.indent << m_option.indent << "unknown_ID\n"
            << m_option.indent << "};\n";

         WriteIconInitialization(m_uic).acceptUI(node);
      }
   }

   if (m_activateScripts) {
      m_output << "\nprivate:\n\n";
      writeScriptContextClass(m_option.indent, m_output);
   }

   m_output << "};\n\n";

   closeNameSpaces(namespaceList, m_output);

   if (namespaceList.count()) {
      m_output << "\n";
   }

   if (m_option.generateNamespace && !m_option.prefix.isEmpty()) {
      namespaceList.append("Ui");
      openNameSpaces(namespaceList, m_output);

      m_output << m_option.indent << "class "  << exportMacro << className << " : public "
               << m_option.prefix << className << " {};\n";

      closeNameSpaces(namespaceList, m_output);

      if (namespaceList.count()) {
         m_output << "\n";
      }
   }
}

void WriteDeclaration::acceptWidget(DomWidget *node)
{
   QString className = "QWidget";

   if (node->hasAttributeClass()) {
      className = node->attributeClass();
   }

   m_output << m_option.indent << m_uic->customWidgetsInfo()->realClassName(className) << " *" <<
      m_driver->findOrInsertWidget(node) << ";\n";

   TreeWalker::acceptWidget(node);
}

void WriteDeclaration::acceptSpacer(DomSpacer *node)
{
   m_output << m_option.indent << "QSpacerItem *" << m_driver->findOrInsertSpacer(node) << ";\n";
   TreeWalker::acceptSpacer(node);
}

void WriteDeclaration::acceptLayout(DomLayout *node)
{
   QString className = "QLayout";

   if (node->hasAttributeClass()) {
      className = node->attributeClass();
   }

   m_output << m_option.indent << className << " *" << m_driver->findOrInsertLayout(node) << ";\n";

   TreeWalker::acceptLayout(node);
}

void WriteDeclaration::acceptActionGroup(DomActionGroup *node)
{
   m_output << m_option.indent << "QActionGroup *" << m_driver->findOrInsertActionGroup(node) << ";\n";

   TreeWalker::acceptActionGroup(node);
}

void WriteDeclaration::acceptAction(DomAction *node)
{
   m_output << m_option.indent << "QAction *" << m_driver->findOrInsertAction(node) << ";\n";

   TreeWalker::acceptAction(node);
}

void WriteDeclaration::acceptButtonGroup(const DomButtonGroup *buttonGroup)
{
   m_output << m_option.indent << "QButtonGroup *" << m_driver->findOrInsertButtonGroup(buttonGroup) << ";\n";
   TreeWalker::acceptButtonGroup(buttonGroup);
}

} // namespace CPP

