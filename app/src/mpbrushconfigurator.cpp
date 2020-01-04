#include "mpbrushconfigurator.h"

#include <QTreeWidget>
#include <QLayout>
#include <QSpinBox>
#include <QScrollArea>
#include <QDebug>
#include <QtMath>

#include "spinslider.h"
#include "brushsettingitem.h"
#include "brushsettingwidget.h"
#include "editor.h"

MPBrushConfigurator::MPBrushConfigurator(QWidget *parent)
  : BaseDockWidget(parent)
{
    setWindowTitle(tr("Brush Configurator", "Window title of mypaint brush configurator"));

    QWidget* mainWidget = new QWidget(parent);
    mNavigatorWidget = new QTreeWidget(mainWidget);
    mNavigatorWidget->setRootIsDecorated(false);
    mNavigatorWidget->setHeaderHidden(true);

    setWidget(mainWidget);

    QHBoxLayout* hLayout = new QHBoxLayout(mainWidget);
    QScrollArea* scrollArea = new QScrollArea(nullptr);

    hLayout->addWidget(mNavigatorWidget);
    hLayout->addWidget(scrollArea);
    setLayout(hLayout);

    mBrushSettingsWidget = new QWidget(mainWidget);

    scrollArea->setWidget(mBrushSettingsWidget);
    scrollArea->setWidgetResizable(true);

    vBoxLayout = new QVBoxLayout();
    mBrushSettingsWidget->setLayout(vBoxLayout);

    showBasicBrushSettings();
    addBrushSettingsSpacer();

    addTreeRoot(BrushSettingItem::Basic, mNavigatorWidget, tr("Basic settings"), "test 1");

    auto advanceRoot = addTreeRoot(BrushSettingItem::Advanced, mNavigatorWidget, tr("Advanced settings"), "test 2");
    addTreeChild(BrushSettingItem::Opacity, advanceRoot, tr("Opacity settings"), "test 2");
    addTreeChild(BrushSettingItem::Dab, advanceRoot, tr("Dab settings"), "test 2");
    addTreeChild(BrushSettingItem::Random, advanceRoot, tr("Random settings"), "test 2");
    addTreeChild(BrushSettingItem::Offset, advanceRoot, tr("Offset settings"), "test 2");
    addTreeChild(BrushSettingItem::Tracking, advanceRoot, tr("Tracking settings"), "test 2");
    addTreeChild(BrushSettingItem::Color, advanceRoot, tr("Color settings"), "test 2");
    addTreeChild(BrushSettingItem::Smudge, advanceRoot, tr("Smudge settings"), "test 2");
    addTreeChild(BrushSettingItem::Eraser, advanceRoot, tr("Eraser setting"), "test 2");
    addTreeChild(BrushSettingItem::Stroke, advanceRoot, tr("Stroke settings"), "test 2");
    addTreeChild(BrushSettingItem::Custom_Input, advanceRoot, tr("Custom Input settings"), "test 2");
    addTreeChild(BrushSettingItem::Elliptical_Dab, advanceRoot, tr("Elliptical settings"), "test 2");
    addTreeChild(BrushSettingItem::Other, advanceRoot, tr("Other settings"), "test 2");

    connect(mNavigatorWidget, &QTreeWidget::itemPressed, this, &MPBrushConfigurator::brushCategorySelected);
    connect(mNavigatorWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MPBrushConfigurator::brushCategorySelectionChanged);
}

void MPBrushConfigurator::initUI()
{

}

void MPBrushConfigurator::updateUI()
{
    for (BrushSettingType type : allSettings) {
        qDebug() << mEditor->getMPBrushSetting(type);
    }
}

void MPBrushConfigurator::showBasicBrushSettings()
{
    QList<BrushSettingWidget*> items;

    items.append(new BrushSettingWidget(tr("Opacity"), BrushSettingType::BRUSH_SETTING_OPAQUE));
    items.append(new BrushSettingWidget(tr("Radius"), BrushSettingType::BRUSH_SETTING_RADIUS_LOGARITHMIC));
    items.append(new BrushSettingWidget(tr("Hardness"), BrushSettingType::BRUSH_SETTING_HARDNESS));
    items.append(new BrushSettingWidget(tr("Pressure gain"), BrushSettingType::BRUSH_SETTING_PRESSURE_GAIN_LOG));
    items.append(new BrushSettingWidget(tr("Anti-aliasing"), BrushSettingType::BRUSH_SETTING_ANTI_ALIASING));

    for (BrushSettingWidget* item : items) {
        vBoxLayout->addWidget(item);

        connect(item, &BrushSettingWidget::brushSettingChanged, this, &MPBrushConfigurator::updateBrushSetting);
    }
}

void MPBrushConfigurator::showAdvancedBrushSettings()
{
    showOpacitySettings();
    showDabSettings();
    showRandomSettings();
    showSpeedSettings();
    showOffsetSettings();
    showTrackingSettings();
    showColorSettings();
    showSmudgeSettings();
    showEraserSetting();
    showStrokeSettings();
    showCustomInputSettings();
    showEllipticalDabSettings();
    showOtherSettings();
}

void MPBrushConfigurator::showOpacitySettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Opacity multiply"), BrushSettingType::BRUSH_SETTING_OPAQUE_MULTIPLY));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Opacity linearize"), BrushSettingType::BRUSH_SETTING_OPAQUE_LINEARIZE));
}

void MPBrushConfigurator::showDabSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Dabs per basic radius"), BrushSettingType::BRUSH_SETTING_DABS_PER_BASIC_RADIUS));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Dabs per actual radius"), BrushSettingType::BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Dabs per second"), BrushSettingType::BRUSH_SETTING_DABS_PER_SECOND));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Dab scale"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Dab scale X"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_X));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Dab scale Y"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_Y));
}

void MPBrushConfigurator::showRandomSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Radius by random"), BrushSettingType::BRUSH_SETTING_RADIUS_BY_RANDOM));
}

void MPBrushConfigurator::showSpeedSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Speed start"), BrushSettingType::BRUSH_SETTING_SPEED1_SLOWNESS));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Speed end"), BrushSettingType::BRUSH_SETTING_SPEED2_SLOWNESS));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Speed gamma start"), BrushSettingType::BRUSH_SETTING_SPEED1_GAMMA));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Speed gamma end"), BrushSettingType::BRUSH_SETTING_SPEED2_GAMMA));
}

void MPBrushConfigurator::showOffsetSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset by random"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_RANDOM));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset X"), BrushSettingType::BRUSH_SETTING_OFFSET_X));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset Y"), BrushSettingType::BRUSH_SETTING_OFFSET_Y));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset angle left"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset angle left ascend"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ASC));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset angle right"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset angle right ascend"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2_ASC));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset angle adjecent"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ADJ));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset multiplier"), BrushSettingType::BRUSH_SETTING_OFFSET_MULTIPLIER));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset by speed"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Offset by speed slowness"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS));
}

void MPBrushConfigurator::showTrackingSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Slow tracking"), BrushSettingType::BRUSH_SETTING_SLOW_TRACKING));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Slow tracking per. dab"), BrushSettingType::BRUSH_SETTING_SLOW_TRACKING_PER_DAB));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Tracking noise"), BrushSettingType::BRUSH_SETTING_TRACKING_NOISE));
}

void MPBrushConfigurator::showColorSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Change color: Hue"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_H));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Change color: Lightness"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_L));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Change color: Saturation"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSL_S));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Change color: Value"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_V));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Change color: Saturation"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSV_S));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Restore color"), BrushSettingType::BRUSH_SETTING_RESTORE_COLOR));
}

void MPBrushConfigurator::showSmudgeSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Smudge"), BrushSettingType::BRUSH_SETTING_SMUDGE));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Smudge length"), BrushSettingType::BRUSH_SETTING_SMUDGE_LENGTH));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Smudge radius"), BrushSettingType::BRUSH_SETTING_SMUDGE_RADIUS_LOG));
}

void MPBrushConfigurator::showEraserSetting()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Eraser"), BrushSettingType::BRUSH_SETTING_ERASER));
}

void MPBrushConfigurator::showStrokeSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Stroke threshold"), BrushSettingType::BRUSH_SETTING_STROKE_THRESHOLD));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Stroke duration"), BrushSettingType::BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Stroke holdtime"), BrushSettingType::BRUSH_SETTING_STROKE_HOLDTIME));
}

void MPBrushConfigurator::showCustomInputSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Custom input"), BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Custom input slowness"), BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS));
}

void MPBrushConfigurator::showEllipticalDabSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Elleptical dab ratio"), BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_RATIO));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Elleptical dab angle"), BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE));
}

void MPBrushConfigurator::showOtherSettings()
{
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Lock Alpha"), BrushSettingType::BRUSH_SETTING_LOCK_ALPHA));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Colorize"), BrushSettingType::BRUSH_SETTING_COLORIZE));

    vBoxLayout->addWidget(new BrushSettingWidget(tr("Snap to pixel"), BrushSettingType::BRUSH_SETTING_SNAP_TO_PIXEL));
    vBoxLayout->addWidget(new BrushSettingWidget(tr("Pressure gain"), BrushSettingType::BRUSH_SETTING_PRESSURE_GAIN_LOG));
}

BrushSettingItem* MPBrushConfigurator::addTreeRoot(BrushSettingItem::Category category, QTreeWidget* treeWidget, const QString name, QString description)
{
    BrushSettingItem* treeItem = new BrushSettingItem(category, treeWidget);
    treeItem->setText(0, name);
    treeItem->setText(1, description);
    return treeItem;
}

BrushSettingItem* MPBrushConfigurator::addTreeChild(BrushSettingItem::Category category, QTreeWidgetItem* parent, const QString name, QString description)
{
    BrushSettingItem *treeItem = new BrushSettingItem(category, parent);
    treeItem->setText(0, name);
    treeItem->setText(1, description);
    parent->addChild(treeItem);
    return treeItem;
}

QWidget* MPBrushConfigurator::addBrushSetting(const QString name, BrushSettingType setting)
{
    QWidget* containerWidget = new QWidget();
    QHBoxLayout* hboxLayout = new QHBoxLayout();

    containerWidget->setLayout(hboxLayout);
    SpinSlider* valueSlider = new SpinSlider();
    valueSlider->init(name, SpinSlider::GROWTH_TYPE::LINEAR, SpinSlider::VALUE_TYPE::INTEGER, 0, 100);
    QSpinBox* valueBox = new QSpinBox();

    containerWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    hboxLayout->setMargin(0);

    hboxLayout->addWidget(valueSlider);
    hboxLayout->addWidget(valueBox);

    return containerWidget;
}

void MPBrushConfigurator::updateSettingsView(QTreeWidgetItem* item)
{
    qDeleteAll(mBrushSettingsWidget->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly));
    removeBrushSettingSpacers();

    switch(static_cast<BrushSettingItem*>(item)->ItemCategory()) {
        case BrushSettingItem::Basic: showBasicBrushSettings(); break;
        case BrushSettingItem::Advanced: showAdvancedBrushSettings(); break;
        case BrushSettingItem::Opacity: showOpacitySettings(); break;
        case BrushSettingItem::Dab: showDabSettings(); break;
        case BrushSettingItem::Random: showRandomSettings(); break;
        case BrushSettingItem::Offset: showOffsetSettings(); break;
        case BrushSettingItem::Tracking: showTrackingSettings(); break;
        case BrushSettingItem::Color: showColorSettings(); break;
        case BrushSettingItem::Smudge: showSmudgeSettings(); break;
        case BrushSettingItem::Eraser: showEraserSetting(); break;
        case BrushSettingItem::Stroke: showStrokeSettings(); break;
        case BrushSettingItem::Custom_Input: showCustomInputSettings(); break;
        case BrushSettingItem::Elliptical_Dab: showEllipticalDabSettings(); break;
        case BrushSettingItem::Other: showOtherSettings(); break;
        case BrushSettingItem::Unknown: return;
    }

    addBrushSettingsSpacer();
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
    qDebug() << "value: " << static_cast<float>(value);
    qDebug() << "type" << static_cast<int>(settingType);
    mEditor->setMPBrushSetting(settingType, static_cast<float>(qLn(value)));
    qDebug() << mEditor->getMPBrushSetting(settingType);
}

