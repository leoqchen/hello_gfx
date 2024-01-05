.PHONY: default all clean print
default: all


#########################################################
# 确定机器平台
#
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
  TARGET_ARCH_ABI := x86_64
endif
ifeq ($(UNAME_M),aarch64)
  TARGET_ARCH_ABI := arm64-v8a
endif
$(warning TARGET_ARCH_ABI=$(TARGET_ARCH_ABI))


#########################################################
# 确定gcc版本
#
GCC_VERSION = $(shell gcc -dumpversion)
#$(warning GCC_VERSION=$(GCC_VERSION))


#########################################################
# 确定-j数目
#
MAKE_PID := $(shell echo $$PPID)
JOB_FLAG := $(filter -j%, $(subst -j ,-j,$(shell ps T | grep "^\s*$(MAKE_PID).*$(MAKE)")))
JOBS     := $(subst -j,,$(JOB_FLAG))
ifeq ($(JOBS),)
	JOBS := 1
endif
#$(warning JOBS=$(JOBS))


#########################################################
CC  := gcc
CXX	:= g++
CXXFLAGS +=
LDFLAGS +=
LIBS += 
AR := gcc-ar
ARFLAGS := rcs
glslangValidator := glslangValidator -Os

ObjectDir := obj
ifeq ($(TARGET_ARCH_ABI),x86_64)
	ObjectDir := obj/x86_64
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
	ObjectDir := obj/arm64-v8a
endif

# for -D
CommonFlagsD :=
ifeq ($(TARGET_ARCH_ABI),x86_64)
	CommonFlagsD += 
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
	CommonFlagsD += 
endif
CommonFlagsD += 

# for -I, -isystem
CommonFlagsI := 

# for compiler
O := -O0
CommonFlagsC := $(O) -std=c++20 -pipe -MMD -g -ggdb3 -fPIC -pthread -Wall -Werror
CommonLinkFlags := $(O) -pipe -MMD -g -ggdb3 -pthread -Wl,--no-undefined
CXX_ONLY := -std=c++20 -std=gnu++11 -std=gnu++17
C_ONLY := -std=gnu99

AllTargets :=

#######################################################################################################

#-------------------------------------------------------
# glad
#-------------------------------------------------------
CommonFlags_glad := \
	$(CommonFlagsC) \
	$(CommonFlagsD) \
	$(CommonFlagsI) \
	-Isrc/utils \

SourceDir_glad := src/utils/glad
Source_glad := \
	glad.c
SourceMain_glad :=

CXXFLAGS_glad := \
	$(CommonFlags_glad) \
	$(patsubst %, -I%, $(SourceDir_glad)) \

CFLAGS_glad := $(filter-out $(CXX_ONLY), $(CXXFLAGS_glad))  #remove c++ only flags

LocalLib_glad :=
ExtLib_glad := 
#---------------------------------

Object_glad := $(patsubst %.c, $(ObjectDir)/$(SourceDir_glad)/%.o, $(Source_glad))
Object_glad := $(patsubst %.cpp, $(ObjectDir)/$(SourceDir_glad)/%.o, $(Object_glad))
Object_glad := $(patsubst %.S, $(ObjectDir)/$(SourceDir_glad)/%.o, $(Object_glad))

ObjectMain_glad := $(patsubst %.cpp, $(ObjectDir)/$(SourceDir_glad)/%.o, $(SourceMain_glad))
BinaryMain_glad := $(patsubst %.cpp, $(ObjectDir)/$(SourceDir_glad)/%, $(SourceMain_glad))
AR_glad := $(ObjectDir)/$(SourceDir_glad)/libglad.a

Depand_glad := $(Object_glad:.o=.d) $(ObjectMain_glad:.o=.d)

# *.o <-- *.c
$(ObjectDir)/$(SourceDir_glad)/%.o: $(SourceDir_glad)/%.c
	@printf "\n" && mkdir -p $(@D)
	$(CC) $(CFLAGS_glad) -c $< -o $@

# *.o <-- *.cpp
$(ObjectDir)/$(SourceDir_glad)/%.o: $(SourceDir_glad)/%.cpp
	@printf "\n" && mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_glad) -c $< -o $@

# *.o <-- *.S
$(ObjectDir)/$(SourceDir_glad)/%.o: $(SourceDir_glad)/%.S
	@printf "\n" && mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_glad) -c $< -o $@

# *.a <-- *.o
$(AR_glad): $(Object_glad)
	@printf "\n" && mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

# binary <-- *.o *.a
$(ObjectDir)/$(SourceDir_glad)/%: $(ObjectDir)/$(SourceDir_glad)/%.o $(Object_glad) $(LocalLib_glad)
	@printf "\n" && mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_glad) -Wl,--start-group $^ $(ExtLib_glad) -Wl,--end-group -o $@
	@mkdir -p $(ObjectDir)/bin && ln -s -r -f $@ -t $(ObjectDir)/bin

.PHONY: glad
glad: $(Object_glad) $(AR_glad) $(ObjectMain_glad) $(BinaryMain_glad) 

-include $(Depand_glad)
AllTargets += glad


#-------------------------------------------------------
# gl
#-------------------------------------------------------
CommonFlags_gl := \
	$(CommonFlagsC) \
	$(CommonFlagsD) \
	$(CommonFlagsI) \
	-Isrc/utils \
	-Isrc/gl \

SourceDir_gl := src/gl
Source_gl := 
SourceMain_gl := \
	hello_triangle.cpp \
	glClear.cpp \

CXXFLAGS_gl := \
	$(CommonFlags_gl) \
	$(patsubst %, -I%, $(SourceDir_gl)) \

CFLAGS_gl := $(filter-out $(CXX_ONLY), $(CXXFLAGS_gl))  #remove c++ only flags

LocalLib_gl := ${AR_glad}
ExtLib_gl := \
	-ldl \
	`pkg-config --libs glfw3` \

#---------------------------------

Object_gl := $(patsubst %.c, $(ObjectDir)/$(SourceDir_gl)/%.o, $(Source_gl))
Object_gl := $(patsubst %.cpp, $(ObjectDir)/$(SourceDir_gl)/%.o, $(Object_gl))
Object_gl := $(patsubst %.S, $(ObjectDir)/$(SourceDir_gl)/%.o, $(Object_gl))

ObjectMain_gl := $(patsubst %.cpp, $(ObjectDir)/$(SourceDir_gl)/%.o, $(SourceMain_gl))
BinaryMain_gl := $(patsubst %.cpp, $(ObjectDir)/$(SourceDir_gl)/%, $(SourceMain_gl))
AR_gl := $(ObjectDir)/$(SourceDir_gl)/libgl.a

Depand_gl := $(Object_gl:.o=.d) $(ObjectMain_gl:.o=.d)

# *.o <-- *.c
$(ObjectDir)/$(SourceDir_gl)/%.o: $(SourceDir_gl)/%.c
	@printf "\n" && mkdir -p $(@D)
	$(CC) $(CFLAGS_gl) -c $< -o $@

# *.o <-- *.cpp
$(ObjectDir)/$(SourceDir_gl)/%.o: $(SourceDir_gl)/%.cpp
	@printf "\n" && mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_gl) -c $< -o $@

# *.o <-- *.S
$(ObjectDir)/$(SourceDir_gl)/%.o: $(SourceDir_gl)/%.S
	@printf "\n" && mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_gl) -c $< -o $@

# *.a <-- *.o
$(AR_gl): $(Object_gl)
	@printf "\n" && mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

# binary <-- *.o *.a
$(ObjectDir)/$(SourceDir_gl)/%: $(ObjectDir)/$(SourceDir_gl)/%.o $(Object_gl) $(LocalLib_gl)
	@printf "\n" && mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_gl) -Wl,--start-group $^ $(ExtLib_gl) -Wl,--end-group -o $@
	@mkdir -p $(ObjectDir)/bin && ln -s -r -f $@ -t $(ObjectDir)/bin

.PHONY: gl
gl: $(Object_gl) $(AR_gl) $(ObjectMain_gl) $(BinaryMain_gl) 

-include $(Depand_gl)
AllTargets += gl


#######################################################################################################
all: $(AllTargets)

# debug print
print:
	@printf "JOBS=$(JOBS)\n"
	@printf "Object_XXX=$(Object_XXX)\n"
	@printf "ObjectMain_XXX=$(ObjectMain_XXX)\n"

clean:
	rm -rf $(ObjectDir)/*

