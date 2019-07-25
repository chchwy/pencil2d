TEMPLATE = subdirs

SUBDIRS = json-c \
          libmypaint \
          src

# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
