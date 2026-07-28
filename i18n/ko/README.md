# 개요

이 폴더에는 한국어 번역과 관련된 파일이 들어있습니다.

# 구조

## fonts

Pretendard와 Yeomil Mono 폰트 관련 파일이 들어있습니다. Pretendard는 본문 폰트로, Yeomil Mono는 코드/고정폭 폰트로 사용합니다. Pretendard는 HTML에서 참조하는 공식 CDN 버전과 맞추기 위해 v1.3.9 릴리스 파일을 이 리포지토리에 고정해 두었습니다. Yeomil Mono는 아직 Pretendard처럼 널리 쓰이는 공식 CDN 배포 구조가 있다고 보기 어려워, 릴리스 파일을 이 리포지토리에 고정해 두고 HTML/PDF 빌드가 로컬 파일만 사용하도록 구성했습니다.

## src

루트 아래의 src와 마찬가지이며 메이크파일의 설정변수들이 약간 변경되어 있습니다.

## source

한국어판에서 배포할 예제 소스 코드가 들어있습니다. `stage` 빌드에서는 이 폴더의 파일들이 `dist/source`와 `dist/html/source`로 복사됩니다.

# 빌드

Docker만으로 빌드하려면 루트 경로에서 아래와 같이 실행합니다.

```
docker build -f i18n/ko/Dockerfile -t bgnet-ko-builder .
docker run --rm -v "$PWD":/guide -ti bgnet-ko-builder
```

Docker 이미지 안에 Pandoc, XeLaTeX, 고정된 `bgbspd` 빌드 도구 버전이 포함되므로 호스트에는 별도 빌드 의존성을 설치하지 않아도 됩니다. Pretendard와 Yeomil Mono는 이 리포지토리에 포함된 폰트 파일을 직접 참조하므로 Docker 이미지나 호스트 시스템에 폰트를 설치하지 않습니다.

영어 원본과 동일하게 `BGBSPD_BUILD_DIR` 설정과 `bgbspd/source.make` 빌드 규칙을 사용합니다. 로컬에서 빌드하려면 루트 리포지토리와 같은 부모 폴더에 `bgbspd`를 클론하거나, `BGBSPD_BUILD_DIR` 환경 변수로 경로를 지정하세요. 로컬 빌드에는 Pandoc, XeLaTeX, Make가 필요하지만 폰트 설치는 필요하지 않습니다. 빌드 결과물은 `i18n/ko/dist` 폴더에 생성됩니다.

기본 빌드 명령인 `all`은 `stage`를 실행합니다. 기존 루트 Makefile의 `stage` 구조처럼 `html`, `pdf`, `source`를 나누어 `i18n/ko/dist` 아래에 생성합니다. `source`에는 한국어판 전용 예제 소스 코드와 `bgnet_source.zip`이 들어갑니다.

```
make -C i18n/ko stage
```

로컬에서 자주 쓰는 명령 예시는 아래와 같습니다.

```
make -C i18n/ko all
make -C i18n/ko stage
make -C i18n/ko clean
make -C i18n/ko pristine
make -C i18n/ko/src bgnet.html
make -C i18n/ko/src bgnet-wide.html
make -C i18n/ko/src bgnet_a4_c_1.pdf
```

다른 `bgbspd` 커밋으로 Docker 이미지를 만들 때는 아래와 같이 명시할 수 있습니다.

```
docker build --build-arg BGBSPD_REF=<commit> -f i18n/ko/Dockerfile -t bgnet-ko-builder .
```
