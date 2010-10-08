# openFrameworks universal makefile
#
# make help : shows this message
# make Debug:  makes the application with debug symbols
# make Release: makes the app with optimizations
# make: the same as make Release
# make CleanDebug: cleans the Debug target
# make CleanRelease: cleans the Release target
# make clean: cleans everything
# 
#
# this should work with any OF app, just copy any example
# change the name of the folder and it should compile
# only .cpp support, don't use .c files
# it will look for files in any folder inside the application
# folder except that in the EXCLUDE_FROM_SOURCE variable
# but you'll need to add the include paths in USER_CFLAGS
#
# to add addons to your application, edit the addons.make file
# in this directory and add the names of the addons you want to
# include
#
# edit the following  vars to customize the makefile


COMPILER_OPTIMIZATION = -march=native -mtune=native -finline-functions -funroll-all-loops  -O3
EXCLUDE_FROM_SOURCE=_
USER_CFLAGS = 
USER_LD_FLAGS = 
USER_LIBS = 


# you shouldn't modify anything below this line


SHELL =  /bin/sh
CXX =  g++

ARCH = $(shell uname -m)
ifeq ($(ARCH),x86_64)
	LIBSPATH=linux64
else
	LIBSPATH=linux
endif

NODEPS = clean depend
SOURCE_DIRS = $(shell find ../../../openFrameworks -maxdepth 1 -mindepth 1 -type d )
SOURCES = $(shell find $(SOURCE_DIRS) -name "*.cpp")
OBJFILES = $(patsubst ../../../%.cpp,%.o,$(SOURCES))
APPNAME = $(shell basename `pwd`)
CORE_INCLUDES = $(shell find ../../../openFrameworks/ -type d)
CORE_INCLUDE_FLAGS = $(addprefix -I,$(CORE_INCLUDES))
INCLUDES = $(shell find ../../../*/include -type d)
INCLUDES_FLAGS = $(addprefix -I,$(INCLUDES))

CFLAGS = -Wall -fexceptions
CFLAGS += $(INCLUDES_FLAGS)
CFLAGS += $(CORE_INCLUDE_FLAGS)
CFLAGS += `pkg-config  gstreamer-0.10 gstreamer-video-0.10 gstreamer-base-0.10 libudev libavcodec libavformat libavutil --cflags`

ifeq ($(findstring Debug,$(MAKECMDGOALS)),Debug)
	TARGET_CFLAGS = -g
	OBJ_OUTPUT = obj/Debug/
	SED_OUTPUT = obj\\/Debug\\/
	TARGET_NAME = Debug
	TARGET = ../../lib/$(LIBSPATH)/libopenFrameworksDebug.a
	CLEANTARGET = CleanDebug
endif

ifeq ($(findstring Release,$(MAKECMDGOALS)),Release)
	TARGET_CFLAGS = $(COMPILER_OPTIMIZATION)
	OBJ_OUTPUT = obj/Release/
	SED_OUTPUT = obj\\/Release\\/
	TARGET_NAME = Release
	TARGET = ../../lib/$(LIBSPATH)/libopenFrameworks.a
	CLEANTARGET = CleanRelease
endif

ifeq ($(findstring all,$(MAKECMDGOALS)),all)
	TARGET_CFLAGS = $(COMPILER_OPTIMIZATION)
	OBJ_OUTPUT = obj/Release/
	SED_OUTPUT = obj\\/Release\\/
	TARGET_NAME = Release
	TARGET = ../../lib/$(LIBSPATH)/libopenFrameworks.a
	CLEANTARGET = CleanRelease
endif


ifeq ($(MAKECMDGOALS),depend-Release)
	OBJ_OUTPUT = obj/Release/
	SED_OUTPUT = obj\\/Release\\/
endif

ifeq ($(MAKECMDGOALS),depend-Debug)
	OBJ_OUTPUT = obj/Debug/
	SED_OUTPUT = obj\\/Release\\/
endif

ifeq ($(MAKECMDGOALS),)
	TARGET_CFLAGS = $(COMPILER_OPTIMIZATION)
	OBJ_OUTPUT = obj/Release/
	SED_OUTPUT = obj\\/Release\\/
	TARGET_NAME = Release
	TARGET = ../../lib/$(LIBSPATH)/libopenFrameworks.a
endif

ifeq ($(MAKECMDGOALS),clean)
	TARGET = ../../lib/$(LIBSPATH)/libopenFrameworks.a ../../lib/$(LIBSPATH)/libopenFrameworksDebug.a
endif

DEPFILES = $(addprefix $(OBJ_OUTPUT),$(patsubst ../../../%.cpp,%.d,$(SOURCES)))
OBJS = $(addprefix $(OBJ_OUTPUT), $(OBJFILES))

.PHONY: Debug Release all after depend

Release: $(TARGET) 

Debug: $(TARGET) 

all: $(TARGET)

depend: 
	make depend-Release 
	make depend-Debug

# include dependencies
-include $(DEPFILES)


# we need separate rules for Debug and Release because of different obj/ paths
depend-Release: $(DEPFILES)
depend-Debug: $(DEPFILES)

# This is the rule for creating the dependency files
$(OBJ_OUTPUT)%.d : ../../../%.cpp
	@echo creating dependency file $@ for $<
	@mkdir -p $(@D)
	@# dependency creation flags
	@# -MM: exclude system headers
	@# -MT: change target name
	@# -MF: write to file
	$(CXX) $(TARGET_CFLAGS) $(CFLAGS) $(USER_CFLAGS) -MM -MT $(patsubst %.d,%.o,$@) -MF $@ $<
	

#This rule does the compilation
#$(OBJS): $(SOURCES) $(DEPFILES)
$(OBJ_OUTPUT)%.o : ../../../%.cpp $(OBJ_OUTPUT)%.d 
	@echo compiling $@ from $< 
	@mkdir -p $(@D)
	@# -MMD: update the dependency file
	$(CXX) $(TARGET_CFLAGS) $(CFLAGS) $(USER_CFLAGS) -MMD -o $@ -c $(patsubst $(OBJ_OUTPUT)%.o,../../../%.cpp,$@)

$(TARGET) : $(OBJS)
	@echo "creating " $(TARGET)
	ar cr $@ $(OBJS)

.PHONY: clean CleanDebug CleanRelease
clean:
	rm -Rf obj
	rm -f -v $(TARGET)
	
	
	
$(CLEANTARGET):
	rm -Rf -v $(OBJ_OUTPUT)
	rm -f -v $(TARGET)
	
    

.PHONY: help
help:
 
	# openFrameworks universal makefile
	#
	# targets:
	# make Debug:  makes the application with debug symbols
	# make Release: makes the app with optimizations
	# make: the same as make Release
	# make CleanDebug: cleans the Debug target
	# make CleanRelease: cleans the Release target
	# make clean: cleans everything
	#
	# this should work with any OF app, just copy any example
	# change the name of the folder and it should compile
	# only .cpp support, don't use .c files
	# it will look for files in any folder inside the application
	# folder except that in the EXCLUDE_FROM_SOURCE variable
	# but you'll need to add the include paths in USER_CFLAGS
	#
	# to add addons to your application, edit the addons.make file
	# in this directory and add the names of the addons you want to
	# include
	#
