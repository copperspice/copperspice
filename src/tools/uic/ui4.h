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

#ifndef UI4_H
#define UI4_H

#include <qglobal.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qxmlstreamreader.h>
#include <qxmlstreamwriter.h>

#define QDESIGNER_UILIB_EXTERN Q_DECL_EXPORT
#define QDESIGNER_UILIB_IMPORT Q_DECL_IMPORT

#if defined(QT_DESIGNER_STATIC) || defined(QT_UIC)
#   define QDESIGNER_UILIB_EXPORT

#elif defined(QDESIGNER_UILIB_LIBRARY)
#   define QDESIGNER_UILIB_EXPORT QDESIGNER_UILIB_EXTERN

#else
#   define QDESIGNER_UILIB_EXPORT QDESIGNER_UILIB_IMPORT

#endif

#ifndef QDESIGNER_UILIB_EXPORT
#   define QDESIGNER_UILIB_EXPORT
#endif

class DomUI;
class DomIncludes;
class DomInclude;
class DomResources;
class DomResource;
class DomActionGroup;
class DomAction;
class DomActionRef;
class DomButtonGroup;
class DomButtonGroups;
class DomImages;
class DomImage;
class DomImageData;
class DomCustomWidgets;
class DomHeader;
class DomCustomWidget;
class DomProperties;
class DomPropertyData;
class DomSizePolicyData;
class DomLayoutDefault;
class DomLayoutFunction;
class DomTabStops;
class DomLayout;
class DomLayoutItem;
class DomRow;
class DomColumn;
class DomItem;
class DomWidget;
class DomSpacer;
class DomColor;
class DomGradientStop;
class DomGradient;
class DomBrush;
class DomColorRole;
class DomColorGroup;
class DomPalette;
class DomFont;
class DomPoint;
class DomRect;
class DomLocale;
class DomSizePolicy;
class DomSize;
class DomDate;
class DomTime;
class DomDateTime;
class DomStringList;
class DomResourcePixmap;
class DomResourceIcon;
class DomString;
class DomPointF;
class DomRectF;
class DomSizeF;
class DomChar;
class DomUrl;
class DomProperty;
class DomConnections;
class DomConnection;
class DomConnectionHints;
class DomConnectionHint;
class DomScript;
class DomWidgetData;
class DomDesignerData;
class DomSlots;
class DomPropertySpecifications;
class DomPropertyToolTip;
class DomStringPropertySpecification;

class QDESIGNER_UILIB_EXPORT DomUI
{
 public:
   DomUI();
   ~DomUI();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeVersion() const {
      return m_has_attr_version;
   }

   QString attributeVersion() const {
      return m_attr_version;
   }

   void setAttributeVersion(const QString &a) {
      m_attr_version = a;
      m_has_attr_version = true;
   }

   void clearAttributeVersion() {
      m_has_attr_version = false;
   }

   bool hasAttributeLanguage() const {
      return m_has_attr_language;
   }

   QString attributeLanguage() const {
      return m_attr_language;
   }

   void setAttributeLanguage(const QString &a) {
      m_attr_language = a;
      m_has_attr_language = true;
   }

   void clearAttributeLanguage() {
      m_has_attr_language = false;
   }

   bool hasAttributeDisplayname() const {
      return m_has_attr_displayname;
   }

   QString attributeDisplayname() const {
      return m_attr_displayname;
   }

   void setAttributeDisplayname(const QString &a) {
      m_attr_displayname = a;
      m_has_attr_displayname = true;
   }

   void clearAttributeDisplayname() {
      m_has_attr_displayname = false;
   }

   bool hasAttributeStdsetdef() const {
      return m_has_attr_stdsetdef;
   }

   int attributeStdsetdef() const {
      return m_attr_stdsetdef;
   }

   void setAttributeStdsetdef(int a) {
      m_attr_stdsetdef = a;
      m_has_attr_stdsetdef = true;
   }

   void clearAttributeStdsetdef() {
      m_has_attr_stdsetdef = false;
   }

   bool hasAttributeStdSetDef() const {
      return m_has_attr_stdSetDef;
   }

   int attributeStdSetDef() const {
      return m_attr_stdSetDef;
   }

   void setAttributeStdSetDef(int a) {
      m_attr_stdSetDef = a;
      m_has_attr_stdSetDef = true;
   }

   void clearAttributeStdSetDef() {
      m_has_attr_stdSetDef = false;
   }

   // child element accessors
   QString elementAuthor() const {
      return m_author;
   }

   void setElementAuthor(const QString &a);

   bool hasElementAuthor() const {
      return m_children & Author;
   }

   void clearElementAuthor();

   QString elementComment() const {
      return m_comment;
   }

   void setElementComment(const QString &a);

   bool hasElementComment() const {
      return m_children & Comment;
   }
   void clearElementComment();

   QString elementExportMacro() const {
      return m_exportMacro;
   }

   void setElementExportMacro(const QString &a);

   bool hasElementExportMacro() const {
      return m_children & ExportMacro;
   }

   void clearElementExportMacro();

   QString elementClass() const {
      return m_class;
   }
   void setElementClass(const QString &a);
   bool hasElementClass() const {
      return m_children & Class;
   }
   void clearElementClass();

   DomWidget *elementWidget() const {
      return m_widget;
   }
   DomWidget *takeElementWidget();
   void setElementWidget(DomWidget *a);
   bool hasElementWidget() const {
      return m_children & Widget;
   }
   void clearElementWidget();

   DomLayoutDefault *elementLayoutDefault() const {
      return m_layoutDefault;
   }
   DomLayoutDefault *takeElementLayoutDefault();
   void setElementLayoutDefault(DomLayoutDefault *a);
   bool hasElementLayoutDefault() const {
      return m_children & LayoutDefault;
   }
   void clearElementLayoutDefault();

   DomLayoutFunction *elementLayoutFunction() const {
      return m_layoutFunction;
   }
   DomLayoutFunction *takeElementLayoutFunction();
   void setElementLayoutFunction(DomLayoutFunction *a);
   bool hasElementLayoutFunction() const {
      return m_children & LayoutFunction;
   }
   void clearElementLayoutFunction();

   QString elementPixmapFunction() const {
      return m_pixmapFunction;
   }
   void setElementPixmapFunction(const QString &a);
   bool hasElementPixmapFunction() const {
      return m_children & PixmapFunction;
   }
   void clearElementPixmapFunction();

   DomCustomWidgets *elementCustomWidgets() const {
      return m_customWidgets;
   }
   DomCustomWidgets *takeElementCustomWidgets();
   void setElementCustomWidgets(DomCustomWidgets *a);
   bool hasElementCustomWidgets() const {
      return m_children & CustomWidgets;
   }
   void clearElementCustomWidgets();

   DomTabStops *elementTabStops() const {
      return m_tabStops;
   }
   DomTabStops *takeElementTabStops();
   void setElementTabStops(DomTabStops *a);
   bool hasElementTabStops() const {
      return m_children & TabStops;
   }
   void clearElementTabStops();

   DomImages *elementImages() const {
      return m_images;
   }
   DomImages *takeElementImages();
   void setElementImages(DomImages *a);
   bool hasElementImages() const {
      return m_children & Images;
   }
   void clearElementImages();

   DomIncludes *elementIncludes() const {
      return m_includes;
   }
   DomIncludes *takeElementIncludes();
   void setElementIncludes(DomIncludes *a);
   bool hasElementIncludes() const {
      return m_children & Includes;
   }
   void clearElementIncludes();

   DomResources *elementResources() const {
      return m_resources;
   }
   DomResources *takeElementResources();
   void setElementResources(DomResources *a);
   bool hasElementResources() const {
      return m_children & Resources;
   }
   void clearElementResources();

   DomConnections *elementConnections() const {
      return m_connections;
   }
   DomConnections *takeElementConnections();
   void setElementConnections(DomConnections *a);
   bool hasElementConnections() const {
      return m_children & Connections;
   }
   void clearElementConnections();

   DomDesignerData *elementDesignerdata() const {
      return m_designerdata;
   }
   DomDesignerData *takeElementDesignerdata();
   void setElementDesignerdata(DomDesignerData *a);
   bool hasElementDesignerdata() const {
      return m_children & Designerdata;
   }
   void clearElementDesignerdata();

   DomSlots *elementSlots() const {
      return m_slots;
   }

   DomSlots *takeElementSlots();
   void setElementSlots(DomSlots *a);

   bool hasElementSlots() const {
      return m_children & Slots;
   }

   void clearElementSlots();

   DomButtonGroups *elementButtonGroups() const {
      return m_buttonGroups;
   }

   DomButtonGroups *takeElementButtonGroups();

   void setElementButtonGroups(DomButtonGroups *a);

   bool hasElementButtonGroups() const {
      return m_children & ButtonGroups;
   }

   void clearElementButtonGroups();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_version;
   bool m_has_attr_version;

   QString m_attr_language;
   bool m_has_attr_language;

   QString m_attr_displayname;
   bool m_has_attr_displayname;

   int m_attr_stdsetdef;
   bool m_has_attr_stdsetdef;

   int m_attr_stdSetDef;
   bool m_has_attr_stdSetDef;

   // child element data
   uint m_children;
   QString m_author;
   QString m_comment;
   QString m_exportMacro;
   QString m_class;
   DomWidget *m_widget;
   DomLayoutDefault *m_layoutDefault;
   DomLayoutFunction *m_layoutFunction;
   QString m_pixmapFunction;
   DomCustomWidgets *m_customWidgets;
   DomTabStops *m_tabStops;
   DomImages *m_images;
   DomIncludes *m_includes;
   DomResources *m_resources;
   DomConnections *m_connections;
   DomDesignerData *m_designerdata;
   DomSlots *m_slots;
   DomButtonGroups *m_buttonGroups;

   enum Child {
      Author = 1,
      Comment = 2,
      ExportMacro = 4,
      Class = 8,
      Widget = 16,
      LayoutDefault = 32,
      LayoutFunction = 64,
      PixmapFunction = 128,
      CustomWidgets = 256,
      TabStops = 512,
      Images = 1024,
      Includes = 2048,
      Resources = 4096,
      Connections = 8192,
      Designerdata = 16384,
      Slots = 32768,
      ButtonGroups = 65536
   };

   DomUI(const DomUI &other);
   void operator = (const DomUI &other);
};

class QDESIGNER_UILIB_EXPORT DomIncludes
{
 public:
   DomIncludes();
   ~DomIncludes();

   void read(QXmlStreamReader &reader);

   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomInclude *> elementInclude() const {
      return m_include;
   }
   void setElementInclude(const QList<DomInclude *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomInclude *> m_include;
   enum Child {
      Include = 1
   };

   DomIncludes(const DomIncludes &other);
   void operator = (const DomIncludes &other);
};

class QDESIGNER_UILIB_EXPORT DomInclude
{
 public:
   DomInclude();
   ~DomInclude();

   void read(QXmlStreamReader &reader);

   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeLocation() const {
      return m_has_attr_location;
   }

   QString attributeLocation() const {
      return m_attr_location;
   }

   void setAttributeLocation(const QString &a) {
      m_attr_location = a;
      m_has_attr_location = true;
   }

   void clearAttributeLocation() {
      m_has_attr_location = false;
   }

   bool hasAttributeImpldecl() const {
      return m_has_attr_impldecl;
   }

   QString attributeImpldecl() const {
      return m_attr_impldecl;
   }

   void setAttributeImpldecl(const QString &a) {
      m_attr_impldecl = a;
      m_has_attr_impldecl = true;
   }

   void clearAttributeImpldecl() {
      m_has_attr_impldecl = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_location;
   bool m_has_attr_location;

   QString m_attr_impldecl;
   bool m_has_attr_impldecl;

   // child element data
   uint m_children;

   DomInclude(const DomInclude &other);
   void operator = (const DomInclude &other);
};

class QDESIGNER_UILIB_EXPORT DomResources
{
 public:
   DomResources();
   ~DomResources();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   // child element accessors
   QList<DomResource *> elementInclude() const {
      return m_include;
   }
   void setElementInclude(const QList<DomResource *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   // child element data
   uint m_children;
   QList<DomResource *> m_include;

   enum Child {
      Include = 1
   };

   DomResources(const DomResources &other);
   void operator = (const DomResources &other);
};

class QDESIGNER_UILIB_EXPORT DomResource
{
 public:
   DomResource();
   ~DomResource();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeLocation() const {
      return m_has_attr_location;
   }

   QString attributeLocation() const {
      return m_attr_location;
   }

   void setAttributeLocation(const QString &a) {
      m_attr_location = a;
      m_has_attr_location = true;
   }

   void clearAttributeLocation() {
      m_has_attr_location = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_location;
   bool m_has_attr_location;

   // child element data
   uint m_children;

   DomResource(const DomResource &other);
   void operator = (const DomResource &other);
};

class QDESIGNER_UILIB_EXPORT DomActionGroup
{
 public:
   DomActionGroup();
   ~DomActionGroup();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   // child element accessors
   QList<DomAction *> elementAction() const {
      return m_action;
   }
   void setElementAction(const QList<DomAction *> &a);

   QList<DomActionGroup *> elementActionGroup() const {
      return m_actionGroup;
   }

   void setElementActionGroup(const QList<DomActionGroup *> &a);

   QList<DomProperty *> elementProperty() const {
      return m_property;
   }

   void setElementProperty(const QList<DomProperty *> &a);

   QList<DomProperty *> elementAttribute() const {
      return m_attribute;
   }

   void setElementAttribute(const QList<DomProperty *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   // child element data
   uint m_children;
   QList<DomAction *> m_action;
   QList<DomActionGroup *> m_actionGroup;
   QList<DomProperty *> m_property;
   QList<DomProperty *> m_attribute;

   enum Child {
      Action = 1,
      ActionGroup = 2,
      Property = 4,
      Attribute = 8
   };

   DomActionGroup(const DomActionGroup &other);
   void operator = (const DomActionGroup &other);
};

class QDESIGNER_UILIB_EXPORT DomAction
{
 public:
   DomAction();
   ~DomAction();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   bool hasAttributeMenu() const {
      return m_has_attr_menu;
   }

   QString attributeMenu() const {
      return m_attr_menu;
   }

   void setAttributeMenu(const QString &a) {
      m_attr_menu = a;
      m_has_attr_menu = true;
   }

   void clearAttributeMenu() {
      m_has_attr_menu = false;
   }

   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }
   void setElementProperty(const QList<DomProperty *> &a);

   QList<DomProperty *> elementAttribute() const {
      return m_attribute;
   }

   void setElementAttribute(const QList<DomProperty *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   QString m_attr_menu;
   bool m_has_attr_menu;

   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   QList<DomProperty *> m_attribute;
   enum Child {
      Property = 1,
      Attribute = 2
   };

   DomAction(const DomAction &other);
   void operator = (const DomAction &other);
};

class QDESIGNER_UILIB_EXPORT DomActionRef
{
 public:
   DomActionRef();
   ~DomActionRef();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   // child element data
   uint m_children;

   DomActionRef(const DomActionRef &other);
   void operator = (const DomActionRef &other);
};

class QDESIGNER_UILIB_EXPORT DomButtonGroup
{
 public:
   DomButtonGroup();
   ~DomButtonGroup();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }

   void setElementProperty(const QList<DomProperty *> &a);

   QList<DomProperty *> elementAttribute() const {
      return m_attribute;
   }

   void setElementAttribute(const QList<DomProperty *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   QList<DomProperty *> m_attribute;
   enum Child {
      Property = 1,
      Attribute = 2
   };

   DomButtonGroup(const DomButtonGroup &other);
   void operator = (const DomButtonGroup &other);
};

class QDESIGNER_UILIB_EXPORT DomButtonGroups
{
 public:
   DomButtonGroups();
   ~DomButtonGroups();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomButtonGroup *> elementButtonGroup() const {
      return m_buttonGroup;
   }

   void setElementButtonGroup(const QList<DomButtonGroup *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomButtonGroup *> m_buttonGroup;

   enum Child {
      ButtonGroup = 1
   };

   DomButtonGroups(const DomButtonGroups &other);
   void operator = (const DomButtonGroups &other);
};

class QDESIGNER_UILIB_EXPORT DomImages
{
 public:
   DomImages();
   ~DomImages();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomImage *> elementImage() const {
      return m_image;
   }

   void setElementImage(const QList<DomImage *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomImage *> m_image;

   enum Child {
      Image = 1
   };

   DomImages(const DomImages &other);
   void operator = (const DomImages &other);
};

class QDESIGNER_UILIB_EXPORT DomImage
{
 public:
   DomImage();
   ~DomImage();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   // child element accessors
   DomImageData *elementData() const {
      return m_data;
   }

   DomImageData *takeElementData();
   void setElementData(DomImageData *a);

   bool hasElementData() const {
      return m_children & Data;
   }

   void clearElementData();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   // child element data
   uint m_children;
   DomImageData *m_data;
   enum Child {
      Data = 1
   };

   DomImage(const DomImage &other);
   void operator = (const DomImage &other);
};

class QDESIGNER_UILIB_EXPORT DomImageData
{
 public:
   DomImageData();
   ~DomImageData();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeFormat() const {
      return m_has_attr_format;
   }

   QString attributeFormat() const {
      return m_attr_format;
   }

   void setAttributeFormat(const QString &a) {
      m_attr_format = a;
      m_has_attr_format = true;
   }

   void clearAttributeFormat() {
      m_has_attr_format = false;
   }

   bool hasAttributeLength() const {
      return m_has_attr_length;
   }

   int attributeLength() const {
      return m_attr_length;
   }

   void setAttributeLength(int a) {
      m_attr_length = a;
      m_has_attr_length = true;
   }

   void clearAttributeLength() {
      m_has_attr_length = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_format;
   bool m_has_attr_format;

   int m_attr_length;
   bool m_has_attr_length;

   // child element data
   uint m_children;

   DomImageData(const DomImageData &other);
   void operator = (const DomImageData &other);
};

class QDESIGNER_UILIB_EXPORT DomCustomWidgets
{
 public:
   DomCustomWidgets();
   ~DomCustomWidgets();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomCustomWidget *> elementCustomWidget() const {
      return m_customWidget;
   }

   void setElementCustomWidget(const QList<DomCustomWidget *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomCustomWidget *> m_customWidget;

   enum Child {
      CustomWidget = 1
   };

   DomCustomWidgets(const DomCustomWidgets &other);
   void operator = (const DomCustomWidgets &other);
};

class QDESIGNER_UILIB_EXPORT DomHeader
{
 public:
   DomHeader();
   ~DomHeader();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeLocation() const {
      return m_has_attr_location;
   }

   QString attributeLocation() const {
      return m_attr_location;
   }

   void setAttributeLocation(const QString &a) {
      m_attr_location = a;
      m_has_attr_location = true;
   }

   void clearAttributeLocation() {
      m_has_attr_location = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_location;
   bool m_has_attr_location;

   // child element data
   uint m_children;

   DomHeader(const DomHeader &other);
   void operator = (const DomHeader &other);
};

class QDESIGNER_UILIB_EXPORT DomCustomWidget
{
 public:
   DomCustomWidget();
   ~DomCustomWidget();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QString elementClass() const {
      return m_class;
   }

   void setElementClass(const QString &a);

   bool hasElementClass() const {
      return m_children & Class;
   }

   void clearElementClass();

   QString elementExtends() const {
      return m_extends;
   }

   void setElementExtends(const QString &a);

   bool hasElementExtends() const {
      return m_children & Extends;
   }
   void clearElementExtends();

   DomHeader *elementHeader() const {
      return m_header;
   }

   DomHeader *takeElementHeader();
   void setElementHeader(DomHeader *a);

   bool hasElementHeader() const {
      return m_children & Header;
   }
   void clearElementHeader();

   DomSize *elementSizeHint() const {
      return m_sizeHint;
   }

   DomSize *takeElementSizeHint();
   void setElementSizeHint(DomSize *a);

   bool hasElementSizeHint() const {
      return m_children & SizeHint;
   }
   void clearElementSizeHint();

   QString elementAddPageMethod() const {
      return m_addPageMethod;
   }

   void setElementAddPageMethod(const QString &a);
   bool hasElementAddPageMethod() const {
      return m_children & AddPageMethod;
   }

   void clearElementAddPageMethod();

   int elementContainer() const {
      return m_container;
   }

   void setElementContainer(int a);
   bool hasElementContainer() const {
      return m_children & Container;
   }
   void clearElementContainer();

   DomSizePolicyData *elementSizePolicy() const {
      return m_sizePolicy;
   }

   DomSizePolicyData *takeElementSizePolicy();
   void setElementSizePolicy(DomSizePolicyData *a);

   bool hasElementSizePolicy() const {
      return m_children & SizePolicy;
   }
   void clearElementSizePolicy();

   QString elementPixmap() const {
      return m_pixmap;
   }

   void setElementPixmap(const QString &a);
   bool hasElementPixmap() const {
      return m_children & Pixmap;
   }
   void clearElementPixmap();

   DomScript *elementScript() const {
      return m_script;
   }

   DomScript *takeElementScript();
   void setElementScript(DomScript *a);
   bool hasElementScript() const {
      return m_children & Script;
   }
   void clearElementScript();

   DomProperties *elementProperties() const {
      return m_properties;
   }

   DomProperties *takeElementProperties();
   void setElementProperties(DomProperties *a);
   bool hasElementProperties() const {
      return m_children & Properties;
   }
   void clearElementProperties();

   DomSlots *elementSlots() const {
      return m_slots;
   }

   DomSlots *takeElementSlots();
   void setElementSlots(DomSlots *a);
   bool hasElementSlots() const {
      return m_children & Slots;
   }
   void clearElementSlots();

   DomPropertySpecifications *elementPropertyspecifications() const {
      return m_propertyspecifications;
   }

   DomPropertySpecifications *takeElementPropertyspecifications();
   void setElementPropertyspecifications(DomPropertySpecifications *a);

   bool hasElementPropertyspecifications() const {
      return m_children & Propertyspecifications;
   }

   void clearElementPropertyspecifications();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QString m_class;
   QString m_extends;
   DomHeader *m_header;
   DomSize *m_sizeHint;
   QString m_addPageMethod;
   int m_container;
   DomSizePolicyData *m_sizePolicy;
   QString m_pixmap;
   DomScript *m_script;
   DomProperties *m_properties;
   DomSlots *m_slots;
   DomPropertySpecifications *m_propertyspecifications;

   enum Child {
      Class = 1,
      Extends = 2,
      Header = 4,
      SizeHint = 8,
      AddPageMethod = 16,
      Container = 32,
      SizePolicy = 64,
      Pixmap = 128,
      Script = 256,
      Properties = 512,
      Slots = 1024,
      Propertyspecifications = 2048
   };

   DomCustomWidget(const DomCustomWidget &other);
   void operator = (const DomCustomWidget &other);
};

class QDESIGNER_UILIB_EXPORT DomProperties
{
 public:
   DomProperties();
   ~DomProperties();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomPropertyData *> elementProperty() const {
      return m_property;
   }

   void setElementProperty(const QList<DomPropertyData *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomPropertyData *> m_property;

   enum Child {
      Property = 1
   };

   DomProperties(const DomProperties &other);
   void operator = (const DomProperties &other);
};

class QDESIGNER_UILIB_EXPORT DomPropertyData
{
 public:
   DomPropertyData();
   ~DomPropertyData();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeType() const {
      return m_has_attr_type;
   }

   QString attributeType() const {
      return m_attr_type;
   }

   void setAttributeType(const QString &a) {
      m_attr_type = a;
      m_has_attr_type = true;
   }

   void clearAttributeType() {
      m_has_attr_type = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_type;
   bool m_has_attr_type;

   // child element data
   uint m_children;

   DomPropertyData(const DomPropertyData &other);
   void operator = (const DomPropertyData &other);
};

class QDESIGNER_UILIB_EXPORT DomSizePolicyData
{
 public:
   DomSizePolicyData();
   ~DomSizePolicyData();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   int elementHorData() const {
      return m_horData;
   }

   void setElementHorData(int a);
   bool hasElementHorData() const {
      return m_children & HorData;
   }

   void clearElementHorData();

   int elementVerData() const {
      return m_verData;
   }

   void setElementVerData(int a);
   bool hasElementVerData() const {
      return m_children & VerData;
   }
   void clearElementVerData();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   int m_horData;
   int m_verData;
   enum Child {
      HorData = 1,
      VerData = 2
   };

   DomSizePolicyData(const DomSizePolicyData &other);
   void operator = (const DomSizePolicyData &other);
};

class QDESIGNER_UILIB_EXPORT DomLayoutDefault
{
 public:
   DomLayoutDefault();
   ~DomLayoutDefault();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeSpacing() const {
      return m_has_attr_spacing;
   }

   int attributeSpacing() const {
      return m_attr_spacing;
   }

   void setAttributeSpacing(int a) {
      m_attr_spacing = a;
      m_has_attr_spacing = true;
   }

   void clearAttributeSpacing() {
      m_has_attr_spacing = false;
   }

   bool hasAttributeMargin() const {
      return m_has_attr_margin;
   }
   int attributeMargin() const {
      return m_attr_margin;
   }
   void setAttributeMargin(int a) {
      m_attr_margin = a;
      m_has_attr_margin = true;
   }
   void clearAttributeMargin() {
      m_has_attr_margin = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   int m_attr_spacing;
   bool m_has_attr_spacing;

   int m_attr_margin;
   bool m_has_attr_margin;

   // child element data
   uint m_children;

   DomLayoutDefault(const DomLayoutDefault &other);
   void operator = (const DomLayoutDefault &other);
};

class QDESIGNER_UILIB_EXPORT DomLayoutFunction
{
 public:
   DomLayoutFunction();
   ~DomLayoutFunction();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeSpacing() const {
      return m_has_attr_spacing;
   }

   QString attributeSpacing() const {
      return m_attr_spacing;
   }

   void setAttributeSpacing(const QString &a) {
      m_attr_spacing = a;
      m_has_attr_spacing = true;
   }

   void clearAttributeSpacing() {
      m_has_attr_spacing = false;
   }

   bool hasAttributeMargin() const {
      return m_has_attr_margin;
   }

   QString attributeMargin() const {
      return m_attr_margin;
   }

   void setAttributeMargin(const QString &a) {
      m_attr_margin = a;
      m_has_attr_margin = true;
   }

   void clearAttributeMargin() {
      m_has_attr_margin = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_spacing;
   bool m_has_attr_spacing;

   QString m_attr_margin;
   bool m_has_attr_margin;

   // child element data
   uint m_children;

   DomLayoutFunction(const DomLayoutFunction &other);
   void operator = (const DomLayoutFunction &other);
};

class QDESIGNER_UILIB_EXPORT DomTabStops
{
 public:
   DomTabStops();
   ~DomTabStops();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QStringList elementTabStop() const {
      return m_tabStop;
   }

   void setElementTabStop(const QStringList &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QStringList m_tabStop;
   enum Child {
      TabStop = 1
   };

   DomTabStops(const DomTabStops &other);
   void operator = (const DomTabStops &other);
};

class QDESIGNER_UILIB_EXPORT DomLayout
{
 public:
   DomLayout();
   ~DomLayout();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeClass() const {
      return m_has_attr_class;
   }

   QString attributeClass() const {
      return m_attr_class;
   }

   void setAttributeClass(const QString &a) {
      m_attr_class = a;
      m_has_attr_class = true;
   }

   void clearAttributeClass() {
      m_has_attr_class = false;
   }

   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   bool hasAttributeStretch() const {
      return m_has_attr_stretch;
   }

   QString attributeStretch() const {
      return m_attr_stretch;
   }

   void setAttributeStretch(const QString &a) {
      m_attr_stretch = a;
      m_has_attr_stretch = true;
   }

   void clearAttributeStretch() {
      m_has_attr_stretch = false;
   }

   bool hasAttributeRowStretch() const {
      return m_has_attr_rowStretch;
   }

   QString attributeRowStretch() const {
      return m_attr_rowStretch;
   }

   void setAttributeRowStretch(const QString &a) {
      m_attr_rowStretch = a;
      m_has_attr_rowStretch = true;
   }

   void clearAttributeRowStretch() {
      m_has_attr_rowStretch = false;
   }

   bool hasAttributeColumnStretch() const {
      return m_has_attr_columnStretch;
   }

   QString attributeColumnStretch() const {
      return m_attr_columnStretch;
   }

   void setAttributeColumnStretch(const QString &a) {
      m_attr_columnStretch = a;
      m_has_attr_columnStretch = true;
   }

   void clearAttributeColumnStretch() {
      m_has_attr_columnStretch = false;
   }

   bool hasAttributeRowMinimumHeight() const {
      return m_has_attr_rowMinimumHeight;
   }

   QString attributeRowMinimumHeight() const {
      return m_attr_rowMinimumHeight;
   }

   void setAttributeRowMinimumHeight(const QString &a) {
      m_attr_rowMinimumHeight = a;
      m_has_attr_rowMinimumHeight = true;
   }

   void clearAttributeRowMinimumHeight() {
      m_has_attr_rowMinimumHeight = false;
   }

   bool hasAttributeColumnMinimumWidth() const {
      return m_has_attr_columnMinimumWidth;
   }

   QString attributeColumnMinimumWidth() const {
      return m_attr_columnMinimumWidth;
   }

   void setAttributeColumnMinimumWidth(const QString &a) {
      m_attr_columnMinimumWidth = a;
      m_has_attr_columnMinimumWidth = true;
   }

   void clearAttributeColumnMinimumWidth() {
      m_has_attr_columnMinimumWidth = false;
   }

   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }
   void setElementProperty(const QList<DomProperty *> &a);

   QList<DomProperty *> elementAttribute() const {
      return m_attribute;
   }
   void setElementAttribute(const QList<DomProperty *> &a);

   QList<DomLayoutItem *> elementItem() const {
      return m_item;
   }
   void setElementItem(const QList<DomLayoutItem *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_class;
   bool m_has_attr_class;

   QString m_attr_name;
   bool m_has_attr_name;

   QString m_attr_stretch;
   bool m_has_attr_stretch;

   QString m_attr_rowStretch;
   bool m_has_attr_rowStretch;

   QString m_attr_columnStretch;
   bool m_has_attr_columnStretch;

   QString m_attr_rowMinimumHeight;
   bool m_has_attr_rowMinimumHeight;

   QString m_attr_columnMinimumWidth;
   bool m_has_attr_columnMinimumWidth;

   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   QList<DomProperty *> m_attribute;
   QList<DomLayoutItem *> m_item;

   enum Child {
      Property = 1,
      Attribute = 2,
      Item = 4
   };

   DomLayout(const DomLayout &other);
   void operator = (const DomLayout &other);
};

class QDESIGNER_UILIB_EXPORT DomLayoutItem
{
 public:
   DomLayoutItem();
   ~DomLayoutItem();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeRow() const {
      return m_has_attr_row;
   }

   int attributeRow() const {
      return m_attr_row;
   }

   void setAttributeRow(int a) {
      m_attr_row = a;
      m_has_attr_row = true;
   }

   void clearAttributeRow() {
      m_has_attr_row = false;
   }

   bool hasAttributeColumn() const {
      return m_has_attr_column;
   }

   int attributeColumn() const {
      return m_attr_column;
   }

   void setAttributeColumn(int a) {
      m_attr_column = a;
      m_has_attr_column = true;
   }

   void clearAttributeColumn() {
      m_has_attr_column = false;
   }

   bool hasAttributeRowSpan() const {
      return m_has_attr_rowSpan;
   }

   int attributeRowSpan() const {
      return m_attr_rowSpan;
   }

   void setAttributeRowSpan(int a) {
      m_attr_rowSpan = a;
      m_has_attr_rowSpan = true;
   }

   void clearAttributeRowSpan() {
      m_has_attr_rowSpan = false;
   }

   bool hasAttributeColSpan() const {
      return m_has_attr_colSpan;
   }

   int attributeColSpan() const {
      return m_attr_colSpan;
   }

   void setAttributeColSpan(int a) {
      m_attr_colSpan = a;
      m_has_attr_colSpan = true;
   }

   void clearAttributeColSpan() {
      m_has_attr_colSpan = false;
   }

   bool hasAttributeAlignment() const {
      return m_has_attr_alignment;
   }

   QString attributeAlignment() const {
      return m_attr_alignment;
   }

   void setAttributeAlignment(const QString &a) {
      m_attr_alignment = a;
      m_has_attr_alignment = true;
   }

   void clearAttributeAlignment() {
      m_has_attr_alignment = false;
   }

   // child element accessors
   enum Kind { Unknown = 0, Widget, Layout, Spacer };
   Kind kind() const {
      return m_kind;
   }

   DomWidget *elementWidget() const {
      return m_widget;
   }

   DomWidget *takeElementWidget();
   void setElementWidget(DomWidget *a);

   DomLayout *elementLayout() const {
      return m_layout;
   }

   DomLayout *takeElementLayout();
   void setElementLayout(DomLayout *a);

   DomSpacer *elementSpacer() const {
      return m_spacer;
   }

   DomSpacer *takeElementSpacer();
   void setElementSpacer(DomSpacer *a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   int m_attr_row;
   bool m_has_attr_row;

   int m_attr_column;
   bool m_has_attr_column;

   int m_attr_rowSpan;
   bool m_has_attr_rowSpan;

   int m_attr_colSpan;
   bool m_has_attr_colSpan;

   QString m_attr_alignment;
   bool m_has_attr_alignment;

   // child element data
   Kind m_kind;
   DomWidget *m_widget;
   DomLayout *m_layout;
   DomSpacer *m_spacer;

   DomLayoutItem(const DomLayoutItem &other);
   void operator = (const DomLayoutItem &other);
};

class QDESIGNER_UILIB_EXPORT DomRow
{
 public:
   DomRow();
   ~DomRow();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }

   void setElementProperty(const QList<DomProperty *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   enum Child {
      Property = 1
   };

   DomRow(const DomRow &other);
   void operator = (const DomRow &other);
};

class QDESIGNER_UILIB_EXPORT DomColumn
{
 public:
   DomColumn();
   ~DomColumn();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }

   void setElementProperty(const QList<DomProperty *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   enum Child {
      Property = 1
   };

   DomColumn(const DomColumn &other);
   void operator = (const DomColumn &other);
};

class QDESIGNER_UILIB_EXPORT DomItem
{
 public:
   DomItem();
   ~DomItem();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeRow() const {
      return m_has_attr_row;
   }

   int attributeRow() const {
      return m_attr_row;

   }

   void setAttributeRow(int a) {
      m_attr_row = a;
      m_has_attr_row = true;
   }

   void clearAttributeRow() {
      m_has_attr_row = false;
   }

   bool hasAttributeColumn() const {
      return m_has_attr_column;
   }

   int attributeColumn() const {
      return m_attr_column;
   }

   void setAttributeColumn(int a) {
      m_attr_column = a;
      m_has_attr_column = true;
   }

   void clearAttributeColumn() {
      m_has_attr_column = false;
   }

   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }

   void setElementProperty(const QList<DomProperty *> &a);

   QList<DomItem *> elementItem() const {
      return m_item;
   }

   void setElementItem(const QList<DomItem *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   int m_attr_row;
   bool m_has_attr_row;

   int m_attr_column;
   bool m_has_attr_column;

   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   QList<DomItem *> m_item;

   enum Child {
      Property = 1,
      Item = 2
   };

   DomItem(const DomItem &other);
   void operator = (const DomItem &other);
};

class QDESIGNER_UILIB_EXPORT DomWidget
{
 public:
   DomWidget();
   ~DomWidget();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeClass() const {
      return m_has_attr_class;
   }

   QString attributeClass() const {
      return m_attr_class;
   }

   void setAttributeClass(const QString &a) {
      m_attr_class = a;
      m_has_attr_class = true;
   }

   void clearAttributeClass() {
      m_has_attr_class = false;
   }

   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   bool hasAttributeNative() const {
      return m_has_attr_native;
   }
   bool attributeNative() const {
      return m_attr_native;
   }
   void setAttributeNative(bool a) {
      m_attr_native = a;
      m_has_attr_native = true;
   }
   void clearAttributeNative() {
      m_has_attr_native = false;
   }

   // child element accessors
   QStringList elementClass() const {
      return m_class;
   }
   void setElementClass(const QStringList &a);

   QList<DomProperty *> elementProperty() const {
      return m_property;
   }
   void setElementProperty(const QList<DomProperty *> &a);

   QList<DomScript *> elementScript() const {
      return m_script;
   }
   void setElementScript(const QList<DomScript *> &a);

   QList<DomWidgetData *> elementWidgetData() const {
      return m_widgetData;
   }
   void setElementWidgetData(const QList<DomWidgetData *> &a);

   QList<DomProperty *> elementAttribute() const {
      return m_attribute;
   }
   void setElementAttribute(const QList<DomProperty *> &a);

   QList<DomRow *> elementRow() const {
      return m_row;
   }
   void setElementRow(const QList<DomRow *> &a);

   QList<DomColumn *> elementColumn() const {
      return m_column;
   }
   void setElementColumn(const QList<DomColumn *> &a);

   QList<DomItem *> elementItem() const {
      return m_item;
   }
   void setElementItem(const QList<DomItem *> &a);

   QList<DomLayout *> elementLayout() const {
      return m_layout;
   }
   void setElementLayout(const QList<DomLayout *> &a);

   QList<DomWidget *> elementWidget() const {
      return m_widget;
   }
   void setElementWidget(const QList<DomWidget *> &a);

   QList<DomAction *> elementAction() const {
      return m_action;
   }
   void setElementAction(const QList<DomAction *> &a);

   QList<DomActionGroup *> elementActionGroup() const {
      return m_actionGroup;
   }
   void setElementActionGroup(const QList<DomActionGroup *> &a);

   QList<DomActionRef *> elementAddAction() const {
      return m_addAction;
   }
   void setElementAddAction(const QList<DomActionRef *> &a);

   QStringList elementZOrder() const {
      return m_zOrder;
   }
   void setElementZOrder(const QStringList &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_class;
   bool m_has_attr_class;

   QString m_attr_name;
   bool m_has_attr_name;

   bool m_attr_native;
   bool m_has_attr_native;

   // child element data
   uint m_children;
   QStringList m_class;
   QList<DomProperty *> m_property;
   QList<DomScript *> m_script;
   QList<DomWidgetData *> m_widgetData;
   QList<DomProperty *> m_attribute;
   QList<DomRow *> m_row;
   QList<DomColumn *> m_column;
   QList<DomItem *> m_item;
   QList<DomLayout *> m_layout;
   QList<DomWidget *> m_widget;
   QList<DomAction *> m_action;
   QList<DomActionGroup *> m_actionGroup;
   QList<DomActionRef *> m_addAction;
   QStringList m_zOrder;

   enum Child {
      Class = 1,
      Property = 2,
      Script = 4,
      WidgetData = 8,
      Attribute = 16,
      Row = 32,
      Column = 64,
      Item = 128,
      Layout = 256,
      Widget = 512,
      Action = 1024,
      ActionGroup = 2048,
      AddAction = 4096,
      ZOrder = 8192
   };

   DomWidget(const DomWidget &other);
   void operator = (const DomWidget &other);
};

class QDESIGNER_UILIB_EXPORT DomSpacer
{
 public:
   DomSpacer();
   ~DomSpacer();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }
   void clearAttributeName() {
      m_has_attr_name = false;
   }

   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }
   void setElementProperty(const QList<DomProperty *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   enum Child {
      Property = 1
   };

   DomSpacer(const DomSpacer &other);
   void operator = (const DomSpacer &other);
};

class QDESIGNER_UILIB_EXPORT DomColor
{
 public:
   DomColor();
   ~DomColor();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeAlpha() const {
      return m_has_attr_alpha;
   }

   int attributeAlpha() const {
      return m_attr_alpha;
   }

   void setAttributeAlpha(int a) {
      m_attr_alpha = a;
      m_has_attr_alpha = true;
   }

   void clearAttributeAlpha() {
      m_has_attr_alpha = false;
   }

   // child element accessors
   int elementRed() const {
      return m_red;
   }
   void setElementRed(int a);
   bool hasElementRed() const {
      return m_children & Red;
   }
   void clearElementRed();

   int elementGreen() const {
      return m_green;
   }
   void setElementGreen(int a);
   bool hasElementGreen() const {
      return m_children & Green;
   }
   void clearElementGreen();

   int elementBlue() const {
      return m_blue;
   }
   void setElementBlue(int a);
   bool hasElementBlue() const {
      return m_children & Blue;
   }
   void clearElementBlue();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   int m_attr_alpha;
   bool m_has_attr_alpha;

   // child element data
   uint m_children;
   int m_red;
   int m_green;
   int m_blue;

   enum Child {
      Red = 1,
      Green = 2,
      Blue = 4
   };

   DomColor(const DomColor &other);
   void operator = (const DomColor &other);
};

class QDESIGNER_UILIB_EXPORT DomGradientStop
{
 public:
   DomGradientStop();
   ~DomGradientStop();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributePosition() const {
      return m_has_attr_position;
   }

   double attributePosition() const {
      return m_attr_position;
   }

   void setAttributePosition(double a) {
      m_attr_position = a;
      m_has_attr_position = true;
   }
   void clearAttributePosition() {
      m_has_attr_position = false;
   }

   // child element accessors
   DomColor *elementColor() const {
      return m_color;
   }
   DomColor *takeElementColor();
   void setElementColor(DomColor *a);
   bool hasElementColor() const {
      return m_children & Color;
   }
   void clearElementColor();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   double m_attr_position;
   bool m_has_attr_position;

   // child element data
   uint m_children;
   DomColor *m_color;
   enum Child {
      Color = 1
   };

   DomGradientStop(const DomGradientStop &other);
   void operator = (const DomGradientStop &other);
};

class QDESIGNER_UILIB_EXPORT DomGradient
{
 public:
   DomGradient();
   ~DomGradient();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeStartX() const {
      return m_has_attr_startX;
   }

   double attributeStartX() const {
      return m_attr_startX;
   }

   void setAttributeStartX(double a) {
      m_attr_startX = a;
      m_has_attr_startX = true;
   }
   void clearAttributeStartX() {
      m_has_attr_startX = false;
   }

   bool hasAttributeStartY() const {
      return m_has_attr_startY;
   }
   double attributeStartY() const {
      return m_attr_startY;
   }
   void setAttributeStartY(double a) {
      m_attr_startY = a;
      m_has_attr_startY = true;
   }
   void clearAttributeStartY() {
      m_has_attr_startY = false;
   }

   bool hasAttributeEndX() const {
      return m_has_attr_endX;
   }
   double attributeEndX() const {
      return m_attr_endX;
   }
   void setAttributeEndX(double a) {
      m_attr_endX = a;
      m_has_attr_endX = true;
   }
   void clearAttributeEndX() {
      m_has_attr_endX = false;
   }

   bool hasAttributeEndY() const {
      return m_has_attr_endY;
   }
   double attributeEndY() const {
      return m_attr_endY;
   }
   void setAttributeEndY(double a) {
      m_attr_endY = a;
      m_has_attr_endY = true;
   }
   void clearAttributeEndY() {
      m_has_attr_endY = false;
   }

   bool hasAttributeCentralX() const {
      return m_has_attr_centralX;
   }

   double attributeCentralX() const {
      return m_attr_centralX;
   }

   void setAttributeCentralX(double a) {
      m_attr_centralX = a;
      m_has_attr_centralX = true;
   }

   void clearAttributeCentralX() {
      m_has_attr_centralX = false;
   }

   bool hasAttributeCentralY() const {
      return m_has_attr_centralY;
   }
   double attributeCentralY() const {
      return m_attr_centralY;
   }

   void setAttributeCentralY(double a) {
      m_attr_centralY = a;
      m_has_attr_centralY = true;
   }

   void clearAttributeCentralY() {
      m_has_attr_centralY = false;
   }

   bool hasAttributeFocalX() const {
      return m_has_attr_focalX;
   }

   double attributeFocalX() const {
      return m_attr_focalX;
   }

   void setAttributeFocalX(double a) {
      m_attr_focalX = a;
      m_has_attr_focalX = true;
   }

   void clearAttributeFocalX() {
      m_has_attr_focalX = false;
   }

   bool hasAttributeFocalY() const {
      return m_has_attr_focalY;
   }

   double attributeFocalY() const {
      return m_attr_focalY;
   }

   void setAttributeFocalY(double a) {
      m_attr_focalY = a;
      m_has_attr_focalY = true;
   }

   void clearAttributeFocalY() {
      m_has_attr_focalY = false;
   }

   bool hasAttributeRadius() const {
      return m_has_attr_radius;
   }

   double attributeRadius() const {
      return m_attr_radius;
   }

   void setAttributeRadius(double a) {
      m_attr_radius = a;
      m_has_attr_radius = true;
   }

   void clearAttributeRadius() {
      m_has_attr_radius = false;
   }

   bool hasAttributeAngle() const {
      return m_has_attr_angle;
   }

   double attributeAngle() const {
      return m_attr_angle;
   }

   void setAttributeAngle(double a) {
      m_attr_angle = a;
      m_has_attr_angle = true;
   }

   void clearAttributeAngle() {
      m_has_attr_angle = false;
   }

   bool hasAttributeType() const {
      return m_has_attr_type;
   }

   QString attributeType() const {
      return m_attr_type;
   }

   void setAttributeType(const QString &a) {
      m_attr_type = a;
      m_has_attr_type = true;
   }

   void clearAttributeType() {
      m_has_attr_type = false;
   }

   bool hasAttributeSpread() const {
      return m_has_attr_spread;
   }

   QString attributeSpread() const {
      return m_attr_spread;
   }

   void setAttributeSpread(const QString &a) {
      m_attr_spread = a;
      m_has_attr_spread = true;
   }

   void clearAttributeSpread() {
      m_has_attr_spread = false;
   }

   bool hasAttributeCoordinateMode() const {
      return m_has_attr_coordinateMode;
   }

   QString attributeCoordinateMode() const {
      return m_attr_coordinateMode;
   }

   void setAttributeCoordinateMode(const QString &a) {
      m_attr_coordinateMode = a;
      m_has_attr_coordinateMode = true;
   }

   void clearAttributeCoordinateMode() {
      m_has_attr_coordinateMode = false;
   }

   // child element accessors
   QList<DomGradientStop *> elementGradientStop() const {
      return m_gradientStop;
   }
   void setElementGradientStop(const QList<DomGradientStop *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   double m_attr_startX;
   bool m_has_attr_startX;

   double m_attr_startY;
   bool m_has_attr_startY;

   double m_attr_endX;
   bool m_has_attr_endX;

   double m_attr_endY;
   bool m_has_attr_endY;

   double m_attr_centralX;
   bool m_has_attr_centralX;

   double m_attr_centralY;
   bool m_has_attr_centralY;

   double m_attr_focalX;
   bool m_has_attr_focalX;

   double m_attr_focalY;
   bool m_has_attr_focalY;

   double m_attr_radius;
   bool m_has_attr_radius;

   double m_attr_angle;
   bool m_has_attr_angle;

   QString m_attr_type;
   bool m_has_attr_type;

   QString m_attr_spread;
   bool m_has_attr_spread;

   QString m_attr_coordinateMode;
   bool m_has_attr_coordinateMode;

   // child element data
   uint m_children;
   QList<DomGradientStop *> m_gradientStop;
   enum Child {
      GradientStop = 1
   };

   DomGradient(const DomGradient &other);
   void operator = (const DomGradient &other);
};

class QDESIGNER_UILIB_EXPORT DomBrush
{
 public:
   DomBrush();
   ~DomBrush();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeBrushStyle() const {
      return m_has_attr_brushStyle;
   }

   QString attributeBrushStyle() const {
      return m_attr_brushStyle;
   }

   void setAttributeBrushStyle(const QString &a) {
      m_attr_brushStyle = a;
      m_has_attr_brushStyle = true;
   }

   void clearAttributeBrushStyle() {
      m_has_attr_brushStyle = false;
   }

   // child element accessors
   enum Kind { Unknown = 0, Color, Texture, Gradient };
   Kind kind() const {
      return m_kind;
   }

   DomColor *elementColor() const {
      return m_color;
   }

   DomColor *takeElementColor();
   void setElementColor(DomColor *a);

   DomProperty *elementTexture() const {
      return m_texture;
   }

   DomProperty *takeElementTexture();
   void setElementTexture(DomProperty *a);

   DomGradient *elementGradient() const {
      return m_gradient;
   }

   DomGradient *takeElementGradient();
   void setElementGradient(DomGradient *a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_brushStyle;
   bool m_has_attr_brushStyle;

   // child element data
   Kind m_kind;
   DomColor *m_color;
   DomProperty *m_texture;
   DomGradient *m_gradient;

   DomBrush(const DomBrush &other);
   void operator = (const DomBrush &other);
};

class QDESIGNER_UILIB_EXPORT DomColorRole
{
 public:
   DomColorRole();
   ~DomColorRole();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeRole() const {
      return m_has_attr_role;
   }

   QString attributeRole() const {
      return m_attr_role;
   }

   void setAttributeRole(const QString &a) {
      m_attr_role = a;
      m_has_attr_role = true;
   }

   void clearAttributeRole() {
      m_has_attr_role = false;
   }

   // child element accessors
   DomBrush *elementBrush() const {
      return m_brush;
   }

   DomBrush *takeElementBrush();
   void setElementBrush(DomBrush *a);
   bool hasElementBrush() const {
      return m_children & Brush;
   }

   void clearElementBrush();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_role;
   bool m_has_attr_role;

   // child element data
   uint m_children;
   DomBrush *m_brush;
   enum Child {
      Brush = 1
   };

   DomColorRole(const DomColorRole &other);
   void operator = (const DomColorRole &other);
};

class QDESIGNER_UILIB_EXPORT DomColorGroup
{
 public:
   DomColorGroup();
   ~DomColorGroup();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomColorRole *> elementColorRole() const {
      return m_colorRole;
   }
   void setElementColorRole(const QList<DomColorRole *> &a);

   QList<DomColor *> elementColor() const {
      return m_color;
   }
   void setElementColor(const QList<DomColor *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomColorRole *> m_colorRole;
   QList<DomColor *> m_color;
   enum Child {
      ColorRole = 1,
      Color = 2
   };

   DomColorGroup(const DomColorGroup &other);
   void operator = (const DomColorGroup &other);
};

class QDESIGNER_UILIB_EXPORT DomPalette
{
 public:
   DomPalette();
   ~DomPalette();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   DomColorGroup *elementActive() const {
      return m_active;
   }

   DomColorGroup *takeElementActive();
   void setElementActive(DomColorGroup *a);
   bool hasElementActive() const {
      return m_children & Active;
   }

   void clearElementActive();

   DomColorGroup *elementInactive() const {
      return m_inactive;
   }

   DomColorGroup *takeElementInactive();
   void setElementInactive(DomColorGroup *a);
   bool hasElementInactive() const {
      return m_children & Inactive;
   }

   void clearElementInactive();

   DomColorGroup *elementDisabled() const {
      return m_disabled;
   }

   DomColorGroup *takeElementDisabled();
   void setElementDisabled(DomColorGroup *a);
   bool hasElementDisabled() const {
      return m_children & Disabled;
   }

   void clearElementDisabled();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   DomColorGroup *m_active;
   DomColorGroup *m_inactive;
   DomColorGroup *m_disabled;
   enum Child {
      Active = 1,
      Inactive = 2,
      Disabled = 4
   };

   DomPalette(const DomPalette &other);
   void operator = (const DomPalette &other);
};

class QDESIGNER_UILIB_EXPORT DomFont
{
 public:
   DomFont();
   ~DomFont();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QString elementFamily() const {
      return m_family;
   }
   void setElementFamily(const QString &a);
   bool hasElementFamily() const {
      return m_children & Family;
   }
   void clearElementFamily();

   int elementPointSize() const {
      return m_pointSize;
   }
   void setElementPointSize(int a);
   bool hasElementPointSize() const {
      return m_children & PointSize;
   }
   void clearElementPointSize();

   int elementWeight() const {
      return m_weight;
   }
   void setElementWeight(int a);
   bool hasElementWeight() const {
      return m_children & Weight;
   }
   void clearElementWeight();

   bool elementItalic() const {
      return m_italic;
   }
   void setElementItalic(bool a);
   bool hasElementItalic() const {
      return m_children & Italic;
   }
   void clearElementItalic();

   bool elementBold() const {
      return m_bold;
   }
   void setElementBold(bool a);
   bool hasElementBold() const {
      return m_children & Bold;
   }
   void clearElementBold();

   bool elementUnderline() const {
      return m_underline;
   }
   void setElementUnderline(bool a);
   bool hasElementUnderline() const {
      return m_children & Underline;
   }
   void clearElementUnderline();

   bool elementStrikeOut() const {
      return m_strikeOut;
   }
   void setElementStrikeOut(bool a);
   bool hasElementStrikeOut() const {
      return m_children & StrikeOut;
   }
   void clearElementStrikeOut();

   bool elementAntialiasing() const {
      return m_antialiasing;
   }
   void setElementAntialiasing(bool a);
   bool hasElementAntialiasing() const {
      return m_children & Antialiasing;
   }
   void clearElementAntialiasing();

   QString elementStyleStrategy() const {
      return m_styleStrategy;
   }
   void setElementStyleStrategy(const QString &a);
   bool hasElementStyleStrategy() const {
      return m_children & StyleStrategy;
   }
   void clearElementStyleStrategy();

   bool elementKerning() const {
      return m_kerning;
   }
   void setElementKerning(bool a);
   bool hasElementKerning() const {
      return m_children & Kerning;
   }
   void clearElementKerning();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QString m_family;
   int m_pointSize;
   int m_weight;
   bool m_italic;
   bool m_bold;
   bool m_underline;
   bool m_strikeOut;
   bool m_antialiasing;
   QString m_styleStrategy;
   bool m_kerning;

   enum Child {
      Family = 1,
      PointSize = 2,
      Weight = 4,
      Italic = 8,
      Bold = 16,
      Underline = 32,
      StrikeOut = 64,
      Antialiasing = 128,
      StyleStrategy = 256,
      Kerning = 512
   };

   DomFont(const DomFont &other);
   void operator = (const DomFont &other);
};

class QDESIGNER_UILIB_EXPORT DomPoint
{
 public:
   DomPoint();
   ~DomPoint();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   int elementX() const {
      return m_x;
   }

   void setElementX(int a);

   bool hasElementX() const {
      return m_children & X;
   }

   void clearElementX();

   int elementY() const {
      return m_y;
   }

   void setElementY(int a);

   bool hasElementY() const {
      return m_children & Y;
   }

  void clearElementY();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   int m_x;
   int m_y;
   enum Child {
      X = 1,
      Y = 2
   };

   DomPoint(const DomPoint &other);
   void operator = (const DomPoint &other);
};

class QDESIGNER_UILIB_EXPORT DomRect
{
 public:
   DomRect();
   ~DomRect();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   int elementX() const {
      return m_x;
   }

   void setElementX(int a);
   bool hasElementX() const {
      return m_children & X;
   }
   void clearElementX();

   int elementY() const {
      return m_y;
   }
   void setElementY(int a);
   bool hasElementY() const {
      return m_children & Y;
   }
   void clearElementY();

   int elementWidth() const {
      return m_width;
   }

   void setElementWidth(int a);

   bool hasElementWidth() const {
      return m_children & Width;
   }

   void clearElementWidth();

   int elementHeight() const {
      return m_height;
   }

   void setElementHeight(int a);
   bool hasElementHeight() const {
      return m_children & Height;
   }

   void clearElementHeight();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   int m_x;
   int m_y;
   int m_width;
   int m_height;
   enum Child {
      X = 1,
      Y = 2,
      Width = 4,
      Height = 8
   };

   DomRect(const DomRect &other);
   void operator = (const DomRect &other);
};

class QDESIGNER_UILIB_EXPORT DomLocale
{
 public:
   DomLocale();
   ~DomLocale();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeLanguage() const {
      return m_has_attr_language;
   }

   QString attributeLanguage() const {
      return m_attr_language;
   }

   void setAttributeLanguage(const QString &a) {
      m_attr_language = a;
      m_has_attr_language = true;
   }

   void clearAttributeLanguage() {
      m_has_attr_language = false;
   }

   bool hasAttributeCountry() const {
      return m_has_attr_country;
   }

   QString attributeCountry() const {
      return m_attr_country;
   }

   void setAttributeCountry(const QString &a) {
      m_attr_country = a;
      m_has_attr_country = true;
   }

   void clearAttributeCountry() {
      m_has_attr_country = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_language;
   bool m_has_attr_language;

   QString m_attr_country;
   bool m_has_attr_country;

   // child element data
   uint m_children;

   DomLocale(const DomLocale &other);
   void operator = (const DomLocale &other);
};

class QDESIGNER_UILIB_EXPORT DomSizePolicy
{
 public:
   DomSizePolicy();
   ~DomSizePolicy();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeHSizeType() const {
      return m_has_attr_hSizeType;
   }

   QString attributeHSizeType() const {
      return m_attr_hSizeType;
   }

   void setAttributeHSizeType(const QString &a) {
      m_attr_hSizeType = a;
      m_has_attr_hSizeType = true;
   }

   void clearAttributeHSizeType() {
      m_has_attr_hSizeType = false;
   }

   bool hasAttributeVSizeType() const {
      return m_has_attr_vSizeType;
   }

   QString attributeVSizeType() const {
      return m_attr_vSizeType;
   }

   void setAttributeVSizeType(const QString &a) {
      m_attr_vSizeType = a;
      m_has_attr_vSizeType = true;
   }

   void clearAttributeVSizeType() {
      m_has_attr_vSizeType = false;
   }

   // child element accessors
   int elementHSizeType() const {
      return m_hSizeType;
   }

   void setElementHSizeType(int a);

   bool hasElementHSizeType() const {
      return m_children & HSizeType;
   }

   void clearElementHSizeType();

   int elementVSizeType() const {
      return m_vSizeType;
   }

   void setElementVSizeType(int a);

   bool hasElementVSizeType() const {
      return m_children & VSizeType;
   }

   void clearElementVSizeType();

   int elementHorStretch() const {
      return m_horStretch;
   }

   void setElementHorStretch(int a);

   bool hasElementHorStretch() const {
      return m_children & HorStretch;
   }

   void clearElementHorStretch();

   int elementVerStretch() const {
      return m_verStretch;
   }

   void setElementVerStretch(int a);

   bool hasElementVerStretch() const {
      return m_children & VerStretch;
   }

   void clearElementVerStretch();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_hSizeType;
   bool m_has_attr_hSizeType;

   QString m_attr_vSizeType;
   bool m_has_attr_vSizeType;

   // child element data
   uint m_children;
   int m_hSizeType;
   int m_vSizeType;
   int m_horStretch;
   int m_verStretch;

   enum Child {
      HSizeType = 1,
      VSizeType = 2,
      HorStretch = 4,
      VerStretch = 8
   };

   DomSizePolicy(const DomSizePolicy &other);
   void operator = (const DomSizePolicy &other);
};

class QDESIGNER_UILIB_EXPORT DomSize
{
 public:
   DomSize();
   ~DomSize();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   int elementWidth() const {
      return m_width;
   }

   void setElementWidth(int a);

   bool hasElementWidth() const {
      return m_children & Width;
   }
   void clearElementWidth();

   int elementHeight() const {
      return m_height;
   }

   void setElementHeight(int a);

   bool hasElementHeight() const {
      return m_children & Height;
   }
   void clearElementHeight();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   int m_width;
   int m_height;

   enum Child {
      Width = 1,
      Height = 2
   };

   DomSize(const DomSize &other);
   void operator = (const DomSize &other);
};

class QDESIGNER_UILIB_EXPORT DomDate
{
 public:
   DomDate();
   ~DomDate();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   int elementYear() const {
      return m_year;
   }

   void setElementYear(int a);
   bool hasElementYear() const {
      return m_children & Year;
   }

   void clearElementYear();

   int elementMonth() const {
      return m_month;
   }

   void setElementMonth(int a);
   bool hasElementMonth() const {
      return m_children & Month;
   }

   void clearElementMonth();

   int elementDay() const {
      return m_day;
   }

   void setElementDay(int a);
   bool hasElementDay() const {
      return m_children & Day;
   }
   void clearElementDay();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   int m_year;
   int m_month;
   int m_day;

   enum Child {
      Year = 1,
      Month = 2,
      Day = 4
   };

   DomDate(const DomDate &other);
   void operator = (const DomDate &other);
};

class QDESIGNER_UILIB_EXPORT DomTime
{
 public:
   DomTime();
   ~DomTime();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   int elementHour() const {
      return m_hour;
   }

   void setElementHour(int a);

   bool hasElementHour() const {
      return m_children & Hour;
   }

   void clearElementHour();

   int elementMinute() const {
      return m_minute;
   }

   void setElementMinute(int a);
   bool hasElementMinute() const {
      return m_children & Minute;
   }

   void clearElementMinute();

   int elementSecond() const {
      return m_second;
   }

   void setElementSecond(int a);

   bool hasElementSecond() const {
      return m_children & Second;
   }
   void clearElementSecond();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   int m_hour;
   int m_minute;
   int m_second;

   enum Child {
      Hour = 1,
      Minute = 2,
      Second = 4
   };

   DomTime(const DomTime &other);
   void operator = (const DomTime &other);
};

class QDESIGNER_UILIB_EXPORT DomDateTime
{
 public:
   DomDateTime();
   ~DomDateTime();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   int elementHour() const {
      return m_hour;
   }

   void setElementHour(int a);

   bool hasElementHour() const {
      return m_children & Hour;
   }

   void clearElementHour();

   int elementMinute() const {
      return m_minute;
   }

   void setElementMinute(int a);
   bool hasElementMinute() const {
      return m_children & Minute;
   }

   void clearElementMinute();

   int elementSecond() const {
      return m_second;
   }

   void setElementSecond(int a);

   bool hasElementSecond() const {
      return m_children & Second;
   }

   void clearElementSecond();

   int elementYear() const {
      return m_year;
   }

   void setElementYear(int a);

   bool hasElementYear() const {
      return m_children & Year;
   }

   void clearElementYear();

   int elementMonth() const {
      return m_month;
   }

   void setElementMonth(int a);
   bool hasElementMonth() const {
      return m_children & Month;
   }

   void clearElementMonth();

   int elementDay() const {
      return m_day;
   }

   void setElementDay(int a);
   bool hasElementDay() const {
      return m_children & Day;
   }

   void clearElementDay();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   int m_hour;
   int m_minute;
   int m_second;
   int m_year;
   int m_month;
   int m_day;

   enum Child {
      Hour = 1,
      Minute = 2,
      Second = 4,
      Year = 8,
      Month = 16,
      Day = 32
   };

   DomDateTime(const DomDateTime &other);
   void operator = (const DomDateTime &other);
};

class QDESIGNER_UILIB_EXPORT DomStringList
{
 public:
   DomStringList();
   ~DomStringList();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeNotr() const {
      return m_has_attr_notr;
   }

   QString attributeNotr() const {
      return m_attr_notr;
   }

   void setAttributeNotr(const QString &a) {
      m_attr_notr = a;
      m_has_attr_notr = true;
   }

   void clearAttributeNotr() {
      m_has_attr_notr = false;
   }

   bool hasAttributeComment() const {
      return m_has_attr_comment;
   }

   QString attributeComment() const {
      return m_attr_comment;
   }

   void setAttributeComment(const QString &a) {
      m_attr_comment = a;
      m_has_attr_comment = true;
   }

   void clearAttributeComment() {
      m_has_attr_comment = false;
   }

   bool hasAttributeExtraComment() const {
      return m_has_attr_extraComment;
   }

   QString attributeExtraComment() const {
      return m_attr_extraComment;
   }

   void setAttributeExtraComment(const QString &a) {
      m_attr_extraComment = a;
      m_has_attr_extraComment = true;
   }

   void clearAttributeExtraComment() {
      m_has_attr_extraComment = false;
   }

   // child element accessors
   QStringList elementString() const {
      return m_string;
   }

   void setElementString(const QStringList &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data

   QString m_attr_notr;
   bool m_has_attr_notr;

   QString m_attr_comment;
   bool m_has_attr_comment;

   QString m_attr_extraComment;
   bool m_has_attr_extraComment;

   // child element data
   uint m_children;
   QStringList m_string;

   enum Child {
      String = 1
   };

   DomStringList(const DomStringList &other);
   void operator = (const DomStringList &other);
};

class QDESIGNER_UILIB_EXPORT DomResourcePixmap
{
 public:
   DomResourcePixmap();
   ~DomResourcePixmap();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeResource() const {
      return m_has_attr_resource;
   }

   QString attributeResource() const {
      return m_attr_resource;
   }

   void setAttributeResource(const QString &a) {
      m_attr_resource = a;
      m_has_attr_resource = true;
   }

   void clearAttributeResource() {
      m_has_attr_resource = false;
   }

   bool hasAttributeAlias() const {
      return m_has_attr_alias;
   }

   QString attributeAlias() const {
      return m_attr_alias;
   }

   void setAttributeAlias(const QString &a) {
      m_attr_alias = a;
      m_has_attr_alias = true;
   }

   void clearAttributeAlias() {
      m_has_attr_alias = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_resource;
   bool m_has_attr_resource;

   QString m_attr_alias;
   bool m_has_attr_alias;

   // child element data
   uint m_children;

   DomResourcePixmap(const DomResourcePixmap &other);
   void operator = (const DomResourcePixmap &other);
};

class QDESIGNER_UILIB_EXPORT DomResourceIcon
{
 public:
   DomResourceIcon();
   ~DomResourceIcon();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeTheme() const {
      return m_has_attr_theme;
   }

   QString attributeTheme() const {
      return m_attr_theme;
   }

   void setAttributeTheme(const QString &a) {
      m_attr_theme = a;
      m_has_attr_theme = true;
   }

   void clearAttributeTheme() {
      m_has_attr_theme = false;
   }

   bool hasAttributeResource() const {
      return m_has_attr_resource;
   }

   QString attributeResource() const {
      return m_attr_resource;
   }

   void setAttributeResource(const QString &a) {
      m_attr_resource = a;
      m_has_attr_resource = true;
   }
   void clearAttributeResource() {
      m_has_attr_resource = false;
   }

   // child element accessors
   DomResourcePixmap *elementNormalOff() const {
      return m_normalOff;
   }

   DomResourcePixmap *takeElementNormalOff();
   void setElementNormalOff(DomResourcePixmap *a);
   bool hasElementNormalOff() const {
      return m_children & NormalOff;
   }

   void clearElementNormalOff();

   DomResourcePixmap *elementNormalOn() const {
      return m_normalOn;
   }

   DomResourcePixmap *takeElementNormalOn();
   void setElementNormalOn(DomResourcePixmap *a);

   bool hasElementNormalOn() const {
      return m_children & NormalOn;
   }
   void clearElementNormalOn();

   DomResourcePixmap *elementDisabledOff() const {
      return m_disabledOff;
   }

   DomResourcePixmap *takeElementDisabledOff();
   void setElementDisabledOff(DomResourcePixmap *a);

   bool hasElementDisabledOff() const {
      return m_children & DisabledOff;
   }

   void clearElementDisabledOff();

   DomResourcePixmap *elementDisabledOn() const {
      return m_disabledOn;
   }

   DomResourcePixmap *takeElementDisabledOn();
   void setElementDisabledOn(DomResourcePixmap *a);

   bool hasElementDisabledOn() const {
      return m_children & DisabledOn;
   }
   void clearElementDisabledOn();

   DomResourcePixmap *elementActiveOff() const {
      return m_activeOff;
   }

   DomResourcePixmap *takeElementActiveOff();
   void setElementActiveOff(DomResourcePixmap *a);

   bool hasElementActiveOff() const {
      return m_children & ActiveOff;
   }

   void clearElementActiveOff();

   DomResourcePixmap *elementActiveOn() const {
      return m_activeOn;
   }

   DomResourcePixmap *takeElementActiveOn();
   void setElementActiveOn(DomResourcePixmap *a);

   bool hasElementActiveOn() const {
      return m_children & ActiveOn;
   }

   void clearElementActiveOn();

   DomResourcePixmap *elementSelectedOff() const {
      return m_selectedOff;
   }

   DomResourcePixmap *takeElementSelectedOff();
   void setElementSelectedOff(DomResourcePixmap *a);

   bool hasElementSelectedOff() const {
      return m_children & SelectedOff;
   }

   void clearElementSelectedOff();

   DomResourcePixmap *elementSelectedOn() const {
      return m_selectedOn;
   }

   DomResourcePixmap *takeElementSelectedOn();
   void setElementSelectedOn(DomResourcePixmap *a);

   bool hasElementSelectedOn() const {
      return m_children & SelectedOn;
   }

   void clearElementSelectedOn();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_theme;
   bool m_has_attr_theme;

   QString m_attr_resource;
   bool m_has_attr_resource;

   // child element data
   uint m_children;
   DomResourcePixmap *m_normalOff;
   DomResourcePixmap *m_normalOn;
   DomResourcePixmap *m_disabledOff;
   DomResourcePixmap *m_disabledOn;
   DomResourcePixmap *m_activeOff;
   DomResourcePixmap *m_activeOn;
   DomResourcePixmap *m_selectedOff;
   DomResourcePixmap *m_selectedOn;

   enum Child {
      NormalOff = 1,
      NormalOn = 2,
      DisabledOff = 4,
      DisabledOn = 8,
      ActiveOff = 16,
      ActiveOn = 32,
      SelectedOff = 64,
      SelectedOn = 128
   };

   DomResourceIcon(const DomResourceIcon &other);
   void operator = (const DomResourceIcon &other);
};

class QDESIGNER_UILIB_EXPORT DomString
{
 public:
   DomString();
   ~DomString();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeNotr() const {
      return m_has_attr_notr;
   }

   QString attributeNotr() const {
      return m_attr_notr;
   }

   void setAttributeNotr(const QString &a) {
      m_attr_notr = a;
      m_has_attr_notr = true;
   }

   void clearAttributeNotr() {
      m_has_attr_notr = false;
   }

   bool hasAttributeComment() const {
      return m_has_attr_comment;
   }

   QString attributeComment() const {
      return m_attr_comment;
   }

   void setAttributeComment(const QString &a) {
      m_attr_comment = a;
      m_has_attr_comment = true;
   }

   void clearAttributeComment() {
      m_has_attr_comment = false;
   }

   bool hasAttributeExtraComment() const {
      return m_has_attr_extraComment;
   }

   QString attributeExtraComment() const {
      return m_attr_extraComment;
   }

   void setAttributeExtraComment(const QString &a) {
      m_attr_extraComment = a;
      m_has_attr_extraComment = true;
   }

   void clearAttributeExtraComment() {
      m_has_attr_extraComment = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_notr;
   bool m_has_attr_notr;

   QString m_attr_comment;
   bool m_has_attr_comment;

   QString m_attr_extraComment;
   bool m_has_attr_extraComment;

   // child element data
   uint m_children;

   DomString(const DomString &other);
   void operator = (const DomString &other);
};

class QDESIGNER_UILIB_EXPORT DomPointF
{
 public:
   DomPointF();
   ~DomPointF();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   double elementX() const {
      return m_x;
   }

   void setElementX(double a);

   bool hasElementX() const {
      return m_children & X;
   }

   void clearElementX();

   double elementY() const {
      return m_y;
   }

   void setElementY(double a);

   bool hasElementY() const {
      return m_children & Y;
   }

   void clearElementY();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   double m_x;
   double m_y;

   enum Child {
      X = 1,
      Y = 2
   };

   DomPointF(const DomPointF &other);
   void operator = (const DomPointF &other);
};

class QDESIGNER_UILIB_EXPORT DomRectF
{
 public:
   DomRectF();
   ~DomRectF();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   double elementX() const {
      return m_x;
   }
   void setElementX(double a);
   bool hasElementX() const {
      return m_children & X;
   }
   void clearElementX();

   double elementY() const {
      return m_y;
   }

   void setElementY(double a);
   bool hasElementY() const {
      return m_children & Y;
   }

   void clearElementY();

   double elementWidth() const {
      return m_width;
   }

   void setElementWidth(double a);

   bool hasElementWidth() const {
      return m_children & Width;
   }

   void clearElementWidth();

   double elementHeight() const {
      return m_height;
   }

   void setElementHeight(double a);

   bool hasElementHeight() const {
      return m_children & Height;
   }
   void clearElementHeight();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   double m_x;
   double m_y;
   double m_width;
   double m_height;

   enum Child {
      X = 1,
      Y = 2,
      Width = 4,
      Height = 8
   };

   DomRectF(const DomRectF &other);
   void operator = (const DomRectF &other);
};

class QDESIGNER_UILIB_EXPORT DomSizeF
{
 public:
   DomSizeF();
   ~DomSizeF();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   double elementWidth() const {
      return m_width;
   }

   void setElementWidth(double a);

   bool hasElementWidth() const {
      return m_children & Width;
   }

   void clearElementWidth();

   double elementHeight() const {
      return m_height;
   }

   void setElementHeight(double a);

   bool hasElementHeight() const {
      return m_children & Height;
   }
   void clearElementHeight();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   double m_width;
   double m_height;

   enum Child {
      Width = 1,
      Height = 2
   };

   DomSizeF(const DomSizeF &other);
   void operator = (const DomSizeF &other);
};

class QDESIGNER_UILIB_EXPORT DomChar
{
 public:
   DomChar();
   ~DomChar();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   int elementUnicode() const {
      return m_unicode;
   }
   void setElementUnicode(int a);
   bool hasElementUnicode() const {
      return m_children & Unicode;
   }
   void clearElementUnicode();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   int m_unicode;
   enum Child {
      Unicode = 1
   };

   DomChar(const DomChar &other);
   void operator = (const DomChar &other);
};

class QDESIGNER_UILIB_EXPORT DomUrl
{
 public:
   DomUrl();
   ~DomUrl();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   DomString *elementString() const {
      return m_string;
   }

   DomString *takeElementString();
   void setElementString(DomString *a);

   bool hasElementString() const {
      return m_children & String;
   }
   void clearElementString();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   DomString *m_string;

   enum Child {
      String = 1
   };

   DomUrl(const DomUrl &other);
   void operator = (const DomUrl &other);
};

class QDESIGNER_UILIB_EXPORT DomProperty
{
 public:
   DomProperty();
   ~DomProperty();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   bool hasAttributeStdset() const {
      return m_has_attr_stdset;
   }

   int attributeStdset() const {
      return m_attr_stdset;
   }

   void setAttributeStdset(int a) {
      m_attr_stdset = a;
      m_has_attr_stdset = true;
   }

   void clearAttributeStdset() {
      m_has_attr_stdset = false;
   }

   // child element accessors
   enum Kind { Unknown = 0, Bool, Color, Cstring, Cursor, CursorShape, Enum,
               Font, IconSet, Pixmap, Palette, Point, Rect, Set, Locale,
               SizePolicy, Size, String, StringList, Number, Float, Double,
               Date, Time, DateTime, PointF, RectF, SizeF, LongLong,
               Char, Url, UInt, ULongLong, Brush };

   Kind kind() const {
      return m_kind;
   }

   QString elementBool() const {
      return m_bool;
   }

   void setElementBool(const QString &a);

   DomColor *elementColor() const {
      return m_color;
   }

   DomColor *takeElementColor();
   void setElementColor(DomColor *a);

   QString elementCstring() const {
      return m_cstring;
   }

   void setElementCstring(const QString &a);

   int elementCursor() const {
      return m_cursor;
   }

   void setElementCursor(int a);

   QString elementCursorShape() const {
      return m_cursorShape;
   }

   void setElementCursorShape(const QString &a);

   QString elementEnum() const {
      return m_enum;
   }

   void setElementEnum(const QString &a);

   DomFont *elementFont() const {
      return m_font;
   }

   DomFont *takeElementFont();
   void setElementFont(DomFont *a);

   DomResourceIcon *elementIconSet() const {
      return m_iconSet;
   }

   DomResourceIcon *takeElementIconSet();
   void setElementIconSet(DomResourceIcon *a);

   DomResourcePixmap *elementPixmap() const {
      return m_pixmap;
   }

   DomResourcePixmap *takeElementPixmap();
   void setElementPixmap(DomResourcePixmap *a);

   DomPalette *elementPalette() const {
      return m_palette;
   }

   DomPalette *takeElementPalette();
   void setElementPalette(DomPalette *a);

   DomPoint *elementPoint() const {
      return m_point;
   }

   DomPoint *takeElementPoint();
   void setElementPoint(DomPoint *a);

   DomRect *elementRect() const {
      return m_rect;
   }

   DomRect *takeElementRect();
   void setElementRect(DomRect *a);

   QString elementSet() const {
      return m_set;
   }

   void setElementSet(const QString &a);

   DomLocale *elementLocale() const {
      return m_locale;
   }

   DomLocale *takeElementLocale();
   void setElementLocale(DomLocale *a);

   DomSizePolicy *elementSizePolicy() const {
      return m_sizePolicy;
   }

   DomSizePolicy *takeElementSizePolicy();
   void setElementSizePolicy(DomSizePolicy *a);

   DomSize *elementSize() const {
      return m_size;
   }

   DomSize *takeElementSize();
   void setElementSize(DomSize *a);

   DomString *elementString() const {
      return m_string;
   }

   DomString *takeElementString();
   void setElementString(DomString *a);

   DomStringList *elementStringList() const {
      return m_stringList;
   }

   DomStringList *takeElementStringList();
   void setElementStringList(DomStringList *a);

   int elementNumber() const {
      return m_number;
   }
   void setElementNumber(int a);

   float elementFloat() const {
      return m_float;
   }
   void setElementFloat(float a);

   double elementDouble() const {
      return m_double;
   }
   void setElementDouble(double a);

   DomDate *elementDate() const {
      return m_date;
   }
   DomDate *takeElementDate();
   void setElementDate(DomDate *a);

   DomTime *elementTime() const {
      return m_time;
   }
   DomTime *takeElementTime();
   void setElementTime(DomTime *a);

   DomDateTime *elementDateTime() const {
      return m_dateTime;
   }
   DomDateTime *takeElementDateTime();
   void setElementDateTime(DomDateTime *a);

   DomPointF *elementPointF() const {
      return m_pointF;
   }
   DomPointF *takeElementPointF();
   void setElementPointF(DomPointF *a);

   DomRectF *elementRectF() const {
      return m_rectF;
   }
   DomRectF *takeElementRectF();
   void setElementRectF(DomRectF *a);

   DomSizeF *elementSizeF() const {
      return m_sizeF;
   }
   DomSizeF *takeElementSizeF();
   void setElementSizeF(DomSizeF *a);

   qint64 elementLongLong() const {
      return m_longLong;
   }
   void setElementLongLong(qint64 a);

   DomChar *elementChar() const {
      return m_char;
   }
   DomChar *takeElementChar();
   void setElementChar(DomChar *a);

   DomUrl *elementUrl() const {
      return m_url;
   }
   DomUrl *takeElementUrl();
   void setElementUrl(DomUrl *a);

   uint elementUInt() const {
      return m_UInt;
   }
   void setElementUInt(uint a);

   quint64 elementULongLong() const {
      return m_uLongLong;
   }
   void setElementULongLong(quint64 a);

   DomBrush *elementBrush() const {
      return m_brush;
   }

   DomBrush *takeElementBrush();
   void setElementBrush(DomBrush *a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   int m_attr_stdset;
   bool m_has_attr_stdset;

   // child element data
   Kind m_kind;
   QString m_bool;
   DomColor *m_color;
   QString m_cstring;
   int m_cursor;
   QString m_cursorShape;
   QString m_enum;
   DomFont *m_font;
   DomResourceIcon *m_iconSet;
   DomResourcePixmap *m_pixmap;
   DomPalette *m_palette;
   DomPoint *m_point;
   DomRect *m_rect;
   QString m_set;
   DomLocale *m_locale;
   DomSizePolicy *m_sizePolicy;
   DomSize *m_size;
   DomString *m_string;
   DomStringList *m_stringList;
   int m_number;
   float m_float;
   double m_double;
   DomDate *m_date;
   DomTime *m_time;
   DomDateTime *m_dateTime;
   DomPointF *m_pointF;
   DomRectF *m_rectF;
   DomSizeF *m_sizeF;
   qint64 m_longLong;
   DomChar *m_char;
   DomUrl *m_url;
   uint m_UInt;
   quint64 m_uLongLong;
   DomBrush *m_brush;

   DomProperty(const DomProperty &other);
   void operator = (const DomProperty &other);
};

class QDESIGNER_UILIB_EXPORT DomConnections
{
 public:
   DomConnections();
   ~DomConnections();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomConnection *> elementConnection() const {
      return m_connection;
   }

   void setElementConnection(const QList<DomConnection *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomConnection *> m_connection;

   enum Child {
      Connection = 1
   };

   DomConnections(const DomConnections &other);
   void operator = (const DomConnections &other);
};

class QDESIGNER_UILIB_EXPORT DomConnection
{
 public:
   DomConnection();
   ~DomConnection();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QString elementSender() const {
      return m_sender;
   }

   void setElementSender(const QString &a);

   bool hasElementSender() const {
      return m_children & Sender;
   }
   void clearElementSender();

   QString elementSignal() const {
      return m_signal;
   }
   void setElementSignal(const QString &a);
   bool hasElementSignal() const {
      return m_children & Signal;
   }
   void clearElementSignal();

   QString elementReceiver() const {
      return m_receiver;
   }
   void setElementReceiver(const QString &a);
   bool hasElementReceiver() const {
      return m_children & Receiver;
   }
   void clearElementReceiver();

   QString elementSlot() const {
      return m_slot;
   }
   void setElementSlot(const QString &a);
   bool hasElementSlot() const {
      return m_children & Slot;
   }
   void clearElementSlot();

   DomConnectionHints *elementHints() const {
      return m_hints;
   }
   DomConnectionHints *takeElementHints();
   void setElementHints(DomConnectionHints *a);
   bool hasElementHints() const {
      return m_children & Hints;
   }
   void clearElementHints();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QString m_sender;
   QString m_signal;
   QString m_receiver;
   QString m_slot;
   DomConnectionHints *m_hints;

   enum Child {
      Sender = 1,
      Signal = 2,
      Receiver = 4,
      Slot = 8,
      Hints = 16
   };

   DomConnection(const DomConnection &other);
   void operator = (const DomConnection &other);
};

class QDESIGNER_UILIB_EXPORT DomConnectionHints
{
 public:
   DomConnectionHints();
   ~DomConnectionHints();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomConnectionHint *> elementHint() const {
      return m_hint;
   }
   void setElementHint(const QList<DomConnectionHint *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomConnectionHint *> m_hint;
   enum Child {
      Hint = 1
   };

   DomConnectionHints(const DomConnectionHints &other);
   void operator = (const DomConnectionHints &other);
};

class QDESIGNER_UILIB_EXPORT DomConnectionHint
{
 public:
   DomConnectionHint();
   ~DomConnectionHint();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }
   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeType() const {
      return m_has_attr_type;
   }
   QString attributeType() const {
      return m_attr_type;
   }
   void setAttributeType(const QString &a) {
      m_attr_type = a;
      m_has_attr_type = true;
   }
   void clearAttributeType() {
      m_has_attr_type = false;
   }

   // child element accessors
   int elementX() const {
      return m_x;
   }
   void setElementX(int a);
   bool hasElementX() const {
      return m_children & X;
   }
   void clearElementX();

   int elementY() const {
      return m_y;
   }
   void setElementY(int a);
   bool hasElementY() const {
      return m_children & Y;
   }
   void clearElementY();

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_type;
   bool m_has_attr_type;

   // child element data
   uint m_children;
   int m_x;
   int m_y;

   enum Child {
      X = 1,
      Y = 2
   };

   DomConnectionHint(const DomConnectionHint &other);
   void operator = (const DomConnectionHint &other);
};

class QDESIGNER_UILIB_EXPORT DomScript
{
 public:
   DomScript();
   ~DomScript();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }
   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeSource() const {
      return m_has_attr_source;
   }
   QString attributeSource() const {
      return m_attr_source;
   }
   void setAttributeSource(const QString &a) {
      m_attr_source = a;
      m_has_attr_source = true;
   }
   void clearAttributeSource() {
      m_has_attr_source = false;
   }

   bool hasAttributeLanguage() const {
      return m_has_attr_language;
   }
   QString attributeLanguage() const {
      return m_attr_language;
   }
   void setAttributeLanguage(const QString &a) {
      m_attr_language = a;
      m_has_attr_language = true;
   }
   void clearAttributeLanguage() {
      m_has_attr_language = false;
   }

   // child element accessors
 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_source;
   bool m_has_attr_source;

   QString m_attr_language;
   bool m_has_attr_language;

   // child element data
   uint m_children;

   DomScript(const DomScript &other);
   void operator = (const DomScript &other);
};

class QDESIGNER_UILIB_EXPORT DomWidgetData
{
 public:
   DomWidgetData();
   ~DomWidgetData();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }
   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }
   void setElementProperty(const QList<DomProperty *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   enum Child {
      Property = 1
   };

   DomWidgetData(const DomWidgetData &other);
   void operator = (const DomWidgetData &other);
};

class QDESIGNER_UILIB_EXPORT DomDesignerData
{
 public:
   DomDesignerData();
   ~DomDesignerData();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomProperty *> elementProperty() const {
      return m_property;
   }
   void setElementProperty(const QList<DomProperty *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomProperty *> m_property;
   enum Child {
      Property = 1
   };

   DomDesignerData(const DomDesignerData &other);
   void operator = (const DomDesignerData &other);
};

class QDESIGNER_UILIB_EXPORT DomSlots
{
 public:
   DomSlots();
   ~DomSlots();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }
   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QStringList elementSignal() const {
      return m_signal;
   }

   void setElementSignal(const QStringList &a);

   QStringList elementSlot() const {
      return m_slot;
   }

   void setElementSlot(const QStringList &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QStringList m_signal;
   QStringList m_slot;

   enum Child {
      Signal = 1,
      Slot = 2
   };

   DomSlots(const DomSlots &other);
   void operator = (const DomSlots &other);
};

class QDESIGNER_UILIB_EXPORT DomPropertySpecifications
{
 public:
   DomPropertySpecifications();
   ~DomPropertySpecifications();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   // child element accessors
   QList<DomPropertyToolTip*> elementTooltip() const {
      return m_tooltip;
   }

   void setElementTooltip(const QList<DomPropertyToolTip*> &a);

   QList<DomStringPropertySpecification *> elementStringpropertyspecification() const {
      return m_stringpropertyspecification;
   }

   void setElementStringpropertyspecification(const QList<DomStringPropertySpecification *> &a);

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   // child element data
   uint m_children;
   QList<DomPropertyToolTip*> m_tooltip;
   QList<DomStringPropertySpecification *> m_stringpropertyspecification;

   enum Child {
      Tooltip = 1,
      Stringpropertyspecification = 2
   };

   DomPropertySpecifications(const DomPropertySpecifications &other);
   void operator = (const DomPropertySpecifications &other);
};

class QDESIGNER_UILIB_EXPORT DomPropertyToolTip
{
 public:
   DomPropertyToolTip();
   ~DomPropertyToolTip();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString& a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

 private:
    QString m_text;
    void clear(bool clear_all = true);

    // attribute data
    QString m_attr_name;
    bool m_has_attr_name;

    // child element data
    uint m_children;

    DomPropertyToolTip(const DomPropertyToolTip &other);
    void operator = (const DomPropertyToolTip&other);
};

class QDESIGNER_UILIB_EXPORT DomStringPropertySpecification
{
 public:
   DomStringPropertySpecification();
   ~DomStringPropertySpecification();

   void read(QXmlStreamReader &reader);
   void write(QXmlStreamWriter &writer, const QString &tagName = QString()) const;

   QString text() const {
      return m_text;
   }

   void setText(const QString &s) {
      m_text = s;
   }

   // attribute accessors
   bool hasAttributeName() const {
      return m_has_attr_name;
   }

   QString attributeName() const {
      return m_attr_name;
   }

   void setAttributeName(const QString &a) {
      m_attr_name = a;
      m_has_attr_name = true;
   }

   void clearAttributeName() {
      m_has_attr_name = false;
   }

   bool hasAttributeType() const {
      return m_has_attr_type;
   }

   QString attributeType() const {
      return m_attr_type;
   }

   void setAttributeType(const QString &a) {
      m_attr_type = a;
      m_has_attr_type = true;
   }

   void clearAttributeType() {
      m_has_attr_type = false;
   }

   bool hasAttributeNotr() const {
      return m_has_attr_notr;
   }

   QString attributeNotr() const {
      return m_attr_notr;
   }

   void setAttributeNotr(const QString &a) {
      m_attr_notr = a;
      m_has_attr_notr = true;
   }

   void clearAttributeNotr() {
      m_has_attr_notr = false;
   }

 private:
   QString m_text;
   void clear(bool clear_all = true);

   // attribute data
   QString m_attr_name;
   bool m_has_attr_name;

   QString m_attr_type;
   bool m_has_attr_type;

   QString m_attr_notr;
   bool m_has_attr_notr;

   // child element data
   uint m_children;

   DomStringPropertySpecification(const DomStringPropertySpecification &other);
   void operator = (const DomStringPropertySpecification &other);
};

#endif
