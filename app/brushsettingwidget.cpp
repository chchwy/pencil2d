#include "brushsettingwidget.h"

#include <QLayout>
#include <QSignalBlocker>

BrushSettingWidget::BrushSettingWidget(const QString name, BrushSettingType settingType, QWidget* parent) : QWidget(parent),
    mSettingType(settingType)
{
    QHBoxLayout* hboxLayout = new QHBoxLayout();
    setLayout(hboxLayout);

    mValueSlider = new SpinSlider();
    mValueSlider->init(name, SpinSlider::GROWTH_TYPE::LINEAR, SpinSlider::VALUE_TYPE::FLOAT, 0, 2);
    mValueBox = new QDoubleSpinBox();

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    hboxLayout->setMargin(0);

    hboxLayout->addWidget(mValueSlider);
    hboxLayout->addWidget(mValueBox);

    connect(mValueSlider, &SpinSlider::valueChanged, this, &BrushSettingWidget::updateSetting);
    connect(mValueBox, static_cast<void(QDoubleSpinBox::*)(qreal)>(&QDoubleSpinBox::valueChanged), this, &BrushSettingWidget::updateSetting);
}

void BrushSettingWidget::setSetting(qreal value)
{
    QSignalBlocker b(mValueSlider);
    mValueSlider->setValue(value);
    QSignalBlocker b2(mValueBox);
    mValueBox->setValue(value);
}

void BrushSettingWidget::updateSetting(qreal value)
{
    QSignalBlocker b(mValueSlider);
    mValueSlider->setValue(value);

    QSignalBlocker b2(mValueBox);
    mValueBox->setValue(value);
    emit brushSettingChanged(value, this->mSettingType);
}


