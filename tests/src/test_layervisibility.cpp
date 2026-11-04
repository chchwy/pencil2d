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

#include <QSettings>
#include <QTemporaryDir>

#include "editor.h"
#include "layerbitmap.h"
#include "object.h"
#include "scribblearea.h"
#include "selectionmanager.h"

TEST_CASE("Editor::switchVisibilityOfLayer()")
{
    // Keep the preferences read by Editor::init() away from the user's real settings.
    QTemporaryDir settingsDir;
    const QSettings::Format previousFormat = QSettings::defaultFormat();
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());

    {
        Editor editor;
        ScribbleArea canvas(nullptr);
        editor.setScribbleArea(&canvas);
        editor.init();

        Object* object = new Object;
        object->init();
        Layer* currentLayer = object->addNewBitmapLayer();
        Layer* otherLayer = object->addNewBitmapLayer();
        editor.setObject(object);
        editor.setCurrentLayerIndex(0);
        canvas.setEditor(&editor);
        canvas.init();

        editor.select()->setSelection(QRectF(10, 10, 20, 20));
        REQUIRE(editor.select()->somethingSelected());

        SECTION("Hiding the current layer clears its selection")
        {
            editor.switchVisibilityOfLayer(0);

            REQUIRE_FALSE(currentLayer->visible());
            REQUIRE_FALSE(editor.select()->somethingSelected());
        }

        SECTION("Hiding another layer keeps the selection")
        {
            editor.switchVisibilityOfLayer(1);

            REQUIRE_FALSE(otherLayer->visible());
            REQUIRE(editor.select()->somethingSelected());
        }

        SECTION("Showing the current layer keeps the selection")
        {
            currentLayer->setVisible(false);

            editor.switchVisibilityOfLayer(0);

            REQUIRE(currentLayer->visible());
            REQUIRE(editor.select()->somethingSelected());
        }
    }

    QSettings::setDefaultFormat(previousFormat);
}
