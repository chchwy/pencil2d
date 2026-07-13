/*

Pencil2D - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2012-2020 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include "soundclip.h"

#include <QFile>
#include <QMediaPlayer>
#include <QtMath>

SoundClip::SoundClip()
{
}

SoundClip::SoundClip(const SoundClip& s2) : KeyFrame(s2)
{
    mOriginalSoundClipName = s2.mOriginalSoundClipName;
}

SoundClip::~SoundClip()
{
    //QFile::remove( fileName() );
}

SoundClip& SoundClip::operator=(const SoundClip& a)
{
    if (this == &a)
    {
        return *this; // a self-assignment
    }

    KeyFrame::operator=(a);
    mOriginalSoundClipName = a.mOriginalSoundClipName;
    return *this;
}

SoundClip* SoundClip::clone() const
{
    // Question: need to copy the file?
    // The audio files are not allowed to be edited in Pencil2D, it should be file for now.
    return new SoundClip(*this);
}

Status SoundClip::init(const QString& strSoundFile)
{
    if (strSoundFile.isEmpty())
    {
        return Status::FAIL;
    }
    setFileName(strSoundFile);
    return Status::OK;
}

int64_t SoundClip::duration() const
{
    return mDuration;
}

void SoundClip::setDuration(const int64_t& duration)
{
    mDuration = duration;
}

void SoundClip::updateLength(int fps)
{
    setLength(qCeil(mDuration * fps / 1000.0));
}
