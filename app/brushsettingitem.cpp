#include "brushsettingitem.h"

BrushSettingItem::BrushSettingItem(Category category, QTreeWidget* parent) : QTreeWidgetItem(parent),
    mCategory(category)
{
}

BrushSettingItem::BrushSettingItem(Category category, QTreeWidgetItem* parent) : QTreeWidgetItem(parent),
    mCategory(category)
{
}
