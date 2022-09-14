CC ?= clang
CFLAGS ?= -Wall
LDFLAGS ?= `pkg-config --libs xcb` `pkg-config --libs xcb-keysyms`

DBGFLAGS = -g -O0
CFLAGS += $(DBGFLAGS)

srcdir = src
testdir = $(srcdir)/test
builddir = build
buildtestdir = $(builddir)/test

src = $(srcdir)/po_window.c 
obj = $(patsubst %.c,%.o, $(src))

.PHONY: all test clean

all: test compdb

$(obj):

tests = $(buildtestdir)/window_init
test: $(tests)

$(tests): $(wildcard $(srcdir)/*.[ch]) | $(builddir)
	$(CC) $(CFLAGS) $(LDFLAGS) $(testdir)/$(notdir $@).c -o $@

clean:
	rm -f $(obj)
	rm -rf $(builddir)

$(builddir):
	@mkdir -p $(builddir)/test

# REF: https://pypi.org/project/compiledb/
.PHONY: compdb
compdb: compile_commands.json

compile_commands.json: Makefile
	make -Bnwk | compiledb
