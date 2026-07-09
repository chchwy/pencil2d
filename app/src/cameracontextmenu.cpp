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

#include "cameracontextmenu.h"

#include "cameraeasingtype.h"
#include "layercamera.h"
#include "camera.h"
#include "editor.h"
#include "undoredomanager.h"

CameraContextMenu::CameraContextMenu(int frameNumber, const LayerCamera* layer, Editor* editor) :
    mFrameNumber(frameNumber), mCurrentLayer(layer), mEditor(editor)

{
    int nextFrame = layer->getNextKeyFramePosition(frameNumber);

    QMenu* cameraInterpolationMenu = addMenu(tr("Easing: frame %1 to %2").arg(frameNumber).arg(nextFrame));

    cameraInterpolationMenu->setEnabled(layer->getMaxKeyFramePosition() != frameNumber);

    Camera* selectedKey = layer->getCameraAtFrame(frameNumber);
    if (selectedKey != nullptr) {
        QAction* selectedAction = cameraInterpolationMenu->addAction(tr("Selected: ") + getInterpolationText(selectedKey->getEasingType()));
        selectedAction->setDisabled(true);
    }

    cameraInterpolationMenu->addAction(tr("Linear"), [=] { setEasing(CameraEasingType::LINEAR); });
    cameraInterpolationMenu->addSeparator();
    QMenu* inMenu = cameraInterpolationMenu->addMenu(tr("In"));
    QMenu* outMenu = cameraInterpolationMenu->addMenu(tr("Out"));
    QMenu* inOutMenu = cameraInterpolationMenu->addMenu(tr("In-Out"));
    QMenu* outInMenu = cameraInterpolationMenu->addMenu(tr("Out-In"));

    QString slow = tr("Slow");
    QString moderate = tr("Moderate");
    QString quick = tr("Quick");
    QString fast = tr("Fast");
    QString faster = tr("Faster");
    QString fastest = tr("Fastest");
    QString circleBased = tr("Circle-based");
    QString overshoot = tr("Overshoot");
    QString elastic = tr("Elastic");
    QString bounce = tr("Bounce");

    inMenu->addAction(slow, [=] { setEasing(CameraEasingType::INSINE); });
    outMenu->addAction(slow, [=] { setEasing(CameraEasingType::OUTSINE); });
    inOutMenu->addAction(slow, [=] { setEasing(CameraEasingType::INOUTSINE); });
    outInMenu->addAction(slow, [=] { setEasing(CameraEasingType::OUTINSINE); });
    inMenu->addAction(moderate, [=] { setEasing(CameraEasingType::INQUAD); });
    outMenu->addAction(moderate, [=] { setEasing(CameraEasingType::OUTQUAD); });
    inOutMenu->addAction(moderate, [=] { setEasing(CameraEasingType::INOUTQUAD); });
    outInMenu->addAction(moderate, [=] { setEasing(CameraEasingType::OUTINQUAD); });
    inMenu->addAction(quick, [=] { setEasing(CameraEasingType::INCUBIC); });
    outMenu->addAction(quick, [=] { setEasing(CameraEasingType::OUTCUBIC); });
    inOutMenu->addAction(quick, [=] { setEasing(CameraEasingType::INOUTCUBIC); });
    outInMenu->addAction(quick, [=] { setEasing(CameraEasingType::OUTINCUBIC); });
    inMenu->addAction(fast, [=] { setEasing(CameraEasingType::INQUART); });
    outMenu->addAction(fast, [=] { setEasing(CameraEasingType::OUTQUART); });
    inOutMenu->addAction(fast, [=] { setEasing(CameraEasingType::INOUTQUART); });
    outInMenu->addAction(fast, [=] { setEasing(CameraEasingType::OUTINQUART); });
    inMenu->addAction(faster, [=] { setEasing(CameraEasingType::INQUINT); });
    outMenu->addAction(faster, [=] { setEasing(CameraEasingType::OUTQUINT); });
    inOutMenu->addAction(faster, [=] { setEasing(CameraEasingType::INOUTQUINT); });
    outInMenu->addAction(faster, [=] { setEasing(CameraEasingType::OUTINQUINT); });
    inMenu->addAction(fastest, [=] { setEasing(CameraEasingType::INEXPO); });
    outMenu->addAction(fastest, [=] { setEasing(CameraEasingType::OUTEXPO); });
    inOutMenu->addAction(fastest, [=] { setEasing(CameraEasingType::INOUTEXPO); });
    outInMenu->addAction(fastest, [=] { setEasing(CameraEasingType::OUTINEXPO); });
    inMenu->addAction(circleBased, [=] { setEasing(CameraEasingType::INCIRC); });
    outMenu->addAction(circleBased, [=] { setEasing(CameraEasingType::OUTCIRC); });
    inOutMenu->addAction(circleBased, [=] { setEasing(CameraEasingType::INOUTCIRC); });
    outInMenu->addAction(circleBased, [=] { setEasing(CameraEasingType::OUTINCIRC); });
    inMenu->addAction(overshoot, [=] { setEasing(CameraEasingType::INBACK); });
    outMenu->addAction(overshoot, [=] { setEasing(CameraEasingType::OUTBACK); });
    inOutMenu->addAction(overshoot, [=] { setEasing(CameraEasingType::INOUTBACK); });
    outInMenu->addAction(overshoot, [=] { setEasing(CameraEasingType::OUTINBACK); });
    inMenu->addAction(elastic, [=] { setEasing(CameraEasingType::INELASTIC); });
    outMenu->addAction(elastic, [=] { setEasing(CameraEasingType::OUTELASTIC); });
    inOutMenu->addAction(elastic, [=] { setEasing(CameraEasingType::INOUTELASTIC); });
    outInMenu->addAction(elastic, [=] { setEasing(CameraEasingType::OUTINELASTIC); });
    inMenu->addAction(bounce, [=] { setEasing(CameraEasingType::INBOUNCE); });
    outMenu->addAction(bounce, [=] { setEasing(CameraEasingType::OUTBOUNCE); });
    inOutMenu->addAction(bounce, [=] { setEasing(CameraEasingType::INOUTBOUNCE); });
    outInMenu->addAction(bounce, [=] { setEasing(CameraEasingType::OUTINBOUNCE); });

    QMenu* cameraFieldMenu = addMenu(tr("Transform"));
    cameraFieldMenu->addAction(tr("Reset all"), [=] { resetTransform(CameraFieldOption::RESET_FIELD); });
    cameraFieldMenu->addSeparator();
    cameraFieldMenu->addAction(tr("Reset position"), [=] { resetTransform(CameraFieldOption::RESET_TRANSLATION); });
    cameraFieldMenu->addAction(tr("Reset scale"), [=] { resetTransform(CameraFieldOption::RESET_SCALING); });
    cameraFieldMenu->addAction(tr("Reset rotation"), [=] { resetTransform(CameraFieldOption::RESET_ROTATION); });
    cameraFieldMenu->addSeparator();
    QAction* alignHAction = cameraFieldMenu->addAction(tr("Align horizontally to frame %1").arg(nextFrame), [=] { resetTransform(CameraFieldOption::ALIGN_HORIZONTAL); });
    QAction* alignVAction = cameraFieldMenu->addAction(tr("Align vertically to frame %1").arg(nextFrame), [=] { resetTransform(CameraFieldOption::ALIGN_VERTICAL); });
    cameraFieldMenu->addSeparator();
    QAction* holdAction = cameraFieldMenu->addAction(tr("Hold to keyframe %1").arg(nextFrame), [=] { resetTransform(CameraFieldOption::HOLD_FRAME); });
    if (frameNumber == layer->getMaxKeyFramePosition()) {
        holdAction->setDisabled(true);
        alignHAction->setDisabled(true);
        alignVAction->setDisabled(true);
    }
}

void CameraContextMenu::setEasing(CameraEasingType type)
{
    UndoTransaction transaction = mEditor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                                        mCurrentLayer->id(), mFrameNumber);
    mCurrentLayer->setCameraEasingAtFrame(type, mFrameNumber);
    transaction.commit(tr("Camera easing change"));
}

void CameraContextMenu::resetTransform(CameraFieldOption option)
{
    UndoRedoManager* undoRedo = mEditor->undoRedo();

    // resetCameraAtFrame() can modify up to three keyframes: the one at
    // this frame, the next one (align/hold options) and the path flag on
    // the keyframe covering the previous frame. Capture them all and group
    // the result into a single undo step.
    undoRedo->beginMacro(tr("Camera transform reset"));

    UndoTransaction mainTransaction = undoRedo->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                                 mCurrentLayer->id(), mFrameNumber);
    const int nextPos = mCurrentLayer->getNextKeyFramePosition(mFrameNumber);
    UndoTransaction nextTransaction;
    if (nextPos > mFrameNumber) {
        nextTransaction = undoRedo->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                     mCurrentLayer->id(), nextPos);
    }
    UndoTransaction previousTransaction;
    if (mFrameNumber > 1) {
        previousTransaction = undoRedo->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                         mCurrentLayer->id(), mFrameNumber - 1);
    }

    mCurrentLayer->resetCameraAtFrame(option, mFrameNumber);

    mainTransaction.commit(tr("Camera transform reset"));
    nextTransaction.commit(tr("Camera transform reset"));
    previousTransaction.commit(tr("Camera transform reset"));
    undoRedo->endMacro();
}
