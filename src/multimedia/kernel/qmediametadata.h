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

#ifndef QMEDIAMETADATA_H
#define QMEDIAMETADATA_H

#include <qstring.h>

namespace QMediaMetaData {

// Common
static const QString Title               = "Title";
static const QString SubTitle            = "SubTitle";
static const QString Author              = "Author";
static const QString Comment             = "Comment";
static const QString Description         = "Description";
static const QString Category            = "Category";
static const QString Genre               = "Genre";
static const QString Year                = "Year";
static const QString Date                = "Date";
static const QString UserRating          = "UserRating";
static const QString Keywords            = "Keywords";
static const QString Language            = "Language";
static const QString Publisher           = "Publisher";
static const QString Copyright           = "Copyright";
static const QString ParentalRating      = "ParentalRating";
static const QString RatingOrganization  = "RatingOrganization";

// Media
static const QString Size                = "Size";
static const QString MediaType           = "MediaType";
static const QString Duration            = "Duration";

// Audio
static const QString AudioBitRate        = "AudioBitRate";
static const QString AudioCodec          = "AudioCodec";
static const QString AverageLevel        = "AverageLevel";
static const QString ChannelCount        = "ChannelCount";
static const QString PeakValue           = "PeakValue";
static const QString SampleRate          = "SampleRate";

// Music
static const QString AlbumTitle          = "AlbumTitle";
static const QString AlbumArtist         = "AlbumArtist";
static const QString ContributingArtist  = "ContributingArtist";
static const QString Composer            = "Composer";
static const QString Conductor           = "Conductor";
static const QString Lyrics              = "Lyrics";
static const QString Mood                = "Mood";
static const QString TrackNumber         = "TrackNumber";
static const QString TrackCount          = "TrackCount";

static const QString CoverArtUrlSmall    = "CoverArtUrlSmall";
static const QString CoverArtUrlLarge    = "CoverArtUrlLarge";

// Image/Video
static const QString Resolution          = "Resolution";
static const QString PixelAspectRatio    = "PixelAspectRatio";

// Video
static const QString VideoFrameRate      = "VideoFrameRate";
static const QString VideoBitRate        = "VideoBitRate";
static const QString VideoCodec          = "VideoCodec";

static const QString PosterUrl           = "PosterUrl";

// Movie
static const QString ChapterNumber       = "ChapterNumber";
static const QString Director            = "Director";
static const QString LeadPerformer       = "LeadPerformer";
static const QString Writer              = "Writer";

// Photos
static const QString CameraManufacturer  = "CameraManufacturer";
static const QString CameraModel         = "CameraModel";
static const QString Event               = "Event";
static const QString Subject             = "Subject";
static const QString Orientation         = "Orientation";
static const QString ExposureTime        = "ExposureTime";
static const QString FNumber             = "FNumber";
static const QString ExposureProgram     = "ExposureProgram";
static const QString ISOSpeedRatings     = "ISOSpeedRatings";
static const QString ExposureBiasValue   = "ExposureBiasValue";
static const QString DateTimeOriginal    = "DateTimeOriginal";
static const QString DateTimeDigitized   = "DateTimeDigitized";
static const QString SubjectDistance     = "SubjectDistance";
static const QString MeteringMode        = "MeteringMode";
static const QString LightSource         = "LightSource";
static const QString Flash               = "Flash";
static const QString FocalLength         = "FocalLength";
static const QString ExposureMode        = "ExposureMode";
static const QString WhiteBalance        = "WhiteBalance";
static const QString DigitalZoomRatio    = "DigitalZoomRatio";
static const QString SceneCaptureType    = "SceneCaptureType";
static const QString GainControl         = "GainControl";
static const QString Contrast            = "Contrast";
static const QString Saturation          = "Saturation";
static const QString Sharpness           = "Sharpness";

static const QString FocalLengthIn35mmFilm    = "FocalLengthIn35mmFilm";
static const QString DeviceSettingDescription = "DeviceSettingDescription";

// Location
static const QString GPSLatitude         = "GPSLatitude";
static const QString GPSLongitude        = "GPSLongitude";
static const QString GPSAltitude         = "GPSAltitude";
static const QString GPSTimeStamp        = "GPSTimeStamp";
static const QString GPSSatellites       = "GPSSatellites";
static const QString GPSStatus           = "GPSStatus";
static const QString GPSDOP              = "GPSDOP";
static const QString GPSSpeed            = "GPSSpeed";
static const QString GPSTrack            = "GPSTrack";
static const QString GPSTrackRef         = "GPSTrackRef";
static const QString GPSImgDirection     = "GPSImgDirection";
static const QString GPSImgDirectionRef  = "GPSImgDirectionRef";
static const QString GPSMapDatum         = "GPSMapDatum";
static const QString GPSProcessingMethod = "GPSProcessingMethod";
static const QString GPSAreaInformation  = "GPSAreaInformation";

static const QString PosterImage         = "PosterImage";
static const QString CoverArtImage       = "CoverArtImage";
static const QString ThumbnailImage      = "ThumbnailImage";
}

#endif
