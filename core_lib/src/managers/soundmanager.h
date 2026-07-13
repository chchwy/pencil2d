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

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <cstdint>
#include "basemanager.h"
#include "keyframe.h"

#include <QHash>

class Layer;
class SoundClip;
class SoundPlayer;


class SoundManager : public BaseManager, public KeyFrameEventListener
{
    Q_OBJECT
public:

    explicit SoundManager(Editor* editor);
    ~SoundManager() override;

    bool init() override;
    Status load(Object*) override;
    Status save(Object*) override;

    Status loadSound(SoundClip* soundClip, QString strSoundFile);
    Status processSound(SoundClip* soundClip);

    int soundClipCount() const;

    /** Playback control for a clip's media player. The players live in a
     *  map here rather than inside the SoundClip keyframes; the document
     *  model carries no playback machinery. All are safe no-ops for clips
     *  without a player. */
    void play(const SoundClip* clip) const;
    void playFromPosition(const SoundClip* clip, int frameNumber, int fps) const;
    void pause(const SoundClip* clip) const;
    void stop(const SoundClip* clip) const;

    /** True when the clip has a media player without errors. */
    bool hasValidPlayer(const SoundClip* clip) const;

    /** Drops the player of a destroyed clip. */
    void onKeyFrameDestroy(KeyFrame* keyframe) override;

signals:
    void soundClipDurationChanged();

private:
    void onDurationChanged(SoundPlayer* player, int64_t duration);

    Status createMediaPlayer(SoundClip*);

    /** Media players by clip; entries are removed when the clip is
     *  destroyed (KeyFrameEventListener) and replaced on reload. */
    QHash<const KeyFrame*, SoundPlayer*> mSoundPlayers;
};

#endif // SOUNDMANAGER_H
