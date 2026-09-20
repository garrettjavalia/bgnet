# 한국어판 빌드

의존성은 `책 → bgbspd-ko → bgbspd` 순서다. 이 책은 공개 bgbspd-ko의 지정 커밋만 사용하며 원본 조판 엔진이나 시스템 도구의 설치 절차를 관리하지 않는다.

## 실행

저장소 루트에서:

```sh
make -C i18n/ko docker
make -C i18n/ko html
make -C i18n/ko pdf
make -C i18n/ko status
```

Git, make, Docker가 필요하다. 최초 실행 시 toolchain.lock의 bgbspd-ko 커밋을 `.toolchain/<SHA>/`에 내려받고 **그 저장소의 Dockerfile**을 직접 빌드한다. 원본 bgbspd와 Pandoc/TeX 설치는 공통 Dockerfile이 담당한다. 호스트에 별도 Python·폰트·조판 저장소를 설치할 필요는 없다.

캐시가 있으면 커밋과 작업 트리를 확인한 뒤 재사용한다. 공통 도구를 바꾸려면 toolchain.lock의 bgbspd_ko 커밋만 갱신한다. 엔진의 버전은 그 공통 커밋 내부에서 정한다. 책에는 Dockerfile·실행 어댑터·조판 로직을 복제하지 않는다.

```sh
make -C i18n/ko docker PDF_TARGETS=a4_c_1,a4_bw_2
make -C i18n/ko docker OUTPUT=/absolute/output JOBS=2
```

기본 출력은 i18n/ko/dist의 HTML과 PDF 8종이다. index.html/index-wide.html, 일반·와이드 분할본, ZIP, 로컬 폰트·그림, 예제 소스가 있는 책의 source와 소스 ZIP을 포함한다. all/build/stage도 같은 Docker 경로를 사용한다. clean/pristine은 해당 책이 생성한 출력만 정리하며 번역문이나 다운로드 캐시는 지우지 않는다.

이미지 생성에는 네트워크가 필요하지만 책 빌드 실행은 네트워크 없이 수행한다. 책을 읽기 전용으로 연결하고 출력 폴더에만 쓴다. HTML 검사와 도구 버전 확인은 공통 실행기가 자동 수행한다. tests 역시 `make -C i18n/ko test`로 공통 저장소의 테스트를 실행한다.

책별 설정은 build.json의 profile, lang, 선택적 extra_head다. lang은 실제 번역 상태에 맞춰 지정한다. 원문 루트 src와 번역 i18n/ko/src는 독립적이다.

## bgnet

기존 한국어 본문과 예제 코드를 유지한다. build.json의 extra_head로 기존 ko-head-src.html을 명시한다. 과거 폰트·래퍼 파일은 이 표준 빌드에서 사용하지 않는다. 기준 manifest가 없는 기존 번역에 status를 실행해도 초기화하거나 덮어쓰지 않는다.
