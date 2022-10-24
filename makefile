
CFLAGS = -Wall -g
LIBS =  -lsdl2 -lopengl32 -lglew32 -mwindows
APP = te.exe

SRCS = test.c editor.c la.c
OBJS = $(subst .c,.o,$(SRCS))

default_target : $(APP)

all:

-include $(DEPS)

$(APP): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o *.d $(APP)

%.o: %.c
	$(CC) $(CFLAGS) -c $<  -o $@

