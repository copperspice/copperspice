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

#include <write_initialization.h>

#include <databaseinfo.h>
#include <driver.h>
#include <globaldefs.h>
#include <ui4.h>
#include <uic.h>
#include <utils.h>
#include <write_iconinitialization.h>

#include <qdebug.h>
#include <qstringfwd.h>
#include <qtextstream.h>

namespace {

QString accessibilityDefineC = "QT_NO_ACCESSIBILITY";
QString toolTipDefineC       = "QT_NO_TOOLTIP";
QString whatsThisDefineC     = "QT_NO_WHATSTHIS";
QString statusTipDefineC     = "QT_NO_STATUSTIP";
QString shortcutDefineC      = "QT_NO_SHORTCUT";

// fix an enumeration name, was "BottomToolBarArea" instead of "Qt::BottomToolBarArea"
void fixQtEnumerationName(QString &name)
{
   static const QString prefix("Qt::");

   if (name.indexOf(prefix) != 0) {
      name.prepend(prefix);
   }
}

// figure out the toolbar area of a DOM attrib list
// By legacy it is stored as an integer, later on it is the enumeration value
QString toolBarAreaStringFromDOMAttributes(const CPP::WriteInitialization::DomPropertyMap &attributes)
{
   const DomProperty *pstyle = attributes.value("toolBarArea");

   if (! pstyle) {
      return QString();
   }

   switch (pstyle->kind()) {
      case DomProperty::Number: {
         QString area = "static_cast<Qt::ToolBarArea>(";
         area += QString::number(pstyle->elementNumber());
         area += "), ";
         return area;
      }

      case DomProperty::Enum: {
         QString area = pstyle->elementEnum();
         fixQtEnumerationName(area);
         area += ", ";
         return area;
      }

      default:
         break;
   }

   return QString();
}

// Write a statement to create a spacer item.
void writeSpacerItem(const DomSpacer *node, QTextStream &output)
{
   const QHash<QString, DomProperty *> properties = propertyMap(node->elementProperty());

   output << "new QSpacerItem(";

   if (properties.contains("sizeHint")) {
      const DomSize *sizeHint = properties.value("sizeHint")->elementSize();
      output << sizeHint->elementWidth() << ", " << sizeHint->elementHeight() << ", ";
   }

   // size type
   QString sizeType = properties.contains("sizeType")  ?
      properties.value("sizeType")->elementEnum() : QString("Expanding");

   if (! sizeType.startsWith("QSizePolicy::")) {
      sizeType.prepend("QSizePolicy::");
   }

   // orientation
   bool isVspacer = false;

   if (properties.contains("orientation")) {
      const QString orientation = properties.value("orientation")->elementEnum();

      if (orientation == "Qt::Vertical" || orientation == "Vertical") {
         isVspacer = true;
      }
   }

   if (isVspacer) {
      output << "QSizePolicy::Minimum, " << sizeType << ')';
   } else {
      output << sizeType << ", QSizePolicy::Minimum)";
   }
}

// implementing comparison functions for integers
int compareInt(int i1, int i2)
{
   if (i1 < i2) {
      return -1;
   }

   if (i1 > i2) {
      return  1;
   }

   return  0;
}

// Write object->setFoo(x);
template <class Value>
void writeSetter(const QString &indent, const QString &varName, const QString &setter, Value v, QTextStream &str)
{
   str << indent << varName << "->" << setter << '(' << v << ");\n";
}

void writeSetupUIScriptVariableDeclarations(const QString &indent, QTextStream &str)
{
   str << indent << "ScriptContext scriptContext;\n"
       << indent << "QWidgetList childWidgets;\n";
}

static inline bool iconHasStatePixmaps(const DomResourceIcon *i)
{
   return i->hasElementNormalOff()   || i->hasElementNormalOn() ||
      i->hasElementDisabledOff() || i->hasElementDisabledOn() ||
      i->hasElementActiveOff()   || i->hasElementActiveOn() ||
      i->hasElementSelectedOff() || i->hasElementSelectedOn();
}

static inline bool isIconFormat44(const DomResourceIcon *i)
{
   return iconHasStatePixmaps(i) || !i->attributeTheme().isEmpty();
}

// Check properties. Filter out empty legacy pixmap/icon properties
// from older versions of Designer
static bool checkProperty(const QString &fileName, const DomProperty *p)
{
   switch (p->kind()) {
      case DomProperty::IconSet:
         if (const DomResourceIcon *dri = p->elementIconSet()) {

            if (! isIconFormat44(dri)) {
               if (dri->text().isEmpty())  {
                  const QString msg = QString("%1: Warning, an invalid icon property '%2' was found")
                     .formatArg(fileName).formatArg(p->attributeName());
                  qWarning("%s", csPrintable(msg));

                  return false;
               }
            }
         }
         break;

      case DomProperty::Pixmap:
         if (const DomResourcePixmap *drp = p->elementPixmap())
            if (drp->text().isEmpty()) {
               const QString msg = QString("%1: Warning, an invalid pixmap property '%2' was found")
                  .formatArg(fileName).formatArg(p->attributeName());
               qWarning("%s", csPrintable(msg));

               return false;
            }
         break;

      default:
         break;
   }

   return  true;
}

inline void openIfndef(QTextStream &str, const QString &symbol)
{
   if (! symbol.isEmpty()) {
      str << "#if ! defined(" << symbol << ")\n";
   }
}

inline void closeIfndef(QTextStream &str, const QString &symbol)
{
   if (! symbol.isEmpty()) {
      str << "#endif\n\n";
   }
}

}

namespace CPP {

FontHandle::FontHandle(const DomFont *domFont)
   : m_domFont(domFont)
{
}

int FontHandle::compare(const FontHandle &rhs) const
{
   const QString family    = m_domFont->hasElementFamily()     ?     m_domFont->elementFamily() : QString();
   const QString rhsFamily = rhs.m_domFont->hasElementFamily() ? rhs.m_domFont->elementFamily() : QString();

   if (const int frc = family.compare(rhsFamily)) {
      return frc;
   }

   const int pointSize    = m_domFont->hasElementPointSize()     ?     m_domFont->elementPointSize() : -1;
   const int rhsPointSize = rhs.m_domFont->hasElementPointSize() ? rhs.m_domFont->elementPointSize() : -1;

   if (const int crc = compareInt(pointSize, rhsPointSize)) {
      return crc;
   }

   const int bold    = m_domFont->hasElementBold()     ? (m_domFont->elementBold()     ? 1 : 0) : -1;
   const int rhsBold = rhs.m_domFont->hasElementBold() ? (rhs.m_domFont->elementBold() ? 1 : 0) : -1;

   if (const int crc = compareInt(bold, rhsBold)) {
      return crc;
   }

   const int italic    = m_domFont->hasElementItalic()     ? (m_domFont->elementItalic()     ? 1 : 0) : -1;
   const int rhsItalic = rhs.m_domFont->hasElementItalic() ? (rhs.m_domFont->elementItalic() ? 1 : 0) : -1;

   if (const int crc = compareInt(italic, rhsItalic)) {
      return crc;
   }

   const int underline    = m_domFont->hasElementUnderline()     ? (m_domFont->elementUnderline()     ? 1 : 0) : -1;
   const int rhsUnderline = rhs.m_domFont->hasElementUnderline() ? (rhs.m_domFont->elementUnderline() ? 1 : 0) : -1;

   if (const int crc = compareInt(underline, rhsUnderline)) {
      return crc;
   }

   const int weight    = m_domFont->hasElementWeight()     ?     m_domFont->elementWeight() : -1;
   const int rhsWeight = rhs.m_domFont->hasElementWeight() ? rhs.m_domFont->elementWeight() : -1;
   if (const int crc = compareInt(weight, rhsWeight)) {
      return crc;
   }

   const int strikeOut    = m_domFont->hasElementStrikeOut()     ? (m_domFont->elementStrikeOut()     ? 1 : 0) : -1;
   const int rhsStrikeOut = rhs.m_domFont->hasElementStrikeOut() ? (rhs.m_domFont->elementStrikeOut() ? 1 : 0) : -1;

   if (const int crc = compareInt(strikeOut, rhsStrikeOut)) {
      return crc;
   }

   const int kerning    = m_domFont->hasElementKerning()     ? (m_domFont->elementKerning()     ? 1 : 0) : -1;
   const int rhsKerning = rhs.m_domFont->hasElementKerning() ? (rhs.m_domFont->elementKerning() ? 1 : 0) : -1;

   if (const int crc = compareInt(kerning, rhsKerning)) {
      return crc;
   }

   const int antialiasing    = m_domFont->hasElementAntialiasing()     ? (m_domFont->elementAntialiasing()     ? 1 : 0) : -1;
   const int rhsAntialiasing = rhs.m_domFont->hasElementAntialiasing() ? (rhs.m_domFont->elementAntialiasing() ? 1 : 0) : -1;

   if (const int crc = compareInt(antialiasing, rhsAntialiasing)) {
      return crc;
   }

   const QString styleStrategy    = m_domFont->hasElementStyleStrategy()     ?     m_domFont->elementStyleStrategy() : QString();
   const QString rhsStyleStrategy = rhs.m_domFont->hasElementStyleStrategy() ? rhs.m_domFont->elementStyleStrategy() : QString();

   if (const int src = styleStrategy.compare(rhsStyleStrategy)) {
      return src;
   }

   return 0;
}

IconHandle::IconHandle(const DomResourceIcon *domIcon) :
   m_domIcon(domIcon)
{
}

int IconHandle::compare(const IconHandle &rhs) const
{
   if (const int comp = m_domIcon->attributeTheme().compare(rhs.m_domIcon->attributeTheme())) {
      return comp;
   }

   const QString normalOff    = m_domIcon->hasElementNormalOff() ? m_domIcon->elementNormalOff()->text() : QString();
   const QString rhsNormalOff = rhs.m_domIcon->hasElementNormalOff() ? rhs.m_domIcon->elementNormalOff()->text() : QString();

   if (const int comp = normalOff.compare(rhsNormalOff)) {
      return comp;
   }

   const QString normalOn    = m_domIcon->hasElementNormalOn() ? m_domIcon->elementNormalOn()->text() : QString();
   const QString rhsNormalOn = rhs.m_domIcon->hasElementNormalOn() ? rhs.m_domIcon->elementNormalOn()->text() : QString();

   if (const int comp = normalOn.compare(rhsNormalOn)) {
      return comp;
   }

   const QString disabledOff    = m_domIcon->hasElementDisabledOff() ? m_domIcon->elementDisabledOff()->text() : QString();
   const QString rhsDisabledOff = rhs.m_domIcon->hasElementDisabledOff() ? rhs.m_domIcon->elementDisabledOff()->text() : QString();

   if (const int comp = disabledOff.compare(rhsDisabledOff)) {
      return comp;
   }

   const QString disabledOn    = m_domIcon->hasElementDisabledOn() ?     m_domIcon->elementDisabledOn()->text() : QString();
   const QString rhsDisabledOn = rhs.m_domIcon->hasElementDisabledOn() ? rhs.m_domIcon->elementDisabledOn()->text() : QString();

   if (const int comp = disabledOn.compare(rhsDisabledOn)) {
      return comp;
   }

   const QString activeOff    = m_domIcon->hasElementActiveOff() ?     m_domIcon->elementActiveOff()->text() : QString();
   const QString rhsActiveOff = rhs.m_domIcon->hasElementActiveOff() ? rhs.m_domIcon->elementActiveOff()->text() : QString();

   if (const int comp = activeOff.compare(rhsActiveOff)) {
      return comp;
   }

   const QString activeOn    = m_domIcon->hasElementActiveOn() ?     m_domIcon->elementActiveOn()->text() : QString();
   const QString rhsActiveOn = rhs.m_domIcon->hasElementActiveOn() ? rhs.m_domIcon->elementActiveOn()->text() : QString();

   if (const int comp = activeOn.compare(rhsActiveOn)) {
      return comp;
   }

   const QString selectedOff    = m_domIcon->hasElementSelectedOff() ?     m_domIcon->elementSelectedOff()->text() : QString();
   const QString rhsSelectedOff = rhs.m_domIcon->hasElementSelectedOff() ? rhs.m_domIcon->elementSelectedOff()->text() : QString();

   if (const int comp = selectedOff.compare(rhsSelectedOff)) {
      return comp;
   }

   const QString selectedOn    = m_domIcon->hasElementSelectedOn() ?     m_domIcon->elementSelectedOn()->text() : QString();
   const QString rhsSelectedOn = rhs.m_domIcon->hasElementSelectedOn() ? rhs.m_domIcon->elementSelectedOn()->text() : QString();

   if (const int comp = selectedOn.compare(rhsSelectedOn)) {
      return comp;
   }

   // legacy
   if (const int comp = m_domIcon->text().compare(rhs.m_domIcon->text())) {
      return comp;
   }

   return 0;
}

SizePolicyHandle::SizePolicyHandle(const DomSizePolicy *domSizePolicy) :
   m_domSizePolicy(domSizePolicy)
{
}

int SizePolicyHandle::compare(const SizePolicyHandle &rhs) const
{
   const int hSizeType    = m_domSizePolicy->hasElementHSizeType()     ? m_domSizePolicy->elementHSizeType()     : -1;
   const int rhsHSizeType = rhs.m_domSizePolicy->hasElementHSizeType() ? rhs.m_domSizePolicy->elementHSizeType() : -1;
   if (const int crc = compareInt(hSizeType, rhsHSizeType)) {
      return crc;
   }

   const int vSizeType    = m_domSizePolicy->hasElementVSizeType()     ? m_domSizePolicy->elementVSizeType()     : -1;
   const int rhsVSizeType = rhs.m_domSizePolicy->hasElementVSizeType() ? rhs.m_domSizePolicy->elementVSizeType() : -1;
   if (const int crc = compareInt(vSizeType, rhsVSizeType)) {
      return crc;
   }

   const int hStretch    =  m_domSizePolicy->hasElementHorStretch()     ? m_domSizePolicy->elementHorStretch()     : -1;
   const int rhsHStretch =  rhs.m_domSizePolicy->hasElementHorStretch() ? rhs.m_domSizePolicy->elementHorStretch() : -1;
   if (const int crc = compareInt(hStretch, rhsHStretch)) {
      return crc;
   }

   const int vStretch    =  m_domSizePolicy->hasElementVerStretch()     ? m_domSizePolicy->elementVerStretch()     : -1;
   const int rhsVStretch =  rhs.m_domSizePolicy->hasElementVerStretch() ? rhs.m_domSizePolicy->elementVerStretch() : -1;
   if (const int crc = compareInt(vStretch, rhsVStretch)) {
      return crc;
   }

   const QString attributeHSizeType    = m_domSizePolicy->hasAttributeHSizeType()     ?
      m_domSizePolicy->attributeHSizeType()     : QString();
   const QString rhsAttributeHSizeType = rhs.m_domSizePolicy->hasAttributeHSizeType() ?
      rhs.m_domSizePolicy->attributeHSizeType() : QString();

   if (const int hrc = attributeHSizeType.compare(rhsAttributeHSizeType)) {
      return hrc;
   }

   const QString attributeVSizeType    = m_domSizePolicy->hasAttributeVSizeType()     ?
      m_domSizePolicy->attributeVSizeType()     : QString();
   const QString rhsAttributeVSizeType = rhs.m_domSizePolicy->hasAttributeVSizeType() ?
      rhs.m_domSizePolicy->attributeVSizeType() : QString();

   return attributeVSizeType.compare(rhsAttributeVSizeType);
}

WriteInitialization::LayoutDefaultHandler::LayoutDefaultHandler()
{
   std::fill(m_state, m_state + NumProperties, 0u);
   std::fill(m_defaultValues, m_defaultValues + NumProperties, 0);
}

void WriteInitialization::LayoutDefaultHandler::acceptLayoutDefault(DomLayoutDefault *node)
{
   if (! node) {
      return;
   }

   if (node->hasAttributeMargin()) {
      m_state[Margin] |= HasDefaultValue;
      m_defaultValues[Margin] = node->attributeMargin();
   }

   if (node->hasAttributeSpacing()) {
      m_state[Spacing] |= HasDefaultValue;
      m_defaultValues[Spacing]  = node->attributeSpacing();
   }
}

void WriteInitialization::LayoutDefaultHandler::acceptLayoutFunction(DomLayoutFunction *node)
{
   if (!node) {
      return;
   }

   if (node->hasAttributeMargin()) {
      m_state[Margin]     |= HasDefaultFunction;
      m_functions[Margin] =  node->attributeMargin();
      m_functions[Margin] += "()";
   }

   if (node->hasAttributeSpacing()) {
      m_state[Spacing]     |= HasDefaultFunction;
      m_functions[Spacing] =  node->attributeSpacing();
      m_functions[Spacing] += "()";
   }
}

static inline void writeContentsMargins(const QString &indent, const QString &objectName, int value, QTextStream &str)
{
   QString contentsMargins;
   QTextStream(&contentsMargins) << value << ", " << value << ", " << value << ", " << value;
   writeSetter(indent, objectName, "setContentsMargins", contentsMargins, str);
}

void WriteInitialization::LayoutDefaultHandler::writeProperty(int p, const QString &indent, const QString &objectName,
   const DomPropertyMap &properties, const QString &propertyName, const QString &setter,
   int defaultStyleValue, bool suppressDefault, QTextStream &str) const
{
   // User value
   const DomPropertyMap::const_iterator mit = properties.constFind(propertyName);
   const bool found = mit != properties.constEnd();

   if (found) {
      const int value = mit.value()->elementNumber();
      // Emulate legacy behaviour: The value form default value was only used to determine
      // the default value, layout properties were always written

      const bool useLayoutFunctionPre43 = !suppressDefault && (m_state[p] == (HasDefaultFunction | HasDefaultValue))
            && value == m_defaultValues[p];

      if (!useLayoutFunctionPre43) {
         bool ifndefMac = (! (m_state[p] & (HasDefaultFunction | HasDefaultValue))
               && value == defaultStyleValue);

         if (ifndefMac) {
            str << "#if ! defined(Q_OS_DARWIN)\n";
         }

         if (p == Margin) {
            // Use setContentsMargins for numeric values
            writeContentsMargins(indent, objectName, value, str);
         } else {
            writeSetter(indent, objectName, setter, value, str);
         }

         if (ifndefMac) {
            str << "#endif\n\n";
         }
         return;
      }
   }

   if (suppressDefault) {
      return;
   }

   // get default
   if (m_state[p] & HasDefaultFunction) {
      // Do not use setContentsMargins to avoid repetitive evaluations.
      writeSetter(indent, objectName, setter, m_functions[p], str);
      return;
   }
   if (m_state[p] & HasDefaultValue) {
      if (p == Margin) {
         // Use setContentsMargins for numeric values
         writeContentsMargins(indent, objectName, m_defaultValues[p], str);
      } else {
         writeSetter(indent, objectName, setter, m_defaultValues[p], str);
      }
   }
   return;
}

void WriteInitialization::LayoutDefaultHandler::writeProperties(const QString &indent, const QString &varName,
   const DomPropertyMap &properties, int marginType, bool suppressMarginDefault, QTextStream &str) const
{
   // Write out properties and ignore the ones found in
   // subsequent writing of the property list.
   int defaultSpacing = marginType == WriteInitialization::Use43UiFile ? -1 : 6;
   writeProperty(Spacing, indent, varName, properties, "spacing", "setSpacing", defaultSpacing, false, str);

   // We use 9 as TopLevelMargin, since Designer seem to always use 9.
   static const int layoutmargins[4] = { -1, 9, 9, 0};
   writeProperty(Margin,  indent, varName, properties, "margin", "setMargin", layoutmargins[marginType],
         suppressMarginDefault, str);
}

template <class DomElement>
static bool needsTranslation(const DomElement *element)
{
   if (! element) {
      return false;
   }

   return ! element->hasAttributeNotr() || ! toBool(element->attributeNotr());
}

WriteInitialization::WriteInitialization(Uic *uic, bool activateScripts)
   : m_uic(uic), m_driver(uic->driver()), m_output(uic->output()), m_option(uic->option()),
     m_indent(m_option.indent + m_option.indent),
     m_dindent(m_indent + m_option.indent), m_stdsetdef(true),
     m_layoutMarginType(TopLevelMargin), m_mainFormUsedInRetranslateUi(false),
     m_delayedOut(&m_delayedInitialization, QIODevice::WriteOnly),
     m_refreshOut(&m_refreshInitialization, QIODevice::WriteOnly),
     m_actionOut(&m_delayedActionInitialization, QIODevice::WriteOnly),
     m_activateScripts(activateScripts), m_layoutWidget(false),
     m_firstThemeIcon(true)
{
}

void WriteInitialization::acceptUI(DomUI *node)
{
   m_registeredImages.clear();

   m_actionGroupChain.push(nullptr);
   m_widgetChain.push(nullptr);
   m_layoutChain.push(nullptr);

   acceptLayoutDefault(node->elementLayoutDefault());
   acceptLayoutFunction(node->elementLayoutFunction());

   if (node->elementCustomWidgets()) {
      TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());
   }

   if (node->elementImages()) {
      TreeWalker::acceptImages(node->elementImages());
   }

   if (m_option.generateImplemetation) {
      m_output << "#include <" << m_driver->headerFileName() << ">\n\n";
   }

   m_stdsetdef = true;
   if (node->hasAttributeStdSetDef()) {
      m_stdsetdef = node->attributeStdSetDef();
   }

   const QString className = node->elementClass() + m_option.postfix;
   m_generatedClass = className;

   const QString varName = m_driver->findOrInsertWidget(node->elementWidget());
   m_mainFormVarName = varName;
   m_registeredWidgets.insert(varName, node->elementWidget()); // register the main widget

   const QString widgetClassName = node->elementWidget()->attributeClass();

   m_output << m_option.indent << "void " << "setupUi(" << widgetClassName << " *" << varName << ")\n"
      << m_option.indent << "{\n";

   if (m_activateScripts) {
      writeSetupUIScriptVariableDeclarations(m_indent, m_output);
   }

   for (const auto &connection : m_uic->databaseInfo()->connections()) {
      if (connection == "(default)") {
         continue;
      }

      const QString varConn = connection + "Connection";
      m_output << m_indent << varConn << " = QSqlDatabase::database(" << fixString(connection, m_dindent) << ");\n";
   }

   acceptWidget(node->elementWidget());

   if (m_buddies.size() > 0) {
      openIfndef(m_output, shortcutDefineC);
   }

   for (int i = 0; i < m_buddies.size(); ++i) {
      const Buddy &b = m_buddies.at(i);

      if (!m_registeredWidgets.contains(b.objName)) {
         qWarning("%s: Warning: Buddy assignment '%s' is not a valid widget",
            csPrintable(m_option.messagePrefix()), csPrintable(b.objName));
         continue;

      } else if (!m_registeredWidgets.contains(b.buddy)) {
         qWarning("%s: Warning: Buddy assignment '%s' is not a valid widget",
            csPrintable(m_option.messagePrefix()), csPrintable(b.buddy));
         continue;
      }

      m_output << m_indent << b.objName << "->setBuddy(" << b.buddy << ");\n";
   }

   if (m_buddies.size() > 0) {
      closeIfndef(m_output, shortcutDefineC);
   }

   if (node->elementTabStops()) {
      acceptTabStops(node->elementTabStops());
   }

   if (m_delayedActionInitialization.size()) {
      m_output << "\n" << m_delayedActionInitialization;
   }

   m_output << "\n" << m_indent << "retranslateUi(" << varName << ");\n";

   if (node->elementConnections()) {
      acceptConnections(node->elementConnections());
   }

   if (! m_delayedInitialization.isEmpty()) {
      m_output << "\n" << m_delayedInitialization << "\n";
   }

   if (m_option.autoConnection) {
      m_output << "\n" << m_indent << "QMetaObject::connectSlotsByName(" << varName << ");\n";
   }

   m_output << m_option.indent << "}\n\n";

   if (! m_mainFormUsedInRetranslateUi) {
      m_refreshInitialization += m_indent;
      m_refreshInitialization += "(void) ";
      m_refreshInitialization += varName;
      m_refreshInitialization += ";\n";
   }

   m_output << m_option.indent << "void " << "retranslateUi(" << widgetClassName << " *" << varName << ")\n"
      << m_option.indent << "{\n"
      << m_refreshInitialization
      << m_option.indent << "}\n\n";

   m_layoutChain.pop();
   m_widgetChain.pop();
   m_actionGroupChain.pop();
}

void WriteInitialization::addWizardPage(const QString &pageVarName, const DomWidget *page, const QString &parentWidget)
{
   // If the node has a (free-format) string "pageId" attribute (which could
   // an integer or an enumeration value), use setPage(), else addPage().
   QString id;

   const DomPropertyList attributes = page->elementAttribute();

   if (! attributes.empty()) {
      const DomPropertyList::const_iterator acend = attributes.constEnd();

      for (DomPropertyList::const_iterator it = attributes.constBegin(); it != acend; ++it)

         if ((*it)->attributeName() == "pageId") {
            if (const DomString *ds = (*it)->elementString()) {
               id = ds->text();
            }

            break;
         }
   }

   if (id.isEmpty()) {
      m_output << m_indent << parentWidget << "->addPage(" << pageVarName << ");\n";
   } else {
      m_output << m_indent << parentWidget << "->setPage(" << id << ", " << pageVarName << ");\n";
   }
}

void WriteInitialization::acceptWidget(DomWidget *node)
{
   m_layoutMarginType = m_widgetChain.count() == 1 ? TopLevelMargin : ChildMargin;
   const QString className = node->attributeClass();
   const QString varName = m_driver->findOrInsertWidget(node);
   m_registeredWidgets.insert(varName, node); // register the current widget

   QString parentWidget, parentClass;
   if (m_widgetChain.top()) {
      parentWidget = m_driver->findOrInsertWidget(m_widgetChain.top());
      parentClass = m_widgetChain.top()->attributeClass();
   }

   const QString savedParentWidget = parentWidget;

   if (m_uic->isContainer(parentClass)) {
      parentWidget.clear();
   }

   if (m_widgetChain.size() != 1) {
      m_output << m_indent << varName << " = new " << m_uic->customWidgetsInfo()->realClassName(
            className) << '(' << parentWidget << ");\n";
   }

   parentWidget = savedParentWidget;

   if (m_uic->customWidgetsInfo()->extends(className, "QComboBox")) {
      initializeComboBox(node);

   } else if (m_uic->customWidgetsInfo()->extends(className, "QListWidget")) {
      initializeListWidget(node);

   } else if (m_uic->customWidgetsInfo()->extends(className, "QTreeWidget")) {
      initializeTreeWidget(node);

   } else if (m_uic->customWidgetsInfo()->extends(className, "QTableWidget")) {
      initializeTableWidget(node);

   }

   if (m_uic->isButton(className)) {
      addButtonGroup(node, varName);
   }

   writeProperties(varName, className, node->elementProperty());

   if (m_uic->customWidgetsInfo()->extends(className, "QMenu") && parentWidget.size()) {
      initializeMenu(node, parentWidget);
   }

   if (node->elementLayout().isEmpty()) {
      m_layoutChain.push(nullptr);
   }

   m_layoutWidget = false;
   if (className == "QWidget" && ! node->hasAttributeNative()) {

      if (const DomWidget *parentWidget = m_widgetChain.top()) {
         const QString parentClass = parentWidget->attributeClass();

         if (parentClass != "QMainWindow" && !m_uic->isCustomWidgetContainer(parentClass)
            && ! m_uic->isContainer(parentClass)) {
            m_layoutWidget = true;
         }
      }
   }

   m_widgetChain.push(node);
   m_layoutChain.push(nullptr);

   TreeWalker::acceptWidget(node);
   m_layoutChain.pop();
   m_widgetChain.pop();
   m_layoutWidget = false;

   const DomPropertyMap attributes = propertyMap(node->elementAttribute());
   const QString pageDefaultString = "Page";

   if (m_uic->customWidgetsInfo()->extends(parentClass, "QMainWindow")) {

      if (m_uic->customWidgetsInfo()->extends(className, "QMenuBar")) {
         m_output << m_indent << parentWidget << "->setMenuBar(" << varName << ");\n";

      } else if (m_uic->customWidgetsInfo()->extends(className, "QToolBar")) {
         m_output << m_indent << parentWidget << "->addToolBar(" << toolBarAreaStringFromDOMAttributes(
               attributes) << varName << ");\n";

         if (const DomProperty *pbreak = attributes.value("toolBarBreak")) {
            if (pbreak->elementBool() == "true") {
               m_output << m_indent << parentWidget << "->insertToolBarBreak(" <<  varName << ");\n";
            }
         }

      } else if (m_uic->customWidgetsInfo()->extends(className, "QDockWidget")) {
         QString area;
         if (DomProperty *pstyle = attributes.value("dockWidgetArea")) {
            area += "static_cast<Qt::DockWidgetArea>(";
            area += QString::number(pstyle->elementNumber());
            area += "), ";
         }

         m_output << m_indent << parentWidget << "->addDockWidget(" << area << varName << ");\n";

      } else if (m_uic->customWidgetsInfo()->extends(className, "QStatusBar")) {
         m_output << m_indent << parentWidget << "->setStatusBar(" << varName << ");\n";

      } else  {
         m_output << m_indent << parentWidget << "->setCentralWidget(" << varName << ");\n";

      }
   }

   // Check for addPageMethod of a custom plugin first
   const QString addPageMethod = m_uic->customWidgetsInfo()->customWidgetAddPageMethod(parentClass);
   if (! addPageMethod.isEmpty()) {
      m_output << m_indent << parentWidget << "->" << addPageMethod << '(' << varName << ");\n";

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QStackedWidget")) {
      m_output << m_indent << parentWidget << "->addWidget(" << varName << ");\n";

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QToolBar")) {
      m_output << m_indent << parentWidget << "->addWidget(" << varName << ");\n";

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QDockWidget")) {
      m_output << m_indent << parentWidget << "->setWidget(" << varName << ");\n";

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QScrollArea")) {
      m_output << m_indent << parentWidget << "->setWidget(" << varName << ");\n";

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QSplitter")) {
      m_output << m_indent << parentWidget << "->addWidget(" << varName << ");\n";

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QMdiArea")) {
      m_output << m_indent << parentWidget << "->addSubWindow(" << varName << ");\n";

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QWorkspace")) {
      m_output << m_indent << parentWidget << "->addWindow(" << varName << ");\n";

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QWizard")) {
      addWizardPage(varName, node, parentWidget);

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QToolBox")) {
      QString icon;

      if (const DomProperty *picon = attributes.value("icon")) {
         icon += ", ";
         icon += iconCall(picon);
      }

      const DomProperty *plabel = attributes.value("label");
      DomString *plabelString = plabel ? plabel->elementString() : nullptr;

      m_output << m_indent << parentWidget << "->addItem(" << varName << icon << ", "
               << noTrCall(plabelString, pageDefaultString) << ");\n";

      autoTrOutput(plabelString, pageDefaultString) << m_indent << parentWidget << "->setItemText("
         << parentWidget << "->indexOf(" << varName << "), " << autoTrCall(plabelString, pageDefaultString) << ");\n";

#ifndef QT_NO_TOOLTIP
      if (DomProperty *ptoolTip = attributes.value("toolTip")) {
         autoTrOutput(ptoolTip->elementString()) << m_indent << parentWidget << "->setItemToolTip("
            << parentWidget << "->indexOf(" << varName << "), " << autoTrCall(ptoolTip->elementString()) << ");\n";
      }
#endif

   } else if (m_uic->customWidgetsInfo()->extends(parentClass, "QTabWidget")) {

      QString icon;

      if (const DomProperty *picon = attributes.value("icon")) {
         icon += ", ";
         icon += iconCall(picon);
      }

      const DomProperty *ptitle = attributes.value("title");
      DomString *ptitleString = ptitle ? ptitle->elementString() : nullptr;

      m_output << m_indent << parentWidget << "->addTab(" << varName << icon << ", " << "QString());\n";

      autoTrOutput(ptitleString, pageDefaultString) << m_indent << parentWidget << "->setTabText("
         << parentWidget << "->indexOf(" << varName << "), " << autoTrCall(ptitleString, pageDefaultString) << ");\n";

#ifndef QT_NO_TOOLTIP
      if (const DomProperty *ptoolTip = attributes.value("toolTip")) {
         autoTrOutput(ptoolTip->elementString()) << m_indent << parentWidget << "->setTabToolTip("
            << parentWidget << "->indexOf(" << varName << "), " << autoTrCall(ptoolTip->elementString()) << ");\n";
      }
#endif

#ifndef QT_NO_WHATSTHIS
      if (const DomProperty *pwhatsThis = attributes.value("whatsThis")) {
         autoTrOutput(pwhatsThis->elementString()) << m_indent << parentWidget << "->setTabWhatsThis("
            << parentWidget << "->indexOf(" << varName << "), " << autoTrCall(pwhatsThis->elementString()) << ");\n";
      }
#endif

   }

   // Special handling for qtableview/qtreeview fake header attributes
   static QStringList realPropertyNames =
      (QStringList() << "visible"
                     << "cascadingSectionResizes"
                     << "defaultSectionSize"
                     << "highlightSections"
                     << "minimumSectionSize"
                     << "showSortIndicator"
                     << "stretchLastSection");

   if (m_uic->customWidgetsInfo()->extends(className, "QTreeView")
      || m_uic->customWidgetsInfo()->extends(className, "QTreeWidget")) {

      DomPropertyList headerProperties;

      for (const QString &realPropertyName : realPropertyNames) {
         const QString upperPropertyName = realPropertyName.at(0).toUpper() + realPropertyName.mid(1);
         const QString fakePropertyName  = "header" + upperPropertyName;

         if (DomProperty *fakeProperty = attributes.value(fakePropertyName)) {
            fakeProperty->setAttributeName(realPropertyName);
            headerProperties << fakeProperty;
         }
      }

      writeProperties(varName + "->header()", "QHeaderView", headerProperties, WritePropertyIgnoreObjectName);

   } else if (m_uic->customWidgetsInfo()->extends(className, "QTableView")
               || m_uic->customWidgetsInfo()->extends(className, "QTableWidget")) {

      static QStringList headerPrefixes =
         (QStringList() << "horizontalHeader"
                        << "verticalHeader");

      for (const QString &headerPrefix : headerPrefixes) {
         DomPropertyList headerProperties;

         for (const QString &realPropertyName : realPropertyNames) {
            const QString upperPropertyName = realPropertyName.at(0).toUpper() + realPropertyName.mid(1);
            const QString fakePropertyName = headerPrefix + upperPropertyName;

            if (DomProperty *fakeProperty = attributes.value(fakePropertyName)) {
               fakeProperty->setAttributeName(realPropertyName);
               headerProperties << fakeProperty;
            }
         }
         writeProperties(varName + "->" + headerPrefix + "()", "QHeaderView",
            headerProperties, WritePropertyIgnoreObjectName);
      }
   }

   if (node->elementLayout().isEmpty()) {
      m_layoutChain.pop();
   }

   const QStringList zOrder = node->elementZOrder();
   for (int i = 0; i < zOrder.size(); ++i) {
      const QString name = zOrder.at(i);

      if (! m_registeredWidgets.contains(name)) {
         qWarning("%s: Warning: Z-order assignment of '%s' is not a valid widget",
               csPrintable(m_option.messagePrefix()), csPrintable(name));
         continue;
      }

      if (name.isEmpty()) {
         continue;
      }

      m_output << m_indent << name << "->raise();\n";
   }
}

void WriteInitialization::addButtonGroup(const DomWidget *buttonNode, const QString &varName)
{
   const DomPropertyMap attributes = propertyMap(buttonNode->elementAttribute());

   // Look up the button group name as specified in the attribute and find the uniquified name
   const DomProperty *prop = attributes.value("buttonGroup");

   if (! prop) {
      return;
   }

   const QString attributeName = toString(prop->elementString());
   const DomButtonGroup *group = m_driver->findButtonGroup(attributeName);

   // Legacy feature: Create missing groups on the fly as the UIC button group feature
   // was present before the actual Designer support

   const bool createGroupOnTheFly = (group == nullptr);

   if (createGroupOnTheFly) {
      DomButtonGroup *newGroup = new DomButtonGroup;
      newGroup->setAttributeName(attributeName);
      group = newGroup;

      qWarning("%s: Warning: Creating button group `%s'",
         csPrintable(m_option.messagePrefix()), attributeName.toLatin1().data());
   }

   const QString groupName = m_driver->findOrInsertButtonGroup(group);

   // Create on demand
   if (! m_buttonGroups.contains(groupName)) {
      const QString className = "QButtonGroup";
      m_output << m_indent;

      if (createGroupOnTheFly) {
         m_output << className << " *";
      }

      m_output << groupName << " = new " << className << '(' << m_mainFormVarName << ");\n";
      m_buttonGroups.insert(groupName);
      writeProperties(groupName, className, group->elementProperty());
   }

   m_output << m_indent << groupName << "->addButton(" << varName << ");\n";
}

void WriteInitialization::acceptLayout(DomLayout *node)
{
   const QString className = node->attributeClass();
   const QString varName = m_driver->findOrInsertLayout(node);

   const DomPropertyMap properties = propertyMap(node->elementProperty());
   const bool oldLayoutProperties = properties.constFind("margin") != properties.constEnd();

   bool isGroupBox = false;

   if (m_widgetChain.top()) {
      const QString parentWidget = m_widgetChain.top()->attributeClass();
   }

   m_output << m_indent << varName << " = new " << className << '(';

   if (!m_layoutChain.top() && !isGroupBox) {
      m_output << m_driver->findOrInsertWidget(m_widgetChain.top());
   }

   m_output << ");\n";

   if (isGroupBox) {
      const QString tempName = m_driver->unique("boxlayout");

      m_output << m_indent << "QBoxLayout *" << tempName << " = dynamic_cast<QBoxLayout *>(" <<
         m_driver->findOrInsertWidget(m_widgetChain.top()) << "->layout());\n";

      m_output << m_indent << "if (" << tempName << ")\n";
      m_output << m_dindent << tempName << "->addLayout(" << varName << ");\n";
   }

   if (isGroupBox) {
      m_output << m_indent << varName << "->setAlignment(Qt::AlignTop);\n";
   }  else {
      // Suppress margin on a read child layout
      const bool suppressMarginDefault = m_layoutChain.top();
      int marginType = Use43UiFile;
      if (oldLayoutProperties) {
         marginType = m_layoutMarginType;
      }
      m_LayoutDefaultHandler.writeProperties(m_indent, varName, properties, marginType, suppressMarginDefault, m_output);
   }

   m_layoutMarginType = SubLayoutMargin;

   DomPropertyList propList = node->elementProperty();

   if (m_layoutWidget) {
      bool left, top, right, bottom;
      left = top = right = bottom = false;

      for (int i = 0; i < propList.size(); ++i) {
         const DomProperty *p = propList.at(i);
         const QString propertyName = p->attributeName();

         if (propertyName == "leftMargin" && p->kind() == DomProperty::Number) {
            left = true;

         } else if (propertyName == "topMargin" && p->kind() == DomProperty::Number) {
            top = true;

         } else if (propertyName == "rightMargin" && p->kind() == DomProperty::Number) {
            right = true;

         } else if (propertyName == "bottomMargin" && p->kind() == DomProperty::Number) {
            bottom = true;
         }
      }

      if (! left) {
         DomProperty *p = new DomProperty();
         p->setAttributeName("leftMargin");
         p->setElementNumber(0);
         propList.append(p);
      }

      if (! top) {
         DomProperty *p = new DomProperty();
         p->setAttributeName("topMargin");
         p->setElementNumber(0);
         propList.append(p);
      }

      if (! right) {
         DomProperty *p = new DomProperty();
         p->setAttributeName("rightMargin");
         p->setElementNumber(0);
         propList.append(p);
      }

      if (! bottom) {
         DomProperty *p = new DomProperty();
         p->setAttributeName("bottomMargin");
         p->setElementNumber(0);
         propList.append(p);
      }

      m_layoutWidget = false;
   }

   writeProperties(varName, className, propList, WritePropertyIgnoreMargin | WritePropertyIgnoreSpacing);

   m_layoutChain.push(node);
   TreeWalker::acceptLayout(node);
   m_layoutChain.pop();

   const QString numberNull = QString('0');

   writePropertyList(varName, "setStretch",            node->attributeStretch(), numberNull);
   writePropertyList(varName, "setRowStretch",         node->attributeRowStretch(), numberNull);
   writePropertyList(varName, "setColumnStretch",      node->attributeColumnStretch(), numberNull);
   writePropertyList(varName, "setColumnMinimumWidth", node->attributeColumnMinimumWidth(), numberNull);
   writePropertyList(varName, "setRowMinimumHeight",   node->attributeRowMinimumHeight(), numberNull);
}

// Apply a comma-separated list of values using a function "setSomething(int idx, value)"
void WriteInitialization::writePropertyList(const QString &varName, const QString &setFunction,
   const QString &value, const QString &defaultValue)
{
   if (value.isEmpty()) {
      return;
   }

   const QStringList list = value.split(',');
   const int count =  list.count();

   for (int i = 0; i < count; i++) {
      if (list.at(i) != defaultValue) {
         m_output << m_indent << varName << "->" << setFunction << '(' << i << ", " << list.at(i) << ");\n";
      }
   }
}

void WriteInitialization::acceptSpacer(DomSpacer *node)
{
   m_output << m_indent << m_driver->findOrInsertSpacer(node) << " = ";
   writeSpacerItem(node, m_output);
   m_output << ";\n";
}

static inline QString formLayoutRole(int column, int colspan)
{
   if (colspan > 1) {
      return "QFormLayout::SpanningRole";
   }

   return column == 0 ? QString("QFormLayout::LabelRole") : QString("QFormLayout::FieldRole");
}

void WriteInitialization::acceptLayoutItem(DomLayoutItem *node)
{
   TreeWalker::acceptLayoutItem(node);

   DomLayout *layout = m_layoutChain.top();

   if (! layout) {
      return;
   }

   const QString layoutName = m_driver->findOrInsertLayout(layout);
   const QString itemName   = m_driver->findOrInsertLayoutItem(node);

   QString addArgs;
   QString methodPrefix = "add";

   if (layout->attributeClass() == "QGridLayout") {
      const int row = node->attributeRow();
      const int col = node->attributeColumn();

      const int rowSpan = node->hasAttributeRowSpan() ? node->attributeRowSpan() : 1;
      const int colSpan = node->hasAttributeColSpan() ? node->attributeColSpan() : 1;

      addArgs = QString("%1, %2, %3, %4, %5")
            .formatArg(itemName).formatArg(row).formatArg(col).formatArg(rowSpan).formatArg(colSpan);

      if (!node->attributeAlignment().isEmpty()) {
         addArgs += ", " + node->attributeAlignment();
      }

   } else {
      if (layout->attributeClass() == "QFormLayout") {
         methodPrefix = "set";

         const int row      = node->attributeRow();
         const int colSpan  = node->hasAttributeColSpan() ? node->attributeColSpan() : 1;
         const QString role = formLayoutRole(node->attributeColumn(), colSpan);

         addArgs = QString::fromLatin1("%1, %2, %3").formatArg(row).formatArg(role).formatArg(itemName);

      } else {
         addArgs = itemName;

         if (layout->attributeClass().contains("Box") && !node->attributeAlignment().isEmpty()) {
            addArgs += ", 0, " + node->attributeAlignment();
         }
      }
   }

   // figure out "add" method
   m_output << "\n" << m_indent << layoutName << "->";

   switch (node->kind()) {
      case DomLayoutItem::Widget:
         m_output << methodPrefix << "Widget(" <<  addArgs;
         break;

      case DomLayoutItem::Layout:
         m_output <<  methodPrefix << "Layout(" << addArgs;
         break;

      case DomLayoutItem::Spacer:
         m_output << methodPrefix << "Item(" << addArgs;
         break;

      case DomLayoutItem::Unknown:
         Q_ASSERT( 0 );
         break;
   }

   m_output << ");\n\n";
}

void WriteInitialization::acceptActionGroup(DomActionGroup *node)
{
   const QString actionName = m_driver->findOrInsertActionGroup(node);
   QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());

   if (m_actionGroupChain.top()) {
      varName = m_driver->findOrInsertActionGroup(m_actionGroupChain.top());
   }

   m_output << m_indent << actionName << " = new QActionGroup(" << varName << ");\n";
   writeProperties(actionName, "QActionGroup", node->elementProperty());

   m_actionGroupChain.push(node);
   TreeWalker::acceptActionGroup(node);
   m_actionGroupChain.pop();
}

void WriteInitialization::acceptAction(DomAction *node)
{
   if (node->hasAttributeMenu()) {
      return;
   }

   const QString actionName = m_driver->findOrInsertAction(node);
   m_registeredActions.insert(actionName, node);
   QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());

   if (m_actionGroupChain.top()) {
      varName = m_driver->findOrInsertActionGroup(m_actionGroupChain.top());
   }

   m_output << m_indent << actionName << " = new QAction(" << varName << ");\n";
   writeProperties(actionName, "QAction", node->elementProperty());
}

void WriteInitialization::acceptActionRef(DomActionRef *node)
{
   QString actionName = node->attributeName();
   const bool isSeparator = actionName == "separator";
   bool isMenu = false;

   QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());

   if (actionName.isEmpty() || !m_widgetChain.top()) {
      return;

   } else if (m_driver->actionGroupByName(actionName)) {
      return;

   } else if (DomWidget *w = m_driver->widgetByName(actionName)) {
      isMenu = m_uic->isMenu(w->attributeClass());

   } else if (!(m_driver->actionByName(actionName) || isSeparator)) {
      qWarning( "%s: Warning: Action `%s' not declared",
            csPrintable(m_option.messagePrefix()), csPrintable(actionName));

      return;
   }

   if (m_widgetChain.top() && isSeparator) {
      // separator is always reserved
      m_actionOut << m_indent << varName << "->addSeparator();\n";
      return;
   }

   if (isMenu) {
      actionName += "->menuAction()";
   }

   m_actionOut << m_indent << varName << "->addAction(" << actionName << ");\n";
}

QString WriteInitialization::writeStringListProperty(const DomStringList *list) const
{
   QString propertyValue;

   QTextStream str(&propertyValue);
   str << "QStringList()";

   const QStringList values = list->elementString();

   if (values.isEmpty()) {
      return propertyValue;
   }

   if (needsTranslation(list)) {
     const QString comment = list->attributeComment();

     for (int i = 0; i < values.size(); ++i) {
         str << '\n' << m_indent << "    << " << trCall(values.at(i), comment);
     }

   } else {
     for (int i = 0; i < values.size(); ++i) {
         str << " << " << fixString(values.at(i), m_dindent);
     }
   }

   return propertyValue;
}

void WriteInitialization::writeProperties(const QString &varName,
   const QString &className, const DomPropertyList &lst, unsigned flags)
{
   const bool isTopLevel = m_widgetChain.count() == 1;

   if (m_uic->customWidgetsInfo()->extends(className, "QAxWidget")) {
      DomPropertyMap properties = propertyMap(lst);

      if (properties.contains("control")) {
         DomProperty *p = properties.value("control");

         m_output << m_indent << varName << "->setControl(QString::fromUtf8("
            << fixString(toString(p->elementString()), m_dindent) << "));\n";
      }
   }

   QString indent;

   if (! m_widgetChain.top()) {
      indent = m_option.indent;
      m_output << m_indent << "if (" << varName << "->objectName().isEmpty()) {\n";
   }

   if (! (flags & WritePropertyIgnoreObjectName)) {
      m_output << m_indent << indent << varName
            << "->setObjectName(QString::fromUtf8(" << fixString(varName, m_dindent) << "));\n";
   }

   if (! m_widgetChain.top()) {
      m_output << m_indent << "}\n";
   }

   int leftMargin   =-1;
   int topMargin    =-1;
   int rightMargin  =-1;
   int bottomMargin =-1;

   bool frameShadowEncountered = false;

   for (int i = 0; i < lst.size(); ++i) {
      const DomProperty *p = lst.at(i);

      if (! checkProperty(m_option.inputFile, p)) {
         continue;
      }

      const QString propertyName = p->attributeName();
      QString propertyValue;

      // special case for the property `geometry': Do not use position
      if (isTopLevel && propertyName == "geometry" && p->elementRect()) {
         const DomRect *r = p->elementRect();
         m_output << m_indent << varName << "->resize(" << r->elementWidth() << ", " << r->elementHeight() << ");\n";
         continue;

      } else if (propertyName == "buttonGroupId") {
         continue;

      } else if (propertyName == "currentRow"         // QListWidget::currentRow
            && m_uic->customWidgetsInfo()->extends(className, "QListWidget")) {

         m_delayedOut << m_indent << varName << "->setCurrentRow(" << p->elementNumber() << ");\n";
         continue;

      } else if (propertyName == "currentIndex"       // set currentIndex later
            && (m_uic->customWidgetsInfo()->extends(className, "QComboBox")
            ||  m_uic->customWidgetsInfo()->extends(className, "QStackedWidget")
            ||  m_uic->customWidgetsInfo()->extends(className, "QTabWidget")
            ||  m_uic->customWidgetsInfo()->extends(className, "QToolBox"))) {

         m_delayedOut << m_indent << varName << "->setCurrentIndex("
                      << p->elementNumber() << ");\n";
         continue;

      } else if (propertyName == "tabSpacing" && m_uic->customWidgetsInfo()->extends(className, "QToolBox")) {

         m_delayedOut << m_indent << varName << "->layout()->setSpacing("
                      << p->elementNumber() << ");\n";
         continue;

      } else if (propertyName == "control" && m_uic->customWidgetsInfo()->extends(className, "QAxWidget")) {
         // already done
         continue;

      } else if (propertyName == "database" && p->elementStringList()) {
         // Sql support
         continue;

      } else if (propertyName == "frameworkCode" && p->kind() == DomProperty::Bool) {
         // Sql support
         continue;

      } else if (propertyName == "orientation" && m_uic->customWidgetsInfo()->extends(className, "Line")) {
         // Line support
         QString shape = "QFrame::HLine";

         if (p->elementEnum() == "Qt::Vertical") {
            shape = "QFrame::VLine";
         }

         m_output << m_indent << varName << "->setFrameShape(" << shape << ");\n";
         // QFrame Default is 'Plain'. Make the line 'Sunken' unless otherwise specified
         if (! frameShadowEncountered) {
            m_output << m_indent << varName << "->setFrameShadow(QFrame::Sunken);\n";
         }
         continue;

      } else if ((flags & WritePropertyIgnoreMargin)  && propertyName == "margin") {
         continue;

      } else if ((flags & WritePropertyIgnoreSpacing) && propertyName == "spacing") {
         continue;

      } else if (propertyName == "leftMargin" && p->kind() == DomProperty::Number) {
         leftMargin = p->elementNumber();
         continue;

      } else if (propertyName == "topMargin" && p->kind() == DomProperty::Number) {
         topMargin = p->elementNumber();
         continue;

      } else if (propertyName == "rightMargin" && p->kind() == DomProperty::Number) {
         rightMargin = p->elementNumber();
         continue;

      } else if (propertyName == "bottomMargin" && p->kind() == DomProperty::Number) {
         bottomMargin = p->elementNumber();
         continue;

      } else if (propertyName == "frameShadow") {
         frameShadowEncountered = true;
      }

      bool stdset = m_stdsetdef;
      if (p->hasAttributeStdset()) {
         stdset = p->attributeStdset();
      }

      QString setFunction;

      if (stdset) {
         setFunction = "->set";
         setFunction += propertyName.left(1).toUpper();
         setFunction += propertyName.mid(1);
         setFunction += '(';

      } else {
         setFunction = "->setProperty(\"";
         setFunction += propertyName;
         setFunction += "\", QVariant(";
      }

      QString varNewName = varName;

      switch (p->kind()) {
         case DomProperty::Bool: {
            propertyValue = p->elementBool();
            break;
         }

         case DomProperty::Color:
            propertyValue = domColor2QString(p->elementColor());
            break;

         case DomProperty::Cstring:
            if (propertyName == "buddy" && m_uic->customWidgetsInfo()->extends(className, "QLabel")) {
               m_buddies.append(Buddy(varName, p->elementCstring()));

            } else {
               if (stdset) {
                  propertyValue = fixString(p->elementCstring(), m_dindent);
               } else {
                  propertyValue = "QByteArray(";
                  propertyValue += fixString(p->elementCstring(), m_dindent);
                  propertyValue += ')';
               }
            }
            break;

         case DomProperty::Cursor:
            propertyValue = QString("QCursor(static_cast<Qt::CursorShape>(%1))").formatArg(p->elementCursor());
            break;

         case DomProperty::CursorShape:
            if (p->hasAttributeStdset() && ! p->attributeStdset()) {
               varNewName += "->viewport()";
            }

            propertyValue = QString("QCursor(Qt::%1)").formatArg(p->elementCursorShape());
            break;

         case DomProperty::Enum:
            propertyValue = p->elementEnum();
            if (!propertyValue.contains("::")) {
               QString scope  = className;
               scope += "::";
               propertyValue.prepend(scope);
            }
            break;

         case DomProperty::Set:
            propertyValue = p->elementSet();
            break;

         case DomProperty::Font:
            propertyValue = writeFontProperties(p->elementFont());
            break;

         case DomProperty::IconSet:
            propertyValue = writeIconProperties(p->elementIconSet());
            break;

         case DomProperty::Pixmap:
            propertyValue = pixCall(p);
            break;

         case DomProperty::Palette: {
            const DomPalette *pal = p->elementPalette();
            const QString paletteName = m_driver->unique("palette");
            m_output << m_indent << "QPalette " << paletteName << ";\n";

            writeColorGroup(pal->elementActive(),   "QPalette::Active",   paletteName);
            writeColorGroup(pal->elementInactive(), "QPalette::Inactive", paletteName);
            writeColorGroup(pal->elementDisabled(), "QPalette::Disabled", paletteName);

            propertyValue = paletteName;
            break;
         }

         case DomProperty::Point: {
            const DomPoint *po = p->elementPoint();
            propertyValue = QString("QPoint(%1, %2)").formatArg(po->elementX()).formatArg(po->elementY());
            break;
         }

         case DomProperty::PointF: {
            const DomPointF *pof = p->elementPointF();
            propertyValue = QString("QPointF(%1, %2)").formatArg(pof->elementX()).formatArg(pof->elementY());
            break;
         }

         case DomProperty::Rect: {
            const DomRect *r = p->elementRect();
            propertyValue = QString("QRect(%1, %2, %3, %4)")
               .formatArg(r->elementX()).formatArg(r->elementY())
               .formatArg(r->elementWidth()).formatArg(r->elementHeight());
            break;
         }

         case DomProperty::RectF: {
            const DomRectF *rf = p->elementRectF();
            propertyValue = QString("QRectF(%1, %2, %3, %4)")
               .formatArg(rf->elementX()).formatArg(rf->elementY())
               .formatArg(rf->elementWidth()).formatArg(rf->elementHeight());
            break;
         }
         case DomProperty::Locale: {
            const DomLocale *locale = p->elementLocale();
            propertyValue = QString("QLocale(QLocale::%1, QLocale::%2)")
               .formatArg(locale->attributeLanguage()).formatArg(locale->attributeCountry());
            break;
         }

         case DomProperty::SizePolicy: {
            const QString spName = writeSizePolicy( p->elementSizePolicy());
            m_output << m_indent << spName
                  << QString(".setHeightForWidth(%1->sizePolicy().hasHeightForWidth());\n").formatArg(varName);

            propertyValue = spName;
            break;
         }

         case DomProperty::Size: {
            const DomSize *s = p->elementSize();
            propertyValue = QString("QSize(%1, %2)").formatArg(s->elementWidth()).formatArg(s->elementHeight());
            break;
         }

         case DomProperty::SizeF: {
            const DomSizeF *sf = p->elementSizeF();
            propertyValue = QString("QSizeF(%1, %2)").formatArg(sf->elementWidth()).formatArg(sf->elementHeight());
            break;
         }

         case DomProperty::String: {
            if (propertyName == "objectName") {
               const QString v = p->elementString()->text();
               if (v == varName) {
                  break;
               }

               // ### qWarning("Deprecated: the property `objectName' is different from the variable name");
            }

            propertyValue = autoTrCall(p->elementString());
            break;
         }

         case DomProperty::Number:
            propertyValue = QString::number(p->elementNumber());
            break;

         case DomProperty::UInt:
            propertyValue = QString::number(p->elementUInt());
            propertyValue += 'u';
            break;

         case DomProperty::LongLong:
            propertyValue = "Q_INT64_C(";
            propertyValue += QString::number(p->elementLongLong());
            propertyValue += ')';
            break;

         case DomProperty::ULongLong:
            propertyValue = "Q_UINT64_C(";
            propertyValue += QString::number(p->elementULongLong());
            propertyValue += ')';
            break;

         case DomProperty::Float:
            propertyValue = QString::number(p->elementFloat());
            break;

         case DomProperty::Double:
            propertyValue = QString::number(p->elementDouble());
            break;

         case DomProperty::Char: {
            const DomChar *c = p->elementChar();
            propertyValue = QString("QChar(%1)").formatArg(c->elementUnicode());
            break;
         }

         case DomProperty::Date: {
            const DomDate *d = p->elementDate();
            propertyValue = QString("QDate(%1, %2, %3)")
               .formatArg(d->elementYear())
               .formatArg(d->elementMonth())
               .formatArg(d->elementDay());
            break;
         }

         case DomProperty::Time: {
            const DomTime *t = p->elementTime();
            propertyValue = QString("QTime(%1, %2, %3)")
               .formatArg(t->elementHour())
               .formatArg(t->elementMinute())
               .formatArg(t->elementSecond());
            break;
         }

         case DomProperty::DateTime: {
            const DomDateTime *dt = p->elementDateTime();
            propertyValue = QString("QDateTime(QDate(%1, %2, %3), QTime(%4, %5, %6))")
               .formatArg(dt->elementYear())
               .formatArg(dt->elementMonth())
               .formatArg(dt->elementDay())
               .formatArg(dt->elementHour())
               .formatArg(dt->elementMinute())
               .formatArg(dt->elementSecond());
            break;
         }

         case DomProperty::StringList:
            propertyValue = writeStringListProperty(p->elementStringList());
            break;

         case DomProperty::Url: {
            const DomUrl *u = p->elementUrl();
            propertyValue = QString("QUrl(QString::fromUtf8(%1))")
                  .formatArg(fixString(u->elementString()->text(), m_dindent));
            break;
         }

         case DomProperty::Brush:
            propertyValue = writeBrushInitialization(p->elementBrush());
            break;

         case DomProperty::Unknown:
            break;
      }

      if (propertyValue.size()) {
         QString defineC;

         if (propertyName == "toolTip") {
            defineC = toolTipDefineC;

         } else if (propertyName == "whatsThis") {
            defineC = whatsThisDefineC;

         } else if (propertyName == "statusTip") {
            defineC = statusTipDefineC;

         } else if (propertyName == "accessibleName" || propertyName == "accessibleDescription") {
            defineC = accessibilityDefineC;
         }

         QTextStream &outStream = autoTrOutput(p);

         if (! defineC.isEmpty()) {
            openIfndef(outStream, defineC);
         }

         outStream << m_indent << varNewName << setFunction << propertyValue;

         if (! stdset) {
            outStream << ')';
         }

         outStream << ");\n";

         if (! defineC.isEmpty()) {
            closeIfndef(outStream, defineC);
         }

         if ((varName == m_mainFormVarName) && (&outStream == &m_refreshOut)) {
            // only place (currently) where we output mainForm name to the retranslateUi()
            // Other places output merely instances of a certain class (which cannot be main form, e.g. QListWidget).
            m_mainFormUsedInRetranslateUi = true;
         }
      }
   }

   if (leftMargin != -1 || topMargin != -1 || rightMargin != -1 || bottomMargin != -1) {
      QString objectName = varName;

      if (m_widgetChain.top()) {
         const QString parentWidget = m_widgetChain.top()->attributeClass();
      }

      m_output << m_indent << objectName << "->setContentsMargins("
               << leftMargin << ", "
               << topMargin << ", "
               << rightMargin << ", "
               << bottomMargin << ");\n";
   }
}

QString  WriteInitialization::writeSizePolicy(const DomSizePolicy *sp)
{
   // check cache
   const SizePolicyHandle sizePolicyHandle(sp);

   const SizePolicyNameMap::const_iterator iter = m_sizePolicyNameMap.constFind(sizePolicyHandle);
   if (iter != m_sizePolicyNameMap.constEnd()) {
      return iter.value();
   }

   // insert with new name
   const QString spName = m_driver->unique("sizePolicy");
   m_sizePolicyNameMap.insert(sizePolicyHandle, spName);

   m_output << m_indent << "QSizePolicy " << spName;

   do {
      if (sp->hasElementHSizeType() && sp->hasElementVSizeType()) {
         m_output << "(static_cast<QSizePolicy::Policy>(" << sp->elementHSizeType()
            << "), static_cast<QSizePolicy::Policy>(" << sp->elementVSizeType() << "));\n";
         break;
      }

      if (sp->hasAttributeHSizeType() && sp->hasAttributeVSizeType()) {
         m_output << "(QSizePolicy::" << sp->attributeHSizeType() << ", QSizePolicy::"
            << sp->attributeVSizeType() << ");\n";
         break;
      }

      m_output << ";\n";

   } while (false);

   m_output << m_indent << spName << ".setHorizontalStretch("
      << sp->elementHorStretch() << ");\n";

   m_output << m_indent << spName << ".setVerticalStretch("
      << sp->elementVerStretch() << ");\n";

   return spName;
}

// Check for a font with the given properties in the FontPropertiesNameMap
// or create a new one. Returns the name.

QString WriteInitialization::writeFontProperties(const DomFont *f)
{
   // check cache
   const FontHandle fontHandle(f);

   const FontPropertiesNameMap::const_iterator iter = m_fontPropertiesNameMap.constFind(fontHandle);
   if (iter != m_fontPropertiesNameMap.constEnd()) {
      return iter.value();
   }

   // insert with new name
   const QString fontName = m_driver->unique("font");
   m_fontPropertiesNameMap.insert(FontHandle(f), fontName);

   m_output << m_indent << "QFont " << fontName << ";\n";
   if (f->hasElementFamily() && !f->elementFamily().isEmpty()) {
      m_output << m_indent << fontName << ".setFamily(QString::fromUtf8(" << fixString(f->elementFamily(), m_dindent)
         << "));\n";
   }

   if (f->hasElementPointSize() && f->elementPointSize() > 0) {
      m_output << m_indent << fontName << ".setPointSize(" << f->elementPointSize()
         << ");\n";
   }

   if (f->hasElementBold()) {
      m_output << m_indent << fontName << ".setBold("
         << (f->elementBold() ? "true" : "false") << ");\n";
   }

   if (f->hasElementItalic()) {
      m_output << m_indent << fontName << ".setItalic("
         <<  (f->elementItalic() ? "true" : "false") << ");\n";
   }

   if (f->hasElementUnderline()) {
      m_output << m_indent << fontName << ".setUnderline("
         << (f->elementUnderline() ? "true" : "false") << ");\n";
   }

   if (f->hasElementWeight() && f->elementWeight() > 0) {
      m_output << m_indent << fontName << ".setWeight("
         << f->elementWeight() << ");\n";
   }

   if (f->hasElementStrikeOut()) {
      m_output << m_indent << fontName << ".setStrikeOut("
         << (f->elementStrikeOut() ? "true" : "false") << ");\n";
   }

   if (f->hasElementKerning()) {
      m_output << m_indent << fontName << ".setKerning("
         << (f->elementKerning() ? "true" : "false") << ");\n";
   }

   if (f->hasElementAntialiasing()) {
      m_output << m_indent << fontName << ".setStyleStrategy("
         << (f->elementAntialiasing() ? "QFont::PreferDefault" : "QFont::NoAntialias") << ");\n";
   }

   if (f->hasElementStyleStrategy()) {
      m_output << m_indent << fontName << ".setStyleStrategy(QFont::"
         << f->elementStyleStrategy() << ");\n";
   }

   return  fontName;
}

static void writeResourceIcon(QTextStream &output, const QString &iconName, const QString &indent,
      const DomResourceIcon *i)
{
   if (i->hasElementNormalOff()) {
      output << indent << iconName << ".addFile(QString::fromUtf8(" << fixString(i->elementNormalOff()->text(),
            indent) << "), QSize(), QIcon::Normal, QIcon::Off);\n";
   }
   if (i->hasElementNormalOn()) {
      output << indent << iconName << ".addFile(QString::fromUtf8(" << fixString(i->elementNormalOn()->text(),
            indent) << "), QSize(), QIcon::Normal, QIcon::On);\n";
   }
   if (i->hasElementDisabledOff()) {
      output << indent << iconName << ".addFile(QString::fromUtf8(" << fixString(i->elementDisabledOff()->text(),
            indent) << "), QSize(), QIcon::Disabled, QIcon::Off);\n";
   }
   if (i->hasElementDisabledOn()) {
      output << indent << iconName << ".addFile(QString::fromUtf8(" << fixString(i->elementDisabledOn()->text(),
            indent) << "), QSize(), QIcon::Disabled, QIcon::On);\n";
   }
   if (i->hasElementActiveOff()) {
      output << indent << iconName << ".addFile(QString::fromUtf8(" << fixString(i->elementActiveOff()->text(),
            indent) << "), QSize(), QIcon::Active, QIcon::Off);\n";
   }
   if (i->hasElementActiveOn()) {
      output << indent << iconName << ".addFile(QString::fromUtf8(" << fixString(i->elementActiveOn()->text(),
            indent) << "), QSize(), QIcon::Active, QIcon::On);\n";
   }
   if (i->hasElementSelectedOff()) {
      output << indent << iconName << ".addFile(QString::fromUtf8(" << fixString(i->elementSelectedOff()->text(),
            indent) << "), QSize(), QIcon::Selected, QIcon::Off);\n";
   }
   if (i->hasElementSelectedOn()) {
      output << indent << iconName << ".addFile(QString::fromUtf8(" << fixString(i->elementSelectedOn()->text(),
            indent) << "), QSize(), QIcon::Selected, QIcon::On);\n";
   }
}

QString WriteInitialization::writeIconProperties(const DomResourceIcon *i)
{
   // check cache
   const IconHandle iconHandle(i);
   const IconPropertiesNameMap::const_iterator it = m_iconPropertiesNameMap.constFind(iconHandle);

   if (it != m_iconPropertiesNameMap.constEnd()) {
      return it.value();
   }

   // insert with new name
   const QString iconName = m_driver->unique("icon");
   m_iconPropertiesNameMap.insert(IconHandle(i), iconName);

   if (isIconFormat44(i)) {
      if (i->attributeTheme().isEmpty()) {
         // No theme: Write resource icon as is
         m_output << m_indent << "QIcon " << iconName << ";\n";
         writeResourceIcon(m_output, iconName, m_indent, i);

      } else {
         // Theme: Generate code to check the theme and default to resource
         const QString themeIconName = fixString(i->attributeTheme(), QString());

         if (iconHasStatePixmaps(i)) {
            // Theme + default state pixmaps:
            // Generate code to check the theme and default to state pixmaps
            m_output << m_indent << "QIcon " << iconName << ";\n";
            const char themeNameStringVariableC[] = "iconThemeName";

            // Store theme name in a variable
            m_output << m_indent;

            if (m_firstThemeIcon) { // Declare variable string
               m_output << "QString ";
               m_firstThemeIcon = false;
            }

            m_output << themeNameStringVariableC << " = QString::fromUtf8("
                     << themeIconName << ");\n";

            m_output << m_indent << "if (QIcon::hasThemeIcon("
                     << themeNameStringVariableC
                     << ")) {\n"
                     << m_dindent << iconName << " = QIcon::fromTheme(" << themeNameStringVariableC << ");\n"
                     << m_indent << "} else {\n";

            writeResourceIcon(m_output, iconName, m_dindent, i);
            m_output << m_indent << "}\n";

         } else {
            // Theme, but no state pixmaps: Construct from theme directly.
            m_output << m_indent << "QIcon " << iconName
                     << "(QIcon::fromTheme(QString::fromUtf8("
                     << themeIconName << ")));\n";
         }
      }

   } else {
      // legacy code
      m_output <<  m_indent << "const QIcon " << iconName << " = " << pixCall("QIcon", i->text()) << ";\n";
   }

   return iconName;
}

QString WriteInitialization::domColor2QString(const DomColor *c)
{
   if (c->hasAttributeAlpha()) {
      return QString("QColor(%1, %2, %3, %4)")
         .formatArg(c->elementRed())
         .formatArg(c->elementGreen())
         .formatArg(c->elementBlue())
         .formatArg(c->attributeAlpha());
   }

   return QString("QColor(%1, %2, %3)").formatArg(c->elementRed()).formatArg(c->elementGreen()).formatArg(c->elementBlue());
}

void WriteInitialization::writeColorGroup(DomColorGroup *colorGroup, const QString &group, const QString &paletteName)
{
   if (! colorGroup) {
      return;
   }

   // old format
   const QList<DomColor *> colors = colorGroup->elementColor();
   for (int i = 0; i < colors.size(); ++i) {
      const DomColor *color = colors.at(i);

      m_output << m_indent << paletteName << ".setColor(" << group
               << ", " << "static_cast<QPalette::ColorRole>(" << QString::number(i) << ')'
               << ", " << domColor2QString(color)
               << ");\n";
   }

   // new format
   const QList<DomColorRole *> colorRoles = colorGroup->elementColorRole();
   QListIterator<DomColorRole *> itRole(colorRoles);

   while (itRole.hasNext()) {
      const DomColorRole *colorRole = itRole.next();

      if (colorRole->hasAttributeRole()) {
         const QString brushName = writeBrushInitialization(colorRole->elementBrush());
         m_output << m_indent << paletteName << ".setBrush(" << group
            << ", " << "QPalette::" << colorRole->attributeRole()
            << ", " << brushName << ");\n";
      }
   }
}

// Write initialization for brush unless it is found in the cache.
// Returns the name to use in an expression.
QString WriteInitialization::writeBrushInitialization(const DomBrush *brush)
{
   // Simple solid, colored  brushes are cached
   const bool solidColoredBrush = ! brush->hasAttributeBrushStyle() ||
      brush->attributeBrushStyle() == "SolidPattern";

   uint rgb = 0;

   if (solidColoredBrush) {
      if (const DomColor *color = brush->elementColor()) {
         rgb = ((color->elementRed() & 0xFF) << 24) |
            ((color->elementGreen() & 0xFF) << 16) |
            ((color->elementBlue() & 0xFF) << 8) |
            ((color->attributeAlpha() & 0xFF));

         const ColorBrushHash::const_iterator cit = m_colorBrushHash.constFind(rgb);
         if (cit != m_colorBrushHash.constEnd()) {
            return cit.value();
         }
      }
   }

   // Create and enter into cache if simple
   const QString brushName = m_driver->unique("brush");
   writeBrush(brush, brushName);

   if (solidColoredBrush) {
      m_colorBrushHash.insert(rgb, brushName);
   }

   return brushName;
}

void WriteInitialization::writeBrush(const DomBrush *brush, const QString &brushName)
{
   QString style = "SolidPattern";
   if (brush->hasAttributeBrushStyle()) {
      style = brush->attributeBrushStyle();
   }

   if (style == "LinearGradientPattern" || style == "RadialGradientPattern" || style == "ConicalGradientPattern") {

      const DomGradient *gradient = brush->elementGradient();
      const QString gradientType = gradient->attributeType();
      const QString gradientName = m_driver->unique("gradient");

      if (gradientType == "LinearGradient") {
         m_output << m_indent << "QLinearGradient " << gradientName
            << '(' << gradient->attributeStartX()
            << ", " << gradient->attributeStartY()
            << ", " << gradient->attributeEndX()
            << ", " << gradient->attributeEndY() << ");\n";

      } else if (gradientType == "RadialGradient") {
         m_output << m_indent << "QRadialGradient " << gradientName
            << '(' << gradient->attributeCentralX()
            << ", " << gradient->attributeCentralY()
            << ", " << gradient->attributeRadius()
            << ", " << gradient->attributeFocalX()
            << ", " << gradient->attributeFocalY() << ");\n";

      } else if (gradientType == "ConicalGradient") {
         m_output << m_indent << "QConicalGradient " << gradientName
            << '(' << gradient->attributeCentralX()
            << ", " << gradient->attributeCentralY()
            << ", " << gradient->attributeAngle() << ");\n";
      }

      m_output << m_indent << gradientName << ".setSpread(QGradient::"
         << gradient->attributeSpread() << ");\n";

      if (gradient->hasAttributeCoordinateMode()) {
         m_output << m_indent << gradientName << ".setCoordinateMode(QGradient::"
            << gradient->attributeCoordinateMode() << ");\n";
      }

      const  QList<DomGradientStop *> stops = gradient->elementGradientStop();
      QListIterator<DomGradientStop *> it(stops);

      while (it.hasNext()) {
         const DomGradientStop *stop = it.next();
         const DomColor *color = stop->elementColor();
         m_output << m_indent << gradientName << ".setColorAt("
            << stop->attributePosition() << ", "
            << domColor2QString(color) << ");\n";
      }

      m_output << m_indent << "QBrush " << brushName << '(' << gradientName << ");\n";

   } else if (style == "TexturePattern") {
      const DomProperty *property = brush->elementTexture();
      const QString iconValue = iconCall(property);

      m_output << m_indent << "QBrush " << brushName << " = QBrush(" << iconValue << ");\n";

   } else {
      const DomColor *color = brush->elementColor();

      m_output << m_indent << "QBrush " << brushName << '('
         << domColor2QString(color) << ");\n";

      m_output << m_indent << brushName << ".setStyle("
         << "Qt::" << style << ");\n";
   }
}

void WriteInitialization::acceptCustomWidget(DomCustomWidget *node)
{
   (void) node;
}

void WriteInitialization::acceptCustomWidgets(DomCustomWidgets *node)
{
   (void) node;
}

void WriteInitialization::acceptTabStops(DomTabStops *tabStops)
{
   QString lastName;

   const QStringList l = tabStops->elementTabStop();

   for (int i = 0; i < l.size(); ++i) {
      const QString name = l.at(i);

      if (!m_registeredWidgets.contains(name)) {
         qWarning("%s: Warning: Tab stop assignment '%s' is not a valid widget",
               csPrintable(m_option.messagePrefix()), csPrintable(name));

         continue;
      }

      if (i == 0) {
         lastName = name;
         continue;
      } else if (name.isEmpty() || lastName.isEmpty()) {
         continue;
      }

      m_output << m_indent << "QWidget::setTabOrder(" << lastName << ", " << name << ");\n";

      lastName = name;
   }
}

QString WriteInitialization::iconCall(const DomProperty *icon)
{
   if (icon->kind() == DomProperty::IconSet) {
      return writeIconProperties(icon->elementIconSet());
   }

   return pixCall(icon);
}

QString WriteInitialization::pixCall(const DomProperty *p) const
{
   QString type;
   QString s;

   switch (p->kind()) {
      case DomProperty::IconSet:
         type = "QIcon";
         s = p->elementIconSet()->text();
         break;

      case DomProperty::Pixmap:
         type = "QPixmap";
         s = p->elementPixmap()->text();
         break;

      default:
         qWarning("%s: Warning: Unknown icon format found", csPrintable(m_option.messagePrefix()));
         return "QIcon()";
   }

   return pixCall(type, s);
}

QString WriteInitialization::pixCall(const QString &t, const QString &text) const
{
   QString type = t;

   if (text.isEmpty()) {
      type += "()";
      return type;
   }

   if (const DomImage *image = findImage(text)) {
      if (m_option.extractImages) {
         const QString format = image->elementData()->attributeFormat();
         const QString extension = format.left(format.indexOf('.')).toLower();

         QString rc = "QPixmap(QString::fromUtf8(\":/";

         rc += m_generatedClass;
         rc += "/images/";
         rc += text;
         rc += '.';
         rc += extension;
         rc += "\"))";
         return rc;
      }

      QString rc = WriteIconInitialization::iconFromDataFunction();
      rc += '(';
      rc += text;
      rc += "_ID)";
      return rc;
   }

   QString pixFunc = m_uic->pixmapFunction();
   if (pixFunc.isEmpty()) {
      pixFunc = "QString::fromUtf8";
   }

   type += '(';
   type += pixFunc;
   type += '(';
   type += fixString(text, m_dindent);
   type += "))";

   return type;
}

void WriteInitialization::initializeComboBox(DomWidget *w)
{
   const QString varName = m_driver->findOrInsertWidget(w);
   const QString className = w->attributeClass();

   const QList<DomItem *> items = w->elementItem();

   if (items.isEmpty()) {
      return;
   }

   // If possible use qcombobox's addItems() which is much faster then a bunch of addItem() calls
   bool makeStringListCall = true;
   bool translatable = false;
   QStringList list;

   for (int i = 0; i < items.size(); ++i) {
      const DomItem *item = items.at(i);
      const DomPropertyMap properties = propertyMap(item->elementProperty());

      const DomProperty *text   = properties.value("text");
      const DomProperty *pixmap = properties.value("icon");

      bool needsTr = needsTranslation(text->elementString());
      if (pixmap != nullptr || (i > 0 && translatable != needsTr)) {
         makeStringListCall = false;
         break;
      }
      translatable = needsTr;
      list.append(autoTrCall(text->elementString()));  // fix me here
   }

   if (makeStringListCall) {
      QTextStream &o = translatable ? m_refreshOut : m_output;

      if (translatable) {
         o << m_indent << varName << "->clear();\n";
      }

      o << m_indent << varName << "->insertItems(0, QStringList()" << '\n';
      for (int i = 0; i < list.size(); ++i) {
         o << m_indent << " << " << list.at(i) << "\n";
      }

      o << m_indent << ");\n";

   } else {
      for (int i = 0; i < items.size(); ++i) {
         const DomItem *item = items.at(i);
         const DomPropertyMap properties = propertyMap(item->elementProperty());
         const DomProperty *text = properties.value("text");
         const DomProperty *icon = properties.value("icon");

         QString iconValue;

         if (icon) {
            iconValue = iconCall(icon);
         }

         m_output << m_indent << varName << "->addItem(";
         if (icon) {
            m_output << iconValue << ", ";
         }

         if (needsTranslation(text->elementString())) {
            m_output << "QString());\n";
            m_refreshOut << m_indent << varName << "->setItemText(" << i << ", " << trCall(text->elementString()) << ");\n";

         } else {
            m_output << noTrCall(text->elementString()) << ");\n";
         }
      }
      m_refreshOut << "\n";
   }
}

QString WriteInitialization::disableSorting(DomWidget *w, const QString &varName)
{
   // turn off sortingEnabled to force programmatic item order (setItem())
   QString tempName;

   if (! w->elementItem().isEmpty()) {
      tempName = m_driver->unique("__sortingEnabled");

      m_refreshOut << "\n";
      m_refreshOut << m_indent << "const bool " << tempName
         << " = " << varName << "->isSortingEnabled();\n";
      m_refreshOut << m_indent << varName << "->setSortingEnabled(false);\n";
   }

   return tempName;
}

void WriteInitialization::enableSorting(DomWidget *w, const QString &varName, const QString &tempName)
{
   if (!w->elementItem().isEmpty()) {
      m_refreshOut << m_indent << varName << "->setSortingEnabled(" << tempName << ");\n\n";
   }
}

/*
 * Initializers are just strings containing the function call and need to be prepended
 * the line indentation and the object they are supposed to initialize.
 * String initializers come with a preprocessor conditional (ifdef), so the code
 * compiles with QT_NO_xxx. A null pointer means no conditional. String initializers
 * are written to the retranslateUi() function, others to setupUi().
 */

void WriteInitialization::addInitializer(Item *item,
   const QString &name, int column, const QString &value, const QString &directive, bool translatable) const
{
   if (! value.isEmpty()) {
      item->addSetter("->set" + name.at(0).toUpper() + name.mid(1) + '(' +
            (column < 0 ? QString() : QString::number(column) + ", ") + value + ");", directive, translatable);
   }
}

void WriteInitialization::addStringInitializer(Item *item, const DomPropertyMap &properties,
      const QString &name, int column, const QString &directive) const
{
   if (const DomProperty *p = properties.value(name)) {
      DomString *str = p->elementString();
      QString text = toString(str);

      if (! text.isEmpty()) {
         bool translatable = needsTranslation(str);
         QString value = autoTrCall(str);
         addInitializer(item, name, column, value, directive, translatable);
      }
   }
}

void WriteInitialization::addBrushInitializer(Item *item,
   const DomPropertyMap &properties, const QString &name, int column)
{
   if (const DomProperty *p = properties.value(name)) {
      if (p->elementBrush()) {
         addInitializer(item, name, column, writeBrushInitialization(p->elementBrush()));
      } else if (p->elementColor()) {
         addInitializer(item, name, column, domColor2QString(p->elementColor()));
      }
   }
}

void WriteInitialization::addQtFlagsInitializer(Item *item,
   const DomPropertyMap &properties, const QString &name, int column) const
{
   if (const DomProperty *p = properties.value(name)) {
      QString v = p->elementSet();

      if (! v.isEmpty()) {
         v.replace('|', "|Qt::");
         addInitializer(item, name, column, "Qt::" + v);
      }
   }
}

void WriteInitialization::addQtEnumInitializer(Item *item,
   const DomPropertyMap &properties, const QString &name, int column) const
{
   if (const DomProperty *p = properties.value(name)) {
      QString v = p->elementEnum();

      if (!v.isEmpty()) {
         addInitializer(item, name, column, "Qt::" + v);
      }
   }
}

void WriteInitialization::addCommonInitializers(Item *item,
   const DomPropertyMap &properties, int column)
{
   if (const DomProperty *icon = properties.value("icon")) {
      addInitializer(item, "icon", column, iconCall(icon));
   }

   addBrushInitializer(item, properties, "foreground", column);
   addBrushInitializer(item, properties, "background", column);

   if (const DomProperty *font = properties.value("font")) {
      addInitializer(item, "font", column, writeFontProperties(font->elementFont()));
   }

   addQtFlagsInitializer(item, properties, "textAlignment", column);
   addQtEnumInitializer(item,  properties, "checkState",    column);
   addStringInitializer(item,  properties, "text",          column);
   addStringInitializer(item,  properties, "toolTip",       column, toolTipDefineC);
   addStringInitializer(item,  properties, "whatsThis",     column, whatsThisDefineC);
   addStringInitializer(item,  properties, "statusTip",     column, statusTipDefineC);
}

void WriteInitialization::initializeListWidget(DomWidget *w)
{
   const QString varName = m_driver->findOrInsertWidget(w);
   const QString className = w->attributeClass();

   const QList<DomItem *> items = w->elementItem();

   if (items.isEmpty()) {
      return;
   }

   QString tempName = disableSorting(w, varName);

   // items, generated code should be data-driven to reduce its size

   for (int i = 0; i < items.size(); ++i) {
      const DomItem *domItem = items.at(i);

      const DomPropertyMap properties = propertyMap(domItem->elementProperty());

      Item item("QListWidgetItem", m_indent, m_output, m_refreshOut, m_driver);
      addQtFlagsInitializer(&item, properties, "flags");
      addCommonInitializers(&item, properties);

      item.writeSetupUi(varName);
      item.writeRetranslateUi(varName + "->item(" + QString::number(i) + ')');
   }

   enableSorting(w, varName, tempName);
}

void WriteInitialization::initializeTreeWidget(DomWidget *w)
{
   const QString varName = m_driver->findOrInsertWidget(w);

   // columns
   Item item("QTreeWidgetItem", m_indent, m_output, m_refreshOut, m_driver);

   const QList<DomColumn *> columns = w->elementColumn();
   for (int i = 0; i < columns.size(); ++i) {
      const DomColumn *column = columns.at(i);

      const DomPropertyMap properties = propertyMap(column->elementProperty());
      addCommonInitializers(&item, properties, i);
   }

   const QString itemName = item.writeSetupUi(QString(), Item::DontConstruct);
   item.writeRetranslateUi(varName + "->headerItem()");

   if (! itemName.isEmpty()) {
      m_output << m_indent << varName << "->setHeaderItem(" << itemName << ");\n";
   }

   if (w->elementItem().size() == 0) {
      return;
   }

   QString tempName = disableSorting(w, varName);

   QList<Item *> items = initializeTreeWidgetItems(w->elementItem());
   for (int i = 0; i < items.count(); i++) {
      Item *itm = items[i];
      itm->writeSetupUi(varName);
      itm->writeRetranslateUi(varName + "->topLevelItem(" + QString::number(i) + ')');
      delete itm;
   }

   enableSorting(w, varName, tempName);
}

QList<WriteInitialization::Item *> WriteInitialization::initializeTreeWidgetItems(const QList<DomItem *> &domItems)
{
   // items
   QList<Item *> items;

   for (int i = 0; i < domItems.size(); ++i) {
      const DomItem *domItem = domItems.at(i);

      Item *item = new Item("QTreeWidgetItem", m_indent, m_output, m_refreshOut, m_driver);
      items << item;

      QHash<QString, DomProperty *> map;

      int col = -1;
      const DomPropertyList properties = domItem->elementProperty();

      for (int j = 0; j < properties.size(); ++j) {
         DomProperty *p = properties.at(j);

         if (p->attributeName() == "text") {
            if (! map.isEmpty()) {
               addCommonInitializers(item, map, col);
               map.clear();
            }
            col++;
         }
         map.insert(p->attributeName(), p);
      }

      addCommonInitializers(item, map, col);

      // AbstractFromBuilder saves flags last, so they always end up in the last column's map.
      addQtFlagsInitializer(item, map, "flags");

      QList<Item *> subItems = initializeTreeWidgetItems(domItem->elementItem());
      for (Item *subItem : subItems) {
         item->addChild(subItem);
      }
   }

   return items;
}

void WriteInitialization::initializeTableWidget(DomWidget *w)
{
   const QString varName = m_driver->findOrInsertWidget(w);

   // columns
   const QList<DomColumn *> columns = w->elementColumn();

   if (columns.size() != 0) {
      m_output << m_indent << "if (" << varName << "->columnCount() < " << columns.size() << ")\n"
         << m_dindent << varName << "->setColumnCount(" << columns.size() << ");\n";
   }

   for (int i = 0; i < columns.size(); ++i) {
      const DomColumn *column = columns.at(i);
      if (!column->elementProperty().isEmpty()) {
         const DomPropertyMap properties = propertyMap(column->elementProperty());

         Item item("QTableWidgetItem", m_indent, m_output, m_refreshOut, m_driver);
         addCommonInitializers(&item, properties);

         QString itemName = item.writeSetupUi(QString(), Item::ConstructItemAndVariable);
         item.writeRetranslateUi(varName + "->horizontalHeaderItem(" + QString::number(i) + ')');
         m_output << m_indent << varName << "->setHorizontalHeaderItem(" << QString::number(i) << ", " << itemName << ");\n";
      }
   }

   // rows
   const QList<DomRow *> rows = w->elementRow();

   if (rows.size() != 0) {
      m_output << m_indent << "if (" << varName << "->rowCount() < " << rows.size() << ")\n"
               << m_dindent << varName << "->setRowCount(" << rows.size() << ");\n";
   }

   for (int i = 0; i < rows.size(); ++i) {
      const DomRow *row = rows.at(i);
      if (!row->elementProperty().isEmpty()) {
         const DomPropertyMap properties = propertyMap(row->elementProperty());

         Item item("QTableWidgetItem", m_indent, m_output, m_refreshOut, m_driver);
         addCommonInitializers(&item, properties);

         QString itemName = item.writeSetupUi(QString(), Item::ConstructItemAndVariable);
         item.writeRetranslateUi(varName + "->verticalHeaderItem(" + QString::number(i) + ')');
         m_output << m_indent << varName << "->setVerticalHeaderItem(" << QString::number(i) << ", " << itemName << ");\n";
      }
   }

   // items
   QString tempName = disableSorting(w, varName);

   const QList<DomItem *> items = w->elementItem();

   for (int i = 0; i < items.size(); ++i) {
      const DomItem *cell = items.at(i);
      if (cell->hasAttributeRow() && cell->hasAttributeColumn() && !cell->elementProperty().isEmpty()) {
         const int r = cell->attributeRow();
         const int c = cell->attributeColumn();
         const DomPropertyMap properties = propertyMap(cell->elementProperty());

         Item item("QTableWidgetItem", m_indent, m_output, m_refreshOut, m_driver);
         addQtFlagsInitializer(&item, properties, "flags");
         addCommonInitializers(&item, properties);

         QString itemName = item.writeSetupUi(QString(), Item::ConstructItemAndVariable);
         item.writeRetranslateUi(varName + "->item(" + QString::number(r) + ", " + QString::number(c) + ')');

         m_output << m_indent << varName << "->setItem(" << QString::number(r) << ", " << QString::number(
               c) << ", " << itemName << ");\n";
      }
   }
   enableSorting(w, varName, tempName);
}

QString WriteInitialization::trCall(const QString &str, const QString &commentHint) const
{
   if (str.isEmpty()) {
      return QString("QString()");
   }

   QString result;
   const QString comment = commentHint.isEmpty() ? QString("nullptr") : fixString(commentHint, m_dindent);

   if (m_option.translateFunction.isEmpty()) {
      result = "QApplication::translate(\"";
      result += m_generatedClass;
      result += '"';
      result += ", ";

   } else {
      result = m_option.translateFunction;
      result += '(';
   }

   result += fixString(str, m_dindent);
   result += ", ";
   result += comment;

   result += ')';

   return result;
}

void WriteInitialization::initializeMenu(DomWidget *w, const QString &)
{
   const QString menuName = m_driver->findOrInsertWidget(w);
   const QString menuAction = menuName + "Action";

   const DomAction *action = m_driver->actionByName(menuAction);
   if (action && action->hasAttributeMenu()) {
      m_output << m_indent << menuAction << " = " << menuName << "->menuAction();\n";
   }
}

QString WriteInitialization::trCall(DomString *str, const QString &defaultString) const
{
   QString value = defaultString;
   QString comment;

   if (str) {
      value   = toString(str);
      comment = str->attributeComment();
   }

   return trCall(value, comment);
}

QString WriteInitialization::noTrCall(DomString *str, const QString &defaultString) const
{
   QString value = defaultString;

   if (!str && defaultString.isEmpty()) {
      return QString();
   }

   if (str) {
      value = str->text();
   }

   QString ret = "QString::fromUtf8(";
   ret += fixString(value, m_dindent);
   ret += ')';

   return ret;
}

QString WriteInitialization::autoTrCall(DomString *str, const QString &defaultString) const
{
   if ((!str && !defaultString.isEmpty()) || needsTranslation(str)) {
      return trCall(str, defaultString);
   }

   return noTrCall(str, defaultString);
}

QTextStream &WriteInitialization::autoTrOutput(const DomProperty *property)
{
    if (const DomString *str = property->elementString())
        return autoTrOutput(str);
    if (const DomStringList *list = property->elementStringList())
        if (needsTranslation(list))
            return m_refreshOut;
    return m_output;
}

QTextStream &WriteInitialization::autoTrOutput(const DomString *str, const QString &defaultString)
{
   if ((!str && !defaultString.isEmpty()) || needsTranslation(str)) {
      return m_refreshOut;
   }

   return m_output;
}

bool WriteInitialization::isValidObject(const QString &name) const
{
   return m_registeredWidgets.contains(name)
      || m_registeredActions.contains(name);
}

QString WriteInitialization::findDeclaration(const QString &name)
{
   const QString normalized = Driver::normalizedName(name);

   if (DomWidget *widget = m_driver->widgetByName(normalized)) {
      return m_driver->findOrInsertWidget(widget);
   }

   if (DomAction *action = m_driver->actionByName(normalized)) {
      return m_driver->findOrInsertAction(action);
   }

   if (const DomButtonGroup *group = m_driver->findButtonGroup(normalized)) {
      return m_driver->findOrInsertButtonGroup(group);
   }

   return QString();
}

void WriteInitialization::acceptConnection(DomConnection *connection)
{
   const QString sender = findDeclaration(connection->elementSender());
   const QString receiver = findDeclaration(connection->elementReceiver());

   if (sender.isEmpty() || receiver.isEmpty()) {
      return;
   }

   m_output << m_indent << "QObject::connect("
      << sender
      << ", "
      << "SIGNAL(" << connection->elementSignal() << ')'
      << ", "
      << receiver
      << ", "
      << "SLOT(" << connection->elementSlot() << ')'
      << ");\n";
}

DomImage *WriteInitialization::findImage(const QString &name) const
{
   return m_registeredImages.value(name);
}

DomWidget *WriteInitialization::findWidget(const QString &widgetClass)
{
   for (int i = m_widgetChain.count() - 1; i >= 0; --i) {
      DomWidget *widget = m_widgetChain.at(i);

      if (widget && m_uic->customWidgetsInfo()->extends(widget->attributeClass(), widgetClass)) {
         return widget;
      }
   }

   return nullptr;
}

void WriteInitialization::acceptImage(DomImage *image)
{
   if (!image->hasAttributeName()) {
      return;
   }

   m_registeredImages.insert(image->attributeName(), image);
}

void WriteInitialization::acceptWidgetScripts(const DomScripts &widgetScripts, DomWidget *node,
   const  DomWidgets &childWidgets)
{
   // Add the per-class custom scripts to the per-widget ones.
   DomScripts scripts(widgetScripts);

   if (DomScript *customWidgetScript = m_uic->customWidgetsInfo()->customWidgetScript(node->attributeClass())) {
      scripts.push_front(customWidgetScript);
   }

   if (scripts.empty()) {
      return;
   }

   // concatenate script snippets
   QString script;

   for (const DomScript *domScript : scripts) {
      const QString snippet = domScript->text();

      if (!snippet.isEmpty()) {
         script += snippet.trimmed();
         script += '\n';
      }
   }

   if (script.isEmpty()) {
      return;
   }

   // Build the list of children and insert call
   m_output << m_indent << "childWidgets.clear();\n";

   if (! childWidgets.empty()) {
      m_output << m_indent <<  "childWidgets";

      for (DomWidget *child : childWidgets) {
         m_output << " << " << m_driver->findOrInsertWidget(child);
      }

      m_output << ";\n";
   }

   m_output << m_indent << "scriptContext.run(QString::fromUtf8("
      << fixString(script, m_dindent) << "), "
      << m_driver->findOrInsertWidget(node) << ", childWidgets);\n";
}

static void generateMultiDirectiveBegin(QTextStream &outputStream, const QSet<QString> &directives)
{
   if (directives.isEmpty()) {
      return;
   }

   std::set<QString> items;

   for (const QString &str : directives) {
      items.insert(str);
   }

   outputStream << "#if";
   bool moreThanOne = false;

   for (const QString &str : items) {
      if (moreThanOne) {
         outputStream << " ||";
      }

      outputStream << " ! defined(" << str << ')';
      moreThanOne = true;
   }

   outputStream << "\n";
}

static void generateMultiDirectiveEnd(QTextStream &outputStream, const QSet<QString> &directives)
{
   if (directives.isEmpty()) {
      return;
   }

   outputStream << "#endif\n\n";
}

WriteInitialization::Item::Item(const QString &itemClassName, const QString &indent, QTextStream &setupUiStream,
      QTextStream &retranslateUiStream, Driver *driver)
   : m_parent(nullptr), m_itemClassName(itemClassName), m_indent(indent),
     m_setupUiStream(setupUiStream), m_retranslateUiStream(retranslateUiStream), m_driver(driver)
{
}

WriteInitialization::Item::~Item()
{
   for (Item *child : m_children) {
      delete child;
   }
}

QString WriteInitialization::Item::writeSetupUi(const QString &parent, Item::EmptyItemPolicy emptyItemPolicy)
{
   if (emptyItemPolicy == Item::DontConstruct && m_setupUiData.policy == ItemData::DontGenerate) {
      return QString();
   }

   bool generateMultiDirective = false;

   if (emptyItemPolicy == Item::ConstructItemOnly && m_children.size() == 0) {

      if (m_setupUiData.policy == ItemData::DontGenerate) {
         m_setupUiStream << m_indent << "new " << m_itemClassName << '(' << parent << ");\n";
         return QString();

      } else if (m_setupUiData.policy == ItemData::GenerateWithMultiDirective) {
         generateMultiDirective = true;
      }
   }

   if (generateMultiDirective) {
      generateMultiDirectiveBegin(m_setupUiStream, m_setupUiData.directives);
   }

   const QString uniqueName = m_driver->unique("__" + m_itemClassName.toLower());
   m_setupUiStream << m_indent << m_itemClassName << " *" << uniqueName << " = new "
                   << m_itemClassName << '(' << parent << ");\n";

   if (generateMultiDirective) {
      m_setupUiStream << "#else\n";
      m_setupUiStream << m_indent << "new " << m_itemClassName << '(' << parent << ");\n";
      generateMultiDirectiveEnd(m_setupUiStream, m_setupUiData.directives);
   }

   QMultiMap<QString, QString>::const_iterator it = m_setupUiData.setters.constBegin();

   while (it != m_setupUiData.setters.constEnd()) {
      openIfndef(m_setupUiStream, it.key());
      m_setupUiStream << m_indent << uniqueName << it.value() << "\n";
      closeIfndef(m_setupUiStream, it.key());
      ++it;
   }

   for (Item *child : m_children) {
      child->writeSetupUi(uniqueName);
   }

   return uniqueName;
}

void WriteInitialization::Item::writeRetranslateUi(const QString &parentPath)
{
   if (m_retranslateUiData.policy == ItemData::DontGenerate) {
      return;
   }

   if (m_retranslateUiData.policy == ItemData::GenerateWithMultiDirective) {
      generateMultiDirectiveBegin(m_retranslateUiStream, m_retranslateUiData.directives);
   }

   const QString uniqueName = m_driver->unique("___" + m_itemClassName.toLower());
   m_retranslateUiStream << m_indent << m_itemClassName << " *" << uniqueName << " = " << parentPath << ";\n";

   if (m_retranslateUiData.policy == ItemData::GenerateWithMultiDirective) {
      generateMultiDirectiveEnd(m_retranslateUiStream, m_retranslateUiData.directives);
   }

   QString oldDirective;
   QMultiMap<QString, QString>::const_iterator it = m_retranslateUiData.setters.constBegin();

   while (it != m_retranslateUiData.setters.constEnd()) {
      const QString newDirective = it.key();

      if (oldDirective != newDirective) {
         closeIfndef(m_retranslateUiStream, oldDirective);
         openIfndef(m_retranslateUiStream, newDirective);
         oldDirective = newDirective;
      }

      m_retranslateUiStream << m_indent << uniqueName << it.value() << "\n";
      ++it;
   }

   closeIfndef(m_retranslateUiStream, oldDirective);

   for (int i = 0; i < m_children.size(); i++) {
      m_children[i]->writeRetranslateUi(uniqueName + "->child(" + QString::number(i) + ')');
   }
}

void WriteInitialization::Item::addSetter(const QString &setter, const QString &directive, bool translatable)
{
   const ItemData::TemporaryVariableGeneratorPolicy newPolicy = directive.isEmpty() ?
         ItemData::Generate : ItemData::GenerateWithMultiDirective;

   if (translatable) {
      m_retranslateUiData.setters.insert(directive, setter);

      if (ItemData::GenerateWithMultiDirective == newPolicy) {
         m_retranslateUiData.directives << directive;
      }

      if (m_retranslateUiData.policy < newPolicy) {
         m_retranslateUiData.policy = newPolicy;
      }

   } else {
      m_setupUiData.setters.insert(directive, setter);

      if (ItemData::GenerateWithMultiDirective == newPolicy) {
         m_setupUiData.directives << directive;
      }

      if (m_setupUiData.policy < newPolicy) {
         m_setupUiData.policy = newPolicy;
      }
   }
}

void WriteInitialization::Item::addChild(Item *child)
{
   m_children << child;
   child->m_parent = this;

   Item *c = child;
   Item *p = this;

   while (p) {
      p->m_setupUiData.directives |= c->m_setupUiData.directives;
      p->m_retranslateUiData.directives |= c->m_retranslateUiData.directives;

      if (p->m_setupUiData.policy < c->m_setupUiData.policy) {
         p->m_setupUiData.policy = c->m_setupUiData.policy;
      }

      if (p->m_retranslateUiData.policy < c->m_retranslateUiData.policy) {
         p->m_retranslateUiData.policy = c->m_retranslateUiData.policy;
      }

      c = p;
      p = p->m_parent;
   }
}

} // namespace CPP
