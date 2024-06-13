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

#include <driver.h>

#include <qdebug.h>
#include <qfileinfo.h>
#include <ui4.h>
#include <uic.h>

Driver::Driver()
   : m_stdout(stdout, QFile::WriteOnly | QFile::Text)
{
   m_output = &m_stdout;
}

Driver::~Driver()
{
}

QString Driver::findOrInsertWidget(DomWidget *ui_widget)
{
   if (!m_widgets.contains(ui_widget)) {
      m_widgets.insert(ui_widget, unique(ui_widget->attributeName(), ui_widget->attributeClass()));
   }

   return m_widgets.value(ui_widget);
}

QString Driver::findOrInsertSpacer(DomSpacer *ui_spacer)
{
   if (!m_spacers.contains(ui_spacer)) {
      const QString name = ui_spacer->hasAttributeName() ? ui_spacer->attributeName() : QString();
      m_spacers.insert(ui_spacer, unique(name, "QSpacerItem"));
   }

   return m_spacers.value(ui_spacer);
}

QString Driver::findOrInsertLayout(DomLayout *ui_layout)
{
   if (!m_layouts.contains(ui_layout)) {
      const QString name = ui_layout->hasAttributeName() ? ui_layout->attributeName() : QString();
      m_layouts.insert(ui_layout, unique(name, ui_layout->attributeClass()));
   }

   return m_layouts.value(ui_layout);
}

QString Driver::findOrInsertLayoutItem(DomLayoutItem *ui_layoutItem)
{
   switch (ui_layoutItem->kind()) {
      case DomLayoutItem::Widget:
         return findOrInsertWidget(ui_layoutItem->elementWidget());

      case DomLayoutItem::Spacer:
         return findOrInsertSpacer(ui_layoutItem->elementSpacer());

      case DomLayoutItem::Layout:
         return findOrInsertLayout(ui_layoutItem->elementLayout());

      case DomLayoutItem::Unknown:
         break;
   }

   Q_ASSERT(0);

   return QString();
}

QString Driver::findOrInsertActionGroup(DomActionGroup *ui_group)
{
   if (! m_actionGroups.contains(ui_group)) {
      m_actionGroups.insert(ui_group, unique(ui_group->attributeName(), "QActionGroup"));
   }

   return m_actionGroups.value(ui_group);
}

QString Driver::findOrInsertAction(DomAction *ui_action)
{
   if (!m_actions.contains(ui_action)) {
      m_actions.insert(ui_action, unique(ui_action->attributeName(), "QAction"));
   }

   return m_actions.value(ui_action);
}

QString Driver::findOrInsertButtonGroup(const DomButtonGroup *ui_group)
{
   ButtonGroupNameHash::iterator iter = m_buttonGroups.find(ui_group);

   if (iter == m_buttonGroups.end()) {
      iter = m_buttonGroups.insert(ui_group, unique(ui_group->attributeName(), "QButtonGroup"));
   }

   return iter.value();
}

// Find a group by its non-uniqified name
const DomButtonGroup *Driver::findButtonGroup(const QString &attributeName) const
{
   const ButtonGroupNameHash::const_iterator cend = m_buttonGroups.constEnd();

   for (auto iter = m_buttonGroups.constBegin(); iter != cend; ++iter)  {
      if (iter.key()->attributeName() == attributeName) {
         return iter.key();
      }
   }

   return nullptr;
}

QString Driver::findOrInsertName(const QString &name)
{
   return unique(name);
}

QString Driver::normalizedName(const QString &name)
{
   QString retval;

   for (QChar c : name) {

      if (c.isLetterOrNumber()) {
         retval.append(c);

      } else {
         retval.append('_');
      }
   }

   return retval;
}

QString Driver::unique(const QString &instanceName, const QString &className)
{
   QString name;
   bool alreadyUsed = false;

   if (instanceName.size()) {
      int id = 1;

      name = instanceName;
      name = normalizedName(name);
      QString base = name;

      while (m_nameRepository.contains(name)) {
         alreadyUsed = true;
         name = base + QString::number(id);
         ++id;
      }

   } else if (className.size()) {
      name = unique(qtify(className));
   } else {
      name = unique("var");
   }

   if (alreadyUsed && className.size()) {
      fprintf(stderr, "%s: Warning: The name '%s' (%s) is already in use, defaulting to '%s'.\n",
         csPrintable(m_option.messagePrefix()), csPrintable(instanceName), csPrintable(className), csPrintable(name));
   }

   m_nameRepository.insert(name, true);

   return name;
}

QString Driver::qtify(const QString &name)
{
   QString qname = name;

   if (qname.startsWith('Q') || qname.startsWith('K')) {
      qname = qname.mid(1);
   }

   int i = 0;

   while (i < qname.length()) {

      if (! qname.at(i).isLower()) {
         qname.replace(i, 1,  qname.at(i).toLower());

      } else {
         break;

      }

      ++i;
   }

   return qname;
}

static bool isAnsiCCharacter(const QChar &c)
{
   return (c.toUpper() >= "A" && c.toUpper() <= "Z") || c.isDigit() || c == '_';
}

QString Driver::headerFileName() const
{
   QString name = m_option.outputFile;

   if (name.isEmpty()) {
      name = "ui_";
      name.append(m_option.inputFile);
   }

   return headerFileName(name);
}

QString Driver::headerFileName(const QString &fileName)
{
   if (fileName.isEmpty()) {
      return headerFileName("noname");
   }

   QFileInfo info(fileName);
   QString baseName = info.baseName();

   // Transform into a valid C++ identifier
   if (! baseName.isEmpty() && baseName.at(0).isDigit()) {
      baseName.prepend('_');
   }

   for (int i = 0; i < baseName.size(); ++i) {
      QChar c = baseName.at(i);

      if (! isAnsiCCharacter(c)) {
         // Replace character by its unicode value
         QString hex = QString::number(c.unicode(), 16);
         baseName.replace(i, 1, '_' + hex + '_');
         i += hex.size() + 1;
      }
   }

   return baseName.toUpper() + "_H";
}

bool Driver::printDependencies(const QString &fileName)
{
   Q_ASSERT(m_option.dependencies == true);

   m_option.inputFile = fileName;
   Uic tool(this);

   return tool.printDependencies();
}

bool Driver::uic(const QString &fileName, DomUI *ui, QTextStream *out)
{
   m_option.inputFile     = fileName;
   QTextStream *oldOutput = m_output;

   if (out != nullptr) {
      m_output = out;
   } else {
      m_output = &m_stdout;
   }

   Uic tool(this);
   bool retval = false;

#ifdef QT_UIC_CPP_GENERATOR
   retval = tool.write(ui);
#else
   (void) ui;
   fprintf(stderr, "Uic: option to generate cpp code not compiled in [%s:%d]\n", __FILE__, __LINE__);
#endif

   m_output = oldOutput;

   return retval;
}

bool Driver::uic(const QString &fileName, QTextStream *out)
{
   QFile f;

   if (fileName.isEmpty()) {
      f.open(stdin, QIODevice::ReadOnly);

   } else {
      f.setFileName(fileName);

      if (! f.open(QIODevice::ReadOnly)) {
         return false;
      }
   }

   m_option.inputFile = fileName;

   QTextStream *oldOutput = m_output;
   bool deleteOutput = false;

   if (out) {
      m_output = out;

   } else {

#ifdef Q_OS_WIN
      // since the user might redirect the output to a file on win, we should not create the
      // textstream with QFile::Text flag. The redirected file is opened in TextMode and this will
      // result in broken line endings as writing will replace \n again.

      m_output = new QTextStream(stdout, QIODevice::WriteOnly);
#else
      m_output = new QTextStream(stdout, QIODevice::WriteOnly | QFile::Text);
#endif

      deleteOutput = true;
   }

   Uic tool(this);
   bool retval = tool.write(&f);

   f.close();

   if (deleteOutput) {
      delete m_output;
   }

   m_output = oldOutput;

   return retval;
}

void Driver::reset()
{
   Q_ASSERT( m_output == nullptr);

   m_option = Option();
   m_output = nullptr;
   m_problems.clear();

   QStringList m_problems;

   m_widgets.clear();
   m_spacers.clear();
   m_layouts.clear();
   m_actionGroups.clear();
   m_actions.clear();
   m_nameRepository.clear();
   m_pixmaps.clear();
}

void Driver::insertPixmap(const QString &pixmap)
{
   m_pixmaps.insert(pixmap, true);
}

bool Driver::containsPixmap(const QString &pixmap) const
{
   return m_pixmaps.contains(pixmap);
}

DomWidget *Driver::widgetByName(const QString &name) const
{
   return m_widgets.key(name);
}

DomSpacer *Driver::spacerByName(const QString &name) const
{
   return m_spacers.key(name);
}

DomLayout *Driver::layoutByName(const QString &name) const
{
   return m_layouts.key(name);
}

DomActionGroup *Driver::actionGroupByName(const QString &name) const
{
   return m_actionGroups.key(name);
}

DomAction *Driver::actionByName(const QString &name) const
{
   return m_actions.key(name);
}
