INCDIR =../../application_loader/include
LIBDIR =../../application_loader/lib
CC=arm-none-linux-gnueabi-gcc

CFLAGS+= -Wall -I$(INCDIR) -D__DEBUG -O2 -mtune=arm926ej-s -march=armv5te
LDFLAGS+=-L$(LIBDIR) -lprussdrv -lpthread
OBJDIR=obj
BINDIR = ../../application_loader/bin
TARGET=$(BINDIR)/PRU_mem1DTransfer

_DEPS = 
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = PRU_mem1DTransfer.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(INCDIR)/*~  $(TARGET)
