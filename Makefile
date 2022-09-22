CC ?= clang
CFLAGS ?= -Wall
LDFLAGS ?= `pkg-config --libs xcb` `pkg-config --libs xcb-keysyms` -lm

DBGFLAGS = -g -O0
CFLAGS += $(DBGFLAGS)

srcdir = src
builddir = build

# TODO: Handle cross-platform building
platform = unix
app_name = asteroids
app_platform = $(builddir)/$(app_name)
app_core = $(builddir)/$(app_name).so
libflags = -fPIC -shared

.PHONY: all clean compdb

all: $(app_platform) compdb tags

$(app_core): $(srcdir)/$(app_name).c $(wildcard $(srcdir)/*.[ch]) | $(builddir)
	@echo "== Building asteroids core"
	$(CC) $(CFLAGS) $(libflags) $(LDFLAGS) $< -o $@
	@echo ""

$(app_platform): $(srcdir)/$(platform)_$(app_name).c $(app_core) $(wildcard $(srcdir)/*.[ch]) | $(builddir)
	@echo "== Building asteroids for $(platform)"
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@
	@echo ""

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
