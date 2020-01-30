#include "brushsettingwidget.h"

#include <QLayout>
#include <QSignalBlocker>
#include <QDebug>
#include <QtMath>
#include <QDoubleSpinBox>
#include <QToolButton>
#include <QVector>

#include "spinslider.h"
#include "mpmappingwidget.h"
#include "mappingdistributionwidget.h"
#include "mpmappingoptionswidget.h"

#include "mathutils.h"

BrushSettingWidget::BrushSettingWidget(const QString name, BrushSettingType settingType, qreal min, qreal max, QWidget* parent) : QWidget(parent),
    mSettingType(settingType), mParent(parent)
{
    QGridLayout* gridLayout = new QGridLayout(this);
    setLayout(gridLayout);

    mValueSlider = new SpinSlider(this);
    mValueSlider->init(name, SpinSlider::GROWTH_TYPE::LINEAR, SpinSlider::VALUE_TYPE::FLOAT, min, max);
    mValueBox = new QDoubleSpinBox();

    mValueBox->setRange(min, max);

    #if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        mValueBox->setStepType(QDoubleSpinBox::StepType::AdaptiveDecimalStepType);
    #endif

    mValueBox->setDecimals(2);

    mVisualBox = new QDoubleSpinBox(this);

    mMappedMin = min;
    mMappedMax = max;

    mMappingButton = new QToolButton(this);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    gridLayout->setMargin(0);
    gridLayout->addWidget(mValueSlider,0,0);
    gridLayout->addWidget(mValueBox,0,1);
    gridLayout->addWidget(mMappingButton,0,2);

    gridLayout->addWidget(mVisualBox, 0, 1);

    mVisualBox->setGeometry(mValueBox->geometry());
    mVisualBox->setHidden(true);

    mValueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(mValueSlider, &SpinSlider::valueChanged, this, &BrushSettingWidget::updateSetting);
    connect(mValueBox, static_cast<void(QDoubleSpinBox::*)(qreal)>(&QDoubleSpinBox::valueChanged), this, &BrushSettingWidget::updateSetting);
    connect(mMappingButton, &QToolButton::pressed, this, &BrushSettingWidget::openMappingWindow);
}

void BrushSettingWidget::setValue(qreal value)
{
    qreal normalize = MathUtils::normalize(value, mMin, mMax);
    qreal mappedValue = MathUtils::mapFromNormalized(normalize, mMappedMin, mMappedMax);

    QSignalBlocker b(mValueSlider);
    mValueSlider->setValue(mappedValue);
    QSignalBlocker b2(mValueBox);
    mValueBox->setValue(mappedValue);

    mVisualBox->setValue(value);

    qDebug() << "mapped value: " << mappedValue;
    qDebug() << "visual value: " << value;

    mCurrentValue = value;
}

void BrushSettingWidget::changeText()
{
    bool shouldHide = !mVisualBox->isHidden();
    mVisualBox->setHidden(shouldHide);
    mValueBox->setHidden(!shouldHide);
}

void BrushSettingWidget::setValueInternal(qreal value)
{
    QSignalBlocker b(mValueSlider);
    mValueSlider->setValue(value);
    QSignalBlocker b2(mValueBox);
    mValueBox->setValue(value);

    qreal normalize = MathUtils::normalize(value, mMappedMin, mMappedMax);
    qreal mappedToOrig = MathUtils::mapFromNormalized(normalize, mMin, mMax);
    mVisualBox->setValue(mappedToOrig);

    mCurrentValue = value;
}

void BrushSettingWidget::setRange(qreal min, qreal max)
{

    mMin = min;
    mMax = max;

    setValue(mCurrentValue);
}

void BrushSettingWidget::setToolTip(QString toolTip)
{
    mValueBox->setToolTip(toolTip);
    mValueSlider->setToolTip(toolTip);
}

void BrushSettingWidget::updateSetting(qreal value)
{
    qreal normalize = MathUtils::normalize(value, mMappedMin, mMappedMax);
    qreal mappedToOrig = MathUtils::mapFromNormalized(normalize, mMin, mMax);
    setValueInternal(value);

    emit brushSettingChanged(mappedToOrig, this->mSettingType);
}

void BrushSettingWidget::updateBrushMapping(QVector<QPointF> newPoints, BrushInputType inputType)
{
    qDebug() << "updating brush mapping";
    emit brushMappingForInputChanged(newPoints, this->mSettingType, inputType);
}

void BrushSettingWidget::openMappingWindow()
{
    QVector<QPointF> tempPoints = { QPointF(0.0,0.0), QPointF(0.5,0.5), QPointF(1.0,1.0) };
    MPMappingOptionsWidget* widget = new MPMappingOptionsWidget(this->mSettingType);
    widget->setCore(mEditor);
    widget->initUI();

    widget->show();

    connect(widget, &MPMappingOptionsWidget::mappingForInputUpdated, this, &BrushSettingWidget::updateBrushMapping);
}


