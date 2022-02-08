PROJECT_NAME=irrigation_system

AC=arduino-cli 
CFLAG=compile --warnings all 
UFLAG=upload -p

CORE=arduino:avr
BOARDS=${CORE}:nano
PORTS=/dev/ttyACM0

FQBN=--fqbn $(word 1, ${BOARDS})
PORT=$(word 1, ${PORTS})

INO=$(PROJECT_NAME).ino
LIB_DIR=~/Arduino/libraries

all: run #compile


## ARDUINO-CLI MAKE CMD


compile: check_core check_libs
	$(AC) $(CFLAG) $(FQBN) $(INO)

upload: compile
	$(AC) $(UFLAG) $(PORT) $(FQBN) $(INO)

boards:
	$(AC) board list

tty: 
	sudo chmod a+rw $(PORTS)

serial:
	tail -f /dev/cu.usbmodemXXXX

clean:
	$(AC) $(CFLAG) $(FQBN) $(INO) --clean

check_core:
    ifeq ("$(shell arduino-cli core list | grep ${CORE})", "")
		arduino-cli core install ${CORE}
    endif

check_libs:
    ifeq ("$(wildcard ${LIB_DIR}/RTClib)", "")
		arduino-cli lib install "RTClib"
    endif


## EPOXY MAKE CMD

run: epoxy
	@printf "Serial Output:\n"
	@./$(PROJECT_NAME).out

vgd: epoxymk
	@valgrind --leak-check=full ./$(PROJECT).out 

gdb: epoxymk
	@gdb ./$(PROJECT).out -x ./init.gdb

epoxy: check_epoxy
	@$(MAKE) -f epoxy.mk APP_NAME=$(PROJECT_NAME)

epoxyclean:
	@-rm *.out *.o 2> /dev/null
	@$(MAKE) -f epoxy.mk clean APP_NAME=$(PROJECT_NAME)

.PHONY: epoxymk
epoxymk: epoxyclean epoxy

check_epoxy:
    ifeq ("$(wildcard ${LIB_DIR}/EpoxyDuino)", "")
		git clone https://github.com/bxparks/EpoxyDuino ${LIB_DIR}/EpoxyDuino
    endif