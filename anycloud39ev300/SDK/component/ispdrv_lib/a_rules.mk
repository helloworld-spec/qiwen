CC		:= $(PREFIX)gcc
ifdef USE_CPP
CXX		:= $(PREFIX)g++
endif
AS		:= $(CC)
ifdef USE_CPP
LD		:= $(CXX)
else
LD		:= $(CC)
endif
AR		:= $(PREFIX)ar

ifdef O
BUILD_DIR=$(O)
else
BUILD_DIR=BUILD_LIBA
endif

#-g -rdynamic
ifdef USE_CPP
CXXFLAGS	:= $(LOCAL_CFLAGS) $(addprefix -I,$(LOCAL_C_INCLUDES)) -Wall $(DefineUse) -fms-extensions
endif
CFLAGS		:= $(LOCAL_CFLAGS) $(addprefix -I,$(LOCAL_C_INCLUDES)) -Wall $(DefineUse) -std=c99 -fms-extensions
LDFLAGS		:= $(LOCAL_LDFLAGS) $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIBRARIES)))
ARFLAGS		:= $(LOCAL_ARFLAGS)

ifdef USE_CPP
OBJSC		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(LOCAL_SRC_CFILES))
OBJSCXX := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LOCAL_SRC_CXXFILES))
OBJS := $(OBJSCXX) $(OBJSC)
else
OBJS		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(LOCAL_SRC_CFILES))
endif
OBJ_DIRS	:= $(sort $(dir $(OBJS)))


.PHONY: all checkdirs clean rebuild install

TARGET		:= $(BUILD_DIR)/$(LOCAL_MODULE)

all: checkdirs $(TARGET)

$(TARGET): $(OBJS)
	@$(AR) -r $@ $^ $(ARFLAGS)

ifdef USE_CPP
$(OBJSC): $(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJSCXX): $(BUILD_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
else
$(OBJS): $(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
endif

checkdirs: $(OBJ_DIRS)

$(OBJ_DIRS):
	@mkdir -p $@

install:
	@$(CP) $(TARGET) $(KERNEL_LIB_DIR)
	@$(CP) ak_isp_drv.h $(KERNEL_LIB_INC_DIR)

clean:
	@rm -rf $(BUILD_DIR)
rebuild: clean all
