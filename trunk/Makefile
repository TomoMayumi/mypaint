# 
CC	= gcc
CFLAGS	= -O
LDFLAGS	= -O2
INCLUDES = -I/usr/local/include -I/usr/X11R6/include
LIBS	= -L/usr/X11R6/lib -lX11 -lm -lpng -lXext

TARGET = paint
OBJS = paintmain.o layernew.o function.o canvas2.o historynew.o png5.o


# 
all:	$(TARGET)

test:	$(TARGET)
	./$(TARGET) -test

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(TARGET) $(OBJS) $(OLDTARGET) $(OLDOBJS) .nfs* *~ \#* core

.c.o: inc.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

c++:
	g++ $(LDFLAGS) -o $(TARGET) $(TARGET).cc $(LIBS)