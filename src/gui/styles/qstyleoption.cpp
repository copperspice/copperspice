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

#include <qstyleoption.h>
#include <qapplication.h>

#ifdef Q_OS_MAC
# include <qt_mac_p.h>
# include <qmacstyle_mac.h>
#endif

#include <qdebug.h>
#include <qmath.h>

QStyleOption::QStyleOption(int version, int type)
   : version(version), type(type), state(QStyle::State_None),
     direction(QApplication::layoutDirection()), fontMetrics(QFont())
{
}

QStyleOption::~QStyleOption()
{
}

void QStyleOption::init(const QWidget *widget)
{
   QWidget *window = widget->window();
   state = QStyle::State_None;
   if (widget->isEnabled()) {
      state |= QStyle::State_Enabled;
   }
   if (widget->hasFocus()) {
      state |= QStyle::State_HasFocus;
   }
   if (window->testAttribute(Qt::WA_KeyboardFocusChange)) {
      state |= QStyle::State_KeyboardFocusChange;
   }
   if (widget->underMouse()) {
      state |= QStyle::State_MouseOver;
   }
   if (window->isActiveWindow()) {
      state |= QStyle::State_Active;
   }
   if (widget->isWindow()) {
      state |= QStyle::State_Window;
   }

#ifdef Q_OS_MAC
   extern bool qt_mac_can_clickThrough(const QWidget * w); //qwidget_mac.cpp
   if (!(state & QStyle::State_Active) && !qt_mac_can_clickThrough(widget)) {
      state &= ~QStyle::State_Enabled;
   }

   switch (QMacStyle::widgetSizePolicy(widget)) {
      case QMacStyle::SizeSmall:
         state |= QStyle::State_Small;
         break;
      case QMacStyle::SizeMini:
         state |= QStyle::State_Mini;
         break;
      default:
         ;
   }
#endif

#ifdef QT_KEYPAD_NAVIGATION
   if (widget->hasEditFocus()) {
      state |= QStyle::State_HasEditFocus;
   }
#endif

   direction = widget->layoutDirection();
   rect = widget->rect();
   palette = widget->palette();
   fontMetrics = widget->fontMetrics();
}

QStyleOption::QStyleOption(const QStyleOption &other)
   : version(Version), type(Type), state(other.state),
     direction(other.direction), rect(other.rect), fontMetrics(other.fontMetrics),
     palette(other.palette)
{
}

QStyleOption &QStyleOption::operator=(const QStyleOption &other)
{
   state = other.state;
   direction = other.direction;
   rect = other.rect;
   fontMetrics = other.fontMetrics;
   palette = other.palette;
   return *this;
}

QStyleOptionFocusRect::QStyleOptionFocusRect()
   : QStyleOption(Version, SO_FocusRect)
{
   state |= QStyle::State_KeyboardFocusChange; // assume we had one, will be corrected in initFrom()
}

QStyleOptionFocusRect::QStyleOptionFocusRect(int version)
   : QStyleOption(version, SO_FocusRect)
{
   state |= QStyle::State_KeyboardFocusChange;  // assume we had one, will be corrected in initFrom()
}

QStyleOptionFrame::QStyleOptionFrame()
   : QStyleOption(Version, SO_Frame), lineWidth(0), midLineWidth(0)
{
}

QStyleOptionFrame::QStyleOptionFrame(int version)
   : QStyleOption(version, SO_Frame), lineWidth(0), midLineWidth(0)
{
}

QStyleOptionFrameV2::QStyleOptionFrameV2()
   : QStyleOptionFrame(Version), features(None)
{
}

QStyleOptionFrameV2::QStyleOptionFrameV2(int version)
   : QStyleOptionFrame(version), features(None)
{
}

QStyleOptionFrameV2::QStyleOptionFrameV2(const QStyleOptionFrame &other)
{
   QStyleOptionFrame::operator=(other);

   const QStyleOptionFrameV2 *f2 = qstyleoption_cast<const QStyleOptionFrameV2 *>(&other);
   features = f2 ? f2->features : FrameFeatures(QStyleOptionFrameV2::None);
   version = Version;
}

QStyleOptionFrameV2 &QStyleOptionFrameV2::operator=(const QStyleOptionFrame &other)
{
   QStyleOptionFrame::operator=(other);

   const QStyleOptionFrameV2 *f2 = qstyleoption_cast<const QStyleOptionFrameV2 *>(&other);
   features = f2 ? f2->features : FrameFeatures(QStyleOptionFrameV2::None);
   version = Version;
   return *this;
}

QStyleOptionFrameV3::QStyleOptionFrameV3()
   : QStyleOptionFrameV2(Version), frameShape(QFrame::NoFrame), unused(0)
{
}

QStyleOptionFrameV3::QStyleOptionFrameV3(int version)
   : QStyleOptionFrameV2(version), frameShape(QFrame::NoFrame), unused(0)
{
}

QStyleOptionFrameV3::QStyleOptionFrameV3(const QStyleOptionFrame &other)
{
   operator=(other);
}

QStyleOptionFrameV3 &QStyleOptionFrameV3::operator=(const QStyleOptionFrame &other)
{
   QStyleOptionFrameV2::operator=(other);

   const QStyleOptionFrameV3 *f3 = qstyleoption_cast<const QStyleOptionFrameV3 *>(&other);
   frameShape = f3 ? f3->frameShape : QFrame::NoFrame;
   version = Version;
   return *this;
}

QStyleOptionViewItemV2::QStyleOptionViewItemV2()
   : QStyleOptionViewItem(Version), features(None)
{
}

QStyleOptionViewItemV2::QStyleOptionViewItemV2(const QStyleOptionViewItem &other)
   : QStyleOptionViewItem(Version)
{
   (void)QStyleOptionViewItemV2::operator=(other);
}

/*!
    \internal
*/
QStyleOptionViewItemV2::QStyleOptionViewItemV2(int version)
   : QStyleOptionViewItem(version)
{
}

QStyleOptionViewItemV2 &QStyleOptionViewItemV2::operator=(const QStyleOptionViewItem &other)
{
   QStyleOptionViewItem::operator=(other);
   const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&other);
   features = v2 ? v2->features : ViewItemFeatures(QStyleOptionViewItemV2::None);
   return *this;
}

QStyleOptionViewItemV3::QStyleOptionViewItemV3()
   : QStyleOptionViewItemV2(Version), widget(0)
{
}

QStyleOptionViewItemV3::QStyleOptionViewItemV3(const QStyleOptionViewItem &other)
   : QStyleOptionViewItemV2(Version), widget(0)
{
   (void)QStyleOptionViewItemV3::operator=(other);
}

QStyleOptionViewItemV3 &QStyleOptionViewItemV3::operator = (const QStyleOptionViewItem &other)
{
   QStyleOptionViewItemV2::operator=(other);
   const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&other);
   locale = v3 ? v3->locale : QLocale();
   widget = v3 ? v3->widget : 0;
   return *this;
}

/*!
    \internal
*/
QStyleOptionViewItemV3::QStyleOptionViewItemV3(int version)
   : QStyleOptionViewItemV2(version), widget(0)
{
}

#ifndef QT_NO_ITEMVIEWS

QStyleOptionViewItemV4::QStyleOptionViewItemV4()
   : QStyleOptionViewItemV3(Version), checkState(Qt::Unchecked), viewItemPosition(QStyleOptionViewItemV4::Invalid)
{
}

QStyleOptionViewItemV4::QStyleOptionViewItemV4(const QStyleOptionViewItem &other)
   : QStyleOptionViewItemV3(Version)
{
   (void)QStyleOptionViewItemV4::operator=(other);
}

QStyleOptionViewItemV4 &QStyleOptionViewItemV4::operator = (const QStyleOptionViewItem &other)
{
   QStyleOptionViewItemV3::operator=(other);

   if (const QStyleOptionViewItemV4 *v4 = qstyleoption_cast<const QStyleOptionViewItemV4 *>(&other)) {
      index = v4->index;
      checkState = v4->checkState;
      text = v4->text;
      viewItemPosition = v4->viewItemPosition;
      backgroundBrush = v4->backgroundBrush;
      icon = v4->icon;

   } else {
      viewItemPosition = QStyleOptionViewItemV4::Invalid;
      checkState = Qt::Unchecked;
   }
   return *this;
}

/*!
    \internal
*/
QStyleOptionViewItemV4::QStyleOptionViewItemV4(int version)
   : QStyleOptionViewItemV3(version)
{
}

#endif // QT_NO_ITEMVIEWS

QStyleOptionGroupBox::QStyleOptionGroupBox()
   : QStyleOptionComplex(Version, Type), features(QStyleOptionFrameV2::None),
     textAlignment(Qt::AlignLeft), lineWidth(0), midLineWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionGroupBox::QStyleOptionGroupBox(int version)
   : QStyleOptionComplex(version, Type), features(QStyleOptionFrameV2::None),
     textAlignment(Qt::AlignLeft), lineWidth(0), midLineWidth(0)
{
}

QStyleOptionHeader::QStyleOptionHeader()
   : QStyleOption(QStyleOptionHeader::Version, SO_Header),
     section(0), textAlignment(Qt::AlignLeft), iconAlignment(Qt::AlignLeft),
     position(QStyleOptionHeader::Beginning),
     selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None),
     orientation(Qt::Horizontal)
{
}

/*!
    \internal
*/
QStyleOptionHeader::QStyleOptionHeader(int version)
   : QStyleOption(version, SO_Header),
     section(0), textAlignment(Qt::AlignLeft), iconAlignment(Qt::AlignLeft),
     position(QStyleOptionHeader::Beginning),
     selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None),
     orientation(Qt::Horizontal)
{
}

QStyleOptionButton::QStyleOptionButton()
   : QStyleOption(QStyleOptionButton::Version, SO_Button), features(None)
{
}

/*!
    \internal
*/
QStyleOptionButton::QStyleOptionButton(int version)
   : QStyleOption(version, SO_Button), features(None)
{
}

#ifndef QT_NO_TOOLBAR

QStyleOptionToolBar::QStyleOptionToolBar()
   : QStyleOption(Version, SO_ToolBar), positionOfLine(OnlyOne), positionWithinLine(OnlyOne),
     toolBarArea(Qt::TopToolBarArea), features(None), lineWidth(0), midLineWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionToolBar::QStyleOptionToolBar(int version)
   : QStyleOption(version, SO_ToolBar), positionOfLine(OnlyOne), positionWithinLine(OnlyOne),
     toolBarArea(Qt::TopToolBarArea), features(None), lineWidth(0), midLineWidth(0)
{
}

#endif

#ifndef QT_NO_TABBAR

QStyleOptionTab::QStyleOptionTab()
   : QStyleOption(QStyleOptionTab::Version, SO_Tab),
     shape(QTabBar::RoundedNorth),
     row(0),
     position(Beginning),
     selectedPosition(NotAdjacent), cornerWidgets(QStyleOptionTab::NoCornerWidgets)
{
}

/*!
    \internal
*/
QStyleOptionTab::QStyleOptionTab(int version)
   : QStyleOption(version, SO_Tab),
     shape(QTabBar::RoundedNorth),
     row(0),
     position(Beginning),
     selectedPosition(NotAdjacent), cornerWidgets(QStyleOptionTab::NoCornerWidgets)
{
}

QStyleOptionTabV2::QStyleOptionTabV2()
   : QStyleOptionTab(Version)
{
}

/*!
    \internal
*/
QStyleOptionTabV2::QStyleOptionTabV2(int version)
   : QStyleOptionTab(version)
{
}

QStyleOptionTabV2::QStyleOptionTabV2(const QStyleOptionTab &other)
   : QStyleOptionTab(Version)
{
   if (const QStyleOptionTabV2 *tab = qstyleoption_cast<const QStyleOptionTabV2 *>(&other)) {
      *this = *tab;
   } else {
      *((QStyleOptionTab *)this) = other;
      version = Version;
   }
}

QStyleOptionTabV2 &QStyleOptionTabV2::operator=(const QStyleOptionTab &other)
{
   QStyleOptionTab::operator=(other);

   if (const QStyleOptionTabV2 *tab = qstyleoption_cast<const QStyleOptionTabV2 *>(&other)) {
      iconSize = tab->iconSize;
   } else {
      iconSize = QSize();
   }
   return *this;
}

/*!
    Constructs a QStyleOptionTabV3.
*/

QStyleOptionTabV3::QStyleOptionTabV3()
   : QStyleOptionTabV2(Version)
   , documentMode(false)
{
}

/*!
    \internal
*/
QStyleOptionTabV3::QStyleOptionTabV3(int version)
   : QStyleOptionTabV2(version)
{
}

QStyleOptionTabV3::QStyleOptionTabV3(const QStyleOptionTab &other)
   : QStyleOptionTabV2(Version)
{
   if (const QStyleOptionTabV3 *tab = qstyleoption_cast<const QStyleOptionTabV3 *>(&other)) {
      *this = *tab;
   } else {
      *((QStyleOptionTabV2 *)this) = other;
      version = Version;
   }
}

QStyleOptionTabV3 &QStyleOptionTabV3::operator=(const QStyleOptionTab &other)
{
   QStyleOptionTabV2::operator=(other);

   if (const QStyleOptionTabV3 *tab = qstyleoption_cast<const QStyleOptionTabV3 *>(&other)) {
      leftButtonSize = tab->leftButtonSize;
      rightButtonSize = tab->rightButtonSize;
   } else {
      leftButtonSize = QSize();
      rightButtonSize = QSize();
      documentMode = false;
   }
   return *this;
}

#endif // QT_NO_TABBAR

QStyleOptionProgressBar::QStyleOptionProgressBar()
   : QStyleOption(QStyleOptionProgressBar::Version, SO_ProgressBar),
     minimum(0), maximum(0), progress(0), textAlignment(Qt::AlignLeft), textVisible(false)
{
}

/*!
    \internal
*/
QStyleOptionProgressBar::QStyleOptionProgressBar(int version)
   : QStyleOption(version, SO_ProgressBar),
     minimum(0), maximum(0), progress(0), textAlignment(Qt::AlignLeft), textVisible(false)
{
}

QStyleOptionProgressBarV2::QStyleOptionProgressBarV2()
   : QStyleOptionProgressBar(2),
     orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
}

/*!
    \internal
*/
QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(int version)
   : QStyleOptionProgressBar(version),
     orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
}

QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(const QStyleOptionProgressBar &other)
   : QStyleOptionProgressBar(2), orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
   const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(&other);
   if (pb2) {
      *this = *pb2;
   } else {
      *((QStyleOptionProgressBar *)this) = other;
   }
}

QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(const QStyleOptionProgressBarV2 &other)
   : QStyleOptionProgressBar(2), orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
   *this = other;
}

QStyleOptionProgressBarV2 &QStyleOptionProgressBarV2::operator=(const QStyleOptionProgressBar &other)
{
   QStyleOptionProgressBar::operator=(other);

   const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(&other);
   orientation = pb2 ? pb2->orientation : Qt::Horizontal;
   invertedAppearance = pb2 ? pb2->invertedAppearance : false;
   bottomToTop = pb2 ? pb2->bottomToTop : false;
   return *this;
}

QStyleOptionMenuItem::QStyleOptionMenuItem()
   : QStyleOption(QStyleOptionMenuItem::Version, SO_MenuItem), menuItemType(Normal),
     checkType(NotCheckable), checked(false), menuHasCheckableItems(true), maxIconWidth(0), tabWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionMenuItem::QStyleOptionMenuItem(int version)
   : QStyleOption(version, SO_MenuItem), menuItemType(Normal),
     checkType(NotCheckable), checked(false), menuHasCheckableItems(true), maxIconWidth(0), tabWidth(0)
{
}

QStyleOptionComplex::QStyleOptionComplex(int version, int type)
   : QStyleOption(version, type), subControls(QStyle::SC_All), activeSubControls(QStyle::SC_None)
{
}

#ifndef QT_NO_SLIDER

QStyleOptionSlider::QStyleOptionSlider()
   : QStyleOptionComplex(Version, SO_Slider), orientation(Qt::Horizontal), minimum(0), maximum(0),
     tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
     sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0), notchTarget(0.0),
     dialWrapping(false)
{
}

/*!
    \internal
*/
QStyleOptionSlider::QStyleOptionSlider(int version)
   : QStyleOptionComplex(version, SO_Slider), orientation(Qt::Horizontal), minimum(0), maximum(0),
     tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
     sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0), notchTarget(0.0),
     dialWrapping(false)
{
}

#endif // QT_NO_SLIDER

#ifndef QT_NO_SPINBOX

QStyleOptionSpinBox::QStyleOptionSpinBox()
   : QStyleOptionComplex(Version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
     stepEnabled(QAbstractSpinBox::StepNone), frame(false)
{
}

/*!
    \internal
*/
QStyleOptionSpinBox::QStyleOptionSpinBox(int version)
   : QStyleOptionComplex(version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
     stepEnabled(QAbstractSpinBox::StepNone), frame(false)
{
}

#endif // QT_NO_SPINBOX

QStyleOptionDockWidget::QStyleOptionDockWidget()
   : QStyleOption(Version, SO_DockWidget), closable(false),
     movable(false), floatable(false)
{
}

/*!
    \internal
*/
QStyleOptionDockWidget::QStyleOptionDockWidget(int version)
   : QStyleOption(version, SO_DockWidget), closable(false),
     movable(false), floatable(false)
{
}

QStyleOptionDockWidgetV2::QStyleOptionDockWidgetV2()
   : QStyleOptionDockWidget(Version), verticalTitleBar(false)
{
}

QStyleOptionDockWidgetV2::QStyleOptionDockWidgetV2(
   const QStyleOptionDockWidget &other)
   : QStyleOptionDockWidget(Version)
{
   (void)QStyleOptionDockWidgetV2::operator=(other);
}

QStyleOptionDockWidgetV2 &QStyleOptionDockWidgetV2::operator = (
   const QStyleOptionDockWidget &other)
{
   QStyleOptionDockWidget::operator=(other);
   const QStyleOptionDockWidgetV2 *v2
      = qstyleoption_cast<const QStyleOptionDockWidgetV2 *>(&other);
   verticalTitleBar = v2 ? v2->verticalTitleBar : false;
   return *this;
}

QStyleOptionDockWidgetV2::QStyleOptionDockWidgetV2(int version)
   : QStyleOptionDockWidget(version), verticalTitleBar(false)
{
}

QStyleOptionToolButton::QStyleOptionToolButton()
   : QStyleOptionComplex(Version, SO_ToolButton), features(None), arrowType(Qt::DownArrow)
   , toolButtonStyle(Qt::ToolButtonIconOnly)
{
}

/*!
    \internal
*/
QStyleOptionToolButton::QStyleOptionToolButton(int version)
   : QStyleOptionComplex(version, SO_ToolButton), features(None), arrowType(Qt::DownArrow)
   , toolButtonStyle(Qt::ToolButtonIconOnly)

{
}

QStyleOptionComboBox::QStyleOptionComboBox()
   : QStyleOptionComplex(Version, SO_ComboBox), editable(false), frame(true)
{
}

/*!
    \internal
*/
QStyleOptionComboBox::QStyleOptionComboBox(int version)
   : QStyleOptionComplex(version, SO_ComboBox), editable(false), frame(true)
{
}

QStyleOptionToolBox::QStyleOptionToolBox()
   : QStyleOption(Version, SO_ToolBox)
{
}

/*!
    \internal
*/
QStyleOptionToolBox::QStyleOptionToolBox(int version)
   : QStyleOption(version, SO_ToolBox)
{
}

QStyleOptionToolBoxV2::QStyleOptionToolBoxV2()
   : QStyleOptionToolBox(Version), position(Beginning), selectedPosition(NotAdjacent)
{
}

/*!
    \internal
*/
QStyleOptionToolBoxV2::QStyleOptionToolBoxV2(int version)
   : QStyleOptionToolBox(version), position(Beginning), selectedPosition(NotAdjacent)
{
}

QStyleOptionToolBoxV2::QStyleOptionToolBoxV2(const QStyleOptionToolBox &other)
{
   QStyleOptionToolBox::operator=(other);

   const QStyleOptionToolBoxV2 *f2 = qstyleoption_cast<const QStyleOptionToolBoxV2 *>(&other);
   position = f2 ? f2->position : Beginning;
   selectedPosition = f2 ? f2->selectedPosition : NotAdjacent;
   version = Version;
}

QStyleOptionToolBoxV2 &QStyleOptionToolBoxV2::operator=(const QStyleOptionToolBox &other)
{
   QStyleOptionToolBox::operator=(other);

   const QStyleOptionToolBoxV2 *f2 = qstyleoption_cast<const QStyleOptionToolBoxV2 *>(&other);
   position = f2 ? f2->position : Beginning;
   selectedPosition = f2 ? f2->selectedPosition : NotAdjacent;
   version = Version;
   return *this;
}


#ifndef QT_NO_RUBBERBAND

QStyleOptionRubberBand::QStyleOptionRubberBand()
   : QStyleOption(Version, SO_RubberBand), shape(QRubberBand::Line), opaque(false)
{
}

// internal
QStyleOptionRubberBand::QStyleOptionRubberBand(int version)
   : QStyleOption(version, SO_RubberBand), shape(QRubberBand::Line), opaque(false)
{
}

#endif // QT_NO_RUBBERBAND

QStyleOptionTitleBar::QStyleOptionTitleBar()
   : QStyleOptionComplex(Version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}

// internal
QStyleOptionTitleBar::QStyleOptionTitleBar(int version)
   : QStyleOptionComplex(version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}

QStyleOptionViewItem::QStyleOptionViewItem()
   : QStyleOption(Version, SO_ViewItem),
     displayAlignment(Qt::AlignLeft), decorationAlignment(Qt::AlignLeft),
     textElideMode(Qt::ElideMiddle), decorationPosition(Left),
     showDecorationSelected(false)
{
}

// internal
QStyleOptionViewItem::QStyleOptionViewItem(int version)
   : QStyleOption(version, SO_ViewItem),
     displayAlignment(Qt::AlignLeft), decorationAlignment(Qt::AlignLeft),
     textElideMode(Qt::ElideMiddle), decorationPosition(Left),
     showDecorationSelected(false)
{
}

#ifndef QT_NO_TABWIDGET

QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame()
   : QStyleOption(Version, SO_TabWidgetFrame), lineWidth(0), midLineWidth(0),
     shape(QTabBar::RoundedNorth)
{
}

// internal
QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame(int version)
   : QStyleOption(version, SO_TabWidgetFrame), lineWidth(0), midLineWidth(0),
     shape(QTabBar::RoundedNorth)
{
}

QStyleOptionTabWidgetFrameV2::QStyleOptionTabWidgetFrameV2()
   : QStyleOptionTabWidgetFrame(Version)
{
}

// internal
QStyleOptionTabWidgetFrameV2::QStyleOptionTabWidgetFrameV2(int version)
   : QStyleOptionTabWidgetFrame(version)
{
}


QStyleOptionTabWidgetFrameV2::QStyleOptionTabWidgetFrameV2(const QStyleOptionTabWidgetFrame &other)
{
   QStyleOptionTabWidgetFrameV2::operator=(other);

}

QStyleOptionTabWidgetFrameV2 &QStyleOptionTabWidgetFrameV2::operator=(const QStyleOptionTabWidgetFrame &other)
{
   QStyleOptionTabWidgetFrame::operator=(other);
   if (const QStyleOptionTabWidgetFrameV2 *f2 = qstyleoption_cast<const QStyleOptionTabWidgetFrameV2 *>(&other)) {
      selectedTabRect = f2->selectedTabRect;
      tabBarRect = f2->tabBarRect;
   }
   return *this;
}

#endif // QT_NO_TABWIDGET

#ifndef QT_NO_TABBAR

QStyleOptionTabBarBase::QStyleOptionTabBarBase()
   : QStyleOption(Version, SO_TabBarBase), shape(QTabBar::RoundedNorth)
{
}

// internal
QStyleOptionTabBarBase::QStyleOptionTabBarBase(int version)
   : QStyleOption(version, SO_TabBarBase), shape(QTabBar::RoundedNorth)
{
}

QStyleOptionTabBarBaseV2::QStyleOptionTabBarBaseV2()
   : QStyleOptionTabBarBase(Version)
   , documentMode(false)
{
}

QStyleOptionTabBarBaseV2::QStyleOptionTabBarBaseV2(const QStyleOptionTabBarBase &other)
   : QStyleOptionTabBarBase(Version)
{
   (void)QStyleOptionTabBarBaseV2::operator=(other);
}


QStyleOptionTabBarBaseV2 &QStyleOptionTabBarBaseV2::operator = (const QStyleOptionTabBarBase &other)
{
   QStyleOptionTabBarBase::operator=(other);
   const QStyleOptionTabBarBaseV2 *v2 = qstyleoption_cast<const QStyleOptionTabBarBaseV2 *>(&other);
   documentMode = v2 ? v2->documentMode : false;
   return *this;
}

/*! \internal */
QStyleOptionTabBarBaseV2::QStyleOptionTabBarBaseV2(int version)
   : QStyleOptionTabBarBase(version), documentMode(false)
{
}

#endif // QT_NO_TABBAR

#ifndef QT_NO_SIZEGRIP

QStyleOptionSizeGrip::QStyleOptionSizeGrip()
   : QStyleOptionComplex(Version, Type), corner(Qt::BottomRightCorner)
{
}

// internal
QStyleOptionSizeGrip::QStyleOptionSizeGrip(int version)
   : QStyleOptionComplex(version, Type), corner(Qt::BottomRightCorner)
{
}

#endif // QT_NO_SIZEGRIP

QStyleOptionGraphicsItem::QStyleOptionGraphicsItem()
   : QStyleOption(Version, Type), levelOfDetail(1)
{
}

// internal
QStyleOptionGraphicsItem::QStyleOptionGraphicsItem(int version)
   : QStyleOption(version, Type), levelOfDetail(1)
{
}

qreal QStyleOptionGraphicsItem::levelOfDetailFromTransform(const QTransform &worldTransform)
{
   if (worldTransform.type() <= QTransform::TxTranslate) {
      return 1;   // Translation only? The LOD is 1.
   }

   // Two unit vectors.
   QLineF v1(0, 0, 1, 0);
   QLineF v2(0, 0, 0, 1);
   // LOD is the transformed area of a 1x1 rectangle.
   return qSqrt(worldTransform.map(v1).length() * worldTransform.map(v2).length());
}

QStyleHintReturn::QStyleHintReturn(int version, int type)
   : version(version), type(type)
{
}

QStyleHintReturn::~QStyleHintReturn()
{

}

QStyleHintReturnMask::QStyleHintReturnMask() : QStyleHintReturn(Version, Type)
{
}

QStyleHintReturnVariant::QStyleHintReturnVariant() : QStyleHintReturn(Version, Type)
{
}

QDebug operator<<(QDebug debug, const QStyleOption::OptionType &optionType)
{
#if ! defined(QT_NO_DEBUG)

   switch (optionType) {
      case QStyleOption::SO_Default:
         debug << "SO_Default";
         break;
      case QStyleOption::SO_FocusRect:
         debug << "SO_FocusRect";
         break;
      case QStyleOption::SO_Button:
         debug << "SO_Button";
         break;
      case QStyleOption::SO_Tab:
         debug << "SO_Tab";
         break;
      case QStyleOption::SO_MenuItem:
         debug << "SO_MenuItem";
         break;
      case QStyleOption::SO_Frame:
         debug << "SO_Frame";
         break;
      case QStyleOption::SO_ProgressBar:
         debug << "SO_ProgressBar";
         break;
      case QStyleOption::SO_ToolBox:
         debug << "SO_ToolBox";
         break;
      case QStyleOption::SO_Header:
         debug << "SO_Header";
         break;

      case QStyleOption::SO_DockWidget:
         debug << "SO_DockWidget";
         break;

      case QStyleOption::SO_ViewItem:
         debug << "SO_ViewItem";
         break;
      case QStyleOption::SO_TabWidgetFrame:
         debug << "SO_TabWidgetFrame";
         break;
      case QStyleOption::SO_TabBarBase:
         debug << "SO_TabBarBase";
         break;
      case QStyleOption::SO_RubberBand:
         debug << "SO_RubberBand";
         break;
      case QStyleOption::SO_Complex:
         debug << "SO_Complex";
         break;
      case QStyleOption::SO_Slider:
         debug << "SO_Slider";
         break;
      case QStyleOption::SO_SpinBox:
         debug << "SO_SpinBox";
         break;
      case QStyleOption::SO_ToolButton:
         debug << "SO_ToolButton";
         break;
      case QStyleOption::SO_ComboBox:
         debug << "SO_ComboBox";
         break;

      case QStyleOption::SO_TitleBar:
         debug << "SO_TitleBar";
         break;
      case QStyleOption::SO_CustomBase:
         debug << "SO_CustomBase";
         break;
      case QStyleOption::SO_GroupBox:
         debug << "SO_GroupBox";
         break;
      case QStyleOption::SO_ToolBar:
         debug << "SO_ToolBar";
         break;
      case QStyleOption::SO_ComplexCustomBase:
         debug << "SO_ComplexCustomBase";
         break;
      case QStyleOption::SO_SizeGrip:
         debug << "SO_SizeGrip";
         break;
      case QStyleOption::SO_GraphicsItem:
         debug << "SO_GraphicsItem";
         break;
   }
#endif

   return debug;
}

QDebug operator<<(QDebug debug, const QStyleOption &option)
{
#if ! defined(QT_NO_DEBUG)
   debug << "QStyleOption(";
   debug << QStyleOption::OptionType(option.type);
   debug << ',' << (option.direction == Qt::RightToLeft ? "RightToLeft" : "LeftToRight");
   debug << ',' << option.state;
   debug << ',' << option.rect;
   debug << ')';
#endif

   return debug;
}
