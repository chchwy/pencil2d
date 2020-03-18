#include "mpbrushpresetswidget.h"
#include "ui_mpbrushpresetswidget.h"

#include <QListWidgetItem>
#include <QListWidget>
#include <QtMath>
#include <QMessageBox>

MPBrushPresetsWidget::MPBrushPresetsWidget(QVector<MPBrushPreset> presets, QWidget* parent)
    : QDialog(parent, Qt::Tool), ui(new Ui::MPBrushPresetsWidget)
{
    ui->setupUi(this);

    for (const MPBrushPreset& preset : presets) {
        QListWidgetItem* nameItem = new QListWidgetItem(preset.name);
        nameItem->setData(Qt::UserRole, 0);
        nameItem->setFlags(nameItem->flags() | Qt::ItemIsEditable);
        ui->presetsListView->addItem(nameItem);
        mPresets.append(preset.name);
    }

    ui->removePresetButton->setEnabled(false);
    ui->presetsListView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->addPresetButton, &QPushButton::pressed, this, &MPBrushPresetsWidget::addNewPreset);
    connect(ui->removePresetButton, &QPushButton::pressed, this, &MPBrushPresetsWidget::removePreset);
    connect(ui->presetsListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MPBrushPresetsWidget::didChangeSelection);
    connect(ui->presetsListView->itemDelegate(), &QAbstractItemDelegate::commitData, this, &MPBrushPresetsWidget::didCommitChanges);
    connect(ui->presetsListView, &QListWidget::itemChanged, this, &MPBrushPresetsWidget::didChangeItem);

    ui->presetsListView->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked);
}

void MPBrushPresetsWidget::addNewPreset()
{
    QString blankName = createBlankName();

    QListWidgetItem* item = new QListWidgetItem(blankName);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setData(Qt::UserRole, ui->presetsListView->model()->rowCount());
    item->setSelected(true);

    ui->presetsListView->insertItem(0, item);
    mPresets.prepend(item->text());

    ui->presetsListView->editItem(item);
    ui->presetsListView->setCurrentItem(item);

    MPCONF::addPreset(item->text());

    mState = PresetState::ADDING;
}

void MPBrushPresetsWidget::didCommitChanges(QWidget* widgetItem)
{
    Q_UNUSED(widgetItem)

    emit presetsChanged();
}

void MPBrushPresetsWidget::didChangeItem(QListWidgetItem* item)
{
    int row = ui->presetsListView->currentRow();
    QString oldName = mPresets.at(row);
    QString newName =  item->text();

    if (mPresets.contains(newName)) {
        QMessageBox::warning(this, tr("Duplicate error"), tr("The name already exists in the list"));

        ui->presetsListView->item(row)->setText(oldName);
        return;
    }

    MPCONF::renamePreset(oldName, newName);

    mState = PresetState::RENAMING;

    mPresets.replace(row, newName);

    emit presetsChanged();
}

void MPBrushPresetsWidget::removePreset()
{
    int row = ui->presetsListView->currentRow();
    QListWidgetItem* item = ui->presetsListView->item(row);
    MPCONF::removePreset(item->text());
    mPresets.removeAt(row);

    ui->presetsListView->removeItemWidget(item);
    delete item;

    mState = PresetState::REMOVING;

    emit presetsChanged();
}

void MPBrushPresetsWidget::didChangeSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    if (mPresets.count() <= 1 || mPresets.at(ui->presetsListView->currentRow()) == DefaultPreset) {
        ui->removePresetButton->setEnabled(false);
    } else {
        ui->removePresetButton->setEnabled(true);
    }
}

QString MPBrushPresetsWidget::createBlankName() const
{
    QString blankName = "blank";
    QString nameCheck = blankName;
    int numCount = 0;
    for (QString name : mPresets) {
        if (mPresets.contains(nameCheck)) {
            numCount++;
        }
        if (numCount > 0) {
            nameCheck = blankName + QString::number(numCount);
        }
    }
    if (blankName != nameCheck) {
        blankName = nameCheck;
    }
    return blankName;
}
