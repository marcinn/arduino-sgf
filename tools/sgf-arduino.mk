SKETCH ?= $(CURDIR)

board ?= $(if $(BOARD),$(BOARD),$(firstword $(SUPPORTED_BOARDS)))
port ?= $(if $(PORT),$(PORT),$(DEFAULT_PORT_$(board)))
monitor_config ?= $(if $(MONITOR_CONFIG),$(MONITOR_CONFIG),115200)

ARDUINO_SKETCHBOOK ?= $(HOME)/Arduino
ARDUINO_CLI ?= $(shell command -v arduino-cli 2>/dev/null)

ifeq ($(strip $(ARDUINO_CLI)),)
ARDUINO_CLI := /opt/arduino-ide/resources/app/lib/backend/resources/arduino-cli
endif

FQBN := $(FQBN_$(board))
BOARD_OPTIONS := $(BOARD_OPTIONS_$(board))
SGF_HW_PRESET := $(SGF_HW_PRESET_$(board))
BUILD_DIR := $(CURDIR)/build/$(board)
STAGE_ROOT := $(CURDIR)/build-stage/$(board)
STAGE_DIR := $(STAGE_ROOT)/$(SKETCH_NAME)
LOCAL_SOURCES ?= $(wildcard *.ino) $(wildcard *.cpp) $(wildcard *.h)
STAGE_LINKS ?=
LIBRARY_DIRS ?= $(ARDUINO_SKETCHBOOK)/libraries
LIBRARY_FLAGS := $(foreach dir,$(LIBRARY_DIRS),--libraries $(dir))
BUILD_EXTRA_FLAGS ?= -DSGF_HW_PRESET=$(SGF_HW_PRESET)

BOARD_OPTION_FLAG :=
ifneq ($(strip $(BOARD_OPTIONS)),)
BOARD_OPTION_FLAG := --board-options $(BOARD_OPTIONS)
endif

MONITOR_CONFIG_FLAG :=
ifneq ($(strip $(monitor_config)),)
MONITOR_CONFIG_FLAG := --config "$(monitor_config)"
endif

ifeq ($(strip $(SKETCH_NAME)),)
$(error SKETCH_NAME must be set before including sgf-arduino.mk)
endif

ifeq ($(strip $(SUPPORTED_BOARDS)),)
$(error SUPPORTED_BOARDS must be set before including sgf-arduino.mk)
endif

ifeq ($(strip $(FQBN)),)
$(error Unsupported board '$(board)'. Supported values: $(SUPPORTED_BOARDS))
endif

ifeq ($(strip $(SGF_HW_PRESET)),)
$(error Missing SGF_HW_PRESET mapping for board '$(board)')
endif

.PHONY: help boards info stage build upload flash monitor clean

help:
	@printf '%s\n' \
	  'Targets:' \
	  '  make build board=$(firstword $(SUPPORTED_BOARDS))' \
	  '  make upload board=$(firstword $(SUPPORTED_BOARDS))' \
	  '  make flash board=$(firstword $(SUPPORTED_BOARDS))' \
	  '  make monitor board=$(firstword $(SUPPORTED_BOARDS))' \
	  '  make clean board=$(firstword $(SUPPORTED_BOARDS))' \
	  '' \
	  'Supported boards:'
	@$(foreach b,$(SUPPORTED_BOARDS),printf '  $(b) -> $(FQBN_$(b)), $(SGF_HW_PRESET_$(b))\n';)
	@printf '%s\n' '' 'Default ports:'
	@$(foreach b,$(SUPPORTED_BOARDS),printf '  $(b) -> $(DEFAULT_PORT_$(b))\n';)
	@printf '%s\n' '' 'Override with: make flash board=$(firstword $(SUPPORTED_BOARDS)) port=/dev/ttyXXX'

boards:
	@$(foreach b,$(SUPPORTED_BOARDS),printf '$(b): fqbn=$(FQBN_$(b)) preset=$(SGF_HW_PRESET_$(b)) port=$(DEFAULT_PORT_$(b))\n';)

info:
	@printf '%s\n' \
	  'SKETCH=$(SKETCH_NAME)' \
	  'board=$(board)' \
	  'port=$(port)' \
	  'FQBN=$(FQBN)' \
	  'BOARD_OPTIONS=$(BOARD_OPTIONS)' \
	  'SGF_HW_PRESET=$(SGF_HW_PRESET)' \
	  'BUILD_DIR=$(BUILD_DIR)' \
	  'STAGE_DIR=$(STAGE_DIR)' \
	  'ARDUINO_CLI=$(ARDUINO_CLI)'

stage:
	rm -rf "$(STAGE_ROOT)"
	mkdir -p "$(STAGE_DIR)"
	for file in $(LOCAL_SOURCES); do \
	  ln -s "$(CURDIR)/$$file" "$(STAGE_DIR)/$$file"; \
	done
	for link in $(STAGE_LINKS); do \
	  mkdir -p "$(STAGE_DIR)/$$(dirname "$$link")"; \
	  ln -s "$(CURDIR)/$$link" "$(STAGE_DIR)/$$link"; \
	done

build: stage
	"$(ARDUINO_CLI)" compile \
	  --clean \
	  --warnings all \
	  --fqbn "$(FQBN)" \
	  $(BOARD_OPTION_FLAG) \
	  --build-path "$(BUILD_DIR)" \
	  $(LIBRARY_FLAGS) \
	  --build-property "build.extra_flags=$(BUILD_EXTRA_FLAGS)" \
	  "$(STAGE_DIR)"

upload: build
	@test -n "$(port)" || { \
	  echo "port is required, e.g. make board=$(board) port=/dev/ttyACM0 upload" >&2; \
	  exit 1; \
	}
	"$(ARDUINO_CLI)" upload \
	  --fqbn "$(FQBN)" \
	  $(BOARD_OPTION_FLAG) \
	  --build-path "$(BUILD_DIR)" \
	  --port "$(port)" \
	  "$(STAGE_DIR)"

flash: upload

monitor:
	@test -n "$(port)" || { \
	  echo "port is required, e.g. make board=$(board) port=/dev/ttyACM0 monitor" >&2; \
	  exit 1; \
	}
	"$(ARDUINO_CLI)" monitor \
	  --fqbn "$(FQBN)" \
	  $(BOARD_OPTION_FLAG) \
	  --port "$(port)" \
	  $(MONITOR_CONFIG_FLAG)

clean:
	rm -rf "$(BUILD_DIR)" "$(STAGE_ROOT)"
