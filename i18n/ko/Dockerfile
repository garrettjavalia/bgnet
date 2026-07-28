FROM python:3.12-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y git make pandoc texlive-xetex zip && apt-get clean

ARG BGBSPD_REF=74b3cc20642c0dbe89ebeb4507b7dde5684a27f5
RUN git clone https://github.com/beejjorgensen/bgbspd.git /bgbspd \
    && git -C /bgbspd checkout "$BGBSPD_REF"

ENV BGBSPD_BUILD_DIR=/bgbspd

WORKDIR /guide

CMD ["make", "-e", "SHELL=/bin/bash", "-C", "i18n/ko", "pristine", "all"]
