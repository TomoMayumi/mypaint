# 
CC	= gcc
CFLAGS	= -O
LDFLAGS	= -O2
INCLUDES = -I/usr/local/include -I/usr/X11R6/include
LIBS	= -L/usr/X11R6/lib -lX11 -lm -lpng -lXext

TARGET = paint
OBJS = paintmain.o layernew.o function.o canvas2.o historynew.o png5.o

OLDTARGET	= paintb9
OLDOBJS	= paintb9.o historyb8.o png4.o func7.o colorselect3.o layer3.o

#OLDTARGET	= paintb8
#OLDOBJS	= paintb8.o historyb7.o png4.o func6.o colorselect2.o layer2.o

#OLDTARGET	= paintb7
#OLDOBJS	= paintb7.o historyb6.o png4.o func5.o colorselect2.o layer.o

#OLDTARGET	= paintb6
#OLDOBJS	= paintb6.o historyb5.o png4.o func4.o colorselect2.o

#TARGET	= paintb5
#OBJS	= paintb5.o historyb4.o png3.o func3.o colorselect.o

#TARGET	= paintb4
#OBJS	= paintb4.o historyb4.o png3.o func2.o

#TARGET	= paintb4
#OBJS	= paintb4.o historyb4.o png3.o func.o

#TARGET	= paintb3
#OBJS	= paintb3.o historyb3.o png3.o

# 
all:	$(TARGET)

test:	$(TARGET)
	./$(TARGET) -test

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

old:	$(OLDTARGET)
	./$(OLDTARGET) -test

$(OLDTARGET): $(OLDOBJS)
	$(CC) $(LDFLAGS) -o $@ $(OLDOBJS) $(LIBS)

clean:
	-rm -f $(TARGET) $(OBJS) $(OLDTARGET) $(OLDOBJS) .nfs* *~ \#* core

.c.o: inc.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

c++:
	g++ $(LDFLAGS) -o $(TARGET) $(TARGET).cc $(LIBS)