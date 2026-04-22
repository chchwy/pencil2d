/*

Pencil2D - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2012-2020 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/
#include "pencilsettings.h"

#include <QStringList>
#include <QDebug>
#include "commandregistry.h"


// ==== Singleton ====

QSettings& pencilSettings()
{
    static QSettings settings(PENCIL2D, PENCIL2D);

    if ( !settings.contains("InitPencilSetting") )
    {
        restoreToDefaultSetting();
        settings.setValue("InitPencilSetting", true);
    }
    return settings;
}

void restoreToDefaultSetting() // TODO: finish reset list
{
    QSettings s(PENCIL2D, PENCIL2D);

    s.setValue(SETTING_AUTO_SAVE_NUMBER, 15);
    s.setValue(SETTING_TOOL_CURSOR, true);

    s.sync();
    qDebug("restored default tools");
}

void checkExistingShortcuts()
{
    // Primary source of defaults: CommandRegistry (compiled-in).
    // kb.ini is still loaded as a secondary fallback so that legacy
    // entries survive until it is removed in a later phase.
    const CommandRegistry& reg = CommandRegistry::instance();

    QSettings curSetting(PENCIL2D, PENCIL2D);

    // Populate missing shortcuts from the registry
    curSetting.beginGroup(SHORTCUTS_GROUP);
    for (const CommandDefinition* def : reg.all())
    {
        QString cmdId = QString::fromLatin1(def->id);
        if (!curSetting.contains(cmdId))
        {
            curSetting.setValue(cmdId, QString::fromLatin1(def->defaultShortcut));
        }
    }

    // Remove shortcuts that no longer exist in the registry
    for (const QString& key : curSetting.allKeys())
    {
        if (reg.find(key) == nullptr)
        {
            curSetting.remove(key);
        }
    }
    curSetting.endGroup();
    curSetting.sync();
}

void restoreShortcutsToDefault()
{
    const CommandRegistry& reg = CommandRegistry::instance();

    QSettings curSetting(PENCIL2D, PENCIL2D);
    curSetting.remove("shortcuts");

    curSetting.beginGroup(SHORTCUTS_GROUP);
    for (const CommandDefinition* def : reg.all())
    {
        curSetting.setValue(QString::fromLatin1(def->id),
                            QString::fromLatin1(def->defaultShortcut));
    }
    curSetting.endGroup();
    curSetting.sync();
}
