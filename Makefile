SRC_DIR = src
BUILD_DIR = build
BUILD_NAME = sim.out

SRC = $(shell find $(SRC_DIR) -name "*.cpp")
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEP = $(OBJ:.o=.d)
BUILD_TREE = $(shell dirname $(OBJ))

LDFLAGS = -lSDL2 -lGLEW -lGL
CXXFLAGS = -Wall -Wextra #-g



$(BUILD_DIR)/$(BUILD_NAME): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)


%.o: %.d
	$(CXX) $(CXXFLAGS) -o $@ -c $(@:$(BUILD_DIR)/%.o=$(SRC_DIR)/%.cpp)


-include $(DEP)
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.cpp
	@$(CPP) $< -MM -MT $(@:.d=.o) > $@


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/* && mkdir -p $(BUILD_TREE)

.PHONY: debug
debug:
	echo $(BUILD_TREE)
