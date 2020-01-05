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
    BrushSettingWidget(const QString name, BrushSettingType settingType, QWidget* parent = nullptr);

    void setValue(qreal value);
    void setRange(qreal min, qreal max);
    void setToolTip(QString toolTip);

    BrushSettingType setting() { return mSettingType; }

Q_SIGNALS:
    void brushSettingChanged(qreal value, BrushSettingType setting);

private:

    void updateSetting(qreal value);

    QDoubleSpinBox* mValueBox = nullptr;
    SpinSlider* mValueSlider = nullptr;
    BrushSettingType mSettingType;

    qreal mCurrentValue;
};

#endif // BRUSHSETTINGWIDGET_H
