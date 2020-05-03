#ifndef BIBTEX_PARSER_H
#define BIBTEX_PARSER_H
//#ifdef  _MSC_VER
//#include <io.h>
//#else
//#include <sys/io.h>
//#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef __cplusplus
typedef char bool;
#define true  1
#define false 0
#endif

#define STR_ERR_LEN	(1024)
#define STR_TYPE_LEN	(32)
#define STR_KEY_LEN	(128)

#define EC_ENT_0	(0)
#define EC_ENT_1	(1)
#define EC_ENT_2	(2)

#define ISKEYCH(c) ((c>= '0' && c<='9') || (c>= 'a' && c<='z') || (c>= 'A' && c<='Z') || (c == '-') || (c=='_'))
#define ISTYPECH(c) ISKEYCH(c)
#define ISNAMECH(c) ISKEYCH(c)
#define ISCTRLCH(c) ((c=='\r')||(c=='\n')||(c=='\t')||(c==' '))
#define ISOPENCH(c) ((c=='{') || (c =='('))
#define ISQUOTA(c) ((c == '{' || c == '}'))
#define CLOSECH(c) (c=='('?')':'}')

#define BTE_UNKNOWN (0)
#define BTE_COMMENT (1)
#define BTE_PREAMBLE (2)
#define BTE_MACRODEF (3)
#define BTE_STRING (4)
#define BTE_PLAIN	(5)

#define BTE_REGULAR				(6)
#define BTE_ARTICLE				(BTE_REGULAR+0)
#define BTE_BOOK				(BTE_REGULAR+1)
#define BTE_BOOKLET				(BTE_REGULAR+2)
#define BTE_INBOOK				(BTE_REGULAR+3)
#define BTE_INCOLLECTION		(BTE_REGULAR+4)	
#define BTE_INPROCEEDINGS		(BTE_REGULAR+5)
#define BTE_MANUAL				(BTE_REGULAR+6)
#define BTE_MASTERSTHESIS		(BTE_REGULAR+7)
#define BTE_MISC				(BTE_REGULAR+8)
#define BTE_PHDTHESIS			(BTE_REGULAR+9)
#define BTE_PROCEEDINGS			(BTE_REGULAR+10)
#define BTE_TECHREPORT			(BTE_REGULAR+11)
#define BTE_UNPUBLISHED			(BTE_REGULAR+12)

#if defined(_MSC_VER) && (_MSC_VER < 1500)
#define vsnprintf _vsnprintf
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf _snprintf
#endif


typedef unsigned char uchar;

typedef struct _BIB_ENTITY_RAW{
	char *buf;
	int len;	
	struct _BIB_ENTITY_RAW *next;
} BIB_ENTITY_RAW;
typedef BIB_ENTITY_RAW* BIB_FILE;

typedef struct _BIB_FIELD{
	char *name;
	char *value;
	struct _BIB_FIELD *next;
} BIB_FIELD;

typedef struct _BIB_ENTITY{
	uchar type;	
	char *stype;
	char *key;	
	BIB_FIELD *fields;
} BIB_ENTITY;

BIB_FILE bib_fopen(const char * path);
BIB_FILE bib_append(BIB_FILE fa, BIB_FILE fb);
void bib_fclose(BIB_FILE file);

int bib_itype(const char * type);

BIB_ENTITY *bib_parse_entity(BIB_ENTITY_RAW *entity);
void bib_free_entity(BIB_ENTITY *bib_entity);
bool bib_isempty_entity(char *raw);

char *bib_errstr();
void bib_errclr();
bool bib_iserr();

char *bib_log(const char *msg, ...);

int bib_tolwr(const char *src, char *dest, const int maxd);
int bib_toupr(const char *src, char *dest, const int maxd);

#define VT_TYPE (0)
#define VT_KEY (1)
#define VT_NAME (2)
#define VT_VALUE (3)

char *bib_token(char **praw, int type);

#endif