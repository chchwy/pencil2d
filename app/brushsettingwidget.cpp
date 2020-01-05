#include "brushsettingwidget.h"

#include <QLayout>
#include <QSignalBlocker>
#include <QDebug>
#include <QtMath>

BrushSettingWidget::BrushSettingWidget(const QString name, BrushSettingType settingType, QWidget* parent) : QWidget(parent),
    mSettingType(settingType)
{
    QGridLayout* gridLayout = new QGridLayout();
    setLayout(gridLayout);

    mValueSlider = new SpinSlider();
    mValueSlider->init(name, SpinSlider::GROWTH_TYPE::LINEAR, SpinSlider::VALUE_TYPE::FLOAT, 0, 100);
    mValueBox = new QDoubleSpinBox();
    mValueBox->setStepType(QSpinBox::StepType::AdaptiveDecimalStepType);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    gridLayout->setMargin(0);
    gridLayout->addWidget(mValueSlider,0,0);
    gridLayout->addWidget(mValueBox,0,1);

    mValueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(mValueSlider, &SpinSlider::valueChanged, this, &BrushSettingWidget::updateSetting);
    connect(mValueBox, static_cast<void(QDoubleSpinBox::*)(qreal)>(&QDoubleSpinBox::valueChanged), this, &BrushSettingWidget::updateSetting);
}

void BrushSettingWidget::setValue(qreal value)
{
    QSignalBlocker b(mValueSlider);
    mValueSlider->setValue(value);
    QSignalBlocker b2(mValueBox);
    mValueBox->setValue(value);

    mCurrentValue = value;
}

void BrushSettingWidget::setRange(qreal min, qreal max)
{
    mValueBox->setRange(min, max);
    mValueSlider->setRange(min, max);

    setValue(mCurrentValue);
}

void BrushSettingWidget::setToolTip(QString toolTip)
{
    mValueBox->setToolTip(toolTip);
    mValueSlider->setToolTip(toolTip);
}

void BrushSettingWidget::updateSetting(qreal value)
{
    setValue(value);
    emit brushSettingChanged(value, this->mSettingType);
}


