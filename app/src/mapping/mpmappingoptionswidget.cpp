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

#include "combobox.h"
#include "mpmappingwidget.h"

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

MPMappingOptionsWidget::MPMappingOptionsWidget(QString optionName, BrushSettingType settingType, QWidget* parent)
    : QDialog(parent, Qt::Tool), mSettingType(settingType), mParent(parent)
{
    this->setWindowTitle(QString(optionName) + " " + tr("input mapping"));
}

void MPMappingOptionsWidget::initUI()
{
    setupUI();
}

void MPMappingOptionsWidget::setupUI()
{
    mHBoxLayout = new QHBoxLayout(mParent);

    mHBoxLayout->setContentsMargins(5,5,5,5);
    mGridLayout = new QGridLayout(mParent);
    mGridLayout->setContentsMargins(0,0,0,0);
    QScrollArea* scrollArea = new QScrollArea(mParent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    scrollArea->setMinimumWidth(300);

    int rowY = 2;

    QWidget* container = new QWidget(mParent);
    container->setLayout(mGridLayout);
    mGridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    mMappingOptionsComboBox = new ComboBox(mParent);
    QLabel* descriptionLabel = new QLabel("Select an input from the dropdown\nto add a new input to map");
    descriptionLabel->setMaximumSize(descriptionLabel->minimumSizeHint());
    descriptionLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    mGridLayout->addWidget(descriptionLabel, 0, 0, 1, 0);
    mGridLayout->addWidget(mMappingOptionsComboBox, 1, 0, 1, 3);

    scrollArea->setWidget(container);
    mHBoxLayout->addWidget(scrollArea);

    for (int i = 0; i < static_cast<int>(BrushInputType::BRUSH_INPUTS_COUNT); i++) {

        BrushInputType input = static_cast<BrushInputType>(i);

        auto inputMapping = mEditor->getBrushInputMapping(mSettingType, input);

        mMappingOptionsComboBox->addItem(getBrushInputName(input), i);
        if (inputMapping.controlPoints.numberOfPoints > 0) {

            if (mMappingWidget == nullptr) {
                // if there's at least one input already, show mapper
                showInputMapper(input);
            }

            MPMappingOption option = createMappingOption(input);
            mMappingOptionsComboBox->setItemEnabled(i, false);

            mOptions << option;
        }
        rowY++;
    }

    if (mOptions.isEmpty()) {
        mMappingWidget = new MPMappingWidget(this);
        mHBoxLayout->addWidget(mMappingWidget);
    }

    connect(mMappingOptionsComboBox, &ComboBox::activated, this, &MPMappingOptionsWidget::addOptionField);

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
        mHBoxLayout->removeWidget(mMappingWidget);
        delete mMappingWidget;
    }

    mMappingWidget = new MPMappingWidget(getBrushInputName(inputType), info.soft_min, info.soft_max, inputType, mapping.controlPoints.points, 8, this);
    mHBoxLayout->addWidget(mMappingWidget);

    connect(mMappingWidget, &MPMappingWidget::mappingForInputUpdated, this, &MPMappingOptionsWidget::mappingForInputUpdated);
}

void MPMappingOptionsWidget::addOptionField(int index, QString name, int value)
{
    Q_UNUSED(name)
    BrushInputType inputType = static_cast<BrushInputType>(value);
    MPMappingOption option = createMappingOption(inputType);

    mMappingOptionsComboBox->setItemEnabled(index, false);
    showInputMapper(inputType);

    mOptions << option;
}

void MPMappingOptionsWidget::removeAction(BrushInputType input)
{
    // this should not be possible...
    if (mOptions.empty()) {
        return;
    }

    for (int i = 0; i < mOptions.count(); i++) {

        MPMappingOption& option = mOptions[i];
        if (input == mOptions[i].inputType) {

            delete mMappingWidget;

            option.deleteAll();
            mOptions.removeAt(i);
            break;
        }
    }

    mMappingWidget = new MPMappingWidget(this);
    mHBoxLayout->addWidget(mMappingWidget);

    int index = static_cast<int>(input);
    mMappingOptionsComboBox->setItemEnabled(index, true);

    emit removedInputOption(input);
}

