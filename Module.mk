PREFIX ?= ../..
include $(PREFIX)/Config.mk
COMPILE.cc = g++ $(CFLAGS) $(OPT) -c

headerFiles:=$(wildcard *.h *.tcc)
sourceFiles:=$(wildcard *.cpp)
files:=$(basename $(sourceFiles))
objectFiles:=$(addprefix $(LIBDIR)/,$(addsuffix .o,$(files)))

.PHONY: objs
objs: $(objectFiles) ;

.PHONY: clean
clean:
	rm -f $(objectFiles)

.SILENT: depend
.PHONY: depend
depend:
	for file in $(files); do                            				\
	    $(COMPILE.cc) -MM -MP -MT $(LIBDIR)/$$file.o $$file.cpp >> Makefile;    	\
	done

$(LIBDIR)/%.o : %.cpp
	$(COMPILE.cc) $< -o $@

# Auto generated dependencies

