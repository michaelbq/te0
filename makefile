THIRDLIB = sdl2 opengl glfw3
LDFLAG = $(shell pkg-config --libs $(THIRDLIB)) -lm
APP = te0
CFLAGS = -I. -ggdb
CXXFLAGS = -I. -ggdb
CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)
OBJS = $(CSRCS:.c=.o)
OBJS += $(CPPSRCS:.cpp=.o)
DEPS = $(OBJS:.o=.d)

ECHO = @echo

default_target: $(APP)

$(APP): $(OBJS)
	$(ECHO) "make $@ ..."
	$(ECHO) "ldflag: $(LDFLAG)"
	$(CXX) -o $@ $^ $(LDFLAG)

%.o:%.c
	$(ECHO) "make pattern c $@ ..."
	$(CC) $(CFLAGS) -o $@ -c $< -MMD -MF $*.d -MP

%.o:%.cpp
	$(ECHO) "make pattern cpp $@ ..."
	$(CXX) $(CXXFLAGS) -o $@ -c $< -MMD -MF $*.d -MP

-include $(DEPS)

clean:
	$(RM) $(OBJS) $(DEPS) $(APP)

