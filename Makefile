# tool macros
CC := clang++
CCFLAG := `llvm-config --cxxflags --ldflags --system-libs --libs all`
INCLUDE_PATH := include
INCLUDES = -I include/ -I /usr/local/include
DBGFLAG := -g
CCOBJFLAG := $(CCFLAG)

# path macros
BUILD_PATH := build
OBJ_PATH := $(BUILD_PATH)/obj
BIN_PATH := $(BUILD_PATH)/bin
SRC_PATH := src

# flex
SCANNER := $(SRC_PATH)/scanner.cpp

# bison
PARSER := $(SRC_PATH)/parser.cpp
PARSER_HEADER := $(INCLUDE_PATH)/parser.hpp

# compile macros
TARGET_NAME := coo
ifeq ($(OS),Windows_NT)
	TARGET_NAME := $(addsuffix .exe,$(TARGET_NAME))
endif
TARGET := $(BIN_PATH)/$(TARGET_NAME)
BUILTIN := $(OBJ_PATH)/builtin.o
MAIN_SRC := coo.cpp

# src files & obj files
SCANNER_SRC := $(SRC_PATH)/scanner.l
PARSER_SRC := $(SRC_PATH)/parser.y
BUILTIN_SRC := $(SRC_PATH)/builtin.c
SRC = $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.cpp)))
OBJ = $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# test file
TEST_OUTPUT := test/output/*

# clean files list
DISTCLEAN_LIST := $(OBJ) \
				  $(PARSER_HEADER)\
				  $(PARSER)\
				  $(SCANNER)\
				  *.o
CLEAN_LIST := $(TARGET) \
			  $(DISTCLEAN_LIST) \
			  $(TARGET_NAME) \
			  $(TEST_OUTPUT)

# default rule
default: dirs
	@$(MAKE) parser
	@$(MAKE) all

parser: $(SCANNER)

all: $(TARGET) $(BUILTIN)
	@echo "Making symlink: $(TARGET_NAME) -> $<"
	@$(RM) $(TARGET_NAME)
	@ln -s `readlink -f $(TARGET)` $(TARGET_NAME)

$(BUILTIN) : $(BUILTIN_SRC)
	cc -o $@ -c $^

# non-phony targets
$(TARGET): $(OBJ)
	$(CC) $(CCFLAG) $(INCLUDES) -o $@ $^

$(OBJ): $(SCANNER)

$(SCANNER): $(SCANNER_SRC) $(PARSER_HEADER)
	flex -o $@ $^

$(PARSER_HEADER): $(PARSER)
	mv $(SRC_PATH)/parser.hpp $@

$(PARSER): $(PARSER_SRC)
	bison -d -o $@ $^

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAG) $(INCLUDES) -o $@ -c $<

# phony rules
.PHONY: all

.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(BUILD_PATH))
	@mkdir -p $(OBJ_PATH)
	@mkdir -p $(BIN_PATH)

# test #

.PHONY: test
test: default
	bash script/testall.sh

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)

.PHONY: testclean
testclean:
	@echo CLEAN $(TEST_OUTPUT)
	@rm -f $(TEST_OUTPUT)