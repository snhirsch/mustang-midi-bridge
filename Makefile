

SRC = $(wildcard *.cpp)
OBJ = $(subst .cpp,.o,$(SRC))
DEP = $(subst .cpp,.d,$(SRC))

# The -M* switches automatically generate .d dependency files
CPPFLAGS += -MP -MMD $(INCDIRS)

LDLIBS = -ljack -lrtmidi -lusb-1.0

BIN = mustang_midi

opt: CXXFLAGS += -O3 -DNDEBUG
opt: $(BIN)

debug: CXXFLAGS += -g -DDEBUG
debug: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $^ -o $@ $(LDLIBS)

clean: 
	rm -f $(DEP) $(OBJ) $(BIN)

-include $(SRC:.cpp=.d)

