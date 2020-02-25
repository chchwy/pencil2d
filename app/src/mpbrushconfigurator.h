#ifndef MPBRUSHCONFIGURATOR_H
#define MPBRUSHCONFIGURATOR_H

#include "basedockwidget.h"

#include <QDialog>
#include <QTreeWidgetItem>

#include "brushsetting.h"
#include "brushsettingitem.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include <QPointer>

#include "pencildef.h"
#include "mpbrushinfodialog.h"

class QVBoxLayout;
class QSpacerItem;
class BrushSettingWidget;
class QLabel;
class MPBrushInfoDialog;

struct InputChanges {
    QVector<QPointF> mappedPoints;
    BrushInputType inputType;
    bool enabled;

    InputChanges(QVector<QPointF> newMappedPoints, BrushInputType newInputType, bool newEnabled)
    {
        mappedPoints = newMappedPoints;
        inputType = newInputType;
        enabled = newEnabled;
    }

    InputChanges(QVector<QPointF> newMappedPoints, BrushInputType newInputType)
    {
        mappedPoints = newMappedPoints;
        inputType = newInputType;
        enabled = true;
    }

    void write(QJsonArray& object) const
    {
        for (int i = 0; i < mappedPoints.count(); i++) {
            const auto mappedPoint = mappedPoints[i];
            QJsonArray pointArray = { mappedPoint.x(), mappedPoint.y() };
            object.removeAt(i);
            object.insert(i, pointArray);
        }
    }
};

struct BrushChanges {
    QHash<int, InputChanges> listOfinputChanges;
    qreal baseValue;
    BrushSettingType settingsType;

    void write(QJsonObject& object) const
    {
        QJsonObject::iterator baseValueObjIt = object.find("base_value");

        if (baseValueObjIt->isUndefined()) {
            object.insert("base_value", baseValue);
        } else {
            object.remove("base_value");
            object.insert("base_value", baseValue);
        }

        QJsonObject::iterator inputsObjIt = object.find("inputs");
        if (inputsObjIt->isUndefined()) {
            object.insert("inputs", QJsonObject());
        }

        QJsonValueRef inputsContainerRef = object.find("inputs").value();
        QJsonObject inputsContainerObj = inputsContainerRef.toObject();
        QHashIterator<int, InputChanges> inputIt(listOfinputChanges);
        while (inputIt.hasNext()) {
            inputIt.next();

            InputChanges inputChanges = inputIt.value();

            QString inputId = getBrushInputIdentifier(inputChanges.inputType);
            QJsonObject::iterator inputContainerObjIt = inputsContainerObj.find(inputId);

            if (inputContainerObjIt->isUndefined()) {
                if (inputChanges.enabled) {
                    QJsonArray inputArray;
                    inputChanges.write(inputArray);
                    inputsContainerObj.insert(inputId, inputArray);
                }
            } else {

                if (inputChanges.enabled) {
                    QJsonValueRef inputArrRef = inputContainerObjIt.value();
                    QJsonArray inputArray = inputArrRef.toArray();
                    inputChanges.write(inputArray);

                    inputsContainerObj.remove(inputId);
                    inputsContainerObj.insert(inputId, inputArray);
                } else {
                    QString inputKey = inputContainerObjIt.key();
                    inputsContainerObj.remove(inputKey);
                }
            }
            inputsContainerRef = inputsContainerObj;
        }
    }
};

class MPBrushConfigurator : public QDialog
{
    Q_OBJECT
public:
    MPBrushConfigurator(QWidget* parent = nullptr);

    void initUI();
    void updateUI();

    void setCore(Editor* editor) { mEditor = editor; }

    void updateConfig(ToolType toolName, const QString& brushGroup, const QString& brushName, const QByteArray& content);

signals:
    void updateBrushList(QString brushName, QString brushPreset);
    void refreshBrushList();
    void reloadBrushSettings();

private:

    void updateMapValuesButton();
    void updateSettingsView(QTreeWidgetItem* item);

    void updateBrushSetting(qreal value, BrushSettingType settingType);
    void updateBrushMapping(QVector<QPointF> points, BrushSettingType settingType, BrushInputType input);
    void removeBrushMappingForInput(BrushSettingType setting, BrushInputType input);

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

    void showNotImplementedPopup();

    void pressedSaveBrush();
    void pressedRemoveBrush();
    void pressedEditBrush();
    void pressedCloneBrush();
    void pressedDiscardBrush();

    void writeBrushChanges(QJsonDocument& document, QJsonParseError& error);

    void openBrushInfoWidget(DialogContext dialogContext);

    BrushSettingItem* addTreeRoot(BrushSettingItem::Category category, QTreeWidget* treeWidget, const QString name);
    BrushSettingItem* addTreeChild(BrushSettingItem::Category category, QTreeWidgetItem* parent, const QString name);

//    QWidget* addBrushSetting(const QString name, BrushSettingType setting);
    void brushCategorySelected(QTreeWidgetItem* item, int);
    void brushCategorySelectionChanged(const QItemSelection &selected, const QItemSelection &);

    QVBoxLayout* vBoxLayout = nullptr;
    QWidget* mBrushSettingsWidget = nullptr;
    QTreeWidget* mNavigatorWidget = nullptr;
    QLabel* mBrushImageWidget = nullptr;
    QLabel* mBrushNameWidget = nullptr;

    QPointer<MPBrushInfoDialog> mBrushInfoWidget = nullptr;

    QPushButton* mDiscardChangesButton = nullptr;
    QPushButton* mMapValuesButton = nullptr;
    QPushButton* mSaveBrushButton = nullptr;
    bool mMapValuesButtonPressed = false;

    Editor* mEditor = nullptr;

    QList<BrushSettingWidget*> mBrushWidgets;
    QHash<int, BrushChanges> mBrushChanges;
    QString mBrushName;
    QString mBrushGroup;
    ToolType mToolType;

    QSize mImageSize = QSize(32,32);

};

#endif // MPBRUSHCONFIGURATOR_H
