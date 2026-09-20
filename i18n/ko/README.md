# 공통 한국어판 Docker 빌드

한국어 조판은 공개 저장소 [garrettjavalia/bgbspd-ko](https://github.com/garrettjavalia/bgbspd-ko)의 고정 커밋을 사용합니다. `toolchain.lock`은 공통 도구와 upstream bgbspd를 함께 고정합니다. Docker 이미지가 두 저장소를 직접 가져오므로 호스트에는 별도 조판 저장소·Python·Pandoc·TeX·폰트 설치가 필요하지 않습니다. 아래 make 명령은 호스트의 make와 Docker를 사용합니다.

## 동일한 실행 방법

각 책 저장소 루트에서:

```sh
make -C i18n/ko docker
make -C i18n/ko html
make -C i18n/ko pdf
```

`all`, `build`, `stage`도 같은 Docker 빌드를 실행합니다. 기본값은 일반/와이드 단일·분할 HTML과 A4/US Letter × 컬러/흑백 × 단면/양면 PDF 8종입니다.

```sh
make -C i18n/ko docker PDF_TARGETS=a4_c_1,a4_bw_2
make -C i18n/ko docker OUTPUT=/absolute/path/to/output JOBS=2
```

이미지 이름은 두 책 모두 `bgbspd-ko-builder`입니다. 같은 lock과 Dockerfile을 쓰므로 같은 이미지를 재사용합니다. 매번 docker build가 lock 변경을 확인하며, 변경이 없으면 캐시를 사용합니다. `IMAGE=다른이름` 또는 `DOCKER=/path/to/docker`로 덮어쓸 수 있습니다.

make 없이 Docker만 사용할 수도 있습니다.

```sh
docker build -f i18n/ko/Dockerfile -t bgbspd-ko-builder .
mkdir -p i18n/ko/dist
docker run --rm --network none \
  -v "$PWD:/guide:ro" -v "$PWD/i18n/ko/dist:/output" \
  bgbspd-ko-builder
```

이미지 생성 시에는 GitHub 및 패키지 다운로드가 필요합니다. 책 빌드 시에는 네트워크를 차단하고 책을 읽기 전용으로 연결합니다. 출력만 `/output`에 씁니다. Docker 빌드 컨텍스트는 전용 Dockerfile.dockerignore로 lock과 실행 어댑터만 전달합니다. 이미지에 책의 원문/번역, 로컬 폰트, Git 자격증명이 복사되지 않습니다.

## 출력과 검사

기본 출력은 `i18n/ko/dist/`입니다.

- `html/index.html`, `html/index-wide.html`: 기존 배포 진입점. 책 이름의 HTML도 함께 있습니다.
- `html/split/`, `html/split-wide/`: 장별 HTML, 이미지, 로컬 폰트.
- `html/<책이름>.zip`, `html/<책이름>-wide.zip`: 분할본 ZIP 별칭. 공통 도구의 `*-split.zip`, `*-split-wide.zip`도 유지합니다.
- `pdf/`: 요청한 PDF들.
- `source/`, `html/source/`: 예제 소스가 있는 책만 생성합니다. 소스 ZIP도 제공합니다.
- `build.log`, `evidence/`, `build-info.json`: 빌드 진단 정보.

HTML 빌드 후 로컬 링크·앵커·자산·종료 태그·언어 메타데이터를 자동 검사하며 오류가 있으면 실패합니다. 웹/PDF 폰트는 공통 저장소에서 가져온 로컬 파일을 사용합니다. 컬러/흑백 코드 줄바꿈도 동일한 공통 규칙을 사용합니다. 흑백은 코드 강조 옵션이며 그림 자체를 회색조로 만들지는 않습니다.

`make -C i18n/ko clean` 또는 `pristine`은 이 도구가 해당 책용으로 생성한 출력만 지웁니다. 소유 표시가 없는 기존 결과물 폴더는 자동으로 지우거나 덮어쓰지 않습니다. 예전 파이프라인 결과가 남아 있으면 별도 보관한 뒤 비어 있는 출력 폴더를 사용하세요.

## 설정과 버전 변경

책별 차이는 `build.json`의 profile, lang, 선택적인 extra_head뿐입니다. 분석 스크립트 등은 책별로 명시하며 다른 책에 자동 상속되지 않습니다. 실제 한국어 번역이 준비되면 lang을 ko로 설정합니다. 일회성 언어 변경은 docker run 명령 뒤에 `--lang ko`를 붙일 수 있습니다.

공통 도구를 갱신할 때 두 책의 `toolchain.lock`에 검증한 커밋을 함께 반영하고 이미지를 다시 빌드합니다. lock과 이미지가 다르면 실행을 거절합니다. Python 기반 이미지와 apt 패키지는 완전히 고정하지 않았으므로 동일 lock만으로 바이트 단위 재현성을 보장하지 않습니다.

원문 루트 `src/`와 원문용 빌드 파일은 변경하지 않습니다. 번역 원문은 `i18n/ko/src/`에만 둡니다. `i18n/ko/src/Makefile` 등 기존 직접 빌드 파일은 메타데이터/과거 경로이며, 표준 빌드 진입점은 이 문서의 `make -C i18n/ko ...`입니다.

## bgnet 한국어판

본문은 기존 한국어 번역을 사용합니다. 한국어 예제 코드는 i18n/ko/source에 있습니다. 과거에 포함한 fonts 및 조판 래퍼는 이번 Docker 파이프라인에서 사용하지 않고, 공통 저장소의 고정 버전을 사용합니다. 기존 ko-head-src.html은 build.json에서 명시적으로 연결해 유지합니다. 원문 추적 manifest가 없는 기존 bgnet에 status를 실행해도 번역을 초기화하거나 덮어쓰지 않습니다.
