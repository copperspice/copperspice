/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QMEDIAMETADATA_H
#define QMEDIAMETADATA_H

#include <qmetatype.h>
#include <qpair.h>
#include <qstring.h>

#define Q_DECLARE_METADATA(key) Q_MULTIMEDIA_EXPORT extern const QString key

namespace QMediaMetaData {

// Common
Q_DECLARE_METADATA(Title);
Q_DECLARE_METADATA(SubTitle);
Q_DECLARE_METADATA(Author);
Q_DECLARE_METADATA(Comment);
Q_DECLARE_METADATA(Description);
Q_DECLARE_METADATA(Category);
Q_DECLARE_METADATA(Genre);
Q_DECLARE_METADATA(Year);
Q_DECLARE_METADATA(Date);
Q_DECLARE_METADATA(UserRating);
Q_DECLARE_METADATA(Keywords);
Q_DECLARE_METADATA(Language);
Q_DECLARE_METADATA(Publisher);
Q_DECLARE_METADATA(Copyright);
Q_DECLARE_METADATA(ParentalRating);
Q_DECLARE_METADATA(RatingOrganization);

// Media
Q_DECLARE_METADATA(Size);
Q_DECLARE_METADATA(MediaType);
Q_DECLARE_METADATA(Duration);

// Audio
Q_DECLARE_METADATA(AudioBitRate);
Q_DECLARE_METADATA(AudioCodec);
Q_DECLARE_METADATA(AverageLevel);
Q_DECLARE_METADATA(ChannelCount);
Q_DECLARE_METADATA(PeakValue);
Q_DECLARE_METADATA(SampleRate);

// Music
Q_DECLARE_METADATA(AlbumTitle);
Q_DECLARE_METADATA(AlbumArtist);
Q_DECLARE_METADATA(ContributingArtist);
Q_DECLARE_METADATA(Composer);
Q_DECLARE_METADATA(Conductor);
Q_DECLARE_METADATA(Lyrics);
Q_DECLARE_METADATA(Mood);
Q_DECLARE_METADATA(TrackNumber);
Q_DECLARE_METADATA(TrackCount);

Q_DECLARE_METADATA(CoverArtUrlSmall);
Q_DECLARE_METADATA(CoverArtUrlLarge);

// Image/Video
Q_DECLARE_METADATA(Resolution);
Q_DECLARE_METADATA(PixelAspectRatio);

// Video
Q_DECLARE_METADATA(VideoFrameRate);
Q_DECLARE_METADATA(VideoBitRate);
Q_DECLARE_METADATA(VideoCodec);

Q_DECLARE_METADATA(PosterUrl);

// Movie
Q_DECLARE_METADATA(ChapterNumber);
Q_DECLARE_METADATA(Director);
Q_DECLARE_METADATA(LeadPerformer);
Q_DECLARE_METADATA(Writer);

// Photos
Q_DECLARE_METADATA(CameraManufacturer);
Q_DECLARE_METADATA(CameraModel);
Q_DECLARE_METADATA(Event);
Q_DECLARE_METADATA(Subject);
Q_DECLARE_METADATA(Orientation);
Q_DECLARE_METADATA(ExposureTime);
Q_DECLARE_METADATA(FNumber);
Q_DECLARE_METADATA(ExposureProgram);
Q_DECLARE_METADATA(ISOSpeedRatings);
Q_DECLARE_METADATA(ExposureBiasValue);
Q_DECLARE_METADATA(DateTimeOriginal);
Q_DECLARE_METADATA(DateTimeDigitized);
Q_DECLARE_METADATA(SubjectDistance);
Q_DECLARE_METADATA(MeteringMode);
Q_DECLARE_METADATA(LightSource);
Q_DECLARE_METADATA(Flash);
Q_DECLARE_METADATA(FocalLength);
Q_DECLARE_METADATA(ExposureMode);
Q_DECLARE_METADATA(WhiteBalance);
Q_DECLARE_METADATA(DigitalZoomRatio);
Q_DECLARE_METADATA(FocalLengthIn35mmFilm);
Q_DECLARE_METADATA(SceneCaptureType);
Q_DECLARE_METADATA(GainControl);
Q_DECLARE_METADATA(Contrast);
Q_DECLARE_METADATA(Saturation);
Q_DECLARE_METADATA(Sharpness);
Q_DECLARE_METADATA(DeviceSettingDescription);

// Location
Q_DECLARE_METADATA(GPSLatitude);
Q_DECLARE_METADATA(GPSLongitude);
Q_DECLARE_METADATA(GPSAltitude);
Q_DECLARE_METADATA(GPSTimeStamp);
Q_DECLARE_METADATA(GPSSatellites);
Q_DECLARE_METADATA(GPSStatus);
Q_DECLARE_METADATA(GPSDOP);
Q_DECLARE_METADATA(GPSSpeed);
Q_DECLARE_METADATA(GPSTrack);
Q_DECLARE_METADATA(GPSTrackRef);
Q_DECLARE_METADATA(GPSImgDirection);
Q_DECLARE_METADATA(GPSImgDirectionRef);
Q_DECLARE_METADATA(GPSMapDatum);
Q_DECLARE_METADATA(GPSProcessingMethod);
Q_DECLARE_METADATA(GPSAreaInformation);

Q_DECLARE_METADATA(PosterImage);
Q_DECLARE_METADATA(CoverArtImage);
Q_DECLARE_METADATA(ThumbnailImage);
}

#undef Q_DECLARE_METADATA

#endif
