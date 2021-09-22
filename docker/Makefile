TARGET = server

SRCS  = $(shell find ./src     -type f -name *.cpp)
HEADS = $(shell find ./src -type f -name *.hpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = Makefile.depend

INCLUDES = -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/libmongoc-1.0 \
  -I/usr/local/include/bsoncxx/v_noabi -I/usr/local/include/libbson-1.0 \
  -I/opt/local/include -I/usr/include/openssl/ -I/usr/local/include \

CXX = g++

LDLIBS= -lmongocxx -lbsoncxx -pthread -lssl -lcrypto -lm -lcpr
LDFLAGS= -L/opt/local/lib -L/usr/local/lib
CXXFLAGS = -Wall $(INCLUDES) -g -std=c++17 -Wl,-rpath,/usr/local/lib

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADS)
	$(CXX) -v $(LDFLAGS) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS)

run: all
	@./$(TARGET)

.PHONY: depend clean
depend:
	$(CXX) $(CXXFLAGS) -MM $(SRCS) > $(DEPS)
	@sed -i -E "s/^(.+?).o: ([^ ]+?)\1/\2\1.o: \2\1/g" $(DEPS)

clean:
	$(RM) $(OBJS) $(TARGET)

clean-deps:
	$(RM) $(OBJS)

-include $(DEPS)
