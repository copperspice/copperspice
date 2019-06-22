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

#include <qmediametadata.h>

#define Q_DEFINE_METADATA(key) const QString QMediaMetaData::key(#key)

// Common
Q_DEFINE_METADATA(Title);
Q_DEFINE_METADATA(SubTitle);
Q_DEFINE_METADATA(Author);
Q_DEFINE_METADATA(Comment);
Q_DEFINE_METADATA(Description);
Q_DEFINE_METADATA(Category);
Q_DEFINE_METADATA(Genre);
Q_DEFINE_METADATA(Year);
Q_DEFINE_METADATA(Date);
Q_DEFINE_METADATA(UserRating);
Q_DEFINE_METADATA(Keywords);
Q_DEFINE_METADATA(Language);
Q_DEFINE_METADATA(Publisher);
Q_DEFINE_METADATA(Copyright);
Q_DEFINE_METADATA(ParentalRating);
Q_DEFINE_METADATA(RatingOrganization);

// Media
Q_DEFINE_METADATA(Size);
Q_DEFINE_METADATA(MediaType);
Q_DEFINE_METADATA(Duration);

// Audio
Q_DEFINE_METADATA(AudioBitRate);
Q_DEFINE_METADATA(AudioCodec);
Q_DEFINE_METADATA(AverageLevel);
Q_DEFINE_METADATA(ChannelCount);
Q_DEFINE_METADATA(PeakValue);
Q_DEFINE_METADATA(SampleRate);

// Music
Q_DEFINE_METADATA(AlbumTitle);
Q_DEFINE_METADATA(AlbumArtist);
Q_DEFINE_METADATA(ContributingArtist);
Q_DEFINE_METADATA(Composer);
Q_DEFINE_METADATA(Conductor);
Q_DEFINE_METADATA(Lyrics);
Q_DEFINE_METADATA(Mood);
Q_DEFINE_METADATA(TrackNumber);
Q_DEFINE_METADATA(TrackCount);

Q_DEFINE_METADATA(CoverArtUrlSmall);
Q_DEFINE_METADATA(CoverArtUrlLarge);

// Image/Video
Q_DEFINE_METADATA(Resolution);
Q_DEFINE_METADATA(PixelAspectRatio);

// Video
Q_DEFINE_METADATA(VideoFrameRate);
Q_DEFINE_METADATA(VideoBitRate);
Q_DEFINE_METADATA(VideoCodec);

Q_DEFINE_METADATA(PosterUrl);

// Movie
Q_DEFINE_METADATA(ChapterNumber);
Q_DEFINE_METADATA(Director);
Q_DEFINE_METADATA(LeadPerformer);
Q_DEFINE_METADATA(Writer);

// Photos
Q_DEFINE_METADATA(CameraManufacturer);
Q_DEFINE_METADATA(CameraModel);
Q_DEFINE_METADATA(Event);
Q_DEFINE_METADATA(Subject);
Q_DEFINE_METADATA(Orientation);
Q_DEFINE_METADATA(ExposureTime);
Q_DEFINE_METADATA(FNumber);
Q_DEFINE_METADATA(ExposureProgram);
Q_DEFINE_METADATA(ISOSpeedRatings);
Q_DEFINE_METADATA(ExposureBiasValue);
Q_DEFINE_METADATA(DateTimeOriginal);
Q_DEFINE_METADATA(DateTimeDigitized);
Q_DEFINE_METADATA(SubjectDistance);
Q_DEFINE_METADATA(MeteringMode);
Q_DEFINE_METADATA(LightSource);
Q_DEFINE_METADATA(Flash);
Q_DEFINE_METADATA(FocalLength);
Q_DEFINE_METADATA(ExposureMode);
Q_DEFINE_METADATA(WhiteBalance);
Q_DEFINE_METADATA(DigitalZoomRatio);
Q_DEFINE_METADATA(FocalLengthIn35mmFilm);
Q_DEFINE_METADATA(SceneCaptureType);
Q_DEFINE_METADATA(GainControl);
Q_DEFINE_METADATA(Contrast);
Q_DEFINE_METADATA(Saturation);
Q_DEFINE_METADATA(Sharpness);
Q_DEFINE_METADATA(DeviceSettingDescription);

// Location
Q_DEFINE_METADATA(GPSLatitude);
Q_DEFINE_METADATA(GPSLongitude);
Q_DEFINE_METADATA(GPSAltitude);
Q_DEFINE_METADATA(GPSTimeStamp);
Q_DEFINE_METADATA(GPSSatellites);
Q_DEFINE_METADATA(GPSStatus);
Q_DEFINE_METADATA(GPSDOP);
Q_DEFINE_METADATA(GPSSpeed);
Q_DEFINE_METADATA(GPSTrack);
Q_DEFINE_METADATA(GPSTrackRef);
Q_DEFINE_METADATA(GPSImgDirection);
Q_DEFINE_METADATA(GPSImgDirectionRef);
Q_DEFINE_METADATA(GPSMapDatum);
Q_DEFINE_METADATA(GPSProcessingMethod);
Q_DEFINE_METADATA(GPSAreaInformation);

Q_DEFINE_METADATA(PosterImage);
Q_DEFINE_METADATA(CoverArtImage);
Q_DEFINE_METADATA(ThumbnailImage);


