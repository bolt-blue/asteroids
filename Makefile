app_name = asteroids
platform = unix
internal_flag = -DINTERNAL_BUILD
# Can be debug or release
build ?= debug

CC ?= clang

debug_FLAGS = -g -ggdb -O0 $(internal_flag)
release_FLAGS = -O2 -DNDEBUG
BUILD_FLAGS = $($(build)_FLAGS)

CFLAGS ?= -Wall -Werror
CFLAGS += $(BUILD_FLAGS)

unix_LDFLAGS = `pkg-config --libs xcb` `pkg-config --libs xcb-keysyms`
platform_LDFLAGS = $($(platform)_LDFLAGS)
core_LDFLAGS = -lm -ldl

srcdir = src
builddir = build

# TODO: Handle cross-platform building
app_platform = $(builddir)/$(app_name)
app_core = $(builddir)/build_$(app_name).so
lib_CFLAGS = -fPIC -shared

.PHONY: all clean compdb

all: $(app_platform) compdb tags

$(app_core): $(srcdir)/$(app_name).c $(wildcard $(srcdir)/*.[ch]) | $(builddir)
	@echo "== Building asteroids core"
	$(CC) $(CFLAGS) $(lib_CFLAGS) $(core_LDFLAGS) $(LDFLAGS) $< -o $@
	mv $@ $(subst build_,,$@)
	@echo ""

$(app_platform): $(srcdir)/$(platform)_$(app_name).c $(app_core) $(wildcard $(srcdir)/*.[ch]) | $(builddir)
	@echo "== Building asteroids for $(platform)"
	$(CC) $(CFLAGS) $(platform_LDFLAGS) $(LDFLAGS) $< -o $@
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
