INCDIR =-I../../application_loader/include -I../../peripheral_lib/eDMA_driver/include/
LIBDIR =../../application_loader/lib ../../peripheral_lib/eDMA_driver/lib/libedma.a ../../peripheral_lib/eDMA_driver/lib/libedmad.a
CC=arm-none-linux-gnueabi-gcc

CFLAGS+=$(INCDIR) -D__DEBUG -O2 -mtune=arm926ej-s -march=armv5te
LDFLAGS+=-L$(LIBDIR) -lprussdrv -lpthread
OBJDIR=obj
BINDIR = ../../application_loader/bin
TARGET=$(BINDIR)/PRU_edmaConfig
_DEPS = 
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = PRU_edmaConfig.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(TARGET)
