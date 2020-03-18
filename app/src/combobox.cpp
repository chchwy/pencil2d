#include "combobox.h"

#include <QStandardItemModel>
#include <QDebug>

ComboBox::ComboBox(QWidget* parent)
    : QComboBox(parent)
{
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &ComboBox::triggerActivatedVariant);
}

void ComboBox::addItem(const QString& text)
{
    QComboBox::addItem(text);
}

void ComboBox::addItem(const QString& text, int value)
{
    QComboBox::addItem(text, QVariant(value));
}

void ComboBox::insertItem(const QString& text, int index, int value)
{
    QComboBox::insertItem(index, text, QVariant(value));
}

void ComboBox::triggerActivatedVariant(int index)
{
    int data = itemData(index).toInt();
    QString text = itemText(index);
    activated(index, text, data);
}

/// Sets current index based on data given
/// \param value value should match the data in your list
///
void ComboBox::setCurrentItemFrom(int value)
{
    int index = this->findData(QVariant(value));
    setCurrentIndex(index);
}

/// Sets current index based on data given
/// \param value value should match the data in your list
///
void ComboBox::setCurrentItemFrom(QString text)
{
    int index = this->findText(text);
    setCurrentIndex(index);
}

void ComboBox::setItemEnabled(int index, bool enabled)
{
    auto model = this->model();
    auto itemModel = static_cast<QStandardItemModel*>(model);
    auto item = itemModel->item(index, 0);
    item->setEnabled( enabled );
}
