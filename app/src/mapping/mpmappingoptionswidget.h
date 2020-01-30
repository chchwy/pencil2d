#ifndef MPMAPPINGSETTINGSWIDGET_H
#define MPMAPPINGSETTINGSWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>

#include "brushsetting.h"

class Editor;
class QGridLayout;
class QComboBox;
class MPMappingWidget;
class QHBoxLayout;
class MPInputButton;

class MPComboBox : public QComboBox
{
    Q_OBJECT
public:
    MPComboBox(QWidget* parent = nullptr);

    void addItem(const QString& text);
    void addItem(const QString& text, BrushInputType input);
    void insertItem(const QString& text, int index, BrushInputType input);
    void setItemEnabled(int index, bool disable);

signals:
    void activated(int index, const BrushInputType& input);

private slots:
    void triggerActivatedVariant(int index);

};

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


class MPMappingOptionsWidget : public QWidget
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

    MPMappingOptionsWidget(BrushSettingType settingType, QWidget* parent = nullptr);

    void setCore(Editor* editor) { mEditor = editor; }
    void showInputMapper(BrushInputType inputType);
    void initUI();

signals:
    void mappingForInputUpdated(QVector<QPointF> points, BrushInputType inputType);

private:
    MPMappingOption createMappingOption(BrushInputType input);
    void removeAction(BrushInputType input);
    void addOptionField(int index, BrushInputType input);
    void setupUI();

    QGridLayout* mGridLayout = nullptr;
    QHBoxLayout* mHBoxLayout = nullptr;
    MPComboBox* mMappingOptionsCombBox = nullptr;

    MPMappingWidget* mMappingWidget = nullptr;

    BrushSettingType mSettingType;
    Editor* mEditor = nullptr;

    QWidget* mParent = nullptr;

    QList<MPMappingOption> mOptions;
};

#endif // MPMAPPINGSETTINGSWIDGET_H
