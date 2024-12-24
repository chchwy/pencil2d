######################################################################
# Pencil2D QMake Project Entry
######################################################################

TEMPLATE = app
CONFIG += precompile_header lrelease embed_translations
QT += core widgets gui xml multimedia svg network
TARGET = pencil2d

include(util/common.pri)
include(core_lib/core_lib.pri)
include(app/app.pri)

TRANSLATIONS += \
        $$PWD/translations/pencil.ts \
        $$PWD/translations/pencil_ar.ts \
        $$PWD/translations/pencil_bg.ts \
        $$PWD/translations/pencil_ca.ts \
        $$PWD/translations/pencil_cs.ts \
        $$PWD/translations/pencil_da.ts \
        $$PWD/translations/pencil_de.ts \
        $$PWD/translations/pencil_el.ts \
        $$PWD/translations/pencil_en.ts \
        $$PWD/translations/pencil_es.ts \
        $$PWD/translations/pencil_et.ts \
        $$PWD/translations/pencil_fa.ts \
        $$PWD/translations/pencil_fr.ts \
        $$PWD/translations/pencil_he.ts \
        $$PWD/translations/pencil_hu_HU.ts \
        $$PWD/translations/pencil_id.ts \
        $$PWD/translations/pencil_it.ts \
        $$PWD/translations/pencil_ja.ts \
        $$PWD/translations/pencil_kab.ts \
        $$PWD/translations/pencil_ko.ts \
        $$PWD/translations/pencil_nb.ts \
        $$PWD/translations/pencil_nl_NL.ts \
        $$PWD/translations/pencil_pl.ts \
        $$PWD/translations/pencil_pt.ts \
        $$PWD/translations/pencil_pt_BR.ts \
        $$PWD/translations/pencil_ru.ts \
        $$PWD/translations/pencil_sl.ts \
        $$PWD/translations/pencil_sv.ts \
        $$PWD/translations/pencil_tr.ts \
        $$PWD/translations/pencil_vi.ts \
        $$PWD/translations/pencil_yue.ts \
        $$PWD/translations/pencil_zh_CN.ts \
        $$PWD/translations/pencil_zh_TW.ts
