# BibtexParser
A parser for bibtex file and a program to combine many files into one with distinct records.

# About the Hashmap
Hashmap is used to remove the duplications. In this project, we use a hashmap implementation from https://github.com/petewarden/c_hashmap . Moreover, we modify the source code a bit so that 1) null data is accepted; 2) save a copy of key in the hashmap itself.

# About the bibtex file format
Although we make some checks, it is the file provider's responsibility to ensure that the bibtex file is in a correct form. Otherwise, it may produce an unexpected result.

# About the functionality
We implement a parser for a bibtex which produce bib_entity with many types. Some of the types are well-known, such as article, book. Some is defined by us. For example, the BTE_PLAIN type is for those non-bibitem text blocks. By default, this combine program will not output any such blocks. Also any BTE_REGULAR type (defined in https://metacpan.org/pod/distribution/Text-BibTeX/btparse/ ) is the type greater than or equal BTE_REGULAR (defined in our project).

  the bib file having the format:
PLAIN_BLOCK*
BIBITEM_BLOCK
PLAIN_BLOCK*

  BIBITEM_BLOCK has the format:
...@typename...{|(...[key...,]...[name...[=...value...],...]* }|)...

where ... denote any control characters such as ' ', '\t', '\r', '\n'. 

# About the combine program
We write this program to show that this bibparser should work correctly. However, any bug report is welcomed.

# About the compilation
1) For MSVC along with a C89 complier, please use the MSVC workspace file "BibtexParser.dsw"; We had tested that source files are ok in Windows 10 with VC 6.0.
2) For gcc along with a C, please use the Makefile. We had tested that source files are ok in ubuntu18.04 (WSL) with gcc 7.5.0.
