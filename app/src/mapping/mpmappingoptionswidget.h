#ifndef MPMAPPINGSETTINGSWIDGET_H
#define MPMAPPINGSETTINGSWIDGET_H

#include <QToolButton>
#include <QDialog>
#include <QLabel>

#include "brushsetting.h"

class Editor;
class QGridLayout;
class ComboBox;
class MPMappingWidget;
class QHBoxLayout;
class MPInputButton;


class MPInputButton : public QToolButton
{
    Q_OBJECT

public:
    MPInputButton(MPInputButton* inputButton);
    MPInputButton(BrushInputType inputType, QWidget* parent = nullptr);

    void pressed();

signals:
    void didPress(BrushInputType inputType);

private:

    BrushInputType mInputType;
//    const MappingControlPoints* mControlPoints = nullptr;
};


class MPMappingOptionsWidget : public QDialog
{
    Q_OBJECT
public:
    struct MPMappingOption
    {
        MPInputButton* mappingButton;
        MPInputButton* removeActionButton;
        QLabel* settingDescLabel;
        BrushInputType inputType;

        MPMappingOption(MPInputButton* newMappingButton, MPInputButton* newRemoveActionButton, QLabel* newSettingDescLabel) {
            mappingButton = newMappingButton;
            removeActionButton = newRemoveActionButton;
            settingDescLabel = newSettingDescLabel;
        }

        void deleteAll()
        {
            delete mappingButton;
            delete removeActionButton;
            delete settingDescLabel;
        }
    };

    MPMappingOptionsWidget(QString optionName, BrushSettingType settingType, QWidget* parent = nullptr);

    void setCore(Editor* editor) { mEditor = editor; }
    void showInputMapper(BrushInputType inputType);
    void initUI();

signals:
    void mappingForInputUpdated(QVector<QPointF> points, BrushInputType inputType);
    void removedInputOption(BrushInputType inputType);

private:
    MPMappingOption createMappingOption(BrushInputType input);
    void removeAction(BrushInputType input);
    void addOptionField(int index, QString name, int value);
    void setupUI();

    QGridLayout* mGridLayout = nullptr;
    QHBoxLayout* mHBoxLayout = nullptr;
    ComboBox* mMappingOptionsComboBox = nullptr;

    MPMappingWidget* mMappingWidget = nullptr;

    BrushSettingType mSettingType;
    Editor* mEditor = nullptr;

    QWidget* mParent = nullptr;

    QList<MPMappingOption> mOptions;

    QList<bool> mRemovedInputs;
};

#endif // MPMAPPINGSETTINGSWIDGET_H
