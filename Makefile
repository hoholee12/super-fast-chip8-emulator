.PHONY: default
.DEFAULT_GOAL: default

SRC_DIR = $(CURDIR)/src
IMGUI_DIR = $(SRC_DIR)/imgui
GLAD_DIR = $(SRC_DIR)/glad
BUILD_DIR = $(CURDIR)/build

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
SOURCES += $(wildcard $(IMGUI_DIR)/*.cpp)
SOURCES += $(wildcard $(IMGUI_DIR)/backends/*.cpp)
OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -m32
CXXFLAGS += -g -O1 -Wno-unused-result
LIBS =

## Using OpenGL loader: glad
SOURCES += $(wildcard $(GLAD_DIR)/src/*.c)
CXXFLAGS += -I$(GLAD_DIR)/include -DIMGUI_IMPL_OPENGL_LOADER_GLAD

LIBS += -lGL -ldl `sdl2-config --libs`

CXXFLAGS += `sdl2-config --cflags`
CFLAGS = $(CXXFLAGS)

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

default: $(BUILD_DIR) $(BUILD_DIR)/chip8-emu

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(GLAD_DIR)/src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/chip8-emu: $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
