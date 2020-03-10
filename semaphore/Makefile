CXX= g++
CXXFLAGS= -std=c++11

INCLUDE= -I./include
LIB= -lpthread

OBJDIR= obj
SRCDIR= src
BINDIR= bin

OBJS= $(addprefix $(OBJDIR)/, semaphore.o philosopher.o)
EXEC= $(addprefix $(BINDIR)/, philosopher)

mkdirs:= $(shell mkdir -p $(OBJDIR) $(BINDIR))

# BUILD EVERYTHING
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDE)


# REMOVE OLD FILES
clean:
	rm -f $(OBJS) $(EXEC)
