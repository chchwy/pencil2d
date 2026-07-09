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

#ifndef CAMERACONTEXTMENU_H
#define CAMERACONTEXTMENU_H

#include <QPoint>
#include <QObject>

#include <QMenu>

#include "cameraeasingtype.h"
#include "camerafieldoption.h"

class Editor;
class LayerCamera;

class CameraContextMenu : public QMenu
{
    Q_OBJECT
public:
    CameraContextMenu(int frameNumber, const LayerCamera* layer, Editor* editor);

private:
    /// Applies an easing change to the keyframe, recorded as one undo step.
    void setEasing(CameraEasingType type);
    /// Applies a transform reset option, recorded as one undo step.
    void resetTransform(CameraFieldOption option);

    int mFrameNumber;
    const LayerCamera* mCurrentLayer;
    Editor* mEditor;
};

#endif // CAMERACONTEXTMENU_H
