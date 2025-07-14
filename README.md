# OEIS\_01

* [日本語版](README_ja.md)
* [English Version](README.md)

## Overview

I am **manman4** ([GitHub profile](https://github.com/manman4)) and have been actively contributing to the OEIS (Online Encyclopedia of Integer Sequences) since 2016.

Over the years, I have developed and collected a variety of code snippets and tools related to OEIS sequence editing and analysis. This repository serves as a platform to gradually release and share these resources with the community.

> **Note:** Some files or texts may contain garbled characters due to encoding or formatting issues.

### Repository Organization by Sequence Ranges

| Sequence Number Range | Corresponding Repository                       |
| --------------------- | ---------------------------------------------- |
| A000001–A099999       | [OEIS\_00](https://github.com/manman4/OEIS_00) |
| A100000–A199999       | [OEIS\_01](https://github.com/manman4/OEIS_01) |
| A200000–A299999       | [OEIS\_02](https://github.com/manman4/OEIS_02) |
| A300000–A399999       | [OEIS\_03](https://github.com/manman4/OEIS_03) |

---

## About b-files

Due to the typically large size of b-files, they will **not** be uploaded here. Instead, they are managed in a separate dedicated repository.

* Text files related to sequences are also excluded to avoid bulk uploads.
* All files are limited to **1000 digits** (including the sign) to keep the data manageable.

### Example PARI/GP script for generating b-file data (A336975):

```pari
\\ A336975
v(n) = { x = 'x + O('x^(n + 10)); 1 / prod(k = 1, n, 1 - x^k * (k + x)) };
M = 1000;
v = v(M);
for (n = 0, M,
    i = polcoef(v, n);
    if ((i < 0) + #digits(i) > 1000, break);
    write("/Users/xxx/Desktop/b336975_gp_test.txt", n, " ", i)
)
```

* **Note:** The computation is performed for a slightly larger range than required (e.g., calculating up to 10,100 terms but only outputting up to 10,000) to ensure accuracy.

---

## File Format

* All scripts in this repository are saved as **PARI/GP** (`.gp`) files for easy reuse and modification.

---

## Reference Materials

* Official OEIS data repository:
  [https://github.com/oeis/oeisdata/](https://github.com/oeis/oeisdata/)

* My curated OEIS-related study repositories (organized by themes):
  [https://github.com/manman4/study\_OEIS](https://github.com/manman4/study_OEIS)

---

If you need any assistance or have questions about these tools and files, feel free to reach out!
