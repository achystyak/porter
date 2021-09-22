TARGET = server

SRCS  = $(shell find ./src     -type f -name *.cpp)
HEADS = $(shell find ./src -type f -name *.hpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = Makefile.depend

# INCLUDES = -I./include
CXXFLAGS = -O2 -Wall $(INCLUDES) -g -std=c++17
LDFLAGS = -lm
CXX = g++

INCLUDES = -Wno-everything -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/libmongoc-1.0 \
  -I/usr/local/include/bsoncxx/v_noabi -I/usr/local/include/libbson-1.0 \
  -I/opt/local/include \
  -L/opt/local/lib -lcrypto \
  -L/usr/local/lib -lmongocxx -lbsoncxx -lcpr \

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADS)
	$(CXX) $(LDFLAGS) $(LIBS) $(INCLUDES) -o $@ $(OBJS)

run: all
	@./$(TARGET)

.PHONY: depend clean
depend:
	$(CXX) $(INCLUDES) -MM $(SRCS) > $(DEPS)
	@sed -i -E "s/^(.+?).o: ([^ ]+?)\1/\2\1.o: \2\1/g" $(DEPS)

clean:
	$(RM) $(OBJS) $(TARGET)

clean-deps:
	$(RM) $(OBJS)

-include $(DEPS)
