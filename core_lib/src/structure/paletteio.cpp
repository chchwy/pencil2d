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
#include "paletteio.h"

#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace
{

/* Import the .gpl GIMP palette format.
 *
 * This functions supports importing both the old and new .gpl formats.
 * This should load colors the same as GIMP, with the following intentional exceptions:
 * - Whitespace before and after a name does not appear in the name
 * - The last line is processed, even if there is not a trailing newline
 * - Colors without a name will use our automatic naming system rather than "Untitled"
 */
void importGPL(QFile& file, QList<ColorRef>& palette)
{
    QTextStream in(&file);
    QString line;

    // The first line must start with "GIMP Palette"
    // Displaying an error here would be nice
    in.readLineInto(&line);
    if (!line.startsWith("GIMP Palette")) return;

    in.readLineInto(&line);

    // There are two GPL formats, the new one must start with "Name: " on the second line
    if (line.startsWith("Name: "))
    {
        in.readLineInto(&line);
        // The new format contains an optional third line starting with "Columns: "
        if (line.startsWith("Columns: "))
        {
            // Skip to the next line
            in.readLineInto(&line);
        }
    }

    // Colors inherit the value from the previous color for missing channels
    // Some palettes may rely on this behavior, so we should try to replicate it
    QColor prevColor(Qt::black);

    do
    {
        // Ignore comments and empty lines
        if (line.isEmpty() || line.startsWith("#")) continue;

        int red = 0;
        int green = 0;
        int blue = 0;

        int countInLine = 0;
        QString name = "";
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        for(const QString& snip : line.split(QRegularExpression("\\s|\\t"), Qt::SkipEmptyParts))
#else
        for(const QString& snip : line.split(QRegularExpression("\\s|\\t"), QString::SkipEmptyParts))
#endif
        {
            switch (countInLine)
            {
            case 0:
                red = snip.toInt();
                break;
            case 1:
                green = snip.toInt();
                break;
            case 2:
                blue = snip.toInt();
                break;
            default:
                name += snip + " ";
            }
            countInLine++;
        }

        // trim additional spaces
        name = name.trimmed();

        // Get values from previous color if necessary
        if (countInLine < 2) green = prevColor.green();
        if (countInLine < 3) blue = prevColor.blue();

        // GIMP assigns colors the name "Untitled" by default now,
        // so in addition to missing names, we also use automatic
        // naming for this
        if (name.isEmpty() || name == "Untitled") name = QString();

        QColor color(red, green, blue);
        if (color.isValid())
        {
            palette.append(ColorRef(color, name));
            prevColor = color;
        }
    } while (in.readLineInto(&line));
}

void importPencil(QFile& file, QList<ColorRef>& palette)
{
    QDomDocument doc;
    doc.setContent(&file);

    QDomElement docElem = doc.documentElement();
    QDomNode tag = docElem.firstChild();
    while (!tag.isNull())
    {
        QDomElement e = tag.toElement(); // try to convert the node to an element.
        if (!e.isNull())
        {
            QString name = e.attribute("name");
            int r = e.attribute("red").toInt();
            int g = e.attribute("green").toInt();
            int b = e.attribute("blue").toInt();
            int a = e.attribute("alpha", "255").toInt();
            palette.append(ColorRef(QColor(r, g, b, a), name));
        }
        tag = tag.nextSibling();
    }
}

void exportGPL(QFile& file, const QList<ColorRef>& palette)
{
    QString fileName = QFileInfo(file).baseName();
    QTextStream out(&file);

    out << "GIMP Palette" << "\n";
    out << "Name: " << fileName << "\n";
    out << "#" << "\n";

    for (const ColorRef& ref : palette)
    {
        QColor toRgb = ref.color.toRgb();
        out << QString("%1 %2 %3").arg(toRgb.red()).arg(toRgb.green()).arg(toRgb.blue());
        out << " " << ref.name << "\n";
    }
}

void exportPencil(QFile& file, const QList<ColorRef>& palette)
{
    QTextStream out(&file);

    QDomDocument doc("PencilPalette");
    QDomElement root = doc.createElement("palette");
    doc.appendChild(root);
    for (const ColorRef& ref : palette)
    {
        QDomElement tag = doc.createElement("Color");
        tag.setAttribute("name", ref.name);
        tag.setAttribute("red", ref.color.red());
        tag.setAttribute("green", ref.color.green());
        tag.setAttribute("blue", ref.color.blue());
        tag.setAttribute("alpha", ref.color.alpha());
        root.appendChild(tag);
    }
    int indentSize = 2;
    doc.save(out, indentSize);
}

} // namespace

bool PaletteIO::importPalette(const QString& filePath, QList<ColorRef>& palette)
{
    QFile file(filePath);

    if (!file.open(QFile::ReadOnly))
    {
        return false;
    }

    if (file.fileName().endsWith(".gpl", Qt::CaseInsensitive))
    {
        importGPL(file, palette);
    } else {
        importPencil(file, palette);
    }
    return true;
}

bool PaletteIO::exportPalette(const QString& filePath, const QList<ColorRef>& palette)
{
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug("Error: cannot export palette");
        return false;
    }

    if (file.fileName().endsWith(".gpl", Qt::CaseInsensitive))
        exportGPL(file, palette);
    else
        exportPencil(file, palette);

    return true;
}
