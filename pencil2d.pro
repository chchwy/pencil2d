
TEMPLATE = app
CONFIG += precompile_header lrelease embed_translations
QT += core widgets gui xml multimedia svg network

include(util/common.pri)
include(core_lib/core_lib.pri)
include(app/app.pri)

#NO_TESTS {

#  SUBDIRS -= tests
#}

TRANSLATIONS += $$PWD/translations/pencil.ts
