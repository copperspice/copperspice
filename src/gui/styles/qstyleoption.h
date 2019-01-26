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

#ifndef QSTYLEOPTION_H
#define QSTYLEOPTION_H

#include <QtCore/qvariant.h>
#include <QtGui/qabstractspinbox.h>
#include <QtGui/qicon.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qslider.h>
#include <QtGui/qstyle.h>
#include <QtGui/qtabbar.h>
#include <QtGui/qtabwidget.h>
#include <QtGui/qrubberband.h>
#include <QtGui/qframe.h>

#ifndef QT_NO_ITEMVIEWS
#   include <QtCore/qabstractitemmodel.h>
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

   int version;
   int type;
   QStyle::State state;
   Qt::LayoutDirection direction;
   QRect rect;
   QFontMetrics fontMetrics;
   QPalette palette;
   QObject *styleObject;

   QStyleOption(int version = QStyleOption::Version, int type = SO_Default);
   QStyleOption(const QStyleOption &other);
   ~QStyleOption();

   void init(const QWidget *w);
   inline void initFrom(const QWidget *w) {
      init(w);
   }

   QStyleOption &operator=(const QStyleOption &other);
};

class Q_GUI_EXPORT QStyleOptionFocusRect : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_FocusRect };
   enum StyleOptionVersion { Version = 1 };

   QColor backgroundColor;

   QStyleOptionFocusRect();
   QStyleOptionFocusRect(const QStyleOptionFocusRect &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   int lineWidth;
   int midLineWidth;

   FrameFeatures features;
   QFrame::Shape frameShape;

   QStyleOptionFrame();
   QStyleOptionFrame(const QStyleOptionFrame &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   int lineWidth;
   int midLineWidth;

   QTabBar::Shape shape;
   QSize tabBarSize;
   QSize rightCornerWidgetSize;
   QSize leftCornerWidgetSize;
   QRect tabBarRect;
   QRect selectedTabRect;

   QStyleOptionTabWidgetFrame();
   inline QStyleOptionTabWidgetFrame(const QStyleOptionTabWidgetFrame &other)
      : QStyleOption(Version, Type) {
      *this = other;
   }

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

   QTabBar::Shape shape;
   QRect tabBarRect;
   QRect selectedTabRect;
   bool documentMode;

   QStyleOptionTabBarBase();
   QStyleOptionTabBarBase(const QStyleOptionTabBarBase &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   int section;
   QString text;
   Qt::Alignment textAlignment;
   QIcon icon;
   Qt::Alignment iconAlignment;
   SectionPosition position;
   SelectedPosition selectedPosition;
   SortIndicator sortIndicator;
   Qt::Orientation orientation;

   QStyleOptionHeader();
   QStyleOptionHeader(const QStyleOptionHeader &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   ButtonFeatures features;
   QString text;
   QIcon icon;
   QSize iconSize;

   QStyleOptionButton();
   QStyleOptionButton(const QStyleOptionButton &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   QStyleOptionTab();
   QStyleOptionTab(const QStyleOptionTab &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   ToolBarPosition positionOfLine; // The toolbar line position
   ToolBarPosition positionWithinLine; // The position within a toolbar
   Qt::ToolBarArea toolBarArea; // The toolbar docking area
   ToolBarFeatures features;
   int lineWidth;
   int midLineWidth;

   QStyleOptionToolBar();
   QStyleOptionToolBar(const QStyleOptionToolBar &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   int minimum;
   int maximum;
   int progress;
   QString text;
   Qt::Alignment textAlignment;
   bool textVisible;

   Qt::Orientation orientation;
   bool invertedAppearance;
   bool bottomToTop;

   QStyleOptionProgressBar();
   QStyleOptionProgressBar(const QStyleOptionProgressBar &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   QStyleOptionMenuItem();
   QStyleOptionMenuItem(const QStyleOptionMenuItem &other) : QStyleOption(Version, Type) {
      *this = other;
   }

 protected:
   QStyleOptionMenuItem(int version);
};

class Q_GUI_EXPORT QStyleOptionDockWidget : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_DockWidget };
   enum StyleOptionVersion { Version = 2 };

   QString title;
   bool closable;
   bool movable;
   bool floatable;
   bool verticalTitleBar;

   QStyleOptionDockWidget();
   QStyleOptionDockWidget(const QStyleOptionDockWidget &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   QStyleOptionViewItem();
   QStyleOptionViewItem(const QStyleOptionViewItem &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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

   TabPosition position;
   SelectedPosition selectedPosition;

   QStyleOptionToolBox();
   QStyleOptionToolBox(const QStyleOptionToolBox &other) : QStyleOption(Version, Type) {
      *this = other;
   }

 protected:
   QStyleOptionToolBox(int version);
};

#ifndef QT_NO_RUBBERBAND
class Q_GUI_EXPORT QStyleOptionRubberBand : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_RubberBand };
   enum StyleOptionVersion { Version = 1 };

   QRubberBand::Shape shape;
   bool opaque;

   QStyleOptionRubberBand();
   QStyleOptionRubberBand(const QStyleOptionRubberBand &other) : QStyleOption(Version, Type) {
      *this = other;
   }

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
   QStyleOptionComplex(const QStyleOptionComplex &other) : QStyleOption(Version, Type) {
      *this = other;
   }
};

#ifndef QT_NO_SLIDER
class Q_GUI_EXPORT QStyleOptionSlider : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_Slider };
   enum StyleOptionVersion { Version = 1 };

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

   QStyleOptionSlider();
   QStyleOptionSlider(const QStyleOptionSlider &other) : QStyleOptionComplex(Version, Type) {
      *this = other;
   }

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

   QAbstractSpinBox::ButtonSymbols buttonSymbols;
   QAbstractSpinBox::StepEnabled stepEnabled;
   bool frame;

   QStyleOptionSpinBox();
   QStyleOptionSpinBox(const QStyleOptionSpinBox &other) : QStyleOptionComplex(Version, Type) {
      *this = other;
   }

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

   ToolButtonFeatures features;
   QIcon icon;
   QSize iconSize;
   QString text;
   Qt::ArrowType arrowType;
   Qt::ToolButtonStyle toolButtonStyle;
   QPoint pos;
   QFont font;

   QStyleOptionToolButton();
   QStyleOptionToolButton(const QStyleOptionToolButton &other) : QStyleOptionComplex(Version, Type) {
      *this = other;
   }

 protected:
   QStyleOptionToolButton(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionToolButton::ToolButtonFeatures)

class Q_GUI_EXPORT QStyleOptionComboBox : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_ComboBox };
   enum StyleOptionVersion { Version = 1 };

   bool editable;
   QRect popupRect;
   bool frame;
   QString currentText;
   QIcon currentIcon;
   QSize iconSize;

   QStyleOptionComboBox();
   QStyleOptionComboBox(const QStyleOptionComboBox &other) : QStyleOptionComplex(Version, Type) {
      *this = other;
   }

 protected:
   QStyleOptionComboBox(int version);
};

class Q_GUI_EXPORT QStyleOptionTitleBar : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_TitleBar };
   enum StyleOptionVersion { Version = 1 };

   QString text;
   QIcon icon;
   int titleBarState;
   Qt::WindowFlags titleBarFlags;

   QStyleOptionTitleBar();
   QStyleOptionTitleBar(const QStyleOptionTitleBar &other) : QStyleOptionComplex(Version, Type) {
      *this = other;
   }

 protected:
   QStyleOptionTitleBar(int version);
};

class Q_GUI_EXPORT QStyleOptionGroupBox : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_GroupBox };
   enum StyleOptionVersion { Version = 1 };

   QStyleOptionFrame::FrameFeatures features;
   QString text;
   Qt::Alignment textAlignment;
   QColor textColor;
   int lineWidth;
   int midLineWidth;

   QStyleOptionGroupBox();
   QStyleOptionGroupBox(const QStyleOptionGroupBox &other) : QStyleOptionComplex(Version, Type) {
      *this = other;
   }

 protected:
   QStyleOptionGroupBox(int version);
};

class Q_GUI_EXPORT QStyleOptionSizeGrip : public QStyleOptionComplex
{
 public:
   enum StyleOptionType { Type = SO_SizeGrip };
   enum StyleOptionVersion { Version = 1 };

   Qt::Corner corner;

   QStyleOptionSizeGrip();
   QStyleOptionSizeGrip(const QStyleOptionSizeGrip &other) : QStyleOptionComplex(Version, Type) {
      *this = other;
   }
 protected:
   QStyleOptionSizeGrip(int version);
};

class Q_GUI_EXPORT QStyleOptionGraphicsItem : public QStyleOption
{
 public:
   enum StyleOptionType { Type = SO_GraphicsItem };
   enum StyleOptionVersion { Version = 1 };

   QRectF exposedRect;
   QMatrix matrix;
   qreal levelOfDetail;

   QStyleOptionGraphicsItem();
   QStyleOptionGraphicsItem(const QStyleOptionGraphicsItem &other) : QStyleOption(Version, Type) {
      *this = other;
   }
   static qreal levelOfDetailFromTransform(const QTransform &worldTransform);
 protected:
   QStyleOptionGraphicsItem(int version);
};

template <typename T>
T qstyleoption_cast(const QStyleOption *opt)
{
   typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type Opt;

   if (opt && opt->version >= Opt::Version && (opt->type == Opt::Type
         || int(Opt::Type) == QStyleOption::SO_Default
         || (int(Opt::Type) == QStyleOption::SO_Complex
            && opt->type > QStyleOption::SO_Complex))) {
      return static_cast<T>(opt);
   }


   return nullptr;
}

template <typename T>
T qstyleoption_cast(QStyleOption *opt)
{
   typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type Opt;

   if (opt && opt->version >= Opt::Version && (opt->type == Opt::Type
         || int(Opt::Type) == QStyleOption::SO_Default
         || (int(Opt::Type) == QStyleOption::SO_Complex
            && opt->type > QStyleOption::SO_Complex))) {
      return static_cast<T>(opt);
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


#endif // QSTYLEOPTION_H
