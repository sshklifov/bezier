include Makefile.conf

objectFiles := $(shell find -regex '.*\.o')

$(EXECUTABLE): $(objectFiles)
	g++ $(CFLAGS) $(OPT) $(objectFiles) $(LDFLAGS) -o $@
