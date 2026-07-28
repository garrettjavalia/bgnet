PACKAGE=bgnet
WEB_IMAGES=$(wildcard src/*.svg)

BGBSPD_BUILD_DIR?=../bgbspd

include $(BGBSPD_BUILD_DIR)/main.make

I18N_KO_SRCDIR?=i18n/ko/src

.PHONY: ko ko-clean ko-pristine ko-stage

ko:
	$(MAKE) -C $(I18N_KO_SRCDIR) all

ko-clean:
	$(MAKE) -C $(I18N_KO_SRCDIR) clean

ko-pristine:
	$(MAKE) -C $(I18N_KO_SRCDIR) pristine

ko-stage: ko
	mkdir -p $(STAGEDIR)/translations
	cp -v $(I18N_KO_SRCDIR)/$(PACKAGE).html $(STAGEDIR)/translations/$(PACKAGE)_ko.html
	cp -v $(I18N_KO_SRCDIR)/$(PACKAGE)-wide.html $(STAGEDIR)/translations/$(PACKAGE)_ko_wide.html
	cp -v $(I18N_KO_SRCDIR)/$(PACKAGE)*.pdf $(STAGEDIR)/translations 2>/dev/null || :
