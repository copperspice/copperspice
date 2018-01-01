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

#ifndef DS9_QEvr9_H
#define DS9_QEvr9_H

#include <d3d9.h>

#define DXVA2_ProcAmp_Brightness 1
#define DXVA2_ProcAmp_Contrast   2
#define DXVA2_ProcAmp_Hue        4
#define DXVA2_ProcAmp_Saturation 8

typedef enum {
    MFVideoARMode_None             = 0x00000000,
    MFVideoARMode_PreservePicture  = 0x00000001,
    MFVideoARMode_PreservePixel    = 0x00000002,
    MFVideoARMode_NonLinearStretch = 0x00000004,
    MFVideoARMode_Mask             = 0x00000007
} MFVideoAspectRatioMode;

typedef struct {
    float  left;
    float  top;
    float  right;
    float  bottom;
} MFVideoNormalizedRect;

typedef struct {
    UINT    DeviceCaps;
    D3DPOOL InputPool;
    UINT    NumForwardRefSamples;
    UINT    NumBackwardRefSamples;
    UINT    Reserved;
    UINT    DeinterlaceTechnology;
    UINT    ProcAmpControlCaps;
    UINT    VideoProcessorOperations;
    UINT    NoiseFilterTechnology;
    UINT    DetailFilterTechnology;
} DXVA2_VideoProcessorCaps;

typedef struct {
    union {
        struct {
            USHORT Fraction;
            SHORT  Value;
        };
        LONG ll;
    };
} DXVA2_Fixed32;

typedef struct {
    DXVA2_Fixed32 MinValue;
    DXVA2_Fixed32 MaxValue;
    DXVA2_Fixed32 DefaultValue;
    DXVA2_Fixed32 StepSize;
} DXVA2_ValueRange;

typedef struct {
    DXVA2_Fixed32 Brightness;
    DXVA2_Fixed32 Contrast;
    DXVA2_Fixed32 Hue;
    DXVA2_Fixed32 Saturation;
} DXVA2_ProcAmpValues;

DXVA2_Fixed32 DXVA2FloatToFixed(const float _float_)
{
    DXVA2_Fixed32 _fixed_;
    _fixed_.Fraction = LOWORD(_float_ * 0x10000);
    _fixed_.Value = HIWORD(_float_ * 0x10000);
    return _fixed_;
}

float DXVA2FixedToFloat(const DXVA2_Fixed32 _fixed_)
{
    return (FLOAT)_fixed_.Value + (FLOAT)_fixed_.Fraction / 0x10000;
}

#undef INTERFACE
#define INTERFACE IMFVideoDisplayControl
DECLARE_INTERFACE_(IMFVideoDisplayControl, IUnknown)
{
    STDMETHOD(GetNativeVideoSize)(THIS_ SIZE* pszVideo, SIZE* pszARVideo) PURE;
    STDMETHOD(GetIdealVideoSize)(THIS_ SIZE* pszMin, SIZE* pszMax) PURE;
    STDMETHOD(SetVideoPosition)(THIS_ const MFVideoNormalizedRect* pnrcSource, const LPRECT prcDest) PURE;
    STDMETHOD(GetVideoPosition)(THIS_ MFVideoNormalizedRect* pnrcSource, LPRECT prcDest) PURE;
    STDMETHOD(SetAspectRatioMode)(THIS_ DWORD dwAspectRatioMode) PURE;
    STDMETHOD(GetAspectRatioMode)(THIS_ DWORD* pdwAspectRatioMode) PURE;
    STDMETHOD(SetVideoWindow)(THIS_ HWND hwndVideo) PURE;
    STDMETHOD(GetVideoWindow)(THIS_ HWND* phwndVideo) PURE;
    STDMETHOD(RepaintVideo)(THIS_) PURE;
    STDMETHOD(GetCurrentImage)(THIS_ BITMAPINFOHEADER* pBih, BYTE** pDib, DWORD* pcbDib, LONGLONG* pTimeStamp) PURE;
    STDMETHOD(SetBorderColor)(THIS_ COLORREF Clr) PURE;
    STDMETHOD(GetBorderColor)(THIS_ COLORREF* pClr) PURE;
    STDMETHOD(SetRenderingPrefs)(THIS_ DWORD dwRenderFlags) PURE;
    STDMETHOD(GetRenderingPrefs)(THIS_ DWORD* pdwRenderFlags) PURE;
    STDMETHOD(SetFullScreen)(THIS_ BOOL fFullscreen) PURE;
    STDMETHOD(GetFullScreen)(THIS_ BOOL* pfFullscreen) PURE;
};
#undef INTERFACE
#define INTERFACE IMFVideoMixerControl
DECLARE_INTERFACE_(IMFVideoMixerControl, IUnknown)
{
    STDMETHOD(SetStreamZOrder)(THIS_ DWORD dwStreamID, DWORD dwZ) PURE;
    STDMETHOD(GetStreamZOrder)(THIS_ DWORD dwStreamID, DWORD* pdwZ) PURE;
    STDMETHOD(SetStreamOutputRect)(THIS_ DWORD dwStreamID, const MFVideoNormalizedRect* pnrcOutput) PURE;
    STDMETHOD(GetStreamOutputRect)(THIS_ DWORD dwStreamID, MFVideoNormalizedRect* pnrcOutput) PURE;
};
#undef INTERFACE
#define INTERFACE IMFVideoProcessor
DECLARE_INTERFACE_(IMFVideoProcessor, IUnknown)
{
    STDMETHOD(GetAvailableVideoProcessorModes)(THIS_ UINT* lpdwNumProcessingModes, GUID** ppVideoProcessingModes) PURE;
    STDMETHOD(GetVideoProcessorCaps)(THIS_ LPGUID lpVideoProcessorMode, DXVA2_VideoProcessorCaps* lpVideoProcessorCaps) PURE;
    STDMETHOD(GetVideoProcessorMode)(THIS_ LPGUID lpMode) PURE;
    STDMETHOD(SetVideoProcessorMode)(THIS_ LPGUID lpMode) PURE;
    STDMETHOD(GetProcAmpRange)(THIS_ DWORD dwProperty, DXVA2_ValueRange* pPropRange) PURE;
    STDMETHOD(GetProcAmpValues)(THIS_ DWORD dwFlags, DXVA2_ProcAmpValues* Values) PURE;
    STDMETHOD(SetProcAmpValues)(THIS_ DWORD  dwFlags, DXVA2_ProcAmpValues*  pValues) PURE;
    STDMETHOD(GetFilteringRange)(THIS_ DWORD dwProperty, DXVA2_ValueRange* pPropRange) PURE;
    STDMETHOD(GetFilteringValue)(THIS_ DWORD dwProperty, DXVA2_Fixed32* pValue) PURE;
    STDMETHOD(SetFilteringValue)(THIS_ DWORD dwProperty, DXVA2_Fixed32* pValue) PURE;
    STDMETHOD(GetBackgroundColor)(THIS_ COLORREF* lpClrBkg) PURE;
    STDMETHOD(SetBackgroundColor)(THIS_ COLORREF ClrBkg) PURE;
};
#undef INTERFACE
#define INTERFACE IMFGetService
DECLARE_INTERFACE_(IMFGetService, IUnknown)
{
    STDMETHOD(GetService)(THIS_ REFGUID guidService, REFIID riid, LPVOID* ppvObject) PURE;
};
#undef INTERFACE

#endif