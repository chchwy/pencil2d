#ifndef BRUSHSETTINGWIDGET_H
#define BRUSHSETTINGWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>
#include "spinslider.h"

#include "brushsetting.h"

class BrushSettingWidget : public QWidget
{
    Q_OBJECT
public:
    BrushSettingWidget(const QString name, BrushSettingType settingType, qreal min, qreal max, QWidget* parent = nullptr);

    void setValue(qreal value);
    void setRange(qreal min, qreal max);
    void setToolTip(QString toolTip);

    void changeText();

    BrushSettingType setting() { return mSettingType; }

Q_SIGNALS:
    void brushSettingChanged(qreal value, BrushSettingType setting);

private:

    void setValueInternal(qreal value);

    void updateSetting(qreal value);

    QDoubleSpinBox* mValueBox = nullptr;
    QDoubleSpinBox* mVisualBox = nullptr;
    SpinSlider* mValueSlider = nullptr;
    BrushSettingType mSettingType;

    qreal mMin = 0.0;
    qreal mMax = 0.0;

    qreal mMappedMin = 0.0;
    qreal mMappedMax = 0.0;

    qreal mMappedValue = 0.0;

    qreal mCurrentValue;
};

#endif // BRUSHSETTINGWIDGET_H
