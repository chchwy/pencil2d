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

    connect(mNavigatorWidget, &QTreeWidget::itemPressed, this, &MPBrushConfigurator::brushCategorySelected);
    connect(mNavigatorWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MPBrushConfigurator::brushCategorySelectionChanged);
}

void MPBrushConfigurator::initUI()
{
    auto basicRoot = addTreeRoot(BrushSettingItem::Basic, mNavigatorWidget, tr("Basic settings"), "test 1");

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
    mBrushWidgets.append(new BrushSettingWidget(tr("Opacity"), BrushSettingType::BRUSH_SETTING_OPAQUE));
    mBrushWidgets.append(new BrushSettingWidget(tr("Radius"), BrushSettingType::BRUSH_SETTING_RADIUS_LOGARITHMIC));
    mBrushWidgets.append(new BrushSettingWidget(tr("Hardness"), BrushSettingType::BRUSH_SETTING_HARDNESS));
    mBrushWidgets.append(new BrushSettingWidget(tr("Pressure gain"), BrushSettingType::BRUSH_SETTING_PRESSURE_GAIN_LOG));
    mBrushWidgets.append(new BrushSettingWidget(tr("Anti-aliasing"), BrushSettingType::BRUSH_SETTING_ANTI_ALIASING));
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
    mBrushWidgets.append(new BrushSettingWidget(tr("Opacity multiply"), BrushSettingType::BRUSH_SETTING_OPAQUE_MULTIPLY));
    mBrushWidgets.append(new BrushSettingWidget(tr("Opacity linearize"), BrushSettingType::BRUSH_SETTING_OPAQUE_LINEARIZE));
}

void MPBrushConfigurator::prepareDabSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Dabs per basic radius"), BrushSettingType::BRUSH_SETTING_DABS_PER_BASIC_RADIUS));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dabs per actual radius"), BrushSettingType::BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dabs per second"), BrushSettingType::BRUSH_SETTING_DABS_PER_SECOND));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dab scale"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dab scale X"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_X));
    mBrushWidgets.append(new BrushSettingWidget(tr("Dab scale Y"), BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_Y));
}

void MPBrushConfigurator::prepareRandomSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Radius by random"), BrushSettingType::BRUSH_SETTING_RADIUS_BY_RANDOM));
}

void MPBrushConfigurator::prepareSpeedSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Speed start"), BrushSettingType::BRUSH_SETTING_SPEED1_SLOWNESS));
    mBrushWidgets.append(new BrushSettingWidget(tr("Speed end"), BrushSettingType::BRUSH_SETTING_SPEED2_SLOWNESS));
    mBrushWidgets.append(new BrushSettingWidget(tr("Speed gamma start"), BrushSettingType::BRUSH_SETTING_SPEED1_GAMMA));
    mBrushWidgets.append(new BrushSettingWidget(tr("Speed gamma end"), BrushSettingType::BRUSH_SETTING_SPEED2_GAMMA));
}

void MPBrushConfigurator::prepareOffsetSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset by random"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_RANDOM));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset X"), BrushSettingType::BRUSH_SETTING_OFFSET_X));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset Y"), BrushSettingType::BRUSH_SETTING_OFFSET_Y));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle left"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle left ascend"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ASC));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle right"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle right ascend"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2_ASC));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset angle adjecent"), BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ADJ));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset multiplier"), BrushSettingType::BRUSH_SETTING_OFFSET_MULTIPLIER));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset by speed"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED));
    mBrushWidgets.append(new BrushSettingWidget(tr("Offset by speed slowness"), BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS));
}

void MPBrushConfigurator::prepareTrackingSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Slow tracking"), BrushSettingType::BRUSH_SETTING_SLOW_TRACKING));
    mBrushWidgets.append(new BrushSettingWidget(tr("Slow tracking per. dab"), BrushSettingType::BRUSH_SETTING_SLOW_TRACKING_PER_DAB));
    mBrushWidgets.append(new BrushSettingWidget(tr("Tracking noise"), BrushSettingType::BRUSH_SETTING_TRACKING_NOISE));
}

void MPBrushConfigurator::prepareColorSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Hue"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_H));
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Lightness"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_L));
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Saturation"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSL_S));
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Value"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_V));
    mBrushWidgets.append(new BrushSettingWidget(tr("Change color: Saturation"), BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSV_S));
    mBrushWidgets.append(new BrushSettingWidget(tr("Restore color"), BrushSettingType::BRUSH_SETTING_RESTORE_COLOR));
}

void MPBrushConfigurator::prepareSmudgeSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Smudge"), BrushSettingType::BRUSH_SETTING_SMUDGE));
    mBrushWidgets.append(new BrushSettingWidget(tr("Smudge length"), BrushSettingType::BRUSH_SETTING_SMUDGE_LENGTH));
    mBrushWidgets.append(new BrushSettingWidget(tr("Smudge radius"), BrushSettingType::BRUSH_SETTING_SMUDGE_RADIUS_LOG));
}

void MPBrushConfigurator::prepareEraserSetting()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Eraser"), BrushSettingType::BRUSH_SETTING_ERASER));
}

void MPBrushConfigurator::prepareStrokeSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Stroke threshold"), BrushSettingType::BRUSH_SETTING_STROKE_THRESHOLD));
    mBrushWidgets.append(new BrushSettingWidget(tr("Stroke duration"), BrushSettingType::BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC));
    mBrushWidgets.append(new BrushSettingWidget(tr("Stroke holdtime"), BrushSettingType::BRUSH_SETTING_STROKE_HOLDTIME));
}

void MPBrushConfigurator::prepareCustomInputSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Custom input"), BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT));
    mBrushWidgets.append(new BrushSettingWidget(tr("Custom input slowness"), BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS));
}

void MPBrushConfigurator::prepareEllipticalDabSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Elleptical dab ratio"), BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_RATIO));
    mBrushWidgets.append(new BrushSettingWidget(tr("Elleptical dab angle"), BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE));
}

void MPBrushConfigurator::prepareOtherSettings()
{
    mBrushWidgets.append(new BrushSettingWidget(tr("Lock Alpha"), BrushSettingType::BRUSH_SETTING_LOCK_ALPHA));
    mBrushWidgets.append(new BrushSettingWidget(tr("Colorize"), BrushSettingType::BRUSH_SETTING_COLORIZE));
    mBrushWidgets.append(new BrushSettingWidget(tr("Snap to pixel"), BrushSettingType::BRUSH_SETTING_SNAP_TO_PIXEL));
    mBrushWidgets.append(new BrushSettingWidget(tr("Pressure gain"), BrushSettingType::BRUSH_SETTING_PRESSURE_GAIN_LOG));
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

