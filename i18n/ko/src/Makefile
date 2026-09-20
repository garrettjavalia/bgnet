TITLE="Beej의 네트워크 프로그래밍 안내서"
SUBTITLE="인터넷 소켓 사용법"
AUTHOR='Brian “Beej Jorgensen” Hall'
VERSION_DATE="v3.3.2, Copyright © April 18, 2026"
GUIDE_ID=bgnet

# Book metadata stays here; all build behavior comes from the pinned shared tool.
.DEFAULT_GOAL := all
.PHONY: all clean pristine
all:
	$(MAKE) -C .. docker
$(GUIDE_ID).html $(GUIDE_ID)-wide.html:
	$(MAKE) -C .. html
$(GUIDE_ID)_%.pdf:
	$(MAKE) -C .. pdf PDF_TARGETS=$*
clean pristine:
	$(MAKE) -C .. $@
