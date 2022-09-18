CC ?= clang
CFLAGS ?= -Wall
LDFLAGS ?= `pkg-config --libs xcb` `pkg-config --libs xcb-keysyms`

DBGFLAGS = -g -O0
CFLAGS += $(DBGFLAGS)

srcdir = src
builddir = build

# TODO: Handle cross-platform building
platform = unix
app_name = asteroids
app_build = $(builddir)/$(app_name)

.PHONY: all clean compdb

all: $(app_build) compdb tags

$(app_build): $(srcdir)/$(platform)_$(app_name).c $(wildcard $(srcdir)/*.[ch]) | $(builddir)
	@echo "== Building asteroids"
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -rf $(builddir)

$(builddir):
	@mkdir -p $(builddir)

# REF: https://pypi.org/project/compiledb/
compdb: compile_commands.json

compile_commands.json: Makefile
	make -Bnwk | compiledb

tags: $(wildcard $(srcdir)/*.[ch]) $(obj)
	ctags -R
