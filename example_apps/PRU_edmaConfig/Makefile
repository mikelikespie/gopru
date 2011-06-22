CCTOOLS?=arm-none-linux-gnueabi-gcc

LIBDIR_APP_LOADER?=../../app_loader/lib
LIBDIR_EDMA_DRIVER?=../../peripheral_lib/edma_driver/lib/
INCDIR_APP_LOADER?=../../app_loader/include
INCDIR_EDMA_DRIVER?=../../peripheral_lib/edma_driver/include
BINDIR?=../bin

CFLAGS+= -Wall -I$(INCDIR_APP_LOADER) -I$(INCDIR_EDMA_DRIVER) -D__DEBUG -O2 -mtune=arm926ej-s -march=armv5te
LDFLAGS+=-L$(LIBDIR_APP_LOADER) -L$(LIBDIR_EDMA_DRIVER) -lprussdrv -lpthread -ledmautils
OBJDIR=obj
TARGET=$(BINDIR)/PRU_edmaConfig

_DEPS = 
DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(_DEPS))
#DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(INCDIR_EDMA_DRIVER)/%,$(_DEPS))


_OBJ = PRU_edmaConfig.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(CCTOOLS) $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	$(CCTOOLS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(TARGET)
