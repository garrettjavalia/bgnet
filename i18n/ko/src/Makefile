TITLE="Beej의 네트워크 프로그래밍 안내서"
SUBTITLE="인터넷 소켓 사용법"
AUTHOR='Brian “Beej Jorgensen” Hall'
VERSION_DATE="v3.1.11, Copyright © April 8, 2023"

GUIDE_ID=bgnet
BGBSPD_BUILD_DIR?=../../../../bgbspd

WEB_IMAGES=$(wildcard *.svg)
EPUB_SVGS=$(wildcard *.svg)

include $(BGBSPD_BUILD_DIR)/source.make

PREPROC=./preproc_ko.py

bg-css.html: ko-css-src.html
bg-css-wide.html: ko-css-src.html

PDF_OPTS= \
	-H $(BGBSPD_BUILD_DIR)/latex/header_general.latex \
	-A $(BGBSPD_BUILD_DIR)/latex/after_index.latex \
	--pdf-engine=xelatex \
	--variable geometry:"top=1in,bottom=1in" \
	-V documentclass=book \
	--lua-filter $(BGBSPD_BUILD_DIR)/lua/latex_filters.lua \
	-H latex/ko-fonts.latex \
	$(COMMON_OPTS)
