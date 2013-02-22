#### --Section-- global configuration  ####
TargetName = main
CC = c99
CFLAGS = -g -O3 -Wall -Wextra  -rdynamic -DNDEBUG $(DEFS) $(OPTFLAGS) -I$(SRCDIR) $(INCFLAGS)
DEV_CFLAGS = -g -Wall -Wextra  $(DEFS) $(OPTFLAGS) -I$(SRCDIR) $(INCFLAGS)
TEST_CFLAGS = -g -Wall -Wextra  $(DEFS) $(OPTFLAGS) $(TESTFLAGS) -I$(SRCDIR) $(INCFLAGS)
LDLIBS = $(LIBDIRS) $(LIBS)


SRCDIR = src
INCDIR = include
SOURCES = $(shell find $(SRCDIR)/ -name "*.c" -print)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))


BUILD = build/release
BUILD_DEV = build/debug
#if '*/main.c' is included in src directory, an executable program is produced.
IS_OUTPUT_BIN = $(if $(findstring main.c,$(notdir $(SOURCES))),yes,)
TARGET_A = lib/lib$(TargetName).a
TARGET_SO = $(patsubst %.a,%.so,$(TARGET_A))
TARGET_BIN = bin/$(TargetName)

TESTDIR = tests
TEST_SRC = $(shell find $(TESTDIR)/ -name "*.c" -print)
TESTS = $(patsubst %.c,%,$(TEST_SRC))
#### --Section-- user defined configueration ####
include ./cfg.mk
#### --Section-- target build ####
.PHONY: all dev run run-dev
#all
ifeq ($(IS_OUTPUT_BIN),yes)
all: $(TARGET_BIN)
else
all: $(TARGET_A) $(TARGET_SO)
endif
#dev
dev: BUILD = $(BUILD_DEV)
dev: CFLAGS = $(DEV_CFLAGS)
dev: all
#run
ifeq ($(IS_OUTPUT_BIN),yes)
run: all
	@echo  "\033[90m==============Running $(BUILD)/bin/$(TargetName)==============\033[m"
	@echo ""
	@$(BUILD)/bin/$(TargetName)
	@echo ""
	@echo "\033[90m================End $(BUILD)/bin/$(TargetName)================\033[m"
run-dev: dev
	@echo  "\033[90m==============Running $(BUILD_DEV)/bin/$(TargetName)==============\033[m"
	@echo ""
	@$(BUILD_DEV)/bin/$(TargetName)
	@echo ""
	@echo "\033[90m================End $(BUILD_DEV)/bin/$(TargetName)================\033[m"
else
run run-dev:
	@echo "Nothing to run!"
endif
#target
ifeq ($(IS_OUTPUT_BIN),yes)
$(TARGET_BIN): bin $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $(BUILD)/$@
else
$(TARGET_A): CFLAGS += -fPIC
$(TARGET_A): lib $(OBJECTS)
	ar rcs $(BUILD)/$@ $(OBJECTS)
	ranlib $(BUILD)/$@

$(TARGET_SO): $(TARGET_A) $(OBJECTS)
	$(CC) -shared -o $(BUILD)/$@ $(OBJECTS) $(LDLIBS)
endif
bin:
	@mkdir -p $(BUILD)/bin

lib:
	@mkdir -p $(BUILD)/lib

#### --Section-- Unit Tests ####
.PHONY: test-build test test-v
TEST_SKIPFILES = $(patsubst %,$(TESTDIR)/%,$(TEST_SKIPS))
TEST_RUNS = $(filter-out $(TEST_SKIPFILES),$(TESTS))
test:  test-build
	@echo "Running unit tests:"
	@for t in $(TEST_RUNS); do \
		./$$t -s 2>/dev/null; \
	done
	@echo ""
	
test-v: test-build
	@echo "Running unit tests:"
	@for t in $(TEST_RUNS); do \
		./$$t; \
	done
	@echo ""

test-build: $(TESTS)

$(TESTS): CFLAGS = $(DEV_CFLAGS)
$(TESTS): dev $(TEST_SRC)
$(TESTDIR)/%: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) -I$(TESTDIR) $< $(BUILD_DEV)/$(TARGET_SO) $(TESTLIBS) -o $@

#### --Section-- Automatically generated  headers dependencies ####
%.d: %.c
	@set -e
	@rm -f $@
	@$(CC) -MM $(CFLAGS) $< | sed 's,^.*\.o[ :]*,$*.o $@ : ,g' > $@

$(TESTDIR)/%.d: $(TESTDIR)/%.c
	@set -e
	@rm -f $@
	@$(CC) -MM $(CFLAGS) -I$(TESTDIR) $< | sed 's,^.*\.o[ :]*,$(basename $<) $@ : ,g' > $@
-include $(SOURCES:.c=.d) $(TEST_SRC:.c=.d)


#### --Section-- Export include files ####
.PHONY: incl
# header file names starting with an underscore like "_*.h" are not exported 
SRC_PRIVATE_H = $(shell find $(SRCDIR)/ -name "_*.h" -print)
SRC_HFILES = $(filter-out $(SRC_PRIVATE_H),$(shell find $(SRCDIR)/ -name "*.h" -print))
INC_HFILES = $(foreach h,$(SRC_HFILES),$(patsubst $(SRCDIR)/%,%,$(h)))
incl: $(INCFILES)
	@echo Exporting include headers...
	@mkdir -p $(INCDIR)
	@for f in $(INC_HFILES); do \
        mkdir -p $(INCDIR)/`dirname $$f` && cp $(SRCDIR)/$$f $(INCDIR)/`dirname $$f`; \
	done
	@echo Headers exported at directory : $(INCDIR)



#### --Section-- Phony target ####
.PHONY: clean 

clean:
	rm -rf build $(INCDIR) $(OBJECTS) $(SOURCES:.c=.d) $(TEST_SRC:.c=.d) $(TESTS)
