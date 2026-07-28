TITLE="Beej의 네트워크 프로그래밍 안내서"
SUBTITLE="인터넷 소켓 사용법"
AUTHOR='Brian “Beej Jorgensen” Hall'
VERSION_DATE="v3.1.11, Copyright © April 8, 2023"

GUIDE_ID=bgnet
BGBSPD_BUILD_DIR?=../../../../bgbspd

WEB_IMAGES=$(wildcard *.svg)
EPUB_SVGS=$(wildcard *.svg)

all: install-fonts

include $(BGBSPD_BUILD_DIR)/source.make

PDF_MAINFONT="Pretendard"
PDF_SANSFONT="Pretendard"
PDF_MONOFONT="Pretendard"

install-fonts:
	bash ./install_fonts.sh

.PHONY: install-fonts
