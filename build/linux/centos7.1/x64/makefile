PROJECT=HlsDownloader

#prebuild depends libs
#index 0 is project name
#index 1 is version
#index 2 is copy include flag
#index 3 is copy lib flag
PREBUILD_LIB_PATH=../../../../../../../../../Versions/Baselib
PREBUILD_LIBS="PlatformBase 1.0.0 true true"

POSTBUILD_LIB_PATH=../../../../../../../../../Versions/Baselib
POSTBUILD_LIBS="PlatformBase 1.0.0"

#manually specify
INCLUDE=-I../../../../include \
-I../../../../include_internal \
-I../../../../deps/include \
-I../../../../deps/include/PlatformBase
LIBS=-Wl,-rpath-link=../../../../deps/lib -L../../../../deps/lib -lPlatformBase -lpthread -lcurl
WARNING_SWITCH=-Wall -Wno-unused-function
DEFINES=

TARGET=../../../../bin/$(PROJECT)
CC:=gcc
CXX:=g++
SRC_DIRS=../../../../src ../../../../src/*
CSRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c))
COBJ:=$(CSRC:.c=.o)
CPPSRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.cpp))
CPPOBJ:=$(CPPSRC:.cpp=.o)

all:prebuild $(TARGET)
prebuild:
	chmod +x *.sh
	./prebuild.sh $(PREBUILD_LIB_PATH) $(PREBUILD_LIBS)

$(TARGET):$(COBJ) $(CPPOBJ)
	$(CXX) -Wall -g -o $(TARGET) $(COBJ) $(CPPOBJ) $(LIBS)
	./postbuild.sh $(POSTBUILD_LIB_PATH) $(POSTBUILD_LIBS)

%.o:%.cpp
	$(CXX) $(DEFINES) $(WARNING_SWITCH) -g -fPIC -o $@ -c $< $(INCLUDE) -std=c++11

%.o:%.c
	$(CC) $(DEFINES) $(WARNING_SWITCH) -g -fPIC $(INCLUDE) -o $@ -c $<

.PHONY:clean

clean:
	-rm -f $(COBJ) $(CPPOBJ) $(TARGET)
