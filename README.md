# OEIS\_01

* [日本語](README_ja.md)
* [English](README.md)

## Overview

Welcome to the **OEIS\_01** repository! I am [manman4](https://github.com/manman4), and I have been contributing to the OEIS (Online Encyclopedia of Integer Sequences) project since 2016.

Throughout my work editing OEIS entries, I have developed and collected a variety of supporting code snippets and scripts. This repository is dedicated to gradually releasing those resources for public use and further development.

Please be aware that some files or text may contain minor encoding issues or garbled characters due to legacy formats or export processes.

---

## Sequence Number Ranges and Repository Organization

The OEIS sequences are organized across multiple repositories based on their sequence number ranges for easier management:

| Sequence Number Range | Corresponding Repository                       |
| --------------------- | ---------------------------------------------- |
| A000001 – A099999     | [OEIS\_00](https://github.com/manman4/OEIS_00) |
| A100000 – A199999     | [OEIS\_01](https://github.com/manman4/OEIS_01) |
| A200000 – A299999     | [OEIS\_02](https://github.com/manman4/OEIS_02) |
| A300000 – A399999     | [OEIS\_03](https://github.com/manman4/OEIS_03) |

---

## Handling b-files and Large Numerical Data

* Due to the large size of **b-files** (binary files storing sequence data), these are **not uploaded to this repository**. Instead, they are maintained in a separate dedicated repository to manage storage efficiently.
* Similarly, corresponding **text files** representing sequence values are not included here.
* Numerical data files stored in this repository are limited to **1000 digits per value**, including the sign, to keep file sizes manageable.

---

### Example: Generating Sequence Data with PARI/GP

Below is an example PARI/GP script snippet demonstrating how to generate sequence values and save them, limited to 1000 digits per entry:

```pari
\\ Example for sequence A336975
v(n) = { x = 'x + O('x^(n + 10)); 1 / prod(k = 1, n, 1 - x^k * (k + x)) };
M = 1000;
v = v(M);
for (n = 0, M,
  i = polcoef(v, n);
  if ((i < 0) + #digits(i) > 1000, break);
  write("/Users/xxx/Desktop/b336975_gp_test.txt", n, " ", i)
)
```

**Note:** To ensure accuracy, computations are typically done for a slightly larger range than needed (e.g., calculate up to 10100 terms but display only the first 10000).

---

## Repository Contents

This repository primarily contains:

* **PARI/GP script files (`.gp`)** used for generating, analyzing, and verifying integer sequences.
* Supporting code snippets and utilities developed throughout the OEIS editing process.

---

## Additional Resources and References

* The **official OEIS data repository** maintained by the OEIS foundation can be found here:
  [https://github.com/oeis/oeisdata/](https://github.com/oeis/oeisdata/)

* For thematic or specialized collections, I have curated several related repositories:
  [https://github.com/manman4/study\_OEIS](https://github.com/manman4/study_OEIS)

---

## Feedback and Contributions

If you have questions, improvements, or would like to contribute, please feel free to open issues or submit pull requests. Your collaboration is highly appreciated!
