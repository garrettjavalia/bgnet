FROM python:3.8.2

RUN apt-get update && apt-get install -y git pandoc texlive-xetex fonts-liberation fontconfig && apt-get clean
RUN git clone --depth 1 https://github.com/beejjorgensen/bgbspd.git /bgbspd

ENV BGBSPD_BUILD_DIR=/bgbspd

WORKDIR /guide

CMD make -e SHELL=/bin/bash pristine all stage
