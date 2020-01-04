#ifndef BRUSHSETTINGWIDGET_H
#define BRUSHSETTINGWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>

#include "brushsetting.h"
#include "spinslider.h"

class BrushSettingWidget : public QWidget
{
    Q_OBJECT
public:
    BrushSettingWidget(const QString name, BrushSettingType settingType, QWidget* parent = nullptr);

    void setSetting(qreal value);

Q_SIGNALS:
    void brushSettingChanged(qreal value, BrushSettingType setting);

private:

    void updateSetting(qreal value);

    QDoubleSpinBox* mValueBox = nullptr;
    SpinSlider* mValueSlider = nullptr;
    BrushSettingType mSettingType;
};

#endif // BRUSHSETTINGWIDGET_H
