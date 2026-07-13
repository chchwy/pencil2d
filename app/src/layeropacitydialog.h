#ifndef LAYEROPACITYDIALOG_H
#define LAYEROPACITYDIALOG_H

#include <QDialog>
#include "editor.h"

class QTimer;
class LayerManager;
class PlaybackManager;
class Layer;

namespace Ui {
class LayerOpacityDialog;
}

enum class OpacityFadeType {
    IN,
    OUT
};

class LayerOpacityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayerOpacityDialog(QWidget *parent = nullptr);
    ~LayerOpacityDialog();

    void setCore(Editor* editor);
    void initUI();
    void updateUI();

private slots:
    void opacitySliderChanged(int value);
    void opacitySpinboxChanged(double value);
    void fadeInPressed();
    void fadeOutPressed();

    void onObjectLoaded();
    void onCurrentLayerChanged(int index);
    void onCurrentFrameChanged(int frame);
    void onSelectedFramesChanged();
    void onPlayStateChanged(bool isPlaying);
    void opacityValueChanged();

private:

    void setCanAdjust(bool opacity, bool fade);
    void fade(OpacityFadeType fadeType);
    qreal getOpacityForKeyFrame(Layer* layer, const KeyFrame* keyframe) const;
    void setOpacityForKeyFrame(Layer* layer, KeyFrame* keyframe, qreal opacity);

    /** Resolves candidate positions to exact keyframe positions and their
     *  current opacities (parallel lists); positions without a keyframe
     *  are skipped. */
    void gatherOpacities(Layer* layer, const QList<int>& candidates,
                         QList<int>& positions, QList<qreal>& opacities) const;
    /** Records the applied change as one undo step; reads the new values
     *  back from the keyframes. No-ops when nothing changed. */
    void recordOpacityChange(Layer* layer, const QList<int>& positions,
                             const QList<qreal>& oldOpacities, const QString& description);

    void updateValues(qreal opacity);
    void setOpacityForCurrentKeyframe();
    void setOpacityForSelectedKeyframes();
    void setOpacityForLayer();

    void updateSelectedFramesUI();

    Ui::LayerOpacityDialog *ui;

    Editor* mEditor = nullptr;
    LayerManager* mLayerManager = nullptr;
    PlaybackManager* mPlayBack = nullptr;

    bool mPlayerIsPlaying = false;

    qreal mMultiplier = 500.0;
    qreal mSpinBoxMultiplier = 0.2;
    int mMinSelectedFrames = 3;
};

#endif // LAYEROPACITYDIALOG_H
