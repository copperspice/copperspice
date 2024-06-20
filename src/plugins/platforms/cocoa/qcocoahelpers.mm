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

#include <qcocoahelpers.h>

#include <qplatform_screen.h>
#include <qwidget.h>

#include <qapplication_p.h>
#include <qwindow_p.h>

#include <algorithm>

// Conversion Functions

QStringList qt_mac_NSArrayToQStringList(void *nsarray)
{
   QStringList result;
   NSArray *array = static_cast<NSArray *>(nsarray);
   for (NSUInteger i = 0; i < [array count]; ++i) {
      result << QCFString::toQString([array objectAtIndex: i]);
   }
   return result;
}

void *qt_mac_QStringListToNSMutableArrayVoid(const QStringList &list)
{
   NSMutableArray *result = [NSMutableArray arrayWithCapacity: list.size()];
   for (int i = 0; i < list.size(); ++i) {
      [result addObject: list[i].toNSString()];
   }
   return result;
}

static void qt_mac_deleteImage(void *image, const void *, size_t)
{
   delete static_cast<QImage *>(image);
}

// Creates a CGDataProvider with the data from the given image.
// The data provider retains a copy of the image.
CGDataProviderRef qt_mac_CGDataProvider(const QImage &image)
{
   return CGDataProviderCreateWithData(new QImage(image), image.bits(), image.byteCount(), qt_mac_deleteImage);
}

CGImageRef qt_mac_toCGImage(const QImage &inImage)
{
   if (inImage.isNull()) {
      return nullptr;
   }

   QImage image = inImage;

   uint cgflags = kCGImageAlphaNone;
   switch (image.format()) {
      case QImage::Format_ARGB32:
         cgflags = kCGImageAlphaFirst | kCGBitmapByteOrder32Host;
         break;
      case QImage::Format_RGB32:
         cgflags = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host;
         break;
      case QImage::Format_RGB888:
         cgflags = kCGImageAlphaNone | kCGBitmapByteOrder32Big;
         break;
      case QImage::Format_RGBA8888_Premultiplied:
         cgflags = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;
         break;
      case QImage::Format_RGBA8888:
         cgflags = kCGImageAlphaLast | kCGBitmapByteOrder32Big;
         break;
      case QImage::Format_RGBX8888:
         cgflags = kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder32Big;
         break;
      default:
         // Everything not recognized explicitly is converted to ARGB32_Premultiplied.
         image = inImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
      // no break;
      case QImage::Format_ARGB32_Premultiplied:
         cgflags = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host;
         break;
   }

   QCFType<CGDataProviderRef> dataProvider = qt_mac_CGDataProvider(image);

   return CGImageCreate(image.width(), image.height(), 8, 32, image.bytesPerLine(),
         qt_mac_genericColorSpace(), cgflags, dataProvider, nullptr, false, kCGRenderingIntentDefault);
}

CGImageRef qt_mac_toCGImageMask(const QImage &image)
{
   QCFType<CGDataProviderRef> dataProvider = qt_mac_CGDataProvider(image);
   return CGImageMaskCreate(image.width(), image.height(), 8, image.depth(),
         image.bytesPerLine(), dataProvider, nullptr, false);
}

NSImage *qt_mac_cgimage_to_nsimage(CGImageRef image)
{
   NSImage *newImage = [[NSImage alloc] initWithCGImage: image size: NSZeroSize];
   return newImage;
}

NSImage *qt_mac_create_nsimage(const QPixmap &pm)
{
   if (pm.isNull()) {
      return nullptr;
   }

   QImage image = pm.toImage();
   CGImageRef cgImage = qt_mac_toCGImage(image);
   NSImage *nsImage = qt_mac_cgimage_to_nsimage(cgImage);
   CGImageRelease(cgImage);

   return nsImage;
}

NSImage *qt_mac_create_nsimage(const QIcon &icon, int defaultSize)
{
   if (icon.isNull()) {
      return nil;
   }

   NSImage *nsImage = [[NSImage alloc] init];
   QList<QSize> availableSizes = icon.availableSizes();
   if (availableSizes.isEmpty() && defaultSize > 0) {
      availableSizes << QSize(defaultSize, defaultSize);
   }

   for (QSize size : availableSizes) {
      QPixmap pm = icon.pixmap(size);
      QImage image = pm.toImage();
      CGImageRef cgImage = qt_mac_toCGImage(image);
      NSBitmapImageRep *imageRep = [[NSBitmapImageRep alloc] initWithCGImage: cgImage];
      [nsImage addRepresentation: imageRep];
      [imageRep release];
      CGImageRelease(cgImage);
   }
   return nsImage;
}

HIMutableShapeRef qt_mac_QRegionToHIMutableShape(const QRegion &region)
{
   HIMutableShapeRef shape = HIShapeCreateMutable();
   QVector<QRect> rects = region.rects();
   if (!rects.isEmpty()) {
      int n = rects.count();
      const QRect *qt_r = rects.constData();
      while (n--) {
         CGRect cgRect = CGRectMake(qt_r->x(), qt_r->y(), qt_r->width(), qt_r->height());
         HIShapeUnionWithRect(shape, &cgRect);
         ++qt_r;
      }
   }
   return shape;
}

NSSize qt_mac_toNSSize(const QSize &qtSize)
{
   return NSMakeSize(qtSize.width(), qtSize.height());
}

NSRect qt_mac_toNSRect(const QRect &rect)
{
   return NSMakeRect(rect.x(), rect.y(), rect.width(), rect.height());
}

QRect qt_mac_toQRect(const NSRect &rect)
{
   return QRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

QColor qt_mac_toQColor(const NSColor *color)
{
   QColor qtColor;
   NSString *colorSpace = [color colorSpaceName];
   if (colorSpace == NSDeviceCMYKColorSpace) {
      CGFloat cyan, magenta, yellow, black, alpha;
      [color getCyan: &cyan magenta: &magenta yellow: &yellow black: &black alpha: &alpha];
      qtColor.setCmykF(cyan, magenta, yellow, black, alpha);
   } else {
      NSColor *tmpColor;
      tmpColor = [color colorUsingColorSpaceName: NSDeviceRGBColorSpace];
      CGFloat red, green, blue, alpha;
      [tmpColor getRed: &red green: &green blue: &blue alpha: &alpha];
      qtColor.setRgbF(red, green, blue, alpha);
   }
   return qtColor;
}

QColor qt_mac_toQColor(CGColorRef color)
{
   QColor qtColor;
   CGColorSpaceModel model = CGColorSpaceGetModel(CGColorGetColorSpace(color));
   const CGFloat *components = CGColorGetComponents(color);
   if (model == kCGColorSpaceModelRGB) {
      qtColor.setRgbF(components[0], components[1], components[2], components[3]);
   } else if (model == kCGColorSpaceModelCMYK) {
      qtColor.setCmykF(components[0], components[1], components[2], components[3]);
   } else if (model == kCGColorSpaceModelMonochrome) {
      qtColor.setRgbF(components[0], components[0], components[0], components[1]);
   } else {
      // Colorspace we can't deal with.
      qWarning("Qt: qt_mac_toQColor: cannot convert from colorspace model: %d", model);
      Q_ASSERT(false);
   }
   return qtColor;
}

struct dndenum_mapper {
   NSDragOperation mac_code;
   Qt::DropAction qt_code;
   bool Qt2Mac;
};

static dndenum_mapper dnd_enums[] = {
   { NSDragOperationLink,  Qt::LinkAction, true },
   { NSDragOperationMove,  Qt::MoveAction, true },
   { NSDragOperationCopy,  Qt::CopyAction, true },
   { NSDragOperationGeneric,  Qt::CopyAction, false },
   { NSDragOperationEvery, Qt::ActionMask, false },
   { NSDragOperationNone, Qt::IgnoreAction, false }
};

NSDragOperation qt_mac_mapDropAction(Qt::DropAction action)
{
   for (int i = 0; dnd_enums[i].qt_code; i++) {
      if (dnd_enums[i].Qt2Mac && (action & dnd_enums[i].qt_code)) {
         return dnd_enums[i].mac_code;
      }
   }
   return NSDragOperationNone;
}

NSDragOperation qt_mac_mapDropActions(Qt::DropActions actions)
{
   NSDragOperation nsActions = NSDragOperationNone;
   for (int i = 0; dnd_enums[i].qt_code; i++) {
      if (dnd_enums[i].Qt2Mac && (actions & dnd_enums[i].qt_code)) {
         nsActions |= dnd_enums[i].mac_code;
      }
   }
   return nsActions;
}

Qt::DropAction qt_mac_mapNSDragOperation(NSDragOperation nsActions)
{
   Qt::DropAction action = Qt::IgnoreAction;
   for (int i = 0; dnd_enums[i].mac_code; i++) {
      if (nsActions & dnd_enums[i].mac_code) {
         return dnd_enums[i].qt_code;
      }
   }
   return action;
}

Qt::DropActions qt_mac_mapNSDragOperations(NSDragOperation nsActions)
{
   Qt::DropActions actions = Qt::IgnoreAction;

   for (int i = 0; dnd_enums[i].mac_code; i++) {
      if (dnd_enums[i].mac_code == NSDragOperationEvery) {
         continue;
      }

      if (nsActions & dnd_enums[i].mac_code) {
         actions |= dnd_enums[i].qt_code;
      }
   }
   return actions;
}

// Changes the process type for this process to kProcessTransformToForegroundApplication,
// unless either LSUIElement or LSBackgroundOnly is set in the Info.plist.
void qt_mac_transformProccessToForegroundApplication()
{
   ProcessSerialNumber psn;

   if (GetCurrentProcess(&psn) == noErr) {
      bool forceTransform = true;
      CFTypeRef value = CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), CFSTR("LSUIElement"));

      if (value) {
         CFTypeID valueType = CFGetTypeID(value);
         // Officially it's supposed to be a string, a boolean makes sense, so we'll check.
         // A number less so, but OK.

         if (valueType == CFStringGetTypeID()) {
            forceTransform = !(QCFString::toQString(static_cast<CFStringRef>(value)).toInteger<int>());

         } else if (valueType == CFBooleanGetTypeID()) {
            forceTransform = ! CFBooleanGetValue(static_cast<CFBooleanRef>(value));

         } else if (valueType == CFNumberGetTypeID()) {
            int valueAsInt;
            CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberIntType, &valueAsInt);
            forceTransform = !valueAsInt;
         }
      }

      if (forceTransform) {
         value = CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), CFSTR("LSBackgroundOnly"));

         if (value) {
            CFTypeID valueType = CFGetTypeID(value);

            if (valueType == CFBooleanGetTypeID()) {
               forceTransform = !CFBooleanGetValue(static_cast<CFBooleanRef>(value));

            } else if (valueType == CFStringGetTypeID()) {
               forceTransform = !(QCFString::toQString(static_cast<CFStringRef>(value)).toInteger<int>());

            } else if (valueType == CFNumberGetTypeID()) {
               int valueAsInt;
               CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberIntType, &valueAsInt);
               forceTransform = !valueAsInt;
            }
         }
      }

      if (forceTransform) {
         TransformProcessType(&psn, kProcessTransformToForegroundApplication);
      }
   }
}

QString qt_mac_removeMnemonics(const QString &str)
{
   QString retval;

   int curPos = 0;
   int maxLen  = str.length();

   while (maxLen > 0) {

      if (str.at(curPos) == '&' && (maxLen == 1 || str.at(curPos + 1) != '&')) {
         ++curPos;
         --maxLen;

         if (maxLen == 0) {
            break;
         }

      } else if (str.at(curPos) == '(' &&
         maxLen >= 4 && str.at(curPos + 1) == '&' && str.at(curPos + 2) != '&' && str.at(curPos + 3) == ')') {

         // remove mnemonics which match "(&X)"
         curPos += 4;
         maxLen -= 4;

         continue;
      }

      retval.append(str.at(curPos));

      ++curPos;
      --maxLen;
   }

   return retval;
}

static CGColorSpaceRef m_genericColorSpace = nullptr;
static QHash<CGDirectDisplayID, CGColorSpaceRef> m_displayColorSpaceHash;
static bool m_postRoutineRegistered = false;

CGColorSpaceRef qt_mac_genericColorSpace()
{
#if 0
   if (! m_genericColorSpace) {
      m_genericColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

      if (! m_postRoutineRegistered) {
         m_postRoutineRegistered = true;
         qAddPostRoutine(QCoreGraphicsPaintEngine::cleanUpMacColorSpaces);
      }
   }

   return m_genericColorSpace;

#else
   // Just return the main display colorspace for the moment.
   return qt_mac_displayColorSpace(nullptr);
#endif
}

/*
    Ideally we should pass the widget in here and use CGGetDisplaysWithRect() etc.
    to support multiple displays correctly.
*/
CGColorSpaceRef qt_mac_displayColorSpace(const QWidget *widget)
{
   CGColorSpaceRef colorSpace;

   CGDirectDisplayID displayID;
   if (widget == nullptr) {
      displayID = CGMainDisplayID();

   } else {
      displayID = CGMainDisplayID();
      /*
      ### get correct display
      const QRect &qrect = widget->window()->geometry();
      CGRect rect = CGRectMake(qrect.x(), qrect.y(), qrect.width(), qrect.height());
      CGDisplayCount throwAway;
      CGDisplayErr dErr = CGGetDisplaysWithRect(rect, 1, &displayID, &throwAway);
      if (dErr != kCGErrorSuccess)
          return macDisplayColorSpace(0); // fall back on main display
      */
   }

   if ((colorSpace = m_displayColorSpaceHash.value(displayID))) {
      return colorSpace;
   }

   colorSpace = CGDisplayCopyColorSpace(displayID);
   if (colorSpace == nullptr) {
      colorSpace = CGColorSpaceCreateDeviceRGB();
   }

   m_displayColorSpaceHash.insert(displayID, colorSpace);

   if (!m_postRoutineRegistered) {
      m_postRoutineRegistered = true;
      void qt_mac_cleanUpMacColorSpaces();
      qAddPostRoutine(qt_mac_cleanUpMacColorSpaces);
   }

   return colorSpace;
}

void qt_mac_cleanUpMacColorSpaces()
{
   if (m_genericColorSpace) {
      CFRelease(m_genericColorSpace);
      m_genericColorSpace = nullptr;
   }

   QHash<CGDirectDisplayID, CGColorSpaceRef>::const_iterator it = m_displayColorSpaceHash.constBegin();
   while (it != m_displayColorSpaceHash.constEnd()) {
      if (it.value()) {
         CFRelease(it.value());
      }

      ++it;
   }

   m_displayColorSpaceHash.clear();
}

QString qt_mac_applicationName()
{
   QString appName;
   CFTypeRef string = CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), CFSTR("CFBundleName"));

   if (string) {
      appName = QCFString::toQString(static_cast<CFStringRef>(string));
   }

   if (appName.isEmpty()) {
      QString arg0 = QApplicationPrivate::instance()->appName();

      if (arg0.contains("/")) {
         QStringList parts = arg0.split('/');
         appName = parts.at(parts.count() - 1);

      } else {
         appName = arg0;
      }
   }

   return appName;
}

int qt_mac_mainScreenHeight()
{
   QMacAutoReleasePool pool;
   NSArray *screens = [NSScreen screens];
   if ([screens count] > 0) {
      // The first screen in the screens array is documented
      // to have the (0,0) origin.
      NSRect screenFrame = [[screens objectAtIndex: 0] frame];
      return screenFrame.size.height;
   }
   return 0;
}

int qt_mac_flipYCoordinate(int y)
{
   return qt_mac_mainScreenHeight() - y;
}

qreal qt_mac_flipYCoordinate(qreal y)
{
   return qt_mac_mainScreenHeight() - y;
}

QPointF qt_mac_flipPoint(const NSPoint &p)
{
   return QPointF(p.x, qt_mac_flipYCoordinate(p.y));
}

NSPoint qt_mac_flipPoint(const QPoint &p)
{
   return NSMakePoint(p.x(), qt_mac_flipYCoordinate(p.y()));
}

NSPoint qt_mac_flipPoint(const QPointF &p)
{
   return NSMakePoint(p.x(), qt_mac_flipYCoordinate(p.y()));
}

NSRect qt_mac_flipRect(const QRect &rect)
{
   int flippedY = qt_mac_flipYCoordinate(rect.y() + rect.height());
   return NSMakeRect(rect.x(), flippedY, rect.width(), rect.height());
}

void qt_mac_drawCGImage(CGContextRef inContext, const CGRect *inBounds, CGImageRef inImage)
{
   CGContextSaveGState( inContext );
   CGContextTranslateCTM (inContext, 0, inBounds->origin.y + CGRectGetMaxY(*inBounds));
   CGContextScaleCTM(inContext, 1, -1);

   CGContextDrawImage(inContext, *inBounds, inImage);

   CGContextRestoreGState(inContext);
}

Qt::MouseButton cocoaButton2QtButton(NSInteger buttonNum)
{
   if (buttonNum == 0) {
      return Qt::LeftButton;
   }
   if (buttonNum == 1) {
      return Qt::RightButton;
   }
   if (buttonNum == 2) {
      return Qt::MiddleButton;
   }
   if (buttonNum >= 3 && buttonNum <= 31) { // handle XButton1 and higher via logical shift
      return Qt::MouseButton(uint(Qt::MiddleButton) << (buttonNum - 3));
   }
   // else error: buttonNum too high, or negative
   return Qt::NoButton;
}

bool qt_mac_execute_apple_script(const char *script, long script_len, AEDesc *ret)
{
   OSStatus err;
   AEDesc scriptTextDesc;
   ComponentInstance theComponent = nullptr;
   OSAID scriptID = kOSANullScript, resultID = kOSANullScript;

   // set up locals to a known state
   AECreateDesc(typeNull, nullptr, 0, &scriptTextDesc);
   scriptID = kOSANullScript;
   resultID = kOSANullScript;

   // open the scripting component
   theComponent = OpenDefaultComponent(kOSAComponentType, typeAppleScript);
   if (!theComponent) {
      err = paramErr;
      goto bail;
   }

   // put the script text into an aedesc
   err = AECreateDesc(typeUTF8Text, script, script_len, &scriptTextDesc);
   if (err != noErr) {
      goto bail;
   }

   // compile the script
   err = OSACompile(theComponent, &scriptTextDesc, kOSAModeNull, &scriptID);
   if (err != noErr) {
      goto bail;
   }

   // run the script
   err = OSAExecute(theComponent, scriptID, kOSANullScript, kOSAModeNull, &resultID);

   // collect the results - if any
   if (ret) {
      AECreateDesc(typeNull, nullptr, 0, ret);

      if (err == errOSAScriptError) {
         OSAScriptError(theComponent, kOSAErrorMessage, typeChar, ret);
      } else if (err == noErr && resultID != kOSANullScript) {
         OSADisplay(theComponent, resultID, typeChar, kOSAModeNull, ret);
      }
   }
bail:
   AEDisposeDesc(&scriptTextDesc);
   if (scriptID != kOSANullScript) {
      OSADispose(theComponent, scriptID);
   }
   if (resultID != kOSANullScript) {
      OSADispose(theComponent, resultID);
   }
   if (theComponent) {
      CloseComponent(theComponent);
   }
   return err == noErr;
}

bool qt_mac_execute_apple_script(const char *script, AEDesc *ret)
{
   return qt_mac_execute_apple_script(script, qstrlen(script), ret);
}

bool qt_mac_execute_apple_script(const QString &script, AEDesc *ret)
{
   const QByteArray l = script.toUtf8();
   return qt_mac_execute_apple_script(l.constData(), l.size(), ret);
}

QString qt_mac_removeAmpersandEscapes(QString s)
{
   return qt_mac_removeMnemonics(s).trimmed();
}

/*! \internal

   Returns the CoreGraphics CGContextRef of the paint device. 0 is
   returned if it can't be obtained. It is the caller's responsibility to
   CGContextRelease the context when finished using it.

   \warning This function is duplicated in qmacstyle_mac.mm
*/

CGContextRef qt_mac_cg_context(QPaintDevice *pdev)
{
   // QWidget and QPixmap (and QImage) paint devices are all QImages under the hood
   QImage *image = nullptr;

   if (pdev->devType() == QInternal::Image) {
      image = static_cast<QImage *>(pdev);

   } else if (pdev->devType() == QInternal::Pixmap) {
      const QPixmap *pm = static_cast<const QPixmap *>(pdev);

      QPlatformPixmap *data = const_cast<QPixmap *>(pm)->handle();

      if (data && data->classId() == QPlatformPixmap::RasterClass) {
         image = data->buffer();

      } else {
#if defined(CS_SHOW_DEBUG_PLATFORM)
         qDebug("qt_mac_cg_context() Unsupported pixmap class");
#endif
      }

   } else if (pdev->devType() == QInternal::Widget) {
#if defined(CS_SHOW_DEBUG_PLATFORM)
      qDebug("qt_mac_cg_context() Not implemented for Widget class");
#endif

   }

   if (! image) {
      return nullptr;   // Context type not supported
   }

   CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

   uint flags = kCGImageAlphaPremultipliedFirst;
   flags |= kCGBitmapByteOrder32Host;
   CGContextRef ret = nullptr;

   ret = CGBitmapContextCreate(image->bits(), image->width(), image->height(),
         8, image->bytesPerLine(), colorspace, flags);

   CGContextTranslateCTM(ret, 0, image->height());
   CGContextScaleCTM(ret, 1, -1);

   return ret;
}

QImage qt_mac_toQImage(CGImageRef image)
{
   const size_t w = CGImageGetWidth(image), h = CGImageGetHeight(image);
   QImage ret(w, h, QImage::Format_ARGB32_Premultiplied);
   ret.fill(Qt::transparent);

   CGRect rect = CGRectMake(0, 0, w, h);
   CGContextRef ctx = qt_mac_cg_context(&ret);
   qt_mac_drawCGImage(ctx, &rect, image);
   CGContextRelease(ctx);

   return ret;
}

