CC := gcc
LD := ld

THISMACHINE ?= $(shell uname -srm | sed -e 's/ /-/g')
THISSYSTEM	?= $(shell uname -s)

VERSION     ?= 1.0.0
PACKAGEDIR  ?= ./../_hbpkg/$(THISMACHINE)/cJSON.$(VERSION)


ifeq ($(THISSYSTEM),Darwin)
# Mac can't do conditional selection of static and dynamic libs at link time.
#	PRODUCTS := libcJSON.dylib libcJSON.a
	PRODUCTS := libcJSON.a
else ifeq ($(THISSYSTEM),Linux)
	PRODUCTS := libcJSON.so libcJSON.a
else ifeq ($(THISSYSTEM),CYGWIN_NT-10.0)
	PRODUCTS := libcJSON.a
else
	error "THISSYSTEM set to unknown value: $(THISSYSTEM)"
endif

SRCDIR      := .
INCDIR      := .
BUILDDIR    := build/$(THISMACHINE)
TARGETDIR   := bin/$(THISMACHINE)
RESDIR      := 
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

CFLAGS      ?= -std=gnu99 -fPIC -O3
LIB         := 
INC         := -I$(INCDIR)
INCDEP      := -I$(INCDIR)

SOURCES     := $(shell ls $(SRCDIR)/*.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))



all: lib
lib: resources $(PRODUCTS)
remake: cleaner all
pkg: lib install

install:
	@mkdir -p $(PACKAGEDIR)
	@cp -R $(TARGETDIR)/* $(PACKAGEDIR)/
	@cp -R ./*.h $(PACKAGEDIR)/
	@rm -f $(PACKAGEDIR)/../cJSON
	@ln -s cJSON.$(VERSION) ./$(PACKAGEDIR)/../cJSON
	cd ../_hbsys && $(MAKE) sys_install INS_MACHINE=$(THISMACHINE) INS_PKGNAME=cJSON


#Copy Resources from Resources Directory to Target Directory
resources: directories

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

#Clean only Objects
clean:
	@$(RM) -rf $(BUILDDIR)

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(PRODUCTS)
	@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Build the dynamic library
libcJSON.so: $(OBJECTS)
	$(CC) -shared -fPIC -Wl,-soname,libcJSON.so.1 -o $(TARGETDIR)/$@.$(VERSION) $(OBJECTS) -lc

libcJSON.dylib: $(OBJECTS)
	$(CC) -dynamiclib -o $(TARGETDIR)/$@ $(OBJECTS)
	
libcJSON.a: $(OBJECTS)
	ar -rcs $(TARGETDIR)/$@ $(OBJECTS)
	ranlib $(TARGETDIR)/$@

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

#Non-File Targets
.PHONY: all lib pkg remake clean cleaner resources


