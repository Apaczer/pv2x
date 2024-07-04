include Makefile.setup

LDFLAGS+=-rdynamic -ldl

SDMOUNTPOINT=/media/sd
INSTALLDIR=dev

TARGETGP2X=	pv2x.gpe
TARGETLINUX=	pv2x
TARGETMIYOO=	pv2x-miyoo

OBJ=	filelistrandomizer.o \
	pv2xplugin.o \
	plugins.o \
	sdltools.o \
	scaledimage.o \
	filelist.o \
	sdlttf.o \
	misctools.o \
	projectfunctions.o \
	main.o

%.o:%.cpp
	$(CPP) -o $@ -Wall $(DEBUG) $(CFLAGS) -c $<

all:$(OBJ) allplugins
	$(CPP) -o $(TARGET) $(OBJ) $(LDFLAGS)

allplugins:
	$(MAKE) -C plugins

clean:
	$(MAKE) -C plugins clean
	rm -f $(OBJ)
	rm -f $(TARGETGP2X) $(TARGETLINUX) $(TARGETMIYOO)
	rm -rf dist

clean-dist: clean
	rm -rf dist
	rm -f *.ipk
	rm -f pv2x.cfg

dist: all
	mkdir -p dist/pv2x-$(VERSION)/plugins
	mkdir -p dist/pv2x-$(VERSION)/doc
	cp $(TARGET) dist/pv2x-$(VERSION)
	cp Vera.ttf dist/pv2x-$(VERSION)
	cp pv2x.png dist/pv2x-$(VERSION)
	cp README dist/pv2x-$(VERSION)
	cp COPYING dist/pv2x-$(VERSION)
	cp plugins/*.so dist/pv2x-$(VERSION)/plugins
	cp doc/* dist/pv2x-$(VERSION)/doc

dist-comp: dist
	cd dist && tar -czvf pv2x-$(VERSION).tar.gz pv2x-$(VERSION)
	cd dist && zip -r pv2x-$(VERSION).zip pv2x-$(VERSION)

dist-ipk: dist
	-rm dist/pv2x-$(VERSION)/$(TARGET)
	-cp $(TARGET) pv2x
	cp pv2x dist/pv2x-$(VERSION)/
	gm2xpkg -i pkg.cfg
	mv pv2x.ipk $(TARGET).ipk
	@gm2xpkg -c pkg.cfg >/dev/null 2>&1

install: dist
	mount $(SDMOUNTPOINT)
	cd dist/pv2x-$(VERSION) && cp * $(SDMOUNTPOINT)/$(INSTALLDIR)
	umount $(SDMOUNTPOINT)
