# Beej의 네트워크 프로그래밍 안내서 소스 마크다운

## Beej의 마크다운 확장

이 확장들은 `bin/preproc` 스크립트로 처리됩니다.

* `[[pagebreak]]`

  인쇄용 판본에서 페이지 나눔을 만듭니다. LaTeX에서는 `\newpage`로 렌더링됩니다.

* `[nh[word]]`

  이 단어의 하이픈 분리를 막습니다. `\hyphenation{word}`로 변환됩니다.
  LaTeX 제약 때문에 밑줄 문자는 사용할 수 없습니다.

* `[ix[entry]]`

  LaTeX 색인 항목을 만들고 `\index{}` 요소 안에 그대로 넣습니다.
  **이 항목 안에서는 밑줄을 반드시 `\_`로 이스케이프해야 합니다!**
  [LaTeX 색인 예제는 여기](https://en.wikibooks.org/wiki/LaTeX/Indexing#Sophisticated_indexing)에
  있습니다.

* `[ixtt[entry]]`

  단일하고 단순한 색인 항목을 `\index{entry@\texttt{entry}}` 형태의
  타자체 텍스트로 렌더링합니다. 더 복잡한 항목은 `[ix[]]`에 원시 LaTeX를
  넘겨서 렌더링해야 합니다.

* `[fl[linktext|url]]`

  각주 링크입니다. 링크 텍스트에 URL을 연결하고, URL을 각주에 표시합니다.
  `[linktext](url)^[url]`로 변환됩니다.
    
* `[flx[linktext|file]]`

  Beej의 예제 파일로 가는 각주 링크입니다. 파일 이름 앞에
  `https://beej.us/guide/bgnet/examples/`를 자동으로 붙이고 URL을 각주에
  표시합니다.

* `[flr[link|id]]`

   Beej의 리다이렉트로 가는 각주 링크입니다. 링크 ID 앞에
   `https://beej.us/guide/url/`을 자동으로 붙이고 URL을 각주에 표시합니다.

* `[flrfc[link|num]]`

   RFC로 가는 각주 링크입니다. RFC 번호 앞에
   `https://tools.ietf.org/html/rfc`를 자동으로 붙이고 URL을 각주에 표시합니다.
   

## pandoc 마크다운 특이사항

같은 문단 안에 여러 개의 인라인 각주가 있고 그 각주들이 마크다운의 서로 다른
줄에 있다면, 마지막 줄을 제외한 모든 줄은 끝에 공백이 있어야 합니다.

맨페이지 하위 절은 목차에 나타나지 않도록 h4 `####`로 둡니다. Pandoc 2.8에는
h3가 목차에 나타나지 않게 하는 방법이 있을 것입니다.

표 행은 줄바꿈이 제대로 되도록 한 줄에 있어야 합니다. 표 셀 안에서는 줄바꿈이
보존됩니다.

표에서는 헤더의 상대 너비가 최종 출력에 반영됩니다.

표 헤더 템플릿:

```
| 매크로          | 설명                                                   |
|-----------------|--------------------------------------------------------|
```

`<a name>`은 동작하지 않습니다. 헤더의 `{#tags}`를 사용하세요.

표준 언어 이름을 붙인 fenced code는 때때로 LaTeX를 실패하게 만들 수 있습니다.

````
```c                   이렇게 하지 마세요

``` {.c}               이렇게 하세요
``` {.c .numberLines}  또는 이렇게 하세요
````

색인은 LaTeX `\index{foo}` 마커로 만듭니다. 이 마커는 HTML 출력에는 나타나지
않습니다.

LaTeX 색인 예제:

(밑줄은 `\_`로 이스케이프하세요!)

```latex
\index{foo} 일반 요소
\index{foo\_bar} 밑줄이 있는 요소
\index{foo()@\texttt{foo()}} 색인에서 foo()를 고정폭으로 렌더링
\index{O\_NONBLOCK@\texttt{O\_NONBLOCK}} 밑줄이 있는 고정폭 항목
\index{foo!bar} foo 아래의 하위 색인 bar
\index{bind()@\texttt{bind()}!implicit} 고정폭이 들어간 하위 색인
```

[더 많은 LaTeX 색인 예제는 여기](https://en.wikibooks.org/wiki/LaTeX/Indexing#Sophisticated_indexing)에
있습니다.

색인 항목은 헤더, 굵은 글씨, 기울임 글씨 안이 아니라 본문 최상위 수준에 있어야
합니다. 그렇지 않으면 색인에 별도 항목으로 나타납니다.

각 맨페이지 앞에는 페이지 나눔을 강제하기 위해 `\newpage`를 넣습니다.

`bgnet_amazon.md`를 편집할 때는 widow를 다음 페이지로 넘기기 위해 `\newpage`를
사용합니다.

연속된 줄의 링크 뒤에 붙은 각주가 제대로 동작하려면 줄 끝에 공백이 필요할 수
있습니다.

```markdown
[Hey](url)^[footnote],   <-- 이 줄 끝에 공백이 필요합니다
[Again](url2)^[footnote2].
```
