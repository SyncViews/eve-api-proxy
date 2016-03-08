
LIB_CPP := $(shell find libeve-api-proxy/source/ -name *.cpp)
LIB_CPP := $(filter-out libeve-api-proxy/source/socket/WinTlsSocket.cpp, $(LIB_CPP))
LIB_OBJ := $(addprefix obj/, $(LIB_CPP:.cpp=.o))

TEST_CPP := $(shell find testeve-api-proxy/tests/ -name *.cpp)
TEST_OBJ := $(addprefix obj/, $(TEST_CPP:.cpp=.o))

all: bin/eve-api-proxy test

CC_VARIATION_FLAGS := -O3 -DNDEBUG
debug: CC_VARIATION_FLAGS := -ggdb -Og -D_DEBUG
debug: all

LD_FLAGS := -pthread
CC_FLAGS := $(CC_VARIATION_FLAGS) -std=c++11 -pthread -Ilibeve-api-proxy/source/ -Ithird-party/rapidjson/include -DBOOST_TEST_DYN_LINK

LIBS := -lz -lssl -lcrypto
TEST_LIBS := -lboost_unit_test_framework

all: bin/eve-api-proxy test

DEPS := $(LIB_OBJ:.o=.d) $(TEST_OBJ:.o=.d)
-include $(DEPS)

clean:
	rm -rf bin
	rm -rf obj

obj/%.o: %.cpp
	@mkdir -p $(@D)
	g++ $(CC_FLAGS) -MT $@ -MMD -MP -MF $(patsubst %.o,%.d,$@) -c -o $@ $<

bin/eve-api-proxy: $(LIB_OBJ) obj/eve-api-proxy/source/Main.o
	@mkdir -p $(@D)
	g++ $(LD_FLAGS) -o $@ $^ $(LIBS)

bin/test: $(LIB_OBJ) $(TEST_OBJ)
	@mkdir -p $(@D)
	g++ $(LD_FLAGS) -o $@ $^ $(LIBS) $(TEST_LIBS)

test: bin/test
	./bin/test

