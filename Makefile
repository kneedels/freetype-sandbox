objects = ftsandbox.o qdbmp.o
CFLAGS += -g `pkg-config --cflags freetype2`
LDFLAGS += -g
LDLIBS += `pkg-config --libs freetype2`

ftsandbox : $(objects)

$(objects): qdbmp.h

.PHONY : clean
clean :
	-rm ftsandbox $(objects)
