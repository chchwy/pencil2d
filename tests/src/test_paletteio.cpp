/*

Pencil2D - Traditional Animation Software
Copyright (C) 2012-2020 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/
#include "catch.hpp"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include "paletteio.h"

namespace
{
QString tempPalettePath(const QString& name)
{
    return QDir::temp().filePath(name);
}
} // namespace

TEST_CASE("PaletteIO round-trips the Pencil2D XML format")
{
    QList<ColorRef> palette;
    palette.append(ColorRef(QColor(255, 0, 0, 255), "Red"));
    palette.append(ColorRef(QColor(0, 128, 0, 200), "Half Green"));
    palette.append(ColorRef(QColor(0, 0, 255, 255), QString()));

    const QString path = tempPalettePath("pencil2d_test_palette.xml");
    REQUIRE(PaletteIO::exportPalette(path, palette));

    QList<ColorRef> loaded;
    REQUIRE(PaletteIO::importPalette(path, loaded));

    REQUIRE(loaded.size() == palette.size());
    for (int i = 0; i < palette.size(); i++)
    {
        REQUIRE(loaded.at(i).color == palette.at(i).color);
        REQUIRE(loaded.at(i).name == palette.at(i).name);
    }

    QFile::remove(path);
}

TEST_CASE("PaletteIO round-trips the GIMP .gpl format")
{
    QList<ColorRef> palette;
    palette.append(ColorRef(QColor(255, 0, 0), "Red"));
    palette.append(ColorRef(QColor(10, 20, 30), "Dark Something"));

    const QString path = tempPalettePath("pencil2d_test_palette.gpl");
    REQUIRE(PaletteIO::exportPalette(path, palette));

    QList<ColorRef> loaded;
    REQUIRE(PaletteIO::importPalette(path, loaded));

    // .gpl has no alpha channel; names with spaces must survive.
    REQUIRE(loaded.size() == palette.size());
    REQUIRE(loaded.at(0).color == QColor(255, 0, 0));
    REQUIRE(loaded.at(0).name == "Red");
    REQUIRE(loaded.at(1).color == QColor(10, 20, 30));
    REQUIRE(loaded.at(1).name == "Dark Something");

    QFile::remove(path);
}

TEST_CASE("PaletteIO reads the old headerless GPL format and skips comments")
{
    const QString path = tempPalettePath("pencil2d_test_old.gpl");
    {
        QFile file(path);
        REQUIRE(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        out << "GIMP Palette\n";
        out << "# a comment\n";
        out << "\n";
        out << "255 0 0 Red\n";
        out << "0 255 0 Untitled\n"; // "Untitled" becomes an automatic name
    }

    QList<ColorRef> loaded;
    REQUIRE(PaletteIO::importPalette(path, loaded));

    REQUIRE(loaded.size() == 2);
    REQUIRE(loaded.at(0).color == QColor(255, 0, 0));
    REQUIRE(loaded.at(0).name == "Red");
    REQUIRE(loaded.at(1).color == QColor(0, 255, 0));
    // "Untitled" is discarded and the automatic color-dictionary naming
    // kicks in via the ColorRef constructor.
    REQUIRE(loaded.at(1).name != "Untitled");
    REQUIRE_FALSE(loaded.at(1).name.isEmpty());

    QFile::remove(path);
}

TEST_CASE("PaletteIO import appends to an existing palette")
{
    QList<ColorRef> palette;
    palette.append(ColorRef(QColor(1, 2, 3), "Existing"));

    const QString path = tempPalettePath("pencil2d_test_append.xml");
    QList<ColorRef> toWrite;
    toWrite.append(ColorRef(QColor(4, 5, 6), "New"));
    REQUIRE(PaletteIO::exportPalette(path, toWrite));

    REQUIRE(PaletteIO::importPalette(path, palette));
    REQUIRE(palette.size() == 2);
    REQUIRE(palette.at(0).name == "Existing");
    REQUIRE(palette.at(1).name == "New");

    QFile::remove(path);
}

TEST_CASE("PaletteIO reports failure for a missing file")
{
    QList<ColorRef> palette;
    REQUIRE_FALSE(PaletteIO::importPalette(tempPalettePath("pencil2d_no_such_palette.xml"), palette));
    REQUIRE(palette.isEmpty());
}
