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
#include <QMessageBox>
#include <QLabel>

#include <QJsonObject>
#include <QJsonValueRef>
#include <QJsonDocument>
#include <QJsonArray>

#include "spinslider.h"
#include "brushsettingitem.h"
#include "brushsettingwidget.h"
#include "editor.h"
#include "mpbrushutils.h"
#include "mpbrushinfodialog.h"

MPBrushConfigurator::MPBrushConfigurator(QWidget *parent)
  : QDialog(parent, Qt::Tool)
{
    setBaseSize(QSize(450,400));
    setWindowTitle(tr("Brush Configurator", "Window title of mypaint brush configurator"));

    mNavigatorWidget = new QTreeWidget(parent);
    mNavigatorWidget->setRootIsDecorated(false);
    mNavigatorWidget->setHeaderHidden(true);

    mBrushImageWidget = new QLabel();
    mBrushImageWidget->setFixedSize(mImageSize);

    mBrushNameWidget = new QLabel();

    QSplitter* viewSplitter = new QSplitter;
    QHBoxLayout* hLayout = new QHBoxLayout(parent);
    QVBoxLayout* vLayout = new QVBoxLayout(parent);

    vLayout->setMargin(0);
    QScrollArea* scrollArea = new QScrollArea(nullptr);

    QToolBar* toolbar = new QToolBar(this);
    mSaveBrushButton = new QPushButton(parent);
    mSaveBrushButton->setText(tr("Save"));

    mSaveBrushButton->setEnabled(false);

    QPushButton* cloneBrushButton = new QPushButton(parent);
    cloneBrushButton->setDefault(false);
    cloneBrushButton->setText("Clone");

    QPushButton* editBrushButton = new QPushButton(parent);
    editBrushButton->setText(tr("Edit Brush"));
    editBrushButton->setToolTip(tr("Here you can rename, change icon, change description and notes of the current brush"));

    mDiscardChangesButton = new QPushButton(parent);
    mDiscardChangesButton->setText(tr("Discard changes"));
    mDiscardChangesButton->setToolTip(tr("Discard current changes"));
    mDiscardChangesButton->setEnabled(false);

    QPushButton* deleteBrushButton = new QPushButton(parent);
    deleteBrushButton->setText(tr("Delete"));
    deleteBrushButton->setToolTip(tr("Delete current brush and close window"));

//    mMapValuesButton = new QPushButton(parent);
//    mMapValuesButton->setText("MyPaint values");

    setLayout(vLayout);

    QWidget* settingsContainer = new QWidget(this);
    settingsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    settingsContainer->setLayout(hLayout);

    toolbar->addWidget(mSaveBrushButton);
    toolbar->addWidget(cloneBrushButton);
    toolbar->addSeparator();
    toolbar->addWidget(editBrushButton);
//    toolbar->addWidget(mMapValuesButton);
    toolbar->addSeparator();

    QWidget* resetSpacer = new QWidget(parent);
    resetSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(resetSpacer);
    toolbar->addWidget(mDiscardChangesButton);
    toolbar->addWidget(deleteBrushButton);

    QHBoxLayout* topLayout = new QHBoxLayout(this);

    topLayout->setContentsMargins(5,5,0,0);

    vLayout->addLayout(topLayout);
    vLayout->setContentsMargins(5,0,0,0);

    topLayout->addWidget(mBrushImageWidget);
    topLayout->addWidget(mBrushNameWidget);

    vLayout->addWidget(settingsContainer);

    vLayout->addWidget(toolbar);

    viewSplitter->addWidget(mNavigatorWidget);
    viewSplitter->addWidget(scrollArea);
    hLayout->addWidget(viewSplitter);
    hLayout->setMargin(0);

    viewSplitter->setSizes({150, 600});
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

    connect(mSaveBrushButton, &QPushButton::pressed, this, &MPBrushConfigurator::pressedSaveBrush);
    connect(cloneBrushButton, &QPushButton::pressed, this, &MPBrushConfigurator::pressedCloneBrush);
    connect(deleteBrushButton, &QPushButton::pressed, this, &MPBrushConfigurator::pressedRemoveBrush);

    connect(mDiscardChangesButton, &QPushButton::pressed, this, &MPBrushConfigurator::pressedDiscardBrush);
    connect(editBrushButton, &QPushButton::pressed, this, &MPBrushConfigurator::pressedEditBrush);
    connect(cloneBrushButton, &QPushButton::pressed, this, &MPBrushConfigurator::pressedEditBrush);
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
                widget->setCore(mEditor);
                widget->updateUI();
            }
        }
    }

    QPixmap pix(QPixmap(MPBrushParser::getBrushImagePath(mBrushGroup, mBrushName)));
    mBrushImageWidget->setPixmap(pix.scaled(mImageSize, Qt::KeepAspectRatio));
    mBrushNameWidget->setText(mBrushName);
}

void MPBrushConfigurator::updateConfig(ToolType toolType, const QString& brushGroup, const QString& brushName, const QByteArray& content)
{
    mToolType = toolType;
    mBrushName = brushName;
    mBrushGroup = brushGroup;
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
        connect(item, &BrushSettingWidget::brushMappingForInputChanged, this, &MPBrushConfigurator::updateBrushMapping);
        connect(item, &BrushSettingWidget::brushMappingRemoved, this, &MPBrushConfigurator::removeBrushMappingForInput);
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
    if (!mBrushChanges.contains(static_cast<int>(settingType))) {
        BrushChanges changes;
        changes.baseValue = value;
        changes.settingsType = settingType;

        mBrushChanges.insert(static_cast<int>(settingType), changes);
    } else {
        BrushChanges outerChanges;
        QHashIterator<int, BrushChanges> changesHash(mBrushChanges);
        while (changesHash.hasNext()) {
            changesHash.next();

            BrushChanges innerChanges = changesHash.value();
            if (settingType == innerChanges.settingsType) {
                innerChanges.baseValue = value;
                innerChanges.settingsType = settingType;
                outerChanges = innerChanges;
                mBrushChanges.insert(static_cast<int>(settingType), outerChanges);
                break;
            }
        }
    }

    if (!mBrushChanges.isEmpty()) {
        mSaveBrushButton->setEnabled(true);
        mDiscardChangesButton->setEnabled(true);
    }

    qDebug() << "number of changes: " << mBrushChanges.count();
//    for (const BrushChanges& changes : mBrushChanges) {
//        qDebug() << static_cast<int>(changes.settingsType);
//    }
    mEditor->setMPBrushSetting(settingType, static_cast<float>(value));
}

void MPBrushConfigurator::updateBrushMapping(QVector<QPointF> points, BrushSettingType setting, BrushInputType input)
{

    // if no base value has been provided, use the default value from brush settings
    qreal baseValue = static_cast<qreal>(mEditor->getBrushSettingInfo(setting).defaultValue);
    if (mBrushChanges.isEmpty()) {
        BrushChanges changes;
        changes.baseValue = baseValue;
        changes.settingsType = setting;
        auto mappedInputs = points;

        changes.listOfinputChanges.insert(static_cast<int>(input), InputChanges { mappedInputs, input });

        mBrushChanges.insert(static_cast<int>(setting),changes);
    } else {
        QHashIterator<int, BrushChanges> it(mBrushChanges);
        while (it.hasNext()) {
            it.next();
            BrushChanges changes = it.value();
            changes.settingsType = setting;
            if (setting == changes.settingsType) {
                auto mappedInputs = points;
                changes.listOfinputChanges.insert(static_cast<int>(input), InputChanges { mappedInputs, input });

                mBrushChanges.insert(static_cast<int>(setting), changes);
            }
        }

        QHashIterator<int, BrushChanges> itBrush(mBrushChanges);
        while (itBrush.hasNext()) {
            itBrush.next();

            QHashIterator<int, InputChanges> i(itBrush.value().listOfinputChanges);
            while (i.hasNext()) {
                  i.next();
            qDebug() << i.value().mappedPoints;
            }
        }
    }

    if (!mBrushChanges.isEmpty()) {
        mSaveBrushButton->setEnabled(true);
        mDiscardChangesButton->setEnabled(true);
    }

    mEditor->setBrushInputMapping(points, setting, input);
}

void MPBrushConfigurator::removeBrushMappingForInput(BrushSettingType setting, BrushInputType input)
{
    if (mBrushChanges.isEmpty()) {
        BrushChanges changes;
        changes.baseValue = static_cast<qreal>(mEditor->getBrushSettingInfo(setting).defaultValue);
        changes.settingsType = setting;

        changes.listOfinputChanges.insert(static_cast<int>(input), InputChanges { {}, input, false });

        mBrushChanges.insert(static_cast<int>(setting),changes);
    } else {
        QHashIterator<int, BrushChanges> brushIt(mBrushChanges);
        while (brushIt.hasNext()) {
            brushIt.next();

            BrushChanges changes = brushIt.value();
            if (changes.settingsType == setting) {

                QHashIterator<int, InputChanges> inputIt(brushIt.value().listOfinputChanges);
                while (inputIt.hasNext()) {
                      inputIt.next();

                      InputChanges inputChanges = inputIt.value();
                      if (inputChanges.inputType == input) {
                          inputChanges.enabled = false;
                          inputChanges.inputType = input;
                          changes.listOfinputChanges.find(inputIt.key()).value() = inputChanges;
                          mBrushChanges.insert(static_cast<int>(setting), changes);
//                          mBrushChanges.find(brushIt.key()).value().listOfinputChanges.find(inputIt.key())
                      }
                }
            }
        }
    }

    if (!mBrushChanges.isEmpty()) {
        mSaveBrushButton->setEnabled(true);
    }
    mEditor->setBrushInputMapping({}, setting, input);
}

void MPBrushConfigurator::updateMapValuesButton()
{
    mMapValuesButtonPressed = !mMapValuesButtonPressed;
    if (mMapValuesButtonPressed) {
        mMapValuesButton->setText(tr("Pencil2D vkalues"));
    } else {
        mMapValuesButton->setText(tr("MyPaint values"));
    }
}

void MPBrushConfigurator::pressedSaveBrush()
{   
    auto status = MPBrushParser::readBrushFromFile(mBrushGroup, mBrushName);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(status.data, &error);

    writeBrushChanges(doc, error);

    Status statusWrite = MPBrushParser::writeBrushToFile(mBrushGroup, mBrushName, doc.toJson());

    if (statusWrite == Status::OK) {
        mSaveBrushButton->setEnabled(false);
        mBrushChanges.clear();
    } else {
        QMessageBox::warning(this, statusWrite.title(), statusWrite.description());
    }
}

void MPBrushConfigurator::writeBrushChanges(QJsonDocument& document, QJsonParseError& error)
{
    QJsonObject rootObject = document.object();

    if (error.error != QJsonParseError::NoError) {
        QMessageBox::information(this, tr("Parse error!"), tr("Could not save brush file\n the following error was given: ") + error.errorString());
        return;
    }

    QJsonObject::iterator settingsContainerObjIt = rootObject.find("settings");

    if (settingsContainerObjIt->isUndefined()) {
        QMessageBox::information(this, tr("Parse error!"), tr("Looks like you are missign a 'settings' field in your brush file, this shouldn't happen...") + error.errorString());
        return;
    }
    QJsonValueRef settingsContainerRef = settingsContainerObjIt.value();

    QJsonObject settingsContainerObj = settingsContainerRef.toObject();
    QHashIterator<int, BrushChanges> settingIt(mBrushChanges);
    while (settingIt.hasNext()) {
        settingIt.next();

        BrushChanges brushChanges = settingIt.value();
        QString settingId = MPBrushParser::getBrushSettingIdentifier(brushChanges.settingsType);

        QJsonObject::iterator settingObjIt = settingsContainerObj.find(settingId);

        if (settingObjIt->isUndefined()) {
            QJsonObject settingObj;
            brushChanges.write(settingObj);
            settingsContainerObj.insert(settingId, settingObj);
        } else {
            QJsonValueRef settingRef = settingObjIt.value();
            QJsonObject settingObj = settingRef.toObject();
            brushChanges.write(settingObj);

            settingsContainerObj.remove(settingId);
            settingsContainerObj.insert(settingId, settingObj);
        }
    }
    settingsContainerRef = settingsContainerObj;
    document.setObject(rootObject);

}

void MPBrushConfigurator::pressedRemoveBrush()
{   
    QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Delete brush"),
                                   tr("Are you sure you want to delete this brush?"),
                                   QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
    if (ret == QMessageBox::Yes) {
        MPBrushParser::blackListBrushFile(mBrushGroup, mBrushName);

        emit refreshBrushList();
        close();
    }
}

void MPBrushConfigurator::openBrushInfoWidget(DialogContext dialogContext)
{
    if (mBrushInfoWidget == nullptr) {
        mBrushInfoWidget = new MPBrushInfoDialog(dialogContext, this);
        mBrushInfoWidget->setAttribute(Qt::WA_DeleteOnClose);

        connect(mBrushInfoWidget, &MPBrushInfoDialog::updatedBrushInfo, this, &MPBrushConfigurator::updateBrushList);
    }
    mBrushInfoWidget->setCore(mEditor);
    mBrushInfoWidget->initUI();

    auto status = MPBrushParser::readBrushFromFile(mBrushGroup, mBrushName);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(status.data, &error);

    writeBrushChanges(doc, error);

    mBrushInfoWidget->setBrushInfo(mBrushName, mBrushGroup, mToolType, doc);
    mBrushInfoWidget->show();
}

void MPBrushConfigurator::pressedEditBrush()
{
    openBrushInfoWidget(DialogContext::Edit);
}

void MPBrushConfigurator::pressedCloneBrush()
{
    openBrushInfoWidget(DialogContext::Clone);
}

void MPBrushConfigurator::pressedDiscardBrush()
{
    mBrushChanges.clear();
    mSaveBrushButton->setEnabled(false);
    mDiscardChangesButton->setEnabled(false);
    emit reloadBrushSettings();
}

void MPBrushConfigurator::showNotImplementedPopup()
{
    QMessageBox::information(this, tr("Not implemented"),
                                  tr("This feature is coming soon"));
}

