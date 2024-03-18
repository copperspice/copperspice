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

#ifndef CS_MAC_P_H
#define CS_MAC_P_H

#ifdef __OBJC__
#include <AppKit/AppKit.h>
#endif

#define kThemeAppearanceAquaGraphite CFSTR("com.apple.theme.appearance.aqua.graphite")

enum MacThemeFlags {
   kThemeTabNorth                           = 0,
   kThemeTabSouth                           = 1,
   kThemeTabEast                            = 2,
   kThemeTabWest                            = 3,

   kThemeStateInactive                      = 0,
   kThemeStateActive                        = 1,
   kThemeStatePressed                       = 2,
   kThemeStateUnavailable                   = 7,
   kThemeStateUnavailableInactive           = 8,
   kThemeStatePressedDown                   = 3,
   kThemeStatePressedUp                     = 2,
   kThemeStateRollover                      = 6,

   kThemeAdornmentNone                      = 0,
   kThemeAdornmentFocus                     = 1 << 2,
   kThemeAdornmentArrowLeftArrow            = 1 << 6,

   kThemeAdornmentHeaderButtonLeftNeighborSelected   = 1 << 6,
   kThemeAdornmentHeaderButtonRightNeighborSelected  = 1 << 7,
   kThemeAdornmentHeaderButtonSortUp                 = 1 << 8,
   kThemeAdornmentDefault                            = 1 << 0,

   kThemeScrollBarMedium                    = 0,
   kThemeScrollBarSmall                     = 1,
   kThemeScrollBarMini                      = 9,

   kThemeSliderMedium                       = 2,
   kThemeSliderSmall                        = 6,
   kThemeSliderMini                         = 10,

   kThemeTrackHorizontal                    = 1 << 0,
   kThemeTrackRightToLeft                   = 1 << 1,
   kThemeTrackShowThumb                     = 1 << 2,
   kThemeTrackActive                        = 0,
   kThemeTrackDisabled                      = 1,
   kThemeTrackInactive                      = 3,
   kThemeTrackHasFocus                      = 1 << 5,

   kThemeThumbPlain                         = 0,
   kThemeThumbUpward                        = 1,
   kThemeThumbDownward                      = 2,

   kThemeArrowDown                          = 1,
   kThemeArrow5pt                           = 1,
   kThemeArrow7pt                           = 2,
   kThemeArrow9pt                           = 3,

   kThemeGrowLeft                           = 1 << 0,
   kThemeGrowRight                          = 1 << 1,
   kThemeGrowDown                           = 1 << 3,

   kThemeMenuItemHasIcon                    = 0x8000,
   kThemeMenuItemHierarchical               = 1,
   kThemeMenuItemHierBackground             = 0x0400,
   kThemeMenuItemPopUpBackground            = 0x0800,
   kThemeMenuActive                         = 0,
   kThemeMenuDisabled                       = 3,
   kThemeMenuSelected                       = 1,
   kThemeMenuItemMarkFont                   = 102,
   kThemeMenuItemScrollDownArrow            = 3,
   kThemeMenuItemScrollUpArrow              = 2,
   kThemeMenuBarNormal                      = 0,
   kThemeMenuItemPlain                      = 0,
   kThemeMenuTypeHierarchical               = 2,

   kThemeLargeProgressBar                   = 7,
   kThemeLargeIndeterminateBar              = 8,
   kThemeProgressBar                        = 3,
   kThemeIndeterminateBar                   = 4,

   kThemeLeftOutsideArrowPressed            = 0x01,
   kThemeLeftInsideArrowPressed             = 0x02,
   kThemeLeftTrackPressed                   = 0x04,
   kThemeThumbPressed                       = 0x08,
   kThemeRightTrackPressed                  = 0x10,
   kThemeRightInsideArrowPressed            = 0x20,
   kThemeRightOutsideArrowPressed           = 0x40,

   kThemeTrackNothingToScroll               = 2,
   kThemeTrackHideTrack                     = 1 << 6,
   kThemeIncDecButton                       = 7,
   kThemeIncDecButtonSmall                  = 21,
   kThemeIncDecButtonMini                   = 22,

   kThemeWindowHasTitleText                 = 1 << 7,
   kThemeWindowHasDirty                     = 1 << 9,
   kThemeWindowHasHorizontalZoom            = 1 << 3,
   kThemeWindowHasVerticalZoom              = 1 << 4,
   kThemeWindowHasFullZoom                  = kThemeWindowHasHorizontalZoom + kThemeWindowHasVerticalZoom,
   kThemeWindowHasCloseBox                  = 1 << 5,
   kThemeWindowHasCollapseBox               = 1 << 6,

   kThemeWidgetCollapseBox                  = 2,
   kThemeWidgetCloseBox                     = 0,

   kThemeTabNonFrontPressed                 = 1,
   kThemeTabNonFrontInactive                = 2,
   kThemeTabFront                           = 3,
   kThemeTabFrontInactive                   = 4,
   kThemeTabNonFrontUnavailable             = 5,
   kThemeTabFrontUnavailable                = 6,

   kThemeRadioButton                        = 2,
   kThemeRadioButtonMini                    = 20,
   kThemeRadioButtonSmall                   = 14,

   kThemeCheckBox                           = 1,
   kThemeCheckBoxMini                       = 19,
   kThemeCheckBoxSmall                      = 13,

   kThemeDisclosureButton                   = 6,
   kThemeDisclosureDown                     = 1,
   kThemeDisclosureRight                    = 0,
   kThemeDisclosureLeft                     = 2,

   kThemeDrawIndicatorOnly                  = 1 << 5,
   kThemeButtonMixed                        = 2,
   kThemeMovableDialogWindow                = 2,
   kThemeListHeaderButton                   = 10,
   kThemeDocumentWindow                     = 0,
   kThemeTabNonFront                        = 0,

   kThemeArrowButton                        = 4,

   kThemeBevelButtonSmall                   = 8,
   kThemeBevelButtonLarge                   = 9,

   //
   kThemeTextColorDialogActive              = 1,
   kThemeTextColorDialogInactive            = 2,
   kThemeTextColorAlertActive               = 3,
   kThemeTextColorAlertInactive             = 4,
   kThemeTextColorModelessDialogActive      = 5,
   kThemeTextColorModelessDialogInactive    = 6,
   kThemeTextColorWindowHeaderActive        = 7,
   kThemeTextColorWindowHeaderInactive      = 8,
   kThemeTextColorPlacardActive             = 9,
   kThemeTextColorPlacardInactive           = 10,
   kThemeTextColorPlacardPressed            = 11,
   kThemeTextColorPushButtonActive          = 12,
   kThemeTextColorPushButtonInactive        = 13,
   kThemeTextColorPushButtonPressed         = 14,
   kThemeTextColorBevelButtonActive         = 15,
   kThemeTextColorBevelButtonInactive       = 16,
   kThemeTextColorBevelButtonPressed        = 17,
   kThemeTextColorPopupButtonActive         = 18,
   kThemeTextColorPopupButtonInactive       = 19,
   kThemeTextColorPopupButtonPressed        = 20,
   kThemeTextColorIconLabel                 = 21,
   kThemeTextColorListView                  = 22,

   kThemeTextColorMenuItemActive            = 34,
   kThemeTextColorMenuItemSelected          = 35,
   kThemeTextColorMenuItemDisabled          = 36,
   kThemeTextColorPopupLabelActive          = 37,
   kThemeTextColorPopupLabelInactive        = 38,
   kThemeTextColorTabFrontActive            = 39,
   kThemeTextColorTabFrontInactive          = 42,

   //
   kThemeBrushPrimaryHighlightColor         = -3,
   kThemeBrushSecondaryHighlightColor       = -4,
   kThemeBrushButtonActiveDarkShadow        = 32,
   kThemeBrushButtonInactiveDarkShadow      = 36,
   kThemeBrushDialogBackgroundActive        = 1,

   kThemeMenuTypePopUp                      = 1,

   kThemeButtonOff                          = 0,
   kThemeButtonOn                           = 1,
   kThemePushButton                         = 0,
   kThemePushButtonSmall                    = 26,
   kThemePushButtonMini                     = 27,
   kThemeBevelButton                        = 3,

   kThemeComboBox                           = 16,
   kThemeComboBoxSmall                      = 17,
   kThemeComboBoxMini                       = 18,
   kThemePopupButton                        = 5,
   kThemePopupButtonSmall                   = 29,
   kThemePopupButtonMini                    = 30,

   kThemeSystemFont                         = 0,
   kThemeSmallSystemFont                    = 1,
   kThemeViewsFont                          = 3,
   kThemeApplicationFont                    = 5,
   kThemeMenuTitleFont                      = 100,
   kThemeMenuItemFont                       = 101,
   kThemeWindowTitleFont                    = 104,
   kThemePushButtonFont                     = 105,
   kThemeMiniSystemFont                     = 109,

   //
   kThemeMetricHSliderHeight                = 41,
   kThemeMetricHSliderTickHeight            = 42,
   kThemeMetricSmallHSliderHeight           = 43,
   kThemeMetricSmallHSliderTickHeight       = 44,
   kThemeMetricMiniHSliderHeight            = 92,
   kThemeMetricMiniHSliderTickHeight        = 94,

   kThemeMetricVSliderWidth                 = 45,
   kThemeMetricVSliderTickWidth             = 46,
   kThemeMetricSmallVSliderWidth            = 47,
   kThemeMetricSmallVSliderTickWidth        = 48,
   kThemeMetricMiniVSliderWidth             = 108,
   kThemeMetricMiniVSliderTickWidth         = 107,
   kThemeMetricCheckBoxWidth                = 50,
   kThemeMetricMiniCheckBoxWidth            = 89,
   kThemeMetricSmallCheckBoxWidth           = 51,

   kThemeMetricScrollBarWidth               = 0,
   kThemeMetricRadioButtonWidth             = 52,
   kThemeMetricMiniRadioButtonWidth         = 100,
   kThemeMetricSmallRadioButtonWidth        = 53,

   kThemeMetricCheckBoxHeight               = 2,
   kThemeMetricMiniCheckBoxHeight           = 88,
   kThemeMetricSmallCheckBoxHeight          = 21,
   kThemeMetricRadioButtonHeight            = 3,
   kThemeMetricMiniRadioButtonHeight        = 99,
   kThemeMetricSmallRadioButtonHeight       = 36,
   kThemeMetricSmallScrollBarWidth          = 1,
   kThemeMetricListBoxFrameOutset           = 6,

   kThemeMetricEditTextFrameOutset          = 5,
   kThemeMetricFocusRectOutset              = 7,

   kThemeMetricPopupButtonHeight            = 30,
   kThemeMetricSmallPopupButtonHeight       = 31,
   kThemeMetricMiniPopupButtonHeight        = 96,

   kThemeMetricPushButtonHeight             = 19,
   kThemeMetricSmallPushButtonHeight        = 35,
   kThemeMetricMiniPushButtonHeight         = 98,

   kThemeMetricLargeProgressBarThickness    = 32,
   kThemeMetricProgressBarShadowOutset      = 59,
   kThemeMetricNormalProgressBarThickness   = 58,
   kThemeMetricSmallProgressBarShadowOutset = 60,
   kThemeMetricListHeaderHeight             = 20,

   kThemeMetricSeparatorSize                = 138,

   //
   kHIThemeSplitterAdornmentMetal           = 1,
   kHIThemeSplitterAdornmentNone            = 0,

   kHIThemeTabSizeSmall                     = 1,
   kHIThemeTabSizeNormal                    = 0,
   kHIThemeTabKindNormal                    = 0,
   kHIThemeTabPaneAdornmentNormal           = 0,
   kHIThemeTabSizeMini                      = 3,
   kHIThemeTabAdornmentTrailingSeparator    = 1 << 4,
   kHIThemeTabAdornmentLeadingSeparator     = 1 << 3,
   kHIThemeTabPositionMiddle                = 1,
   kHIThemeTabAdornmentFocus                = 1 << 2,
   kHIThemeTabAdornmentNone                 = 0,
   kHIThemeTabPositionFirst                 = 0,
   kHIThemeTabPositionLast                  = 2,
   kHIThemeTabPositionOnly                  = 3,

   kHIThemeTextHorizontalFlushCenter        = 1,
   kHIThemeTextVerticalFlushCenter          = 1,
   kHIThemeTextBoxOptionNone                = 0,
   kHIThemeTextTruncationNone               = 0,
   kHIThemeTextHorizontalFlushLeft          = 0,
   kHIThemeTextBoxOptionStronglyVertical    = 1 << 1,

   kHIThemeGroupBoxKindPrimary              = 0,
   kHIThemeGroupBoxKindSecondary            = 1,

   kHIThemeFrameTextFieldRound              = 1000,
   kHIThemeFrameTextFieldSquare             = 0,
   kHIThemeFrameListBox                     = 1,

   kHIThemeGrowBoxKindNormal                = 0,
   kHIThemeGrowBoxSizeSmall                 = 1,
   kHIThemeGrowBoxSizeNormal                = 0,

   kHIThemeOrientationNormal                = 0,
   kHIThemeOrientationInverted              = 1,

   //
   kWindowTitleProxyIconRgn                 = 8,
   kWindowTitleTextRgn                      = 1,
   kWindowTitleBarRgn                       = 0,

   kWindowCloseBoxRgn                       = 2,
   kWindowZoomBoxRgn                        = 3,
   kWindowCollapseBoxRgn                    = 7,
   kWindowGlobalPortRgn                     = 40,

   kControlUpButtonPart                     = 20,
   kControlDownButtonPart                   = 21,
   kControlPageUpPart                       = 22,
   kControlPageDownPart                     = 23,

   kQDParseRegionFromTop         = 1 << 0,
   kQDParseRegionFromLeft        = 1 << 2,
   kQDParseRegionFromTopLeft     = kQDParseRegionFromTop | kQDParseRegionFromLeft,

   shiftKeyBit                   = 9,
   rightShiftKeyBit              = 13,
   controlKeyBit                 = 12,
   rightControlKeyBit            = 15,
   cmdKeyBit                     = 8,
   optionKeyBit                  = 11,
   rightOptionKeyBit             = 14,
   alphaLockBit                  = 10,
   kEventKeyModifierNumLockBit   = 16,

   shiftKey                      = 1 << shiftKeyBit,
   rightShiftKey                 = 1 << rightShiftKeyBit,
   controlKey                    = 1 << controlKeyBit,
   rightControlKey               = 1 << rightControlKeyBit,
   cmdKey                        = 1 << cmdKeyBit,
   optionKey                     = 1 << optionKeyBit,
   rightOptionKey                = 1 << rightOptionKeyBit,
   kEventKeyModifierNumLockMask  = 1 << kEventKeyModifierNumLockBit,

   kBulletUnicode                = 0x2022,
   kCheckUnicode                 = 0x2713,

   kHomeCharCode                 = 1,
   kEnterCharCode                = 3,
   kEndCharCode                  = 4,
   kBackspaceCharCode            = 8,
   kTabCharCode                  = 9,
   kPageUpCharCode               = 11,
   kPageDownCharCode             = 12,
   kReturnCharCode               = 13,
   kEscapeCharCode               = 27,
   kLeftArrowCharCode            = 28,
   kRightArrowCharCode           = 29,
   kUpArrowCharCode              = 30,
   kDownArrowCharCode            = 31,
   kHelpCharCode                 = 5,
   kDeleteCharCode               = 127,
   kClearCharCode                = 27,

   //
   kEventClassKeyboard                 = 'keyb',
   kEventClassTablet                   = 'tblt',
   kEventParamKeyModifiers             = 'kmod',
   kEventParamKeyCode                  = 'kcod',
   kEventParamMouseChord               = 'chor',
   kEventParamQWidget                  = 'qwid',
   kEventParamAccessibleObject         = 'aobj',
   kEventParamTabletProximityRec       = 'tbpx',
   kEventParamAccessibleAttributeValue = 'atvl',
   typeTabletProximityRec              = 'tbpx',

   kEventRawKeyDown              = 1,
   kEventRawKeyRepeat            = 2,
   kEventRawKeyUp                = 3,
   kEventRawKeyModifiersChanged  = 4,

   kEventQtRequestContext        = 13,
   kEventQtRequestMenubarUpdate  = 14,
   kEventQtRequestShowSheet      = 17,
   kEventQtRequestActivate       = 18,
   kEventQtRequestWindowChange   = 20,

   kEventMouseButtonPrimary      = 1,
   kEventMouseButtonSecondary    = 2,
   kEventMouseButtonTertiary     = 3,

   kEventAttributeUserEvent      = 1 << 0,
   kEventPriorityHigh            = 2,
   kEventMouseScroll             = 11,
   kEventTabletProximity         = 2,

   eventNotHandledErr            = -9874,

   kModalWindowClass             = 3,
   kMovableModalWindowClass      = 4,
   kFloatingWindowClass          = 5,
   kDocumentWindowClass          = 6,
   kHelpWindowClass              = 10,
   kSheetWindowClass             = 11,
   kToolbarWindowClass           = 12,
   kPlainWindowClass             = 13,
   kOverlayWindowClass           = 14,
   kSimpleWindowClass            = 18,
   kDrawerWindowClass            = 20,

   kUIModeNormal                 = 0,
   kUIModeAllHidden              = 3,
   kUIOptionAutoShowMenuBar      = 1 << 0,

   kOSANullScript                = 0,
   kOSAModeNull                  = 0,
   kOSAComponentType             = 'osa ',
   kOSAErrorMessage              = 'errs',
   typeAppleScript               = 'ascr',
};

#endif