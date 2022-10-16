STD_DIR = $(arduino_ide_dir)/libraries

APP_NAME := $(PROJECT_NAME)
ARDUINO_LIBS := AUnit RTClib_EPOXY
ARDUINO_LIB_DIRS := $(STD_DIR) ..
CPPFLAGS += -g -D TEST

-include ${HOME}/Arduino/libraries/EpoxyDuino/EpoxyDuino.mk