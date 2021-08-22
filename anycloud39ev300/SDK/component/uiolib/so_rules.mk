CC		:= $(PREFIX)gcc
CPP		:= $(PREFIX)g++
AS		:= $(CC)
LD		:= $(CC)
AR		:= $(PREFIX)ar crv

ifdef O
BUILD_DIR=$(O)
else
BUILD_DIR=BUILD_$(LOCAL_MODULE)_SO
endif

SONAME		:= $(LOCAL_MODULE).so

CFLAGS		:= $(LOCAL_CFLAGS) $(addprefix -I,$(LOCAL_C_INCLUDES)) -Wall -Werror -fPIC
ASFLAGS		:= $(LOCAL_ASFLAGS) -Wall -fPIC
CPPFLAGS	:= $(LOCAL_CPPFLAGS) -Wall -fPIC
LDFLAGS		:= $(LOCAL_LDFLAGS) $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIBRARIES))) \
		-shared -Wl,-soname,$(SONAME) -Wl,--no-undefined

C_SRC		:= $(filter %.c,$(LOCAL_SRC_FILES))
C_OBJ		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRC))

ASM_SRC		:= $(filter %.S,$(LOCAL_SRC_FILES))
ASM_OBJ		:= $(patsubst %.S,$(BUILD_DIR)/%.o,$(ASM_SRC))

CPP_SRC		:= $(filter %.cpp,$(LOCAL_SRC_FILES))
CPP_OBJ		:= $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CPP_SRC))

OBJ_DIRS	:= $(sort $(dir $(C_OBJ)) $(dir $(ASM_OBJ)) $(dir $(CPP_OBJ)))

# rm do not accept the path which was ended by (., ./, .., ../)
OBJ_DIRS	:= $(patsubst %./,%,$(OBJ_DIRS))
OBJ_DIRS	:= $(patsubst %.,%,$(OBJ_DIRS))
OBJ_DIRS	:= $(patsubst %../,%,$(OBJ_DIRS))
OBJ_DIRS	:= $(patsubst %..,%,$(OBJ_DIRS))

.PHONY: all checkdirs clean install

TARGET		:= $(BUILD_DIR)/$(SONAME).$(LOCAL_VERSION)

all: checkdirs $(TARGET)

$(TARGET): $(C_OBJ) $(ASM_OBJ) $(CPP_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	$(AR) $(BUILD_DIR)/$(LOCAL_MODULE).a $^

$(C_OBJ): $(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ASM_OBJ): $(BUILD_DIR)/%.o: %.S
	$(AS) $(ASFLAGS) -c $< -o $@

$(CPP_OBJ): $(BUILD_DIR)/%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

checkdirs: $(OBJ_DIRS)

$(OBJ_DIRS):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIRS)

install:
	$(CP) $(TARGET)		$(PLATFORM_LIB_DIR)
	$(CP) $(TARGET_INC)	$(PLATFORM_LIB_INC_DIR)

rebuild: clean all
