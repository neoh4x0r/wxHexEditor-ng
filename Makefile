WXCONFIG ?= wx-config
HOST=
OPT_FLAGS += -fopenmp
LIBS += -lgomp
WXCXXFLAGS= `$(WXCONFIG) --cxxflags` -Iudis86 -Imhash/include -MMD -Wall -O2 -DNDEBUG
WXLDFLAGS = `$(WXCONFIG) --libs` `$(WXCONFIG) --libs aui` `$(WXCONFIG) --libs core`
#add this ldflags for WinConsole  "-Wl,--subsystem,console -mconsole" for win-debug
#LDFLAGS += -Wl,--subsystem,console -mconsole
RC = `$(WXCONFIG) --rescomp`
#RC = x86_64-w64-mingw32-windres --define WX_CPU_AMD64
RCFLAGS = `$(WXCONFIG) --cxxflags | sed s/' '-m.*//g;`
MSGFMT = msgfmt
SOURCES= src/HexEditorGui.cpp \
			src/FAL.cpp\
			src/HexDialogs.cpp\
			src/HexPanels.cpp\
			src/HexEditorApp.cpp\
			src/HexEditor.cpp\
			src/HexEditorCtrl/HexEditorCtrl.cpp\
			src/HexEditorCtrl/wxHexCtrl/TagDialogGui.cpp\
			src/HexEditorCtrl/wxHexCtrl/wxHexCtrl.cpp\
			src/HexEditorCtrl/wxHexCtrl/Tag.cpp\
			src/HexEditorCtrl/HexEditorCtrlGui.cpp\
			src/HexEditorFrame.cpp
LIBS += udis86/libudis86/.libs/libudis86.a mhash/lib/.libs/libmhash.a
OBJECTS=$(SOURCES:.cpp=.o)
DEPENDS=$(OBJECTS:.o=.d)
LANGUAGEDIRS=`ls -l ./locale | grep ^d | sed s/.*\ //g;`
LANGUAGES=$(wildcard locale/*/wxHexEditor-ng.po)
TEMPLATEDIRS=`ls -l ./templates | grep ^d | sed s/.*\ //g;`
RESOURCES= resources/resource.rc
RESOURCE_OBJ=$(RESOURCES:.rc=.o)
EXECUTABLE=wxHexEditor-ng
EXECUTABLE_WIN=$(EXECUTABLE).exe
EXECUTABLE_DIR_MAC=$(EXECUTABLE).app

PREFIX      = /usr/local
BINDIR      = $(PREFIX)/bin
DATADIR     = $(PREFIX)/share
LOCALEDIR   = $(DATADIR)/locale

VERSION = 0.25 Beta

.DEFAULT_GOAL := all

all:$(EXECUTABLE) langs

#For Clang:
#CC=clang
#CXX=clang++
#OPT_FLAGS=-D_Bool=bool -std=c++11 -lomp

$(OBJECTS): $(LIBS) $(SOURCES)

MOBJECTS=$(LANGUAGES:.po=.mo)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) ${CXXFLAGS} ${CPPFLAGS} $(OPTFLAGS) $(OBJECTS) $(LIBS) $(WXLDFLAGS) ${LDFLAGS} -o $@

.cpp.o: $(LIBS)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(WXCXXFLAGS) $< -o $@

%.o : %.rc
	$(RC) $(RCFLAGS) $< -o $@

langsup:
	tx pull

langs: $(MOBJECTS)

%.mo : %.po
	$(MSGFMT) $< -o $@

udis86/libudis86/.libs/libudis86.a:
	cd udis86; sh ./autogen.sh
	cd udis86;./configure --host=$(HOST) CC="$(CC)" CXX="$(CXX)" CFLAGS="$(CFLAGS) ${OPTFLAGS}" CXXFLAGS="$(CXXFLAGS) ${OPTFLAGS}" CPPFLAGS="$(CPPFLAGS)"
	cd udis86/libudis86; $(MAKE) $(MFLAGS)

mhash/lib/.libs/libmhash.a:
	cd mhash; ./configure --host=$(HOST) CC="$(CC)" CXX="$(CXX)" CFLAGS="$(CFLAGS) ${OPTFLAGS}" CXXFLAGS="$(CXXFLAGS) ${OPTFLAGS}" CPPFLAGS="$(CPPFLAGS)"
	cd mhash; $(MAKE) $(MFLAGS)

src/windrv.o:
	$(CXX) $(LIBS) ${CXXFLAGS} ${OPTFLAGS} $(WXCXXFLAGS) $(WXLDFLAGS) ${LDFLAGS} -c src/windrv.cpp -o src/windrv.o

win_debug: LDFLAGS += -Wl,--subsystem,console -mconsole
win_debug: win

win: $(RESOURCES) $(EXECUTABLE_WIN)

#Stack override required for file comparison function...
$(EXECUTABLE_WIN): $(OBJECTS) $(RESOURCE_OBJ) src/windrv.o
	$(CXX) $(OBJECTS) src/windrv.o $(RESOURCE_OBJ) $(LIBS) ${CXXFLAGS} ${OPTFLAGS} $(WXLDFLAGS) -static ${LDFLAGS} -lpthread -lpsapi -static-libgcc -static-libstdc++ -Wl,--stack,32000000 -o $@

maclink: $(OBJECTS)
	$(CXX) $(OBJECTS) ${LDFLAGS} $(LIBS) ${CXXFLAGS} ${OPTFLAGS} $(WXLDFLAGS) -lexpat -Wl,-stack_size,0x2000000 -o $(EXECUTABLE)

mac: prepmacdir maclink
	install -m 755 wxHexEditor-ng $(EXECUTABLE_DIR_MAC)/Contents/MacOS/

prepmacdir: langs
	mkdir -p $(EXECUTABLE_DIR_MAC)/Contents
	mkdir -p $(EXECUTABLE_DIR_MAC)/Contents/MacOS
	mkdir -p $(EXECUTABLE_DIR_MAC)/Contents/Resources
	install -m 644 resources/wxHexEditor-ng.icns $(EXECUTABLE_DIR_MAC)/Contents/Resources/
	printf "APPLHexE" > $(EXECUTABLE_DIR_MAC)/Contents/PkgInfo
	printf "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
<plist version=\"1.0\">\n\
<dict>\n\
\t<key>CFBundleDevelopmentRegion</key>\n\
\t<string>English</string>\n\
\
\t<key>CFBundleExecutable</key>\n\
\t<string>wxHexEditor-ng</string>\n\
\
\t<key>CFBundleGetInfoString</key>\n\
\t<string>wxHexEditor-ng v$(VERSION)</string>\n\
\
\t<key>CFBundleIconFile</key>\n\
\t<string>wxHexEditor-ng.icns</string>\n\
\
\t<key>CFBundleIdentifier</key>\n\
\t<string>org.wxhexeditor-ng.www</string>\n\
\
\t<key>CFBundleShortVersionString</key>\n\
\t<string>v$(VERSION)</string>\n\
\
\t<key>CFBundleInfoDictionaryVersion</key>\n\
\t<string>6.0</string>\n\
\
\t<key>CFBundleName</key>\n\
\t<string>wxHexEditor-ng</string>\n\
\
\t<key>CFBundlePackageType</key>\n\
\t<string>APPL</string>\n\
\
\t<key>CFBundleSignature</key>\n\
\t<string>HexE</string>\n\
\
\t<key>CFBundleVersion</key>\n\
\t<string>1.0.0</string>\n\
\
\t<key>DRURLs</key>\n\
\t<string>https://github.com/dariox86/wxHexEditor-ng</string>\n\
\
\t<key>NSMainNibFile</key>\n\
\t<string>wxHexEditor-ng</string>\n\
\
\t<key>NSPrincipalClass</key>\n\
\t<string>NSApplication</string>\n\
\
\t<key>NSHumanReadableCopyright</key>\n\
\t<string> (c) 2006-2011, Erdem U. Altinyurt</string>\n\
\
</dict>\n\
</plist>\n\n" > $(EXECUTABLE_DIR_MAC)/Contents/Info.plist
	@for i in $(LANGUAGEDIRS); do \
		echo "mkdir -p $(EXECUTABLE_DIR_MAC)/Contents/Resources/locale/$$i/"; \
		mkdir -p $(EXECUTABLE_DIR_MAC)/Contents/Resources/locale/$$i; done
	@for i in $(LANGUAGEDIRS); do \
		echo "cp locale/$$i/wxHexEditor-ng.mo $(EXECUTABLE_DIR_MAC)/Contents/Resources/locale/$$i/"; \
		cp locale/$$i/wxHexEditor-ng.mo $(EXECUTABLE_DIR_MAC)/Contents/Resources/locale/$$i/; done
	@for i in $(TEMPLATEDIRS); do \
		echo "mkdir -p $(EXECUTABLE_DIR_MAC)/Contents/Resources/templates/$$i/"; \
		mkdir -p $(EXECUTABLE_DIR_MAC)/Contents/Resources/templates/$$i/; \
		echo "cp locale/$$i/* $(EXECUTABLE_DIR_MAC)/Contents/Resources/templates/$$i/"; \
		cp templates/$$i/* $(EXECUTABLE_DIR_MAC)/Contents/Resources/templates/$$i/; done

install:
	# Can't use install -D because FreeBSD doesn't have the semantics.
	mkdir -p $(DESTDIR)/$(BINDIR) $(DESTDIR)/$(DATADIR)/pixmaps $(DESTDIR)/$(DATADIR)/applications
	install -m 755 $(EXECUTABLE) $(DESTDIR)/$(BINDIR)/$(EXECUTABLE)
	install -m 644 resources/wxHexEditor-ng.png $(DESTDIR)/$(DATADIR)/pixmaps/wxHexEditor-ng.png
	install -m 644 resources/wxHexEditor-ng.desktop $(DESTDIR)/$(DATADIR)/applications/wxHexEditor-ng.desktop
	@for i in $(LANGUAGEDIRS); do \
		echo "install -m 644 locale/$$i/wxHexEditor-ng.mo $(DESTDIR)/$(LOCALEDIR)/$$i/LC_MESSAGES/wxHexEditor-ng.mo"; \
		mkdir -p $(DESTDIR)/$(LOCALEDIR)/$$i/LC_MESSAGES/; \
		install -m 644 locale/$$i/wxHexEditor-ng.mo $(DESTDIR)/$(LOCALEDIR)/$$i/LC_MESSAGES/wxHexEditor-ng.mo; done
	@for i in $(TEMPLATEDIRS); do \
		echo "install -m 644 templates/$$i/* $(DESTDIR)/$(LOCALEDIR)/wxHexEditor-ng/$$i/"; \
		mkdir -p $(DESTDIR)/$(DATADIR)/wxHexEditor-ng/templates/$$i/; \
		install -m 644 templates/$$i/* -t $(DESTDIR)/$(DATADIR)/wxHexEditor-ng/templates/$$i; done

uninstall:
	rm -f $(DESTDIR)/$(BINDIR)/$(EXECUTABLE)
	rm -f $(DESTDIR)/$(DATADIR)/pixmaps/wxHexEditor-ng.png
	rm -f $(DESTDIR)/$(DATADIR)/applications/wxHexEditor-ng.desktop
	rm -rf $(DESTDIR)/$(DATADIR)/wxHexEditor-ng/templates
	rm -f $(DESTDIR)/$(LOCALEDIR)/*/LC_MESSAGES/wxHexEditor-ng.mo

clean:
	rm -f $(OBJECTS)
	rm -f $(RESOURCE_OBJ)
	rm -f $(DEPENDS)
	rm -f $(EXECUTABLE)
	rm -f src/windrv.o
	rm -f $(EXECUTABLE_WIN)
	rm -rf $(EXECUTABLE_DIR_MAC)
	rm -f locale/*/wxHexEditor-ng.mo
distclean: clean
	cd udis86;$(MAKE) distclean
	cd mhash;$(MAKE) distclean

# include the auto-generated dependency files
-include $(DEPENDS)
