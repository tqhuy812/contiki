CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

CONTIKI=../../../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CONTIKI_WITH_IPV6 = 1
# # automatically build RESTful resources
REST_RESOURCES_DIR = ./resources
ifndef TARGET
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
else
# ifeq ($(TARGET), native)
# REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
# else
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c' ! -name 'res-plugtest*'))
endif
# endif

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

# linker optimizations
SMALL=1

# REST Engine shall use Erbium CoAP implementation
# APPS += er-coap
# APPS += rest-engine

# CFLAGS += -ffunction-sections
#  LDFLAGS += -Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__


CFLAGS += -DWITH_COAP
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
# APPS += orchestra
# APPS += json
APPS += er-coap
APPS += rest-engine


APPS += squatix-node
# APPS += orchestra-node
MODULES += core/net/mac/tsch


include $(CONTIKI)/Makefile.include
