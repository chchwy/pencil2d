#-------------------------------------------------
#
# Pencil2D GUI
#
#-------------------------------------------------

RESOURCES += $$PWD/data/app.qrc

MUI_TRANSLATIONS += \
        translations/mui_cs.po \
        translations/mui_de.po \
        translations/mui_it.po

RC_LANGS.cs = --lang LANG_CZECH --sublang SUBLANG_NEUTRAL
RC_LANGS.de = --lang LANG_GERMAN --sublang SUBLANG_NEUTRAL
RC_LANGS.it = --lang LANG_ITALIAN --sublang SUBLANG_NEUTRAL

PRECOMPILED_HEADER = $$PWD/src/app-pch.h

INCLUDEPATH += \
    $$PWD/src \
    $$PWD/../core_lib/src/graphics \
    $$PWD/../core_lib/src/graphics/bitmap \
    $$PWD/../core_lib/src/graphics/vector \
    $$PWD/../core_lib/src/interface \
    $$PWD/../core_lib/src/structure \
    $$PWD/../core_lib/src/tool \
    $$PWD/../core_lib/src/util \
    $$PWD/../core_lib/src/managers \
    $$PWD/../core_lib/src/external \
    $$PWD/../core_lib/src \
    $$PWD/../core_lib/ui

HEADERS += \
    $$PWD/src/app-pch.h \
    $$PWD/src/importlayersdialog.h \
    $$PWD/src/importpositiondialog.h \
    $$PWD/src/layeropacitydialog.h \
    $$PWD/src/mainwindow2.h \
    $$PWD/src/onionskinwidget.h \
    $$PWD/src/predefinedsetmodel.h \
    $$PWD/src/pegbaralignmentdialog.h \
    $$PWD/src/shortcutfilter.h \
    $$PWD/src/actioncommands.h \
    $$PWD/src/preferencesdialog.h \
    $$PWD/src/filespage.h \
    $$PWD/src/generalpage.h \
    $$PWD/src/shortcutspage.h \
    $$PWD/src/timelinepage.h \
    $$PWD/src/toolspage.h \
    $$PWD/src/basedockwidget.h \
    $$PWD/src/colorbox.h \
    $$PWD/src/colorinspector.h \
    $$PWD/src/colorpalettewidget.h \
    $$PWD/src/colorwheel.h \
    $$PWD/src/timeline.h \
    $$PWD/src/timelinecells.h \
    $$PWD/src/timecontrols.h \
    $$PWD/src/cameracontextmenu.h \
    $$PWD/src/camerapropertiesdialog.h \
    $$PWD/src/filedialog.h \
    $$PWD/src/pencil2d.h \
    $$PWD/src/exportmoviedialog.h \
    $$PWD/src/app_util.h \
    $$PWD/src/errordialog.h \
    $$PWD/src/aboutdialog.h \
    $$PWD/src/toolbox.h \
    $$PWD/src/tooloptionwidget.h \
    $$PWD/src/bucketoptionswidget.h \
    $$PWD/src/importexportdialog.h \
    $$PWD/src/exportimagedialog.h \
    $$PWD/src/importimageseqdialog.h \
    $$PWD/src/spinslider.h \
    $$PWD/src/doubleprogressdialog.h \
    $$PWD/src/colorslider.h \
    $$PWD/src/checkupdatesdialog.h \
    $$PWD/src/presetdialog.h     \
    $$PWD/src/repositionframesdialog.h \
    $$PWD/src/commandlineparser.h \
    $$PWD/src/commandlineexporter.h \
    $$PWD/src/statusbar.h \
    $$PWD/src/elidedlabel.h \
    $$PWD/src/cameraoptionswidget.h

SOURCES += \
    $$PWD/src/importlayersdialog.cpp \
    $$PWD/src/importpositiondialog.cpp \
    $$PWD/src/layeropacitydialog.cpp \
    $$PWD/src/main.cpp \
    $$PWD/src/mainwindow2.cpp \
    $$PWD/src/onionskinwidget.cpp \
    $$PWD/src/predefinedsetmodel.cpp \
    $$PWD/src/pegbaralignmentdialog.cpp \
    $$PWD/src/shortcutfilter.cpp \
    $$PWD/src/actioncommands.cpp \
    $$PWD/src/preferencesdialog.cpp \
    $$PWD/src/filespage.cpp \
    $$PWD/src/generalpage.cpp \
    $$PWD/src/shortcutspage.cpp \
    $$PWD/src/timelinepage.cpp \
    $$PWD/src/toolspage.cpp \
    $$PWD/src/basedockwidget.cpp \
    $$PWD/src/colorbox.cpp \
    $$PWD/src/colorinspector.cpp \
    $$PWD/src/colorpalettewidget.cpp \
    $$PWD/src/colorwheel.cpp \
    $$PWD/src/timeline.cpp \
    $$PWD/src/timelinecells.cpp \
    $$PWD/src/timecontrols.cpp \
    $$PWD/src/cameracontextmenu.cpp \
    $$PWD/src/camerapropertiesdialog.cpp \
    $$PWD/src/filedialog.cpp \
    $$PWD/src/pencil2d.cpp \
    $$PWD/src/exportmoviedialog.cpp \
    $$PWD/src/errordialog.cpp \
    $$PWD/src/aboutdialog.cpp \
    $$PWD/src/toolbox.cpp \
    $$PWD/src/tooloptionwidget.cpp \
    $$PWD/src/bucketoptionswidget.cpp \
    $$PWD/src/importexportdialog.cpp \
    $$PWD/src/exportimagedialog.cpp \
    $$PWD/src/importimageseqdialog.cpp \
    $$PWD/src/spinslider.cpp \
    $$PWD/src/doubleprogressdialog.cpp \
    $$PWD/src/colorslider.cpp \
    $$PWD/src/checkupdatesdialog.cpp \
    $$PWD/src/presetdialog.cpp \
    $$PWD/src/repositionframesdialog.cpp \
    $$PWD/src/app_util.cpp \
    $$PWD/src/commandlineparser.cpp \
    $$PWD/src/commandlineexporter.cpp \
    $$PWD/src/statusbar.cpp \
    $$PWD/src/elidedlabel.cpp \
    $$PWD/src/cameraoptionswidget.cpp

FORMS += \
    $$PWD/ui/cameraoptionswidget.ui \
    $$PWD/ui/camerapropertiesdialog.ui \
    $$PWD/ui/importimageseqpreview.ui \
    $$PWD/ui/importlayersdialog.ui \
    $$PWD/ui/importpositiondialog.ui \
    $$PWD/ui/layeropacitydialog.ui \
    $$PWD/ui/mainwindow2.ui \
    $$PWD/ui/onionskin.ui \
    $$PWD/ui/pegbaralignmentdialog.ui \
    $$PWD/ui/repositionframesdialog.ui \
    $$PWD/ui/shortcutspage.ui \
    $$PWD/ui/colorinspector.ui \
    $$PWD/ui/colorpalette.ui \
    $$PWD/ui/errordialog.ui \
    $$PWD/ui/importexportdialog.ui \
    $$PWD/ui/exportmovieoptions.ui \
    $$PWD/ui/exportimageoptions.ui \
    $$PWD/ui/importimageseqoptions.ui \
    $$PWD/ui/tooloptions.ui \
    $$PWD/ui/bucketoptionswidget.ui \
    $$PWD/ui/aboutdialog.ui \
    $$PWD/ui/doubleprogressdialog.ui \
    $$PWD/ui/preferencesdialog.ui \
    $$PWD/ui/generalpage.ui \
    $$PWD/ui/timelinepage.ui \
    $$PWD/ui/filespage.ui \
    $$PWD/ui/toolspage.ui \
    $$PWD/ui/toolboxwidget.ui \
    $$PWD/ui/presetdialog.ui

GIT {
    DEFINES += GIT_EXISTS \
    "GIT_CURRENT_SHA1=$$system(git --git-dir=.git --work-tree=. -C $$_PRO_FILE_PWD_/../ rev-parse HEAD)" \
    "GIT_TIMESTAMP=$$system(git --git-dir=.git --work-tree=. -C $$_PRO_FILE_PWD_/../ log -n 1 --pretty=format:"%cd" --date=format:"%Y-%m-%d_%H:%M:%S")"
}

macx {
    RC_FILE = $$PWD/data/pencil2d.icns

    # Use custom Info.plist
    QMAKE_INFO_PLIST = $$PWD/data/Info.plist

    # Add file icons into the application bundle resources
    FILE_ICONS.files = $$PWD/data/icons/mac_pcl_icon.icns data/icons/mac_pclx_icon.icns
    FILE_ICONS.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FILE_ICONS

    QMAKE_TARGET_BUNDLE_PREFIX += org.pencil2d
    QMAKE_APPLICATION_BUNDLE_NAME = Pencil2D
}

win32 {
    target.path = /
    visualelements.path = /
    visualelements.files = $$PWD/data/pencil2d.VisualElementsManifest.xml $$OUT_PWD/resources.pri
    visualelements.CONFIG += no_check_exist
    visualelements.depends += resources.pri
    resources.path = /resources
    resources.files = data/resources/*

    PRI_CONFIG = $$PWD/data/resources.xml
    PRI_INDEX_NAME = Pencil2D
    RC_FILES = $$PWD/data/version.rc $$PWD/data/mui.rc
    INSTALLS += target visualelements resources

    makepri.name = makepri
    makepri.input = PRI_CONFIG
    makepri.output = ${QMAKE_FILE_IN_BASE}.pri
    makepri.commands = makepri new /o /in $$PRI_INDEX_NAME /pr ${QMAKE_FILE_PATH} /cf ${QMAKE_FILE_IN} /of ${QMAKE_FILE_OUT}
    silent: makepri.commands = @echo makepri ${QMAKE_FILE_IN} && $$makepri.commands
    makepri.CONFIG = no_link
    QMAKE_EXTRA_COMPILERS += makepri

    ensurePathEnv()
    isEmpty(PO2RC): for(dir, QMAKE_PATH_ENV) {
        exists("$$dir/po2rc.exe") {
            PO2RC = "$$dir/po2rc.exe"
            break()
        }
    }
    !isEmpty(PO2RC) {
        defineReplace(rcLang) {
            name = $$basename(1)
            base = $$section(name, ., 0, -2)
            return($$member(RC_LANGS.$$section(base, _, 1), 0, -1))
        }
        po2rc.name = po2rc
        po2rc.input = MUI_TRANSLATIONS
        po2rc.output = ${QMAKE_FILE_IN_BASE}.rc
        po2rc.commands = $$shell_path($$PO2RC) -t $$PWD/data/mui.rc ${QMAKE_FILE_IN} ${QMAKE_FUNC_FILE_IN_rcLang} ${QMAKE_FILE_OUT}
        silent: makepri.commands = @echo po2rc ${QMAKE_FILE_IN} && $$makepri.commands
        po2rc.CONFIG = no_link
        QMAKE_EXTRA_COMPILERS += po2rc
        # variable_out doesn't seem to work in this case
        for(file, MUI_TRANSLATIONS): {
            name = $$basename(file)
            RC_FILES += $$replace(name, .po, .rc)
        }
    } else {
        warning("po2rc was not found. MUI resources will not be translated. You can safely ignore this warning if you do not plan to distribute this build of Pencil2D through its installer.")
    }

    for(file, RC_FILES): RC_INCLUDES += "$${LITERAL_HASH}include \"$$file\""
    write_file($$OUT_PWD/pencil2d.rc, RC_INCLUDES)|error()
    RC_FILE = $$OUT_PWD/pencil2d.rc
    RC_INCLUDEPATH += $$PWD $$PWD/data
}

unix:!macx {
    target.path = $${PREFIX}/bin

    bashcompletion.files = $$PWD/data/pencil2d
    bashcompletion.path = $${PREFIX}/share/bash-completion/completions

    zshcompletion.files = $$PWD/data/_pencil2d
    zshcompletion.path = $${PREFIX}/share/zsh/site-functions

    metainfo.files = $$PWD/data/org.pencil2d.Pencil2D.metainfo.xml
    metainfo.path = $${PREFIX}/share/metainfo

    mimepackage.files = $$PWD/data/org.pencil2d.Pencil2D.xml
    mimepackage.path = $${PREFIX}/share/mime/packages

    desktopentry.files = $$PWD/data/org.pencil2d.Pencil2D.desktop
    desktopentry.path = $${PREFIX}/share/applications

    icon.files = $$PWD/data/org.pencil2d.Pencil2D.png
    icon.path = $${PREFIX}/share/icons/hicolor/256x256/apps

    INSTALLS += bashcompletion zshcompletion target metainfo mimepackage desktopentry icon
}
