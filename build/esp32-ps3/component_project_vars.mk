# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(IDF_PATH)/components/esp32-ps3/src/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/esp32-ps3 -lesp32-ps3
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += esp32-ps3
COMPONENT_LDFRAGMENTS += 
component-esp32-ps3-build: component-bt-build
