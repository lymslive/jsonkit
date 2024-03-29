.PHONY : all tast header libdir tastdir echo clean tast.run

-include local-env.mk

RAPIDJON = ../thirdparty/rapidjson/include
COUTAST  = ../thirdparty/couttast/include
INCLUDE = -I./include -I$(RAPIDJON) -I$(COUTAST)

AR = ar -crs
CXX = g++ -std=c++11
CXXFLAGS = -fPIC

ifdef debug
CXXFLAGS += -g -D_DEBUG
else
CXXFLAGS += -O2
endif

ifdef protobuf
CXXFLAGS += -DHAS_GOOGLE_PROBUF
endif

# for lib target
##################################################

JSONKIT_SRC_DIR = ../src
JSONKIT_SRC = $(wildcard $(JSONKIT_SRC_DIR)/*.cpp)
JSONKIT_SRC_NODIR = $(notdir $(JSONKIT_SRC))
JSONKIT_OBJ_NODIR = $(JSONKIT_SRC_NODIR:%.cpp=%.o)

JSONKIT_OBJ_DIR = ./jsonkit
JSONKIT_OBJ = $(addprefix $(JSONKIT_OBJ_DIR)/, $(JSONKIT_OBJ_NODIR))
JSONKIT_DEP := $(patsubst %.o,%.d,$(JSONKIT_OBJ))

JSONKIT_HEADER = $(wildcard $(JSONKIT_SRC_DIR)/*.h)

TARGET = lib/libjsonkit.a
all: libdir $(TARGET) header
-include $(JSONKIT_DEP)

$(TARGET) : $(JSONKIT_OBJ)
	$(AR) $(TARGET) $(JSONKIT_OBJ)

$(JSONKIT_OBJ): $(JSONKIT_OBJ_DIR)/%.o : $(JSONKIT_SRC_DIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) -o $@ -MMD -MT $@ $<

libdir:
	@mkdir -p lib
	@mkdir -p include
	@mkdir -p jsonkit

header:
	@cp -u $(JSONKIT_HEADER) ./include

# for tast target
##################################################

TAST_SRC_DIR = ../tast/src
TAST_SRC = $(wildcard $(TAST_SRC_DIR)/*.cpp)
TAST_SRC_NODIR = $(notdir $(TAST_SRC))
TAST_OBJ_NODIR = $(TAST_SRC_NODIR:%.cpp=%.o)

TAST_OBJ_DIR = ./tast
TAST_OBJ = $(addprefix $(TAST_OBJ_DIR)/, $(TAST_OBJ_NODIR))

LIBRARY = -L./lib -ljsonkit

TAST_RUN_DIR = ../tast
TAST_TARGET = bin/tast_jsonkit.exe
tast: tastdir $(TAST_TARGET)
$(TAST_TARGET) : $(TAST_OBJ) $(TARGET) 
	$(CXX) -o $(TAST_TARGET) $(CXXFLAGS) $(TAST_OBJ) $(LIBRARY)
	cp -u $(TAST_TARGET) $(TAST_RUN_DIR)/

$(TAST_OBJ): $(TAST_OBJ_DIR)/%.o : $(TAST_SRC_DIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) -o $@ $<

tastdir:
	@mkdir -p bin
	@mkdir -p tast
	
tast.run:
	./run_tast.sh

# 
##################################################

clean:
	rm -rf $(TARGET) $(TAST_TARGET)
	rm -rf $(JSONKIT_OBJ) $(TAST_OBJ)
	rm -rf $(JSONKIT_DEP)
echo:
	@echo JSONKIT_SRC
	@echo $(JSONKIT_SRC)
	@echo JSONKIT_OBJ
	@echo $(JSONKIT_OBJ)
