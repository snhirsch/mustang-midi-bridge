
# Newer distributions put rtmidi headers in subdirectory
RTMIDI_INC := $(shell ls -d /usr/include/rtmidi 2>/dev/null | tail -n 1)
ifneq (,$(RTMIDI_INC))
  INCDIRS = -I/usr/include/rtmidi
endif

SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))
DEP = $(subst .cpp,.d,$(SRC))

# The -M* switches automatically generate .d dependency files
CPPFLAGS += -MP -MMD $(INCDIRS)

LDLIBS = -lrtmidi -lusb-1.0 -lpthread

BIN = mustang_midi

opt: CXXFLAGS += -O3 -DNDEBUG
opt: $(BIN)

debug: CXXFLAGS += -g -DDEBUG
debug: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $^ -o $@ $(LDLIBS)

clean: 
	rm -f $(DEP) $(OBJ) $(BIN) *~

-include $(SRC:.cpp=.d)

