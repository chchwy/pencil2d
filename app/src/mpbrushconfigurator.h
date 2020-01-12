#ifndef MPBRUSHCONFIGURATOR_H
#define MPBRUSHCONFIGURATOR_H

#include "basedockwidget.h"

#include <QDialog>
#include <QTreeWidgetItem>

#include "brushsetting.h"
#include "brushsettingitem.h"

class QVBoxLayout;
class QSpacerItem;
class BrushSettingWidget;

class MPBrushConfigurator : public QDialog
{
    Q_OBJECT
public:

    MPBrushConfigurator(QWidget* parent = nullptr);
    void setCore(Editor* editor) { mEditor = editor; }

    void initUI();
    void updateUI();

    void updateConfig(QString toolName, QString brushName, const QByteArray& content);

private:

    void updateMapValuesButton();
    void updateSettingsView(QTreeWidgetItem* item);

    void updateBrushSetting(qreal value, BrushSettingType settingType);

    void addBrushSettingsSpacer();
    void removeBrushSettingSpacers();

    void prepareBasicBrushSettings();
    void prepareAdvancedBrushSettings();

    void prepareOpacitySettings();
    void prepareDabSettings();
    void prepareRandomSettings();
    void prepareSpeedSettings();
    void prepareOffsetSettings();
    void prepareTrackingSettings();
    void prepareColorSettings();
    void prepareSmudgeSettings();
    void prepareEraserSetting();
    void prepareStrokeSettings();
    void prepareCustomInputSettings();
    void prepareEllipticalDabSettings();
    void prepareOtherSettings();

    void showNowImplementedPopup();

    BrushSettingItem* addTreeRoot(BrushSettingItem::Category category, QTreeWidget* treeWidget, const QString name);
    BrushSettingItem* addTreeChild(BrushSettingItem::Category category, QTreeWidgetItem* parent, const QString name);

//    QWidget* addBrushSetting(const QString name, BrushSettingType setting);
    void brushCategorySelected(QTreeWidgetItem* item, int);
    void brushCategorySelectionChanged(const QItemSelection &selected, const QItemSelection &);

    QVBoxLayout* vBoxLayout = nullptr;
    QWidget* mBrushSettingsWidget = nullptr;
    QTreeWidget* mNavigatorWidget = nullptr;

    QPushButton* mMapValuesButton = nullptr;
    bool mMapValuesButtonPressed = false;

    Editor* mEditor = nullptr;

    QList<BrushSettingWidget*> mBrushWidgets;

};

#endif // MPBRUSHCONFIGURATOR_H
