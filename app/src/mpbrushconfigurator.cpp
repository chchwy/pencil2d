#include "mpbrushconfigurator.h"

#include <QTreeWidget>
#include <QLayout>
#include <QSpinBox>
#include <QScrollArea>
#include <QDebug>
#include <QtMath>
#include <QSplitter>
#include <QPushButton>
#include <QToolBar>

#include "spinslider.h"
#include "brushsettingitem.h"
#include "brushsettingwidget.h"
#include "editor.h"

MPBrushConfigurator::MPBrushConfigurator(QWidget *parent)
  : QDialog(parent)
{
    resize(QSize(400,400));
    setWindowTitle(tr("Brush Configurator", "Window title of mypaint brush configurator"));

    mNavigatorWidget = new QTreeWidget(parent);
    mNavigatorWidget->setRootIsDecorated(false);
    mNavigatorWidget->setHeaderHidden(true);

    QSplitter* viewSplitter = new QSplitter;
    QHBoxLayout* hLayout = new QHBoxLayout(parent);
    QVBoxLayout* vLayout = new QVBoxLayout(parent);
    QScrollArea* scrollArea = new QScrollArea(nullptr);

    QToolBar* toolbar = new QToolBar(this);
    QPushButton* saveAndOverwriteBrush = new QPushButton(this);

    saveAndOverwriteBrush->setText("Save");
    QPushButton* createNewBrush = new QPushButton(this);

    createNewBrush->setText("Save as new");
    mMapValuesButton = new QPushButton(this);
    mMapValuesButton->setText("MyPaint values");

    setLayout(vLayout);

    QWidget* settingsContainer = new QWidget(this);

    settingsContainer->setLayout(hLayout);

    toolbar->addWidget(createNewBrush);
    toolbar->addWidget(saveAndOverwriteBrush);

    toolbar->addWidget(mMapValuesButton);

    vLayout->addWidget(toolbar);
    vLayout->addWidget(settingsContainer);

    viewSplitter->addWidget(mNavigatorWidget);
    viewSplitter->addWidget(scrollArea);
    hLayout->addWidget(viewSplitter);
    hLayout->setMargin(0);

    viewSplitter->setSizes({150, 500});
    viewSplitter->setStretchFactor(1,4);
    viewSplitter->setStretchFactor(0,0);

    mBrushSettingsWidget = new QWidget(parent);

    scrollArea->setWidget(mBrushSettingsWidget);
    scrollArea->setWidgetResizable(true);

    vBoxLayout = new QVBoxLayout();
    mBrushSettingsWidget->setLayout(vBoxLayout);

    connect(mNavigatorWidget, &QTreeWidget::itemPressed, this, &MPBrushConfigurator::brushCategorySelected);
    connect(mNavigatorWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MPBrushConfigurator::brushCategorySelectionChanged);
    connect(mMapValuesButton, &QPushButton::pressed, this, &MPBrushConfigurator::updateMapValuesButton);
}

void MPBrushConfigurator::initUI()
{
    auto basicRoot = addTreeRoot(BrushSettingItem::Basic, mNavigatorWidget, tr("Basic settings"));

    auto advanceRoot = addTreeRoot(BrushSettingItem::Advanced, mNavigatorWidget, tr("Advanced settings"));
    addTreeChild(BrushSettingItem::Opacity, advanceRoot, tr("Opacity settings"));
    addTreeChild(BrushSettingItem::Dab, advanceRoot, tr("Dab settings"));
    addTreeChild(BrushSettingItem::Random, advanceRoot, tr("Random settings"));
    addTreeChild(BrushSettingItem::Speed, advanceRoot, tr("Speed settings"));
    addTreeChild(BrushSettingItem::Offset, advanceRoot, tr("Offset settings"));
    addTreeChild(BrushSettingItem::Tracking, advanceRoot, tr("Tracking settings"));
    addTreeChild(BrushSettingItem::Color, advanceRoot, tr("Color settings"));
    addTreeChild(BrushSettingItem::Smudge, advanceRoot, tr("Smudge settings"));
    addTreeChild(BrushSettingItem::Eraser, advanceRoot, tr("Eraser setting"));
    addTreeChild(BrushSettingItem::Stroke, advanceRoot, tr("Stroke settings"));
    addTreeChild(BrushSettingItem::Custom_Input, advanceRoot, tr("Custom Input settings"));
    addTreeChild(BrushSettingItem::Elliptical_Dab, advanceRoot, tr("Elliptical settings"));
    addTreeChild(BrushSettingItem::Other, advanceRoot, tr("Other settings"));

    updateSettingsView(basicRoot);
}

void MPBrushConfigurator::updateUI()
{
    for (BrushSettingType type : allSettings) {
        for (BrushSettingWidget* widget : mBrushWidgets) {
            if (widget->setting() == type) {
                BrushSettingInfo info = mEditor->getBrushSettingInfo(type);
                widget->setValue(static_cast<qreal>(mEditor->getMPBrushSetting(type)));
                widget->setRange(static_cast<qreal>(info.min), static_cast<qreal>(info.max));
                widget->setToolTip(info.tooltip);
            }
        }
    }
}

void MPBrushConfigurator::updateConfig(QString toolName, QString brushName, const QByteArray& content)
{
    Q_UNUSED(toolName)
    Q_UNUSED(brushName)
    Q_UNUSED(content)
    updateUI();
}

void MPBrushConfigurator::prepareBasicBrushSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Opacity"), BrushSettingType::BRUSH_SETTING_OPAQUE, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Radius"), BrushSettingType::BRUSH_SETTING_RADIUS_LOGARITHMIC, 0, 200));
    mBrushWidgets.append(new BrushSettingWidget(tr("Hardness"), BrushSettingType::BRUSH_SETTING_HARDNESS, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Softness"), BrushSettingType::BRUSH_SETTING_SOFTNESS, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Anti-aliasing"), BrushSettingType::BRUSH_SETTING_ANTI_ALIASING,0, 100));

    mBrushWidgets.append(new BrushSettingWidget(tr("Pressure gain"), BrushSettingType::BRUSH_SETTING_PRESSURE_GAIN_LOG, 0, 100));
}

void MPBrushConfigurator::prepareAdvancedBrushSettings()
{
    prepareOpacitySettings();
    prepareDabSettings();
    prepareRandomSettings();
    prepareSpeedSettings();
    prepareOffsetSettings();
    prepareTrackingSettings();
    prepareColorSettings();
    prepareSmudgeSettings();
    prepareEraserSetting();
    prepareStrokeSettings();
    prepareCustomInputSettings();
    prepareEllipticalDabSettings();
    prepareOtherSettings();
}

void MPBrushConfigurator::prepareOpacitySettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Opacity"), BrushSettingType::BRUSH_SETTING_OPAQUE, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Opacity multiply"), BrushSettingType::BRUSH_SETTING_OPAQUE_MULTIPLY, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Opacity linearize"), BrushSettingType::BRUSH_SETTING_OPAQUE_LINEARIZE, 0, 100));
}

void MPBrushConfigurator::prepareDabSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Radius"), BrushSettingType::BRUSH_SETTING_RADIUS_LOGARITHMIC, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Hardness"), BrushSettingType::BRUSH_SETTING_HARDNESS, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dabs per basic radius"), BrushSettingType::BRUSH_SETTING_DABS_PER_BASIC_RADIUS, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dabs per actual radius"), BrushSettingType::BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dabs per second"), BrushSettingType::BRUSH_SETTING_DABS_PER_SECOND, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dab scale"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dab scale X"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_X, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dab scale Y"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_Y, 0, 100));
}

void MPBrushConfigurator::prepareRandomSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Radius by random"), BrushSettingType::BRUSH_SETTING_RADIUS_BY_RANDOM, 0, 100));
}

void MPBrushConfigurator::prepareSpeedSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Speed start"), BrushSettingType::BRUSH_SETTING_SPEED1_SLOWNESS, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Speed end"), BrushSettingType::BRUSH_SETTING_SPEED2_SLOWNESS, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Speed gamma start"), BrushSettingType::BRUSH_SETTING_SPEED1_GAMMA, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Speed gamma end"), BrushSettingType::BRUSH_SETTING_SPEED2_GAMMA, 0, 100));
}

void MPBrushConfigurator::prepareOffsetSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset by random"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_RANDOM, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset X"), BrushSettingType::BRUSH_SETTING_OFFSET_X, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset Y"), BrushSettingType::BRUSH_SETTING_OFFSET_Y, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle left"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle left ascend"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ASC, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle right"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle right ascend"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2_ASC, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle adjecent"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ADJ, -180, 180));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset multiplier"), BrushSettingType::BRUSH_SETTING_OFFSET_MULTIPLIER, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset by speed"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset by speed slowness"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS, 0, 100));
}

void MPBrushConfigurator::prepareTrackingSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Slow tracking"), BrushSettingType::BRUSH_SETTING_SLOW_TRACKING, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Slow tracking per. dab"), BrushSettingType::BRUSH_SETTING_SLOW_TRACKING_PER_DAB, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Tracking noise"), BrushSettingType::BRUSH_SETTING_TRACKING_NOISE, 0, 100));
}

void MPBrushConfigurator::prepareColorSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Hue"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_H, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Lightness"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_L, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Saturation"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSL_S, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Value"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_V, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Saturation"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSV_S, -100, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Restore color"), BrushSettingType::BRUSH_SETTING_RESTORE_COLOR, 0, 100));
}

void MPBrushConfigurator::prepareSmudgeSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Smudge"), BrushSettingType::BRUSH_SETTING_SMUDGE, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Smudge length"), BrushSettingType::BRUSH_SETTING_SMUDGE_LENGTH, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Smudge radius"), BrushSettingType::BRUSH_SETTING_SMUDGE_RADIUS_LOG, 0, 100));
}

void MPBrushConfigurator::prepareEraserSetting()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Eraser"), BrushSettingType::BRUSH_SETTING_ERASER, 0, 100));
}

void MPBrushConfigurator::prepareStrokeSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Stroke threshold"), BrushSettingType::BRUSH_SETTING_STROKE_THRESHOLD, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Stroke duration"), BrushSettingType::BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Stroke holdtime"), BrushSettingType::BRUSH_SETTING_STROKE_HOLDTIME, 0, 100));
}

void MPBrushConfigurator::prepareCustomInputSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Custom input"), BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Custom input slowness"), BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS, 0, 100));
}

void MPBrushConfigurator::prepareEllipticalDabSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Elleptical dab ratio"), BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_RATIO, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Elleptical dab angle"), BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE, 0, 100));
}

void MPBrushConfigurator::prepareOtherSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Anti-aliasing"), BrushSettingType::BRUSH_SETTING_ANTI_ALIASING, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Lock Alpha"), BrushSettingType::BRUSH_SETTING_LOCK_ALPHA, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Colorize"), BrushSettingType::BRUSH_SETTING_COLORIZE, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Snap to pixel"), BrushSettingType::BRUSH_SETTING_SNAP_TO_PIXEL, 0, 100));
    mBrushWidgets.append(new BrushSettingWidget(tr("Pressure gain"), BrushSettingType::BRUSH_SETTING_PRESSURE_GAIN_LOG, 0, 100));
}

BrushSettingItem* MPBrushConfigurator::addTreeRoot(BrushSettingItem::Category category, QTreeWidget* treeWidget, const QString name)
{
    BrushSettingItem* treeItem = new BrushSettingItem(category, treeWidget);
    treeItem->setText(0, name);
    return treeItem;
}

BrushSettingItem* MPBrushConfigurator::addTreeChild(BrushSettingItem::Category category, QTreeWidgetItem* parent, const QString name)
{
    BrushSettingItem *treeItem = new BrushSettingItem(category, parent);
    treeItem->setText(0, name);
    parent->addChild(treeItem);
    return treeItem;
}

void MPBrushConfigurator::updateSettingsView(QTreeWidgetItem* item)
{
    qDeleteAll(mBrushSettingsWidget->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly));
    removeBrushSettingSpacers();

    if (!mBrushWidgets.isEmpty()) {
        mBrushWidgets.clear();
    }

    switch(static_cast<BrushSettingItem*>(item)->ItemCategory()) {
        case BrushSettingItem::Basic: prepareBasicBrushSettings(); break;
        case BrushSettingItem::Advanced: prepareAdvancedBrushSettings(); break;
        case BrushSettingItem::Opacity: prepareOpacitySettings(); break;
        case BrushSettingItem::Speed: prepareSpeedSettings(); break;
        case BrushSettingItem::Dab: prepareDabSettings(); break;
        case BrushSettingItem::Random: prepareRandomSettings(); break;
        case BrushSettingItem::Offset: prepareOffsetSettings(); break;
        case BrushSettingItem::Tracking: prepareTrackingSettings(); break;
        case BrushSettingItem::Color: prepareColorSettings(); break;
        case BrushSettingItem::Smudge: prepareSmudgeSettings(); break;
        case BrushSettingItem::Eraser: prepareEraserSetting(); break;
        case BrushSettingItem::Stroke: prepareStrokeSettings(); break;
        case BrushSettingItem::Custom_Input: prepareCustomInputSettings(); break;
        case BrushSettingItem::Elliptical_Dab: prepareEllipticalDabSettings(); break;
        case BrushSettingItem::Other: prepareOtherSettings(); break;
        case BrushSettingItem::Unknown: return;
    }

    for (BrushSettingWidget* item : mBrushWidgets) {
        vBoxLayout->addWidget(item);

        connect(item, &BrushSettingWidget::brushSettingChanged, this, &MPBrushConfigurator::updateBrushSetting);
        connect(mMapValuesButton, &QPushButton::pressed, item, &BrushSettingWidget::changeText);
    }

    addBrushSettingsSpacer();
    updateUI();
}

void MPBrushConfigurator::brushCategorySelectionChanged(const QItemSelection &, const QItemSelection &)
{
    updateSettingsView(mNavigatorWidget->currentItem());
}

void MPBrushConfigurator::brushCategorySelected(QTreeWidgetItem* item, int )
{
    updateSettingsView(item);
}

void MPBrushConfigurator::removeBrushSettingSpacers()
{
    for (int i = 0; i < vBoxLayout->count(); i++) {
        if (static_cast<QSpacerItem*>(vBoxLayout->itemAt(i)) != nullptr) {
            vBoxLayout->removeItem(vBoxLayout->itemAt(i));
        }
    }
}

void MPBrushConfigurator::addBrushSettingsSpacer()
{
    QSpacerItem *spacer = new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vBoxLayout->addItem(spacer);
}

void MPBrushConfigurator::updateBrushSetting(qreal value, BrushSettingType settingType)
{
    mEditor->setMPBrushSetting(settingType, static_cast<float>(value));
}

void MPBrushConfigurator::updateMapValuesButton()
{
    mMapValuesButtonPressed = !mMapValuesButtonPressed;
    if (mMapValuesButtonPressed) {
        mMapValuesButton->setText(tr("Pencil2D values"));
    } else {
        mMapValuesButton->setText(tr("MyPaint values"));
    }
}

