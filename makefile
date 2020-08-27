.PHONY: default
.DEFAULT_GOAL: default

SRCDIR=$(CURDIR)/src
OBJDIR=$(CURDIR)/build

SOURCES=$(wildcard $(SRCDIR)/*.cpp)
OBJECTS=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
DEPENDS=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.d,$(SOURCES))

CXXFLAGS=-g -O1 -Wno-unused-result -m32
LDFLAGS=`pkg-config --cflags --libs sdl2` -lSDL2main -lSDL2 -lSDL2_mixer -m32

default: $(OBJDIR)/chip8-emu

$(OBJDIR)/%.d: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -MM -MT $(patsubst %.d,%.o,$@) -MF $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/chip8-emu: $(OBJECTS) | $(OBJDIR)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

-include $(DEPENDS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)
