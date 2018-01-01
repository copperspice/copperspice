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

#include "quicktimevideoplayer.h"
#include "mediaobject.h"
#include "videowidget.h"
#include "audiodevice.h"
#include "quicktimestreamreader.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtOpenGL/QGLContext>

#import <QTKit/QTTrack.h>
#import <QTKit/QTMedia.h>
#import <QuartzCore/CIContext.h>
#import <QuartzCore/CIFilter.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{

// Defined in videowidget.cpp:
QGLWidget *PhononSharedQGLWidget();

QuickTimeVideoPlayer::QuickTimeVideoPlayer() : QObject(0)
{
    m_state = NoMedia;
    m_mediaSource = MediaSource();
    m_QTMovie = 0;
    m_streamReader = 0;
    m_playbackRate = 1.0f;
    m_masterVolume = 1.0f;
    m_relativeVolume = 1.0f;
    m_currentTime = 0;
    m_mute = false;
    m_audioEnabled = false;
    m_hasVideo = false;
    m_playbackRateSat = false;
    m_isDrmProtected = false;
    m_isDrmAuthorized = true;
   m_primaryRenderingTarget = 0;
   m_primaryRenderingCIImage = 0;
    m_QImagePixelBuffer = 0;

}

QuickTimeVideoPlayer::~QuickTimeVideoPlayer()
{
    unsetVideo();
    [(NSObject*)m_primaryRenderingTarget release];
    m_primaryRenderingTarget = 0;
}

void QuickTimeVideoPlayer::createVisualContext()
{
}

bool QuickTimeVideoPlayer::videoFrameChanged()
{
    if (! m_QTMovie || !m_hasVideo)
        return false;

    return true;
}

CVOpenGLTextureRef QuickTimeVideoPlayer::currentFrameAsCVTexture()
{
    return 0;
}

QImage QuickTimeVideoPlayer::currentFrameAsQImage()
{
   CIImage *img = (CIImage *)currentFrameAsCIImage();
   if (!img)
      return QImage();

   NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc] initWithCIImage:img];
   CGRect bounds = [img extent];
   QImage qImg([bitmap bitmapData], bounds.size.width, bounds.size.height, QImage::Format_ARGB32);
   QImage swapped = qImg.rgbSwapped();
   [bitmap release];
   [img release];

   return swapped;
}

void QuickTimeVideoPlayer::setPrimaryRenderingCIImage(void *ciImage)
{
   [(CIImage *)m_primaryRenderingCIImage release];
   m_primaryRenderingCIImage = ciImage;
   [(CIImage *)m_primaryRenderingCIImage retain];
}

void QuickTimeVideoPlayer::setPrimaryRenderingTarget(NSObject *target)
{
   [(NSObject*)m_primaryRenderingTarget release];
   m_primaryRenderingTarget = target;
   [(NSObject*)m_primaryRenderingTarget retain];
}

void *QuickTimeVideoPlayer::primaryRenderingCIImage()
{
   return m_primaryRenderingCIImage;
}

void *QuickTimeVideoPlayer::currentFrameAsCIImage()
{
    if (!m_QTMovie)
        return 0;

   if (m_primaryRenderingCIImage){
      CIImage *img = (CIImage *)m_primaryRenderingCIImage;
      if (m_brightness || m_contrast || m_saturation){
         CIFilter *colorFilter = [CIFilter filterWithName:@"CIColorControls"];
         [colorFilter setValue:[NSNumber numberWithFloat:m_brightness] forKey:@"inputBrightness"];
         [colorFilter setValue:[NSNumber numberWithFloat:(m_contrast < 1) ? m_contrast : 1 + ((m_contrast-1)*3)] forKey:@"inputContrast"];
         [colorFilter setValue:[NSNumber numberWithFloat:m_saturation] forKey:@"inputSaturation"];
         [colorFilter setValue:img forKey:@"inputImage"];
         img = [colorFilter valueForKey:@"outputImage"];
      }
      if (m_hue){
         CIFilter *colorFilter = [CIFilter filterWithName:@"CIHueAdjust"];
         [colorFilter setValue:[NSNumber numberWithFloat:(m_hue * 3.14)] forKey:@"inputAngle"];
         [colorFilter setValue:img forKey:@"inputImage"];
         img = [colorFilter valueForKey:@"outputImage"];
      }
      return [img retain];
   }

   return 0;
}

GLuint QuickTimeVideoPlayer::currentFrameAsGLTexture()
{
   CIImage *img = (CIImage *)currentFrameAsCIImage();
   if (!img)
      return 0;

   NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc] initWithCIImage:img];
    GLuint texName = 0;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, [bitmap pixelsWide]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, texName);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER,  GL_LINEAR);

    int samplesPerPixel = [bitmap samplesPerPixel];
    if (![bitmap isPlanar] && (samplesPerPixel == 3 || samplesPerPixel == 4)){
        glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 
            samplesPerPixel == 4 ? GL_RGBA8 : GL_RGB8,
            [bitmap pixelsWide], [bitmap pixelsHigh],
            0, samplesPerPixel == 4 ? GL_RGBA : GL_RGB,
            GL_UNSIGNED_BYTE, [bitmap bitmapData]);
    } else {
        // Handle other bitmap formats.
    }

    [bitmap release];
   [img release];
    return texName;
}

void QuickTimeVideoPlayer::setMasterVolume(float volume)
{
    setVolume(volume, m_relativeVolume);
}

void QuickTimeVideoPlayer::setRelativeVolume(float volume)
{
    setVolume(m_masterVolume, volume);
}

void QuickTimeVideoPlayer::setVolume(float masterVolume, float relativeVolume)
{
    m_masterVolume = masterVolume;
    m_relativeVolume = relativeVolume;
    if (!m_QTMovie || !m_audioEnabled || m_mute)
        return;                
    [m_QTMovie setVolume:(m_masterVolume * m_relativeVolume)];
}

void QuickTimeVideoPlayer::setMute(bool mute)
{
    m_mute = mute;
    if (!m_QTMovie || m_state != Playing || !m_audioEnabled)
        return;

    // Work-around bug that happends if you set/unset mute
    // before movie is playing, and audio is not played 
    // through graph. Then audio is delayed.
    [m_QTMovie setMuted:mute];
    [m_QTMovie setVolume:(mute ? 0 : m_masterVolume * m_relativeVolume)];
}

void QuickTimeVideoPlayer::enableAudio(bool enable)
{
    m_audioEnabled = enable;
    if (!m_QTMovie || m_state != Playing)
        return;

    // Work-around bug that happends if you set/unset mute
    // before movie is playing, and audio is not played 
    // through graph. Then audio is delayed.
    [m_QTMovie setMuted:(!enable || m_mute)];
    [m_QTMovie setVolume:((!enable || m_mute) ? 0 : m_masterVolume * m_relativeVolume)];
}

bool QuickTimeVideoPlayer::audioEnabled()
{
    return m_audioEnabled;
}

bool QuickTimeVideoPlayer::setAudioDevice(int id)
{
    if (!m_QTMovie)
        return false;

    Q_UNUSED(id);
    return false;
}

void QuickTimeVideoPlayer::setColors(qreal brightness, qreal contrast, qreal hue, qreal saturation)
{
    if (!m_QTMovie)
        return;

    // 0 is default value for the colors
    // in phonon, so adjust scale:
    contrast += 1;
    saturation += 1;

   m_brightness = brightness;
   m_contrast = contrast;
   m_hue = hue;
   m_saturation = saturation;
   
}

QRect QuickTimeVideoPlayer::videoRect() const
{
    if (!m_QTMovie)
        return QRect();

   PhononAutoReleasePool pool;
    NSSize size = [[m_QTMovie attributeForKey:@"QTMovieCurrentSizeAttribute"] sizeValue];
    return QRect(0, 0, size.width, size.height);
}

void QuickTimeVideoPlayer::unsetVideo()
{
    if (!m_QTMovie)
        return;

    [m_QTMovie release];
    m_QTMovie = 0;

    delete m_streamReader;
    m_streamReader = 0;
    m_currentTime = 0;
    m_state = NoMedia;
    m_isDrmProtected = false;
    m_isDrmAuthorized = true;
    m_mediaSource = MediaSource();

    [(CIImage *)m_primaryRenderingCIImage release];
    m_primaryRenderingCIImage = 0;

    delete m_QImagePixelBuffer;
    m_QImagePixelBuffer = 0;
}

QuickTimeVideoPlayer::State QuickTimeVideoPlayer::state() const
{
    return m_state;
}

quint64 QuickTimeVideoPlayer::timeLoaded()
{
    if (!m_QTMovie)
        return 0;

    return 0;
}

float QuickTimeVideoPlayer::percentageLoaded()
{
    if (!m_QTMovie || !isSeekable())
        return 0;

    return 0;
}

void QuickTimeVideoPlayer::waitStatePlayable()
{
    long state = [[m_QTMovie attributeForKey:@"QTMovieLoadStateAttribute"] longValue];

    while (state != QTMovieLoadStateError && state < QTMovieLoadStatePlayable)
        state = [[m_QTMovie attributeForKey:@"QTMovieLoadStateAttribute"] longValue];
}

bool QuickTimeVideoPlayer::movieNotLoaded()
{
    if (!m_QTMovie)
        return true;

    long state = [[m_QTMovie attributeForKey:@"QTMovieLoadStateAttribute"] longValue];
    return state == QTMovieLoadStateError;

}

void QuickTimeVideoPlayer::setError(NSError *error)
{
    if (!error)
        return;
    QString desc = QString::fromUtf8([[error localizedDescription] UTF8String]);
    if (desc == "The file is not a movie file.")
        desc = QLatin1String("Could not decode media source.");
    else if (desc == "A necessary data reference could not be resolved."){
      if (codecExistsAccordingToSuffix(mediaSourcePath()))
            desc = QLatin1String("Could not locate media source.");
      else
            desc = QLatin1String("Could not decode media source.");
    } else if (desc == "You do not have sufficient permissions for this operation.")
        desc = QLatin1String("Could not open media source.");
    SET_ERROR(desc, FATAL_ERROR)
}

bool QuickTimeVideoPlayer::errorOccured()
{
    if (gGetErrorType() != NO_ERROR){
        return true;
    } else if (movieNotLoaded()){
        SET_ERROR("Could not open media source.", FATAL_ERROR)
        return true;
    }
   return false;
}

bool QuickTimeVideoPlayer::codecExistsAccordingToSuffix(const QString &fileName)
{
   PhononAutoReleasePool pool;
   NSArray *fileTypes = [QTMovie movieFileTypes:QTIncludeAllTypes];
   for (uint i=0; i<[fileTypes count]; ++i){
      NSString *type = [fileTypes objectAtIndex:i];
      QString formattedType = QString::fromUtf8([type UTF8String]);
      formattedType.remove('\'').remove('.');
      if (fileName.endsWith(QChar('.') + formattedType, Qt::CaseInsensitive))
         return true;
   }
   return false;
}

void QuickTimeVideoPlayer::setMediaSource(const MediaSource &mediaSource)
{
    PhononAutoReleasePool pool;
    unsetVideo();
    m_mediaSource = mediaSource;
    if (mediaSource.type() == MediaSource::Empty || mediaSource.type() == MediaSource::Invalid){
        m_state = NoMedia;
        return;
    }
    openMovieFromCurrentMediaSource();
    if (errorOccured()){
        unsetVideo();
        return;
    }

    waitStatePlayable();
    if (errorOccured()){
        unsetVideo();
        return;
    }

    readProtection();
    preRollMovie();
    if (errorOccured()){
        unsetVideo();
        return;
    }

    if (!m_playbackRateSat)
        m_playbackRate = prefferedPlaybackRate();
    checkIfVideoAwailable();
    enableAudio(m_audioEnabled);
    setMute(m_mute);
    setVolume(m_masterVolume, m_relativeVolume);
    pause();
}

void QuickTimeVideoPlayer::openMovieFromCurrentMediaSource()
{
    switch (m_mediaSource.type()){
    case MediaSource::LocalFile:
        openMovieFromFile();
        break;
    case MediaSource::Url:
        openMovieFromUrl();
        break;
    case MediaSource::Disc:
        CASE_UNSUPPORTED("Could not open media source.", FATAL_ERROR)
        break;
    case MediaSource::Stream:
        openMovieFromStream();
        break;
    case MediaSource::Empty:
    case MediaSource::Invalid:
        break;
    }
}

QString QuickTimeVideoPlayer::mediaSourcePath()
{
    switch (m_mediaSource.type()){
    case MediaSource::LocalFile:{
        QFileInfo fileInfo(m_mediaSource.fileName());
        return fileInfo.isSymLink() ? fileInfo.symLinkTarget() : fileInfo.canonicalFilePath();
        break;}
    case MediaSource::Url:
      return m_mediaSource.url().toEncoded();
        break;
    default:
        break;
    }
   return QString();
}

void QuickTimeVideoPlayer::openMovieFromDataRef(QTDataReference *dataRef)
{
    PhononAutoReleasePool pool;
    NSDictionary *attr = [NSDictionary dictionaryWithObjectsAndKeys:
                dataRef, QTMovieDataReferenceAttribute,
                [NSNumber numberWithBool:YES], QTMovieOpenAsyncOKAttribute,
                [NSNumber numberWithBool:YES], QTMovieIsActiveAttribute,
                [NSNumber numberWithBool:YES], QTMovieResolveDataRefsAttribute,
                [NSNumber numberWithBool:YES], QTMovieDontInteractWithUserAttribute,
                nil];

    NSError *err = 0;
    m_QTMovie = [[QTMovie movieWithAttributes:attr error:&err] retain];
    if (err){
        [m_QTMovie release];
        m_QTMovie = 0;
        setError(err);
    }
}

void QuickTimeVideoPlayer::openMovieFromData(QByteArray *data, const char *fileType)
{
    PhononAutoReleasePool pool;
    NSString *type = [NSString stringWithUTF8String:fileType];
    NSData *nsData = [NSData dataWithBytesNoCopy:data->data() length:data->size() freeWhenDone:NO];
    QTDataReference *dataRef = [QTDataReference dataReferenceWithReferenceToData:nsData name:type MIMEType:@""];
    openMovieFromDataRef(dataRef);
}

void QuickTimeVideoPlayer::openMovieFromDataGuessType(QByteArray *data)
{
    // It turns out to be better to just try the standard file types rather
    // than using e.g [QTMovie movieFileTypes:QTIncludeCommonTypes]. Some
    // codecs *think* they can decode the stream, and crash...

#define TryOpenMovieWithCodec(type) gClearError(); \
    openMovieFromData(data, "." type); \
    if (m_QTMovie) return;

    TryOpenMovieWithCodec("avi");
    TryOpenMovieWithCodec("mp4");
    TryOpenMovieWithCodec("m4p");
    TryOpenMovieWithCodec("m1s");
    TryOpenMovieWithCodec("mp3");
    TryOpenMovieWithCodec("mpeg");
    TryOpenMovieWithCodec("mov");
    TryOpenMovieWithCodec("ogg");
    TryOpenMovieWithCodec("wav");
    TryOpenMovieWithCodec("wmv");
#undef TryOpenMovieWithCodec
}

void QuickTimeVideoPlayer::openMovieFromFile()
{
    NSString *nsFilename = (NSString *)PhononCFString::toCFStringRef(mediaSourcePath());
    QTDataReference *dataRef = [QTDataReference dataReferenceWithReferenceToFile:nsFilename];
    openMovieFromDataRef(dataRef);
}

void QuickTimeVideoPlayer::openMovieFromUrl()
{
    PhononAutoReleasePool pool;
    NSString *urlString = (NSString *)PhononCFString::toCFStringRef(mediaSourcePath());
    NSURL *url = [NSURL URLWithString: urlString];
    QTDataReference *dataRef = [QTDataReference dataReferenceWithReferenceToURL:url];
    openMovieFromDataRef(dataRef);
}

void QuickTimeVideoPlayer::openMovieFromStream()
{
    m_streamReader = new QuickTimeStreamReader(m_mediaSource);
    if (!m_streamReader->readAllData())
        return;
    openMovieFromDataGuessType(m_streamReader->pointerToData());
}

MediaSource QuickTimeVideoPlayer::mediaSource() const
{
    return m_mediaSource;
}

QTMovie *QuickTimeVideoPlayer::qtMovie() const
{
    return m_QTMovie;
}

void QuickTimeVideoPlayer::setPlaybackRate(float rate)
{
   PhononAutoReleasePool pool;
    m_playbackRateSat = true;
    m_playbackRate = rate;
    if (m_QTMovie)
        [m_QTMovie setRate:m_playbackRate];
}

float QuickTimeVideoPlayer::playbackRate() const
{
    return m_playbackRate;
}

quint64 QuickTimeVideoPlayer::currentTime() const
{
    if (!m_QTMovie || m_state == Paused)
        return m_currentTime;

   PhononAutoReleasePool pool;
    QTTime qtTime = [m_QTMovie currentTime];
    quint64 t = static_cast<quint64>(float(qtTime.timeValue) / float(qtTime.timeScale) * 1000.0f);
    const_cast<QuickTimeVideoPlayer *>(this)->m_currentTime = t;
    return m_currentTime;
}

long QuickTimeVideoPlayer::timeScale() const
{
    if (!m_QTMovie)
        return 0;

   PhononAutoReleasePool pool;
    return [[m_QTMovie attributeForKey:@"QTMovieTimeScaleAttribute"] longValue];
}

QString QuickTimeVideoPlayer::timeToString(quint64 ms)
{
    int sec = ms/1000;
    int min = sec/60;
    int hour = min/60;
    return QString(QLatin1String("%1:%2:%3:%4")).arg(hour%60).arg(min%60).arg(sec%60).arg(ms%1000);
}

QString QuickTimeVideoPlayer::currentTimeString()
{
    return timeToString(currentTime());
}

quint64 QuickTimeVideoPlayer::duration() const
{
    if (!m_QTMovie)
        return 0;

   PhononAutoReleasePool pool;
    QTTime qtTime = [m_QTMovie duration];
    return static_cast<quint64>(float(qtTime.timeValue) / float(qtTime.timeScale) * 1000.0f);
}

void QuickTimeVideoPlayer::play()
{
    if (!canPlayMedia())
        return;

   PhononAutoReleasePool pool;
    m_state = Playing;
    enableAudio(m_audioEnabled);
    setMute(m_mute);
    [m_QTMovie setRate:m_playbackRate];
}

void QuickTimeVideoPlayer::pause()
{
    if (!canPlayMedia())
        return;

   PhononAutoReleasePool pool;
    currentTime();
    m_state = Paused;

    if (isSeekable())
        [m_QTMovie setRate:0];
    else // pretend to be paused:
        [m_QTMovie setMuted:0];
}

void QuickTimeVideoPlayer::seek(quint64 milliseconds)
{
    if (!canPlayMedia() || !isSeekable() || milliseconds == currentTime())
        return;
    if (milliseconds > duration())
        milliseconds = duration();

   PhononAutoReleasePool pool;
    QTTime newQTTime = [m_QTMovie currentTime];
    newQTTime.timeValue = (milliseconds / 1000.0f) * newQTTime.timeScale;
    [m_QTMovie setCurrentTime:newQTTime];

    // The movie might not have been able to seek
    // to the exact point we told it to. So set
    // the current time according to what the movie says:
    newQTTime = [m_QTMovie currentTime];
    m_currentTime = static_cast<quint64>
        (float(newQTTime.timeValue) / float(newQTTime.timeScale) * 1000.0f);

    if (m_state == Paused){
        // We need (for reasons unknown) to task
        // the movie twize to make sure that
        // a subsequent call to frameAsCvTexture
        // returns the correct frame:


        qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
    }
}

bool QuickTimeVideoPlayer::canPlayMedia() const
{
    if (!m_QTMovie)
        return false;
    return m_isDrmAuthorized;
}

bool QuickTimeVideoPlayer::isPlaying() const
{
    return m_state == Playing;
}

bool QuickTimeVideoPlayer::isSeekable() const
{
    return canPlayMedia() && (duration()-1) != INT_MAX;
}

float QuickTimeVideoPlayer::prefferedPlaybackRate() const
{
    if (!m_QTMovie)
        return 0;

   PhononAutoReleasePool pool;
    return [[m_QTMovie attributeForKey:@"QTMoviePreferredRateAttribute"] floatValue];
}

bool QuickTimeVideoPlayer::preRollMovie(qint64 startTime)
{
    if (!canPlayMedia())
        return false;

    Q_UNUSED(startTime);
    return false;
}

bool QuickTimeVideoPlayer::hasAudio() const
{
    if (!m_QTMovie)
        return false;

    PhononAutoReleasePool pool;
    return [[m_QTMovie attributeForKey:@"QTMovieHasAudioAttribute"] boolValue] == YES;
}

bool QuickTimeVideoPlayer::hasVideo() const
{
    return m_hasVideo;
}

bool QuickTimeVideoPlayer::hasMovie() const
{
    return m_QTMovie != 0;
}

void QuickTimeVideoPlayer::checkIfVideoAwailable()
{
   PhononAutoReleasePool pool;
    m_hasVideo = [[m_QTMovie attributeForKey:@"QTMovieHasVideoAttribute"] boolValue] == YES;
}

bool QuickTimeVideoPlayer::isDrmProtected() const
{
    return m_isDrmProtected;
}

bool QuickTimeVideoPlayer::isDrmAuthorized() const
{
    return m_isDrmAuthorized;
}
/*
void QuickTimeVideoPlayer::movieCodecIsMPEG()
{
    NSArray *tracks = [m_QTMovie tracks];
    for (QTTrack *track in tracks)
        if ([[track media] hasCharacteristic:QTMediaTypeMPEG])
            return true;
    return false;
}
*/

static void QtGetTrackProtection(QTTrack *track, bool &isDrmProtected, bool &isDrmAuthorized)
{
    isDrmProtected = false;
    isDrmAuthorized = true;

    Q_UNUSED(track);

}

void QuickTimeVideoPlayer::readProtection()
{
    m_isDrmProtected = false;
    m_isDrmAuthorized = true;

    NSArray *tracks = [m_QTMovie tracks];
    for (uint i=0; i<[tracks count]; ++i){
      QTTrack *track = [tracks objectAtIndex:i];
        bool isDrmProtected = false;
        bool isDrmAuthorized = true;
        QtGetTrackProtection(track, isDrmProtected, isDrmAuthorized);
        if (isDrmProtected)
            m_isDrmProtected = true;
        if (!isDrmAuthorized)
            m_isDrmAuthorized = false;
    }
}

}}

QT_END_NAMESPACE
