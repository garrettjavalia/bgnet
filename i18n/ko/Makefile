PACKAGE=bgnet
SRCDIR=src
DISTDIR=dist
SOURCEDIR=source
BUILDTMP=build_tmp

.PHONY: all build stage clean pristine

all: stage

build:
	$(MAKE) -C $(SRCDIR) pristine all
	$(MAKE) -C $(SOURCEDIR) clean

stage:
	$(MAKE) build
	rm -rf $(DISTDIR)
	mkdir -p $(DISTDIR)/pdf
	mkdir -p $(DISTDIR)/html
	mkdir -p $(DISTDIR)/source
	cp -v $(SRCDIR)/$(PACKAGE)*.pdf $(DISTDIR)/pdf
	cp -v $(SRCDIR)/$(PACKAGE).html $(DISTDIR)/html/index.html
	cp -v $(SRCDIR)/$(PACKAGE)-wide.html $(DISTDIR)/html/index-wide.html
	mkdir -p $(DISTDIR)/html/fonts/d2coding/ttf
	cp -v fonts/d2coding/ttf/* $(DISTDIR)/html/fonts/d2coding/ttf
	cp -v $(SRCDIR)/*.svg $(DISTDIR)/html
	mkdir -p $(DISTDIR)/html/$(PACKAGE)
	cp -v $(SRCDIR)/split/* $(DISTDIR)/html/$(PACKAGE)
	cp -rv $(DISTDIR)/html/fonts $(DISTDIR)/html/$(PACKAGE)
	( cd $(DISTDIR)/html; zip -r $(PACKAGE).zip $(PACKAGE); mv $(PACKAGE) split )
	mkdir -p $(DISTDIR)/html/$(PACKAGE)
	cp -v $(SRCDIR)/split-wide/* $(DISTDIR)/html/$(PACKAGE)
	cp -rv $(DISTDIR)/html/fonts $(DISTDIR)/html/$(PACKAGE)
	( cd $(DISTDIR)/html; zip -r $(PACKAGE)-wide.zip $(PACKAGE); mv $(PACKAGE) split-wide )
	cp -v $(SRCDIR)/*.svg $(DISTDIR)/html/split
	cp -v $(SRCDIR)/*.svg $(DISTDIR)/html/split-wide
	cp -rv $(DISTDIR)/html/fonts $(DISTDIR)/html/split
	cp -rv $(DISTDIR)/html/fonts $(DISTDIR)/html/split-wide
	cp -rv $(SOURCEDIR)/* $(DISTDIR)/source
	cp -rv $(SOURCEDIR)/.htaccess $(DISTDIR)/source 2>/dev/null || :
	mkdir -p $(BUILDTMP)/$(PACKAGE)_source
	cp -rv $(SOURCEDIR)/* $(BUILDTMP)/$(PACKAGE)_source
	cp -rv $(SOURCEDIR)/.htaccess $(BUILDTMP)/$(PACKAGE)_source 2>/dev/null || :
	( cd $(BUILDTMP); zip -r $(PACKAGE)_source.zip $(PACKAGE)_source )
	cp -v $(BUILDTMP)/$(PACKAGE)_source.zip $(DISTDIR)/source
	cp -rv $(DISTDIR)/source $(DISTDIR)/html/source
	rm -rf $(BUILDTMP)
	$(MAKE) -C $(SRCDIR) pristine

clean:
	$(MAKE) -C $(SRCDIR) clean
	$(MAKE) -C $(SOURCEDIR) clean
	rm -rf $(DISTDIR)
	rm -rf $(BUILDTMP)

pristine: clean
	$(MAKE) -C $(SRCDIR) pristine
	$(MAKE) -C $(SOURCEDIR) pristine
