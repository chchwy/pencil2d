#ifndef MPBRUSHPRESETSWIDGET_H
#define MPBRUSHPRESETSWIDGET_H

#include <QDialog>
#include <QItemSelection>
#include <QListWidgetItem>

#include "mpbrushutils.h"

struct MPBrushPreset;

namespace Ui {
class MPBrushPresetsWidget;
}

class MPBrushPresetsWidget : public QDialog
{

    enum class PresetState {
        ADDING,
        RENAMING,
        REMOVING,
        NONE
    };

    Q_OBJECT
public:
    MPBrushPresetsWidget(QVector<MPBrushPreset> presets, QWidget* parent = nullptr);

signals:
    void presetsChanged();

private:
    void addNewPreset();
    void removePreset();

    QString createBlankName() const;

    void itemDoubleClicked(QListWidgetItem *item);
    void didChangeSelection(const QItemSelection &selected, const QItemSelection &deselected);

    void didCommitChanges(QWidget* widgetItem);
    void didChangeItem(QListWidgetItem* item);

    Ui::MPBrushPresetsWidget* ui = nullptr;

    QVector<QString> mPresets;

    PresetState mState = PresetState::NONE;
};

#endif // MPBRUSHPRESETSWIDGET_H
