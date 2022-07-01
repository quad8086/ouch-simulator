# quill
QUILL=/elf/apps/quill/2.0.2
CPPFLAGS += -isystem$(QUILL)/include
LDFLAGS += -L$(QUILL)/lib -lquill -Wl,-rpath=$(QUILL)/lib

# boost
BOOST=/elf/apps/boost/1.78.0-gcc1030
CPPFLAGS += -isystem$(BOOST)/include

# args
ARGS=/elf/apps/args/6.3.0
CPPFLAGS+=-I$(ARGS)/include

# zlib
LDFLAGS += -lz -lbz2
