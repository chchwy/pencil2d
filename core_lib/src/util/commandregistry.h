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

#ifndef COMMANDREGISTRY_H
#define COMMANDREGISTRY_H

#include <QHash>
#include <QVector>
#include <QString>
#include <QCoreApplication>

enum class CommandCategory : int
{
    File,
    Edit,
    View,
    Animation,
    Tool,
    Layer,
    Window,
    Help
};

struct CommandDefinition
{
    const char* id;              // e.g. "CmdNewFile"
    const char* defaultShortcut; // e.g. "Ctrl+N"
    const char* displayName;     // QT_TR_NOOP string for translation
    CommandCategory category;
};

class CommandRegistry
{
    Q_DECLARE_TR_FUNCTIONS(CommandRegistry)

public:
    static const CommandRegistry& instance();

    const CommandDefinition* find(const QString& cmdId) const;
    QVector<const CommandDefinition*> byCategory(CommandCategory cat) const;
    QVector<const CommandDefinition*> all() const;
    int count() const;

private:
    CommandRegistry();
    void populate();

    QHash<QString, const CommandDefinition*> mLookup;
    QVector<const CommandDefinition*> mAll;

    static const CommandDefinition kDefinitions[];
    static const int kDefinitionCount;
};

#endif // COMMANDREGISTRY_H
