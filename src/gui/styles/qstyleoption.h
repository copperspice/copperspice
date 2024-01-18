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

#ifndef QSTYLEOPTION_H
#define QSTYLEOPTION_H

#include <qvariant.h>
#include <qabstractspinbox.h>
#include <qicon.h>
#include <qmatrix.h>
#include <qslider.h>
#include <qstyle.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qrubberband.h>
#include <qframe.h>

#ifndef QT_NO_ITEMVIEWS
#   include <qabstractitemmodel.h>
#endif

class QDebug;

class Q_GUI_EXPORT QStyleOption
{
 public:
   enum OptionType {
      SO_Default, SO_FocusRect, SO_Button, SO_Tab, SO_MenuItem,
      SO_Frame, SO_ProgressBar, SO_ToolBox, SO_Header,
      SO_DockWidget, SO_ViewItem, SO_TabWidgetFrame,
      SO_TabBarBase, SO_RubberBand, SO_ToolBar, SO_GraphicsItem,

      SO_Complex = 0xf0000, SO_Slider, SO_SpinBox, SO_ToolButton, SO_ComboBox,
      SO_TitleBar, SO_GroupBox, SO_SizeGrip,

      SO_CustomBase = 0xf00,
      SO_ComplexCustomBase = 0xf000000
   };

   enum StyleOptionType { Type = SO_Default };
   enum StyleOptionVersion { Version = 1 };

   QStyleOption(int version = QStyleOption::Version, int type = SO_Default);
   QStyleOption(const QStyleOption &other);
   ~QStyleOption();

   void initFrom(const QWidget *widget);

   QStyleOption &operator=(const QStyleOption &other);

   int version;
   int type;
   QStyle::State state;
   Qt::LayoutDirection direction;
   QRect rect;
   QFontMetrics fontMetrics;
   QPalette palette;
   QObject *styleObject;
};

class Q_GUI_EXPORT QStyleOptionFocusRect : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_FocusRect };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionFocusRect();

   QStyleOptionFocusRect(const QStyleOptionFocusRect &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionFocusRect &operator=(const QStyleOptionFocusRect &other) = default;

   QColor backgroundColor;

 protected:
   QStyleOptionFocusRect(int version);
};

class Q_GUI_EXPORT QStyleOptionFrame : public QStyleOption
{
 public:
   enum StyleOptionType    { Type = SO_Frame };
   enum StyleOptionVersion { Version = 3 };

   enum FrameFeature {
      None = 0x00,
      Flat = 0x01,
      Rounded = 0x02
   };
   using FrameFeatures = QFlags<FrameFeature>;

   QStyleOptionFrame();
   QStyleOptionFrame(const QStyleOptionFrame &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionFrame &operator=(const QStyleOptionFrame &other) = default;

   int lineWidth;
   int midLineWidth;

   FrameFeatures features;
   QFrame::Shape frameShape;

 protected:
   QStyleOptionFrame(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionFrame::FrameFeatures)

#ifndef QT_NO_TABWIDGET
class Q_GUI_EXPORT QStyleOptionTabWidgetFrame : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_TabWidgetFrame };
   enum StyleOptionVersion { Version = 2 };

   QStyleOptionTabWidgetFrame();
   inline QStyleOptionTabWidgetFrame(const QStyleOptionTabWidgetFrame &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionTabWidgetFrame &operator=(const QStyleOptionTabWidgetFrame &other) = default;

   int lineWidth;
   int midLineWidth;

   QTabBar::Shape shape;
   QSize tabBarSize;
   QSize rightCornerWidgetSize;
   QSize leftCornerWidgetSize;
   QRect tabBarRect;
   QRect selectedTabRect;

 protected:
   QStyleOptionTabWidgetFrame(int version);
};

#endif

#ifndef QT_NO_TABBAR
class Q_GUI_EXPORT QStyleOptionTabBarBase : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_TabBarBase };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionTabBarBase();
   QStyleOptionTabBarBase(const QStyleOptionTabBarBase &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionTabBarBase &operator=(const QStyleOptionTabBarBase &other) = default;

   QTabBar::Shape shape;
   QRect tabBarRect;
   QRect selectedTabRect;
   bool documentMode;

 protected:
   QStyleOptionTabBarBase(int version);
};

#endif

class Q_GUI_EXPORT QStyleOptionHeader : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_Header };
   enum StyleOptionVersion { Version = 1 };

   enum SectionPosition { Beginning, Middle, End, OnlyOneSection };
   enum SelectedPosition { NotAdjacent, NextIsSelected, PreviousIsSelected,
      NextAndPreviousAreSelected
   };
   enum SortIndicator { None, SortUp, SortDown };

   QStyleOptionHeader();
   QStyleOptionHeader(const QStyleOptionHeader &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionHeader &operator=(const QStyleOptionHeader &other) = default;

   int section;
   QString text;
   Qt::Alignment textAlignment;
   QIcon icon;
   Qt::Alignment iconAlignment;
   SectionPosition position;
   SelectedPosition selectedPosition;
   SortIndicator sortIndicator;
   Qt::Orientation orientation;

 protected:
   QStyleOptionHeader(int version);
};

class Q_GUI_EXPORT QStyleOptionButton : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_Button };
   enum StyleOptionVersion { Version = 1 };

   enum ButtonFeature { None = 0x00, Flat = 0x01, HasMenu = 0x02, DefaultButton = 0x04,
      AutoDefaultButton = 0x08, CommandLinkButton = 0x10
   };
   using ButtonFeatures = QFlags<ButtonFeature>;

   QStyleOptionButton();
   QStyleOptionButton(const QStyleOptionButton &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionButton &operator=(const QStyleOptionButton &other) = default;

   ButtonFeatures features;
   QString text;
   QIcon icon;
   QSize iconSize;

 protected:
   QStyleOptionButton(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionButton::ButtonFeatures)

#ifndef QT_NO_TABBAR
class Q_GUI_EXPORT QStyleOptionTab : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_Tab };
   enum StyleOptionVersion { Version = 1 };

   enum TabPosition { Beginning, Middle, End, OnlyOneTab };
   enum SelectedPosition { NotAdjacent, NextIsSelected, PreviousIsSelected };
   enum CornerWidget { NoCornerWidgets = 0x00, LeftCornerWidget = 0x01,
      RightCornerWidget = 0x02
   };
   using CornerWidgets = QFlags<CornerWidget>;

   enum TabFeature { None = 0x00, HasFrame = 0x01 };
   using TabFeatures = QFlags<TabFeature>;

   QStyleOptionTab();
   QStyleOptionTab(const QStyleOptionTab &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionTab &operator=(const QStyleOptionTab &other) = default;

   QTabBar::Shape shape;
   QString text;
   QIcon icon;
   int row;
   TabPosition position;
   SelectedPosition selectedPosition;
   CornerWidgets cornerWidgets;

   QSize iconSize;

   bool documentMode;
   QSize leftButtonSize;
   QSize rightButtonSize;
   TabFeatures features;

 protected:
   QStyleOptionTab(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionTab::CornerWidgets)

#endif // QT_NO_TABBAR

#ifndef QT_NO_TOOLBAR

class Q_GUI_EXPORT QStyleOptionToolBar : public QStyleOption
{
 public:
   enum StyleOptionType    { Type = SO_ToolBar };
   enum StyleOptionVersion { Version = 1 };
   enum ToolBarPosition    { Beginning, Middle, End, OnlyOne };
   enum ToolBarFeature     { None = 0x0, Movable = 0x1 };
   using ToolBarFeatures = QFlags<ToolBarFeature>;

   QStyleOptionToolBar();
   QStyleOptionToolBar(const QStyleOptionToolBar &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionToolBar &operator=(const QStyleOptionToolBar &other) = default;

   ToolBarPosition positionOfLine;       // toolbar line position
   ToolBarPosition positionWithinLine;   // position within a toolbar
   Qt::ToolBarArea toolBarArea;          // toolbar docking area
   ToolBarFeatures features;
   int lineWidth;
   int midLineWidth;

 protected:
   QStyleOptionToolBar(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionToolBar::ToolBarFeatures)

#endif // QT_NO_TOOLBAR

class Q_GUI_EXPORT QStyleOptionProgressBar : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_ProgressBar };
   enum StyleOptionVersion { Version = 2 };

   QStyleOptionProgressBar();
   QStyleOptionProgressBar(const QStyleOptionProgressBar &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionProgressBar &operator=(const QStyleOptionProgressBar &other) = default;

   int minimum;
   int maximum;
   int progress;
   QString text;
   Qt::Alignment textAlignment;
   bool textVisible;

   Qt::Orientation orientation;
   bool invertedAppearance;
   bool bottomToTop;

 protected:
   QStyleOptionProgressBar(int version);
};

class Q_GUI_EXPORT QStyleOptionMenuItem : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_MenuItem };
   enum StyleOptionVersion { Version = 1 };

   enum MenuItemType { Normal, DefaultItem, Separator, SubMenu, Scroller, TearOff, Margin,
      EmptyArea
   };
   enum CheckType { NotCheckable, Exclusive, NonExclusive };

   QStyleOptionMenuItem();
   QStyleOptionMenuItem(const QStyleOptionMenuItem &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionMenuItem &operator=(const QStyleOptionMenuItem &other) = default;

   MenuItemType menuItemType;
   CheckType checkType;
   bool checked;
   bool menuHasCheckableItems;
   QRect menuRect;
   QString text;
   QIcon icon;
   int maxIconWidth;
   int tabWidth;
   QFont font;

 protected:
   QStyleOptionMenuItem(int version);
};

class Q_GUI_EXPORT QStyleOptionDockWidget : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_DockWidget };
   enum StyleOptionVersion { Version = 2 };

   QStyleOptionDockWidget();
   QStyleOptionDockWidget(const QStyleOptionDockWidget &other)
      : QStyleOption(Version, Type) {
      *this = other;
   }

   QStyleOptionDockWidget &operator=(const QStyleOptionDockWidget &other) = default;

   QString title;
   bool closable;
   bool movable;
   bool floatable;
   bool verticalTitleBar;

 protected:
   QStyleOptionDockWidget(int version);
};

#ifndef QT_NO_ITEMVIEWS

class Q_GUI_EXPORT QStyleOptionViewItem : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_ViewItem };
   enum StyleOptionVersion { Version = 4 };

   enum Position { Left, Right, Top, Bottom };

   Qt::Alignment displayAlignment;
   Qt::Alignment decorationAlignment;
   Qt::TextElideMode textElideMode;
   Position decorationPosition;
   QSize decorationSize;
   QFont font;
   bool showDecorationSelected;

   enum ViewItemFeature {
      None = 0x00,
      WrapText = 0x01,
      Alternate = 0x02,
      HasCheckIndicator = 0x04,
      HasDisplay = 0x08,
      HasDecoration = 0x10
   };
   using ViewItemFeatures = QFlags<ViewItemFeature>;

   QStyleOptionViewItem();
   QStyleOptionViewItem(const QStyleOptionViewItem &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionViewItem &operator=(const QStyleOptionViewItem &other) = default;

   ViewItemFeatures features;

   QLocale locale;
   const QWidget *widget;
   enum ViewItemPosition { Invalid, Beginning, Middle, End, OnlyOne };

   QModelIndex index;
   Qt::CheckState checkState;
   QIcon icon;
   QString text;
   ViewItemPosition viewItemPosition;
   QBrush backgroundBrush;

 protected:
   QStyleOptionViewItem(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionViewItem::ViewItemFeatures)
#endif

class Q_GUI_EXPORT QStyleOptionToolBox : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_ToolBox };
   enum StyleOptionVersion { Version = 2 };

   QString text;
   QIcon icon;

   enum TabPosition { Beginning, Middle, End, OnlyOneTab };
   enum SelectedPosition { NotAdjacent, NextIsSelected, PreviousIsSelected };

   QStyleOptionToolBox();
   QStyleOptionToolBox(const QStyleOptionToolBox &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionToolBox &operator=(const QStyleOptionToolBox &other) = default;

   TabPosition position;
   SelectedPosition selectedPosition;

 protected:
   QStyleOptionToolBox(int version);
};

#ifndef QT_NO_RUBBERBAND
class Q_GUI_EXPORT QStyleOptionRubberBand : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_RubberBand };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionRubberBand();
   QStyleOptionRubberBand(const QStyleOptionRubberBand &other)
       : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionRubberBand &operator=(const QStyleOptionRubberBand &other) = default;

   QRubberBand::Shape shape;
   bool opaque;

 protected:
   QStyleOptionRubberBand(int version);
};
#endif // QT_NO_RUBBERBAND

// complex style options
class Q_GUI_EXPORT QStyleOptionComplex : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_Complex };
   enum StyleOptionVersion { Version = 1 };

   QStyle::SubControls subControls;
   QStyle::SubControls activeSubControls;

   QStyleOptionComplex(int version = QStyleOptionComplex::Version, int type = SO_Complex);
   QStyleOptionComplex(const QStyleOptionComplex &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionComplex &operator=(const QStyleOptionComplex &other) = default;
};

#ifndef QT_NO_SLIDER
class Q_GUI_EXPORT QStyleOptionSlider : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_Slider };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionSlider();
   QStyleOptionSlider(const QStyleOptionSlider &other)
      : QStyleOptionComplex(Version, Type) {
      *this = other;
   }

   QStyleOptionSlider &operator=(const QStyleOptionSlider &other) = default;

   Qt::Orientation orientation;
   int minimum;
   int maximum;
   QSlider::TickPosition tickPosition;
   int tickInterval;
   bool upsideDown;
   int sliderPosition;
   int sliderValue;
   int singleStep;
   int pageStep;
   qreal notchTarget;
   bool dialWrapping;

 protected:
   QStyleOptionSlider(int version);
};
#endif // QT_NO_SLIDER

#ifndef QT_NO_SPINBOX
class Q_GUI_EXPORT QStyleOptionSpinBox : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_SpinBox };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionSpinBox();
   QStyleOptionSpinBox(const QStyleOptionSpinBox &other)
      : QStyleOptionComplex(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionSpinBox &operator=(const QStyleOptionSpinBox &other) = default;

   QAbstractSpinBox::ButtonSymbols buttonSymbols;
   QAbstractSpinBox::StepEnabled stepEnabled;
   bool frame;

  protected:
   QStyleOptionSpinBox(int version);
};
#endif

class Q_GUI_EXPORT QStyleOptionToolButton : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_ToolButton };
   enum StyleOptionVersion { Version = 1 };

   enum ToolButtonFeature { None = 0x00, Arrow = 0x01, Menu = 0x04, MenuButtonPopup = Menu, PopupDelay = 0x08,
      HasMenu = 0x10
   };
   using ToolButtonFeatures = QFlags<ToolButtonFeature>;

   QStyleOptionToolButton();
   QStyleOptionToolButton(const QStyleOptionToolButton &other)
      : QStyleOptionComplex(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionToolButton &operator=(const QStyleOptionToolButton &other) = default;

   ToolButtonFeatures features;
   QIcon icon;
   QSize iconSize;
   QString text;
   Qt::ArrowType arrowType;
   Qt::ToolButtonStyle toolButtonStyle;
   QPoint pos;
   QFont font;

 protected:
   QStyleOptionToolButton(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionToolButton::ToolButtonFeatures)

class Q_GUI_EXPORT QStyleOptionComboBox : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_ComboBox };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionComboBox();
   QStyleOptionComboBox(const QStyleOptionComboBox &other)
      : QStyleOptionComplex(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionComboBox &operator=(const QStyleOptionComboBox &other) = default;

   bool editable;
   QRect popupRect;
   bool frame;
   QString currentText;
   QIcon currentIcon;
   QSize iconSize;

 protected:
   QStyleOptionComboBox(int version);
};

class Q_GUI_EXPORT QStyleOptionTitleBar : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_TitleBar };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionTitleBar();
   QStyleOptionTitleBar(const QStyleOptionTitleBar &other)
      : QStyleOptionComplex(Version, Type) {
      *this = other;
   }

   QStyleOptionTitleBar &operator=(const QStyleOptionTitleBar &other) = default;

   QString text;
   QIcon icon;
   int titleBarState;
   Qt::WindowFlags titleBarFlags;

 protected:
   QStyleOptionTitleBar(int version);
};

class Q_GUI_EXPORT QStyleOptionGroupBox : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_GroupBox };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionGroupBox();
   QStyleOptionGroupBox(const QStyleOptionGroupBox &other)
      : QStyleOptionComplex(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionGroupBox &operator=(const QStyleOptionGroupBox &other) = default;

   QStyleOptionFrame::FrameFeatures features;
   QString text;
   Qt::Alignment textAlignment;
   QColor textColor;
   int lineWidth;
   int midLineWidth;

 protected:
   QStyleOptionGroupBox(int version);
};

class Q_GUI_EXPORT QStyleOptionSizeGrip : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_SizeGrip };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionSizeGrip();
   QStyleOptionSizeGrip(const QStyleOptionSizeGrip &other)
      : QStyleOptionComplex(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionSizeGrip &operator=(const QStyleOptionSizeGrip &other) = default;

   Qt::Corner corner;

 protected:
   QStyleOptionSizeGrip(int version);
};

class Q_GUI_EXPORT QStyleOptionGraphicsItem : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_GraphicsItem };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionGraphicsItem();
   QStyleOptionGraphicsItem(const QStyleOptionGraphicsItem &other)
      : QStyleOption(StyleOptionVersion::Version, StyleOptionType::Type) {
      *this = other;
   }

   QStyleOptionGraphicsItem &operator=(const QStyleOptionGraphicsItem &other) = default;

   static qreal levelOfDetailFromTransform(const QTransform &worldTransform);

   QRectF exposedRect;
   QMatrix matrix;
   qreal levelOfDetail;

 protected:
   QStyleOptionGraphicsItem(int version);
};

template <typename T>
T qstyleoption_cast(const QStyleOption *option)
{
   typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type Opt;

   if (option && option->version >= Opt::Version && (option->type == Opt::Type
         || int(Opt::Type) == QStyleOption::SO_Default || (int(Opt::Type) == QStyleOption::SO_Complex
         && option->type > QStyleOption::SO_Complex))) {
      return static_cast<T>(option);
   }

   return nullptr;
}

template <typename T>
T qstyleoption_cast(QStyleOption *option)
{
   typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type Opt;

   if (option && option->version >= Opt::Version && (option->type == Opt::Type
         || int(Opt::Type) == QStyleOption::SO_Default || (int(Opt::Type) == QStyleOption::SO_Complex
         && option->type > QStyleOption::SO_Complex))) {
      return static_cast<T>(option);
   }

   return nullptr;
}

class Q_GUI_EXPORT QStyleHintReturn
{
 public:
   enum HintReturnType {
      SH_Default = 0xf000, SH_Mask, SH_Variant
   };

   enum StyleOptionType { Type = SH_Default };
   enum StyleOptionVersion { Version = 1 };

   QStyleHintReturn(int version = QStyleOption::Version, int type = SH_Default);
   ~QStyleHintReturn();

   int version;
   int type;
};

class Q_GUI_EXPORT QStyleHintReturnMask : public QStyleHintReturn
{
 public:
   enum StyleOptionType { Type = SH_Mask };
   enum StyleOptionVersion { Version = 1 };

   QStyleHintReturnMask();
   ~QStyleHintReturnMask();

   QRegion region;
};

class Q_GUI_EXPORT QStyleHintReturnVariant : public QStyleHintReturn
{
 public:
   enum StyleOptionType { Type = SH_Variant };
   enum StyleOptionVersion { Version = 1 };

   QStyleHintReturnVariant();
   ~QStyleHintReturnVariant();

   QVariant variant;
};

template <typename T>
T qstyleoption_cast(const QStyleHintReturn *hint)
{
   typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type Opt;

   if (hint && hint->version <= Opt::Version &&
      (hint->type == Opt::Type || int(Opt::Type) == QStyleHintReturn::SH_Default)) {
      return static_cast<T>(hint);
   }

   return nullptr;
}

template <typename T>
T qstyleoption_cast(QStyleHintReturn *hint)
{
   typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type Opt;
   if (hint && hint->version <= Opt::Version &&
      (hint->type == Opt::Type || int(Opt::Type) == QStyleHintReturn::SH_Default)) {
      return static_cast<T>(hint);
   }

   return nullptr;
}

Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QStyleOption::OptionType &optionType);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QStyleOption &option);

#endif
