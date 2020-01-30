#include "mpmappingoptionswidget.h"

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QSplitter>
#include <QAction>
#include <QDebug>
#include <QComboBox>
#include <QScrollArea>
#include <QStandardItemModel>

#include <spinslider.h>
#include "brushsetting.h"
#include "editor.h"

#include "mpmappingwidget.h"

MPComboBox::MPComboBox(QWidget* parent)
    : QComboBox(parent)
{

    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MPComboBox::triggerActivatedVariant);
}

void MPComboBox::addItem(const QString& text)
{
    QComboBox::addItem(text);
}

void MPComboBox::addItem(const QString& text, BrushInputType input)
{
    QComboBox::addItem(text, QVariant(static_cast<int>(input)));
}

void MPComboBox::insertItem(const QString& text, int index, BrushInputType input)
{
    QComboBox::insertItem(index, text, QVariant(static_cast<int>(input)));
}

void MPComboBox::triggerActivatedVariant(int index)
{
    int data = itemData(index).toInt();
    BrushInputType input = static_cast<BrushInputType>(data);
    activated(index, input);
}

void MPComboBox::setItemEnabled(int index, bool enabled)
{
    auto model = this->model();
    auto itemModel = static_cast<QStandardItemModel*>(model);
    auto item = itemModel->item(index, 0);
    item->setEnabled( enabled );
}

MPInputButton::MPInputButton(BrushInputType inputType, QWidget* parent)
    : QToolButton(parent), mInputType(inputType)
{
    connect(this, &QToolButton::pressed, this, &MPInputButton::pressed);
}

MPInputButton::MPInputButton(MPInputButton* inputButton)
    : QToolButton(inputButton)
{
    mInputType = inputButton->mInputType;
    connect(this, &QToolButton::pressed, this, &MPInputButton::pressed);
}

void MPInputButton::pressed()
{
    emit didPress(mInputType);
}

MPMappingOptionsWidget::MPMappingOptionsWidget(BrushSettingType settingType, QWidget* parent)
    : QWidget(parent), mSettingType(settingType), mParent(parent)
{
    this->setWindowTitle(tr("Mypaint input mapping"));
}

void MPMappingOptionsWidget::initUI()
{
    setupUI();
}

void MPMappingOptionsWidget::setupUI()
{
    mGridLayout = new QGridLayout(mParent);
    mHBoxLayout = new QHBoxLayout(mParent);
    QScrollArea* scrollArea = new QScrollArea(mParent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    scrollArea->setMinimumWidth(300);

    int rowY = 2;

    QWidget* container = new QWidget(mParent);
    container->setLayout(mGridLayout);
    mGridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    mMappingOptionsCombBox = new MPComboBox(mParent);
    QLabel* descriptionLabel = new QLabel("Select an input from the dropdown\nto add a new input to map");
    descriptionLabel->setMaximumSize(descriptionLabel->minimumSizeHint());
    descriptionLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    mGridLayout->addWidget(descriptionLabel, 0, 0, 1, 0);
    mGridLayout->addWidget(mMappingOptionsCombBox, 1, 0, 1, 3);

//    mMappingOptionsCombBox->addItem("----------");
    for (int i = 0; i < static_cast<int>(BrushInputType::BRUSH_INPUTS_COUNT); i++) {

        BrushInputType input = static_cast<BrushInputType>(i);

        auto inputMapping = mEditor->getBrushInputMapping(mSettingType, input);

        mMappingOptionsCombBox->addItem(getBrushInputName(input), input);
        if (inputMapping.controlPoints.numberOfPoints > 0) {
            MPMappingOption option = createMappingOption(input);
            mMappingOptionsCombBox->setItemEnabled(i, false);

            mOptions << option;
        }
        rowY++;
    }

    scrollArea->setWidget(container);
    mHBoxLayout->addWidget(scrollArea);

    mMappingWidget = new MPMappingWidget(this);
    mHBoxLayout->addWidget(mMappingWidget);

    connect(mMappingOptionsCombBox, static_cast<void (MPComboBox::*)(int,const BrushInputType&)>(&MPComboBox::activated), this, &MPMappingOptionsWidget::addOptionField);

    setLayout(mHBoxLayout);
}

MPMappingOptionsWidget::MPMappingOption MPMappingOptionsWidget::createMappingOption(BrushInputType input)
{
    QLabel* settingDescLabel = new QLabel(getBrushInputName(input), nullptr);

    auto inputMapping = mEditor->getBrushInputMapping(mSettingType, input);
    MPInputButton* mappingButton = new MPInputButton(input, nullptr);
    mappingButton->setIcon(QIcon(":/app/icons/new/mapping-icon.png"));

    MPInputButton* removeActionButton = new MPInputButton(input, nullptr);
    removeActionButton->setIcon(QIcon(":/app/icons/new/trash-changes.png"));

    int row =  mGridLayout->rowCount();

    MPMappingOption option(mappingButton, removeActionButton, settingDescLabel);
    option.inputType = input;

    mGridLayout->addWidget(option.removeActionButton, row, 0, 1,1);
    mGridLayout->addWidget(option.settingDescLabel, row, 1, 1,1);
    mGridLayout->addWidget(option.mappingButton, row, 2, 1,1);

    connect(option.mappingButton, &MPInputButton::didPress, this, &MPMappingOptionsWidget::showInputMapper);
    connect(option.removeActionButton, &MPInputButton::didPress, this, &MPMappingOptionsWidget::removeAction);

    return option;
}

void MPMappingOptionsWidget::showInputMapper(BrushInputType inputType)
{
    auto mapping = mEditor->getBrushInputMapping(mSettingType,inputType);
    auto info = mEditor->getBrushInputInfo(inputType);

    if (mMappingWidget) {
        delete mMappingWidget;
    }

    mMappingWidget = new MPMappingWidget(getBrushInputName(inputType), info.soft_min, info.soft_max, inputType, mapping.controlPoints.points, 8, this);
    mHBoxLayout->addWidget(mMappingWidget);

    connect(mMappingWidget, &MPMappingWidget::mappingForInputUpdated, this, &MPMappingOptionsWidget::mappingForInputUpdated);
}

void MPMappingOptionsWidget::addOptionField(int index, BrushInputType input)
{
    MPMappingOption option = createMappingOption(input);

    mMappingOptionsCombBox->setItemEnabled(index, false);
    showInputMapper(input);

    mOptions << option;
}

void MPMappingOptionsWidget::removeAction(BrushInputType input)
{
    // this should not be possible...
    if (mOptions.empty()) {
        return;
    }

//     TODO: figure out if we can use qscopedpointer instead of deleting manually
//     that's no good... :/

    for (int i = 0; i < mOptions.count(); i++) {

        MPMappingOption& option = mOptions[i];
        if (input == mOptions[i].inputType) {

            delete mMappingWidget;
            mMappingWidget = nullptr;

            option.deleteAll();
            mOptions.removeAt(i);
            break;
        }
    }

    mMappingWidget = new MPMappingWidget(this);
    mHBoxLayout->addWidget(mMappingWidget);

    int index = static_cast<int>(input);
    mMappingOptionsCombBox->setItemEnabled(index, true);
}

