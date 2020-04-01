#ifndef BRUSHSETTINGWIDGET_H
#define BRUSHSETTINGWIDGET_H

#include <QWidget>

#include "brushsetting.h"

class QToolButton;
class SpinSlider;
class QDoubleSpinBox;
class Editor;
class MPMappingOptionsWidget;

class BrushSettingWidget : public QWidget
{
    Q_OBJECT
public:
    BrushSettingWidget(const QString name, BrushSettingType settingType, qreal min, qreal max, QWidget* parent = nullptr);

    void setValue(qreal value);
    void setRange(qreal min, qreal max);
    void setToolTip(QString toolTip);
    void setCore(Editor* editor) { mEditor = editor; }
    void updateUI();

    void closeMappingWindow();

    void changeText();

    BrushSettingType setting() { return mSettingType; }
    qreal currentValue() { return mCurrentValue; }

public slots:
    void notifyInputMappingRemoved(BrushInputType input);

Q_SIGNALS:
    void brushSettingChanged(qreal previousValue, qreal value, BrushSettingType setting);
    void brushMappingForInputChanged(QVector<QPointF> points, BrushSettingType setting, BrushInputType inputType);
    void brushMappingRemoved(BrushSettingType setting, BrushInputType);

private:

    void onSliderChanged(qreal value);
    void openMappingWindow();

    void setValueInternal(qreal value);

    void updateSetting(qreal value);
    void updateBrushMapping(QVector<QPointF> newPoints, BrushInputType inputType);

    QDoubleSpinBox* mValueBox = nullptr;
    QDoubleSpinBox* mVisualBox = nullptr;
    SpinSlider* mValueSlider = nullptr;
    QToolButton* mMappingButton = nullptr;
    BrushSettingType mSettingType;

    Editor* mEditor = nullptr;

    qreal mMin = 0.0;
    qreal mMax = 0.0;

    qreal mMappedMin = 0.0;
    qreal mMappedMax = 0.0;

    qreal mMappedValue = 0.0;

    qreal mCurrentValue;
    qreal mInitialValue;

    QWidget* mParent = nullptr;
    MPMappingOptionsWidget* mMappingWidget = nullptr;

    const QString mSettingName;

    bool first = false;
};

#endif // BRUSHSETTINGWIDGET_H
