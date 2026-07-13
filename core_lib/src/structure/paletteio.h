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
#ifndef PALETTEIO_H
#define PALETTEIO_H

#include <QList>

#include "colorref.h"

class QString;

/** Reading and writing of palette files (GIMP .gpl and Pencil2D XML),
 *  independent of the document model. */
namespace PaletteIO
{
    /** Reads a palette file and APPENDS its colors to the given palette.
     *  .gpl files are parsed as GIMP palettes (old and new format), anything
     *  else as Pencil2D palette XML.
     *  @return false when the file cannot be opened. */
    bool importPalette(const QString& filePath, QList<ColorRef>& palette);

    /** Writes the palette to the given path — as a GIMP palette when the
     *  path ends in .gpl, otherwise as Pencil2D palette XML.
     *  @return false when the file cannot be opened for writing. */
    bool exportPalette(const QString& filePath, const QList<ColorRef>& palette);
}

#endif // PALETTEIO_H
