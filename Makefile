BUILDMODE=debug
ARCH=$(shell uname -i)
VERSION=1.0
SRCDIR=$(shell pwd)
INSTALL_DIR=.install

CXX=g++
CPPFLAGS=-std=c++17 -Wall -I$(SRCDIR) -DVERSION=\"$(VERSION)\" -DBUILDMODE=\"$(BUILDMODE)\"

include thirdparty.mk
LDFLAGS += -lpthread

ifeq ($(BUILDMODE),debug)
  CPPFLAGS+=-g -Werror
  LDFLAGS+=-g
endif

ifeq ($(BUILDMODE),opt)
  CPPFLAGS+=-O2 -g
  LDFLAGS+=-O2 -g
endif

include sources.mk
OBJECTS=$(SOURCES:.cpp=.o)
DEPENDS=$(SOURCES:.cpp=.d)
TARGET=ouch_simulator

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

dep:	$(DEPENDS)

clean:
	$(RM) $(OBJECTS) $(TARGET) $(DEPENDS)

%.d:	%.cpp
	$(CXX) -M $(CPPFLAGS) $< -o $@

install: install_skel install_ouch_simulator

install_skel:
	mkdir -p $(INSTALL_DIR)
	for d in include bin lib; do mkdir -p $(INSTALL_DIR)/$${d}; done
	mkdir -p $(INSTALL_DIR)/bin

install_ouch_simulator:
	install -D --mode 755 $(TARGET) $(INSTALL_DIR)/bin/$(TARGET)
	for i in $(INCLUDES); do install --mode 644 $$i $(INSTALL_DIR)/include/; done
	for b in $(BINARIES); do install --mode 755 $$b $(INSTALL_DIR)/bin/; done
	for l in $(LIBRARIES); do install --mode 644 $$l $(INSTALL_DIR)/lib/; done

-include $(DEPENDS)
