#ifndef MPBRUSHCONFIGURATOR_H
#define MPBRUSHCONFIGURATOR_H

#include "basedockwidget.h"

#include <QTreeWidgetItem>

#include "brushsetting.h"
#include "brushsettingitem.h"

class QVBoxLayout;
class QSpacerItem;

class MPBrushConfigurator : public BaseDockWidget
{
    Q_OBJECT
public:

    MPBrushConfigurator(QWidget* parent = nullptr);
    void setCore(Editor* editor) { mEditor = editor; }

    void initUI() override;
    void updateUI() override;

private:

    void updateSettingsView(QTreeWidgetItem* item);

    void updateBrushSetting(qreal value, BrushSettingType settingType);

    void addBrushSettingsSpacer();
    void removeBrushSettingSpacers();

    void showBasicBrushSettings();
    void showAdvancedBrushSettings();

    void showOpacitySettings();
    void showDabSettings();
    void showRandomSettings();
    void showSpeedSettings();
    void showOffsetSettings();
    void showTrackingSettings();
    void showColorSettings();
    void showSmudgeSettings();
    void showEraserSetting();
    void showStrokeSettings();
    void showCustomInputSettings();
    void showEllipticalDabSettings();
    void showOtherSettings();

    BrushSettingItem* addTreeRoot(BrushSettingItem::Category category, QTreeWidget* treeWidget, const QString name, QString description);
    BrushSettingItem* addTreeChild(BrushSettingItem::Category category, QTreeWidgetItem* parent, const QString name, QString description);

    QWidget* addBrushSetting(const QString name, BrushSettingType setting);
    void brushCategorySelected(QTreeWidgetItem* item, int);
    void brushCategorySelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    QVBoxLayout* vBoxLayout = nullptr;
    QWidget* mBrushSettingsWidget = nullptr;
    QTreeWidget* mNavigatorWidget = nullptr;

    Editor* mEditor = nullptr;

};

//class SpinBoxWSlider : public QWidget
//{
//public:
//    SpinBoxWSlider(QWidget* parent = nullptr);


//};

#endif // MPBRUSHCONFIGURATOR_H
