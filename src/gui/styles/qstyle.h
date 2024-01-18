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

#ifndef QSTYLE_H
#define QSTYLE_H

#include <qobject.h>
#include <qrect.h>
#include <qsize.h>
#include <qicon.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qsizepolicy.h>
#include <qscopedpointer.h>

class QAction;
class QDebug;
class QTab;
class QFontMetrics;
class QStyleHintReturn;
class QStyleOption;
class QStyleOptionComplex;
class QStylePrivate;

class Q_GUI_EXPORT QStyle : public QObject
{
   GUI_CS_OBJECT(QStyle)
   Q_DECLARE_PRIVATE(QStyle)

   GUI_CS_ENUM(StandardPixmap)

 public:
  enum StateFlag {
      State_None =                0x00000000,
      State_Enabled =             0x00000001,
      State_Raised =              0x00000002,
      State_Sunken =              0x00000004,
      State_Off =                 0x00000008,
      State_NoChange =            0x00000010,
      State_On =                  0x00000020,
      State_DownArrow =           0x00000040,
      State_Horizontal =          0x00000080,
      State_HasFocus =            0x00000100,

//    State_Top =                 0x00000200,         // unused, undocumented
//    State_Bottom =              0x00000400,         // unused, undocumented
      State_FocusAtBorder =       0x00000800,

      State_AutoRaise =           0x00001000,
      State_MouseOver =           0x00002000,
      State_UpArrow =             0x00004000,
      State_Selected =            0x00008000,
      State_Active =              0x00010000,
      State_Window =              0x00020000,         // undocumented
      State_Open =                0x00040000,
      State_Children =            0x00080000,
      State_Item =                0x00100000,
      State_Sibling =             0x00200000,
      State_Editing =             0x00400000,
      State_KeyboardFocusChange = 0x00800000,
#ifdef QT_KEYPAD_NAVIGATION
      State_HasEditFocus =        0x01000000,
#endif
      State_ReadOnly =            0x02000000,
      State_Small =               0x04000000,
      State_Mini =                0x08000000
   };
   using State = QFlags<StateFlag>;

   enum PrimitiveElement {
      PE_Frame,
      PE_FrameDefaultButton,
      PE_FrameDockWidget,
      PE_FrameFocusRect,
      PE_FrameGroupBox,
      PE_FrameLineEdit,
      PE_FrameMenu,

      PE_FrameStatusBarItem,                // 7
      PE_FrameTabWidget,
      PE_FrameWindow,
      PE_FrameButtonBevel,
      PE_FrameButtonTool,
      PE_FrameTabBarBase,

      PE_PanelButtonCommand,                // 13
      PE_PanelButtonBevel,
      PE_PanelButtonTool,
      PE_PanelMenuBar,
      PE_PanelToolBar,
      PE_PanelLineEdit,

      PE_IndicatorArrowDown,                // 19
      PE_IndicatorArrowLeft,
      PE_IndicatorArrowRight,
      PE_IndicatorArrowUp,
      PE_IndicatorBranch,
      PE_IndicatorButtonDropDown,

      PE_IndicatorViewItemCheck,            // obsolete
      PE_IndicatorItemViewItemCheck = PE_IndicatorViewItemCheck,

      PE_IndicatorCheckBox,                 // 26
      PE_IndicatorDockWidgetResizeHandle,
      PE_IndicatorHeaderArrow,
      PE_IndicatorMenuCheckMark,
      PE_IndicatorProgressChunk,
      PE_IndicatorRadioButton,
      PE_IndicatorSpinDown,
      PE_IndicatorSpinMinus,
      PE_IndicatorSpinPlus,
      PE_IndicatorSpinUp,                   // 35
      PE_IndicatorToolBarHandle,
      PE_IndicatorToolBarSeparator,
      PE_PanelTipLabel,
      PE_IndicatorTabTear,
      PE_PanelScrollAreaCorner,

      PE_Widget,

      PE_IndicatorColumnViewArrow,          // 42
      PE_IndicatorItemViewItemDrop,

      PE_PanelItemViewItem,
      PE_PanelItemViewRow,
      PE_PanelStatusBar,

      PE_IndicatorTabClose,
      PE_PanelMenu,                         // 48

      // do not add any values below/greater this
      PE_CustomBase = 0xf000000
   };

   enum ControlElement {
      CE_CheckBox,
      CE_CheckBoxLabel,

      CE_ColumnViewGrip,

      CE_ComboBoxLabel,

      CE_DockWidgetTitle,

      CE_FocusFrame,

      CE_Header,
      CE_HeaderEmptyArea,
      CE_HeaderLabel,
      CE_HeaderSection,

      CE_ItemViewItem,                      // 10

      CE_MenuBarItem,
      CE_MenuBarEmptyArea,

      CE_MenuEmptyArea,
      CE_MenuHMargin,
      CE_MenuItem,
      CE_MenuTearoff,
      CE_MenuScroller,
      CE_MenuVMargin,

      CE_ProgressBar,                       // 19
      CE_ProgressBarGroove,
      CE_ProgressBarContents,
      CE_ProgressBarLabel,

      CE_PushButton,
      CE_PushButtonBevel,
      CE_PushButtonLabel,

      CE_RadioButton,
      CE_RadioButtonLabel,

      CE_RubberBand,

      CE_ScrollBarAddLine,                  // 29
      CE_ScrollBarAddPage,
      CE_ScrollBarFirst,
      CE_ScrollBarLast,
      CE_ScrollBarSlider,
      CE_ScrollBarSubLine,
      CE_ScrollBarSubPage,

      CE_ShapedFrame,
      CE_SizeGrip,
      CE_Splitter,

      CE_TabBarTab,                         // 39
      CE_TabBarTabShape,
      CE_TabBarTabLabel,

      CE_ToolBar,

      CE_ToolBoxTab,                        // 43
      CE_ToolBoxTabShape,
      CE_ToolBoxTabLabel,

      CE_ToolButtonLabel,

      // do not add any values below/greater than this
      CE_CustomBase = 0xf0000000
   };

   enum SubElement {
      SE_PushButtonContents,
      SE_PushButtonFocusRect,

      SE_CheckBoxIndicator,
      SE_CheckBoxContents,
      SE_CheckBoxFocusRect,
      SE_CheckBoxClickRect,

      SE_RadioButtonIndicator,
      SE_RadioButtonContents,
      SE_RadioButtonFocusRect,
      SE_RadioButtonClickRect,

      SE_ComboBoxFocusRect,

      SE_SliderFocusRect,

      SE_ProgressBarGroove,
      SE_ProgressBarContents,
      SE_ProgressBarLabel,

      SE_ToolBoxTabContents,

      SE_HeaderLabel,                       // 17
      SE_HeaderArrow,

      SE_TabWidgetTabBar,
      SE_TabWidgetTabPane,
      SE_TabWidgetTabContents,
      SE_TabWidgetLeftCorner,
      SE_TabWidgetRightCorner,

      SE_ItemViewItemCheckIndicator,

      SE_TabBarTearIndicator,

      SE_TreeViewDisclosureItem,

      SE_LineEditContents,
      SE_FrameContents,

      SE_DockWidgetCloseButton,             // 28
      SE_DockWidgetFloatButton,
      SE_DockWidgetTitleBarText,
      SE_DockWidgetIcon,

      SE_CheckBoxLayoutItem,
      SE_ComboBoxLayoutItem,
      SE_DateTimeEditLayoutItem,
      SE_LabelLayoutItem,
      SE_ProgressBarLayoutItem,
      SE_PushButtonLayoutItem,
      SE_RadioButtonLayoutItem,
      SE_SliderLayoutItem,
      SE_SpinBoxLayoutItem,
      SE_ToolButtonLayoutItem,

      SE_FrameLayoutItem,                   // 42
      SE_GroupBoxLayoutItem,
      SE_TabWidgetLayoutItem,

      SE_ItemViewItemDecoration,
      SE_ItemViewItemText,
      SE_ItemViewItemFocusRect,

      SE_TabBarTabLeftButton,
      SE_TabBarTabRightButton,
      SE_TabBarTabText,

      SE_ShapedFrameContents,

      SE_ToolBarHandle,

      // do not add any values below/greater than this
      SE_CustomBase = 0xf0000000
   };

   enum ComplexControl {
      CC_SpinBox,
      CC_ComboBox,
      CC_ScrollBar,
      CC_Slider,
      CC_ToolButton,
      CC_TitleBar,
      CC_Dial,
      CC_GroupBox,
      CC_MdiControls,

      // do not add any values below/greater than this
      CC_CustomBase = 0xf0000000
   };

   enum SubControl {
      SC_None =                  0x00000000,

      SC_ScrollBarAddLine =      0x00000001,
      SC_ScrollBarSubLine =      0x00000002,
      SC_ScrollBarAddPage =      0x00000004,
      SC_ScrollBarSubPage =      0x00000008,
      SC_ScrollBarFirst =        0x00000010,
      SC_ScrollBarLast =         0x00000020,
      SC_ScrollBarSlider =       0x00000040,
      SC_ScrollBarGroove =       0x00000080,

      SC_SpinBoxUp =             0x00000001,
      SC_SpinBoxDown =           0x00000002,
      SC_SpinBoxFrame =          0x00000004,
      SC_SpinBoxEditField =      0x00000008,

      SC_ComboBoxFrame =         0x00000001,
      SC_ComboBoxEditField =     0x00000002,
      SC_ComboBoxArrow =         0x00000004,
      SC_ComboBoxListBoxPopup =  0x00000008,

      SC_SliderGroove =          0x00000001,
      SC_SliderHandle =          0x00000002,
      SC_SliderTickmarks =       0x00000004,

      SC_ToolButton =            0x00000001,
      SC_ToolButtonMenu =        0x00000002,

      SC_TitleBarSysMenu =       0x00000001,
      SC_TitleBarMinButton =     0x00000002,
      SC_TitleBarMaxButton =     0x00000004,
      SC_TitleBarCloseButton =   0x00000008,
      SC_TitleBarNormalButton =  0x00000010,
      SC_TitleBarShadeButton =   0x00000020,
      SC_TitleBarUnshadeButton = 0x00000040,
      SC_TitleBarContextHelpButton = 0x00000080,
      SC_TitleBarLabel =         0x00000100,

      SC_DialGroove =            0x00000001,
      SC_DialHandle =            0x00000002,
      SC_DialTickmarks =         0x00000004,

      SC_GroupBoxCheckBox =      0x00000001,
      SC_GroupBoxLabel =         0x00000002,
      SC_GroupBoxContents =      0x00000004,
      SC_GroupBoxFrame =         0x00000008,

      SC_MdiMinButton     =      0x00000001,
      SC_MdiNormalButton  =      0x00000002,
      SC_MdiCloseButton   =      0x00000004,

      SC_CustomBase =            0xf0000000,
      SC_All =                   0xffffffff
   };
   using SubControls = QFlags<SubControl>;

   enum PixelMetric {
      PM_ButtonMargin,
      PM_ButtonDefaultIndicator,
      PM_MenuButtonIndicator,
      PM_ButtonShiftHorizontal,
      PM_ButtonShiftVertical,

      PM_DefaultFrameWidth,                 // 5
      PM_SpinBoxFrameWidth,
      PM_ComboBoxFrameWidth,

      PM_MaximumDragDistance,

      PM_ScrollBarExtent,
      PM_ScrollBarSliderMin,

      PM_SliderThickness,                   // total slider thickness
      PM_SliderControlThickness,            // thickness of the business part
      PM_SliderLength,                      // total length of slider
      PM_SliderTickmarkOffset,
      PM_SliderSpaceAvailable,              // available space for slider to move

      PM_DockWidgetSeparatorExtent,         // 16
      PM_DockWidgetHandleExtent,
      PM_DockWidgetFrameWidth,

      PM_TabBarTabOverlap,
      PM_TabBarTabHSpace,
      PM_TabBarTabVSpace,
      PM_TabBarBaseHeight,
      PM_TabBarBaseOverlap,

      PM_ProgressBarChunkWidth,             // 24

      PM_SplitterWidth,
      PM_TitleBarHeight,

      PM_MenuScrollerHeight,
      PM_MenuHMargin,
      PM_MenuVMargin,
      PM_MenuPanelWidth,
      PM_MenuTearoffHeight,
      PM_MenuDesktopFrameWidth,

      PM_MenuBarPanelWidth,
      PM_MenuBarItemSpacing,
      PM_MenuBarVMargin,
      PM_MenuBarHMargin,

      PM_IndicatorWidth,
      PM_IndicatorHeight,
      PM_ExclusiveIndicatorWidth,
      PM_ExclusiveIndicatorHeight,          // 40

      PM_DialogButtonsSeparator,
      PM_DialogButtonsButtonWidth,
      PM_DialogButtonsButtonHeight,

      PM_MdiSubWindowFrameWidth,
      PM_MdiSubWindowMinimizedWidth,        // 45

      PM_MDIFrameWidth     = PM_MdiSubWindowFrameWidth,        // obsolete, 44
      PM_MDIMinimizedWidth = PM_MdiSubWindowMinimizedWidth,    // obsolete, 45

      PM_HeaderMargin,                      // 46
      PM_HeaderMarkSize,
      PM_HeaderGripMargin,
      PM_TabBarTabShiftHorizontal,
      PM_TabBarTabShiftVertical,
      PM_TabBarScrollButtonWidth,

      PM_ToolBarFrameWidth,                 // 52
      PM_ToolBarHandleExtent,
      PM_ToolBarItemSpacing,
      PM_ToolBarItemMargin,
      PM_ToolBarSeparatorExtent,
      PM_ToolBarExtensionExtent,

      PM_SpinBoxSliderHeight,               // 58

      PM_DefaultTopLevelMargin,             // obsolete
      PM_DefaultChildMargin,                // obsolete
      PM_DefaultLayoutSpacing,              // obsolete

      PM_ToolBarIconSize,
      PM_ListViewIconSize,
      PM_IconViewIconSize,
      PM_SmallIconSize,
      PM_LargeIconSize,

      PM_FocusFrameVMargin,
      PM_FocusFrameHMargin,

      PM_ToolTipLabelFrameWidth,
      PM_CheckBoxLabelSpacing,
      PM_TabBarIconSize,
      PM_SizeGripSize,
      PM_DockWidgetTitleMargin,
      PM_MessageBoxIconSize,
      PM_ButtonIconSize,

      PM_DockWidgetTitleBarButtonMargin,    // 76

      PM_RadioButtonLabelSpacing,
      PM_LayoutLeftMargin,
      PM_LayoutTopMargin,
      PM_LayoutRightMargin,
      PM_LayoutBottomMargin,
      PM_LayoutHorizontalSpacing,
      PM_LayoutVerticalSpacing,
      PM_TabBar_ScrollButtonOverlap,

      PM_TextCursorWidth,

      PM_TabCloseIndicatorWidth,
      PM_TabCloseIndicatorHeight,           // 87

      PM_ScrollView_ScrollBarSpacing,
      PM_ScrollView_ScrollBarOverlap,
      PM_SubMenuOverlap,
      PM_TreeViewIndentation,

      PM_HeaderDefaultSectionSizeHorizontal,
      PM_HeaderDefaultSectionSizeVertical,  // 93

      // do not add any values below/greater than this
      PM_CustomBase = 0xf0000000
   };

   enum ContentsType {
      CT_CheckBox,
      CT_ComboBox,
      CT_GroupBox,
      CT_HeaderSection,
      CT_ItemViewItem,
      CT_LineEdit,
      CT_MdiControls,
      CT_Menu,
      CT_MenuBar,
      CT_MenuBarItem,
      CT_MenuItem,
      CT_ProgressBar,
      CT_PushButton,
      CT_RadioButton,
      CT_ScrollBar,
      CT_SizeGrip,
      CT_Slider,
      CT_SpinBox,
      CT_Splitter,
      CT_TabBarTab,
      CT_TabWidget,
      CT_ToolButton,

      // do not add any values below/greater than this
      CT_CustomBase = 0xf0000000
   };

   enum RequestSoftwareInputPanel {
      RSIP_OnMouseClickAndAlreadyFocused,
      RSIP_OnMouseClick
   };

   enum StyleHint {
      SH_EtchDisabledText,
      SH_DitherDisabledText,
      SH_ScrollBar_MiddleClickAbsolutePosition,
      SH_ScrollBar_ScrollWhenPointerLeavesControl,
      SH_TabBar_SelectMouseType,
      SH_TabBar_Alignment,
      SH_Header_ArrowAlignment,
      SH_Slider_SnapToValue,
      SH_Slider_SloppyKeyEvents,
      SH_ProgressDialog_CenterCancelButton,
      SH_ProgressDialog_TextLabelAlignment,
      SH_PrintDialog_RightAlignButtons,
      SH_MainWindow_SpaceBelowMenuBar,
      SH_FontDialog_SelectAssociatedText,
      SH_Menu_AllowActiveAndDisabled,
      SH_Menu_SpaceActivatesItem,
      SH_Menu_SubMenuPopupDelay,
      SH_ScrollView_FrameOnlyAroundContents,
      SH_MenuBar_AltKeyNavigation,
      SH_ComboBox_ListMouseTracking,

      SH_Menu_MouseTracking,                          // 20
      SH_MenuBar_MouseTracking,
      SH_ItemView_ChangeHighlightOnFocus,
      SH_Widget_ShareActivation,
      SH_Workspace_FillSpaceOnMaximize,
      SH_ComboBox_Popup,
      SH_TitleBar_NoBorder,

      SH_Slider_StopMouseOverSlider,
      SH_ScrollBar_StopMouseOverSlider = SH_Slider_StopMouseOverSlider, // obsolete

      SH_BlinkCursorWhenTextSelected,
      SH_RichText_FullWidthSelection,
      SH_Menu_Scrollable,                             // 30
      SH_GroupBox_TextLabelVerticalAlignment,
      SH_GroupBox_TextLabelColor,
      SH_Menu_SloppySubMenus,
      SH_Table_GridLineColor,
      SH_LineEdit_PasswordCharacter,
      SH_DialogButtons_DefaultButton,
      SH_ToolBox_SelectedPageTitleBold,
      SH_TabBar_PreferNoArrows,
      SH_ScrollBar_LeftClickAbsolutePosition,
      SH_ListViewExpand_SelectMouseType,
      SH_UnderlineShortcut,
      SH_SpinBox_AnimateButton,
      SH_SpinBox_KeyPressAutoRepeatRate,
      SH_SpinBox_ClickAutoRepeatRate,
      SH_Menu_FillScreenWithScroll,
      SH_ToolTipLabel_Opacity,
      SH_DrawMenuBarSeparator,
      SH_TitleBar_ModifyNotification,
      SH_Button_FocusPolicy,

      SH_MessageBox_UseBorderForButtonSpacing,        // 50
      SH_TitleBar_AutoRaise,
      SH_ToolButton_PopupDelay,
      SH_FocusFrame_Mask,
      SH_RubberBand_Mask,
      SH_WindowFrame_Mask,
      SH_SpinControls_DisableOnBounds,
      SH_Dial_BackgroundRole,
      SH_ComboBox_LayoutDirection,
      SH_ItemView_EllipsisLocation,

      SH_ItemView_ShowDecorationSelected,
      SH_ItemView_ActivateItemOnSingleClick,
      SH_ScrollBar_ContextMenu,                       // 62
      SH_ScrollBar_RollBetweenButtons,
      SH_Slider_AbsoluteSetButtons,
      SH_Slider_PageSetButtons,
      SH_Menu_KeyboardSearch,
      SH_TabBar_ElideMode,
      SH_DialogButtonLayout,
      SH_ComboBox_PopupFrameStyle,
      SH_MessageBox_TextInteractionFlags,             // 70
      SH_DialogButtonBox_ButtonsHaveIcons,
      SH_SpellCheckUnderlineStyle,
      SH_MessageBox_CenterButtons,
      SH_Menu_SelectionWrap,
      SH_ItemView_MovementWithoutUpdatingSelection,
      SH_ToolTip_Mask,
      SH_FocusFrame_AboveWidget,
      SH_TextControl_FocusIndicatorTextCharFormat,
      SH_WizardStyle,
      SH_ItemView_ArrowKeysNavigateIntoChildren,
      SH_Menu_Mask,
      SH_Menu_FlashTriggeredItem,
      SH_Menu_FadeOutOnHide,
      SH_SpinBox_ClickAutoRepeatThreshold,
      SH_ItemView_PaintAlternatingRowColorsForEmptyArea,
      SH_FormLayoutWrapPolicy,
      SH_TabWidget_DefaultTabPosition,
      SH_ToolBar_Movable,
      SH_FormLayoutFieldGrowthPolicy,
      SH_FormLayoutFormAlignment,                     // 90
      SH_FormLayoutLabelAlignment,
      SH_ItemView_DrawDelegateFrame,
      SH_TabBar_CloseButtonPosition,
      SH_DockWidget_ButtonsHaveFrame,
      SH_ToolButtonStyle,
      SH_RequestSoftwareInputPanel,
      SH_ScrollBar_Transient,
      SH_Menu_SupportsSections,
      SH_ToolTip_WakeUpDelay,
      SH_ToolTip_FallAsleepDelay,
      SH_Widget_Animate,
      SH_Splitter_OpaqueResize,

      // Should use a native popup, only supported for non-editable combo boxes on Mac OS X
      SH_ComboBox_UseNativePopup,                     // 103
      SH_LineEdit_PasswordMaskDelay,
      SH_TabBar_ChangeCurrentDelay,
      SH_Menu_SubMenuUniDirection,
      SH_Menu_SubMenuUniDirectionFailCount,
      SH_Menu_SubMenuSloppySelectOtherActions,
      SH_Menu_SubMenuSloppyCloseTimeout,
      SH_Menu_SubMenuResetWhenReenteringParent,
      SH_Menu_SubMenuDontStartSloppyOnLeave,
      SH_CustomBase = 0xf0000000
   };

  enum StandardPixmap {
      SP_TitleBarMinButton,
      SP_TitleBarMenuButton,
      SP_TitleBarMaxButton,
      SP_TitleBarCloseButton,
      SP_TitleBarNormalButton,
      SP_TitleBarShadeButton,
      SP_TitleBarUnshadeButton,
      SP_TitleBarContextHelpButton,
      SP_DockWidgetCloseButton,
      SP_MessageBoxInformation,
      SP_MessageBoxWarning,
      SP_MessageBoxCritical,
      SP_MessageBoxQuestion,
      SP_DesktopIcon,
      SP_TrashIcon,
      SP_ComputerIcon,
      SP_DriveFDIcon,
      SP_DriveHDIcon,
      SP_DriveCDIcon,
      SP_DriveDVDIcon,

      SP_DriveNetIcon,                      // 20
      SP_DirOpenIcon,
      SP_DirClosedIcon,
      SP_DirLinkIcon,
      SP_DirLinkOpenIcon,
      SP_FileIcon,
      SP_FileLinkIcon,
      SP_ToolBarHorizontalExtensionButton,
      SP_ToolBarVerticalExtensionButton,
      SP_FileDialogStart,
      SP_FileDialogEnd,
      SP_FileDialogToParent,
      SP_FileDialogNewFolder,
      SP_FileDialogDetailedView,
      SP_FileDialogInfoView,
      SP_FileDialogContentsView,
      SP_FileDialogListView,
      SP_FileDialogBack,
      SP_DirIcon,
      SP_DialogOkButton,
      SP_DialogCancelButton,
      SP_DialogHelpButton,
      SP_DialogOpenButton,
      SP_DialogSaveButton,
      SP_DialogCloseButton,
      SP_DialogApplyButton,
      SP_DialogResetButton,
      SP_DialogDiscardButton,
      SP_DialogYesButton,
      SP_DialogNoButton,
      SP_ArrowUp,
      SP_ArrowDown,
      SP_ArrowLeft,
      SP_ArrowRight,
      SP_ArrowBack,
      SP_ArrowForward,
      SP_DirHomeIcon,
      SP_CommandLink,
      SP_VistaShield,
      SP_BrowserReload,
      SP_BrowserStop,
      SP_MediaPlay,
      SP_MediaStop,
      SP_MediaPause,
      SP_MediaSkipForward,
      SP_MediaSkipBackward,
      SP_MediaSeekForward,
      SP_MediaSeekBackward,
      SP_MediaVolume,
      SP_MediaVolumeMuted,
      SP_LineEditClearButton,
      // do not add any values below/greater than this
      SP_CustomBase = 0xf0000000
   };

   QStyle();

   QStyle(const QStyle &) = delete;
   QStyle &operator=(const QStyle &) = delete;

   virtual ~QStyle();

   virtual void polish(QWidget *widget);
   virtual void unpolish(QWidget *widget);

   virtual void polish(QApplication *application);
   virtual void unpolish(QApplication *application);

   virtual void polish(QPalette &palette);

   virtual QRect itemTextRect(const QFontMetrics &metrics, const QRect &rect, int alignment, bool enabled, const QString &text) const;
   virtual QRect itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const;

   virtual void drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &palette, bool enabled,
      const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;

   virtual void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;

   virtual QPalette standardPalette() const;

   virtual void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
         const QWidget *widget = nullptr) const = 0;
   virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
         const QWidget *widget = nullptr) const = 0;
   virtual QRect subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget = nullptr) const = 0;

   virtual void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
      const QWidget *widget = nullptr) const = 0;

   virtual SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
      const QPoint &position, const QWidget *widget = nullptr) const = 0;

   virtual QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl,
      const QWidget *widget = nullptr) const = 0;

   virtual int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const = 0;

   virtual QSize sizeFromContents(ContentsType type, const QStyleOption *option,
      const QSize &contentsSize, const QWidget *widget = nullptr) const = 0;

   virtual int styleHint(StyleHint styleHint, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr, QStyleHintReturn *styleHintReturn = nullptr) const = 0;

   virtual QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const = 0;

   virtual QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const = 0;

   virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
      const QStyleOption *option) const = 0;

   static QRect visualRect(Qt::LayoutDirection direction, const QRect &boundingRect, const QRect &logicalRect);
   static QPoint visualPos(Qt::LayoutDirection direction, const QRect &boundingRect, const QPoint &logicalPosition);

   static int sliderPositionFromValue(int min, int max, int logicalValue, int span, bool upsideDown = false);
   static int sliderValueFromPosition(int min, int max, int position, int span, bool upsideDown = false);
   static Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment);

   static QRect alignedRect(Qt::LayoutDirection direction, Qt::Alignment alignment, const QSize &size, const QRect &rect);

   virtual int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
      Qt::Orientation orientation, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const = 0;

   int combinedLayoutSpacing(QSizePolicy::ControlTypes controls1, QSizePolicy::ControlTypes controls2,
      Qt::Orientation orientation, QStyleOption *option = nullptr, QWidget *widget = nullptr) const;

   const QStyle *proxy() const;

 protected:
   QStyle(QStylePrivate &dd);
   QScopedPointer<QStylePrivate> d_ptr;

 private:
   void setProxy(QStyle *style);

   friend class QWidget;
   friend class QWidgetPrivate;
   friend class QApplication;
   friend class QProxyStyle;
   friend class QProxyStylePrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyle::State)
Q_DECLARE_OPERATORS_FOR_FLAGS(QStyle::SubControls)

Q_GUI_EXPORT QDebug operator<<(QDebug debug, QStyle::State state);

#endif
