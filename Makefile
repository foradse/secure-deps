# Example Makefile for secure_deps makefile-check
LIBS = -lssl -lcrypto -lz
LDFLAGS = -Wl,-lfoo -l:libbar.a

all:
	@echo "Dummy target"
