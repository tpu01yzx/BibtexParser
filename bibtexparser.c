#include "bibtexparser.h"

static char strErr[STR_ERR_LEN];

BIB_FILE bib_fopen(const char * path)
{
	int flen;
	int ci;
	char c;
	char *buffer = NULL;
	char nest;
	int p = 0;
	bool isType, isBody, isNewEntity, isRegular;
	int nBracket;
	FILE *file = NULL;

	BIB_ENTITY_RAW *be = NULL;
	BIB_ENTITY_RAW *head = NULL, *tail = NULL;

	bib_errclr();

	file = fopen(path, "r");
	if(file == NULL) {
		bib_log("Cannot open file:%s", path);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	flen = ftell(file);
	rewind(file);

	buffer = (char *)malloc(flen);
	if(buffer == NULL) {
		bib_log("Cannot allocate %d bytes memory.", flen);
		return NULL;
	}
	//puts("file is opened.");
	c = 0; ci = 0;
	isType = false; isBody = false; isRegular = false;
	nBracket = 0; nest = 0;
	p = 0; isNewEntity = false;
	while(ci!=EOF) {
		ci=fgetc(file); c = ci & 0xff;
		//putchar(ci); continue;
		if(ci == EOF) {
			//puts("ready for last entity.");
			if(isRegular && (nBracket != 0 || isType || isBody || nBracket != 0)) {
				bib_log("Expect an ending of entity."); break;
			}
			isNewEntity = true;
		}  else {
			//deal with escape characters that begins with '\';
			if(p<1 || buffer[p-1] != '\\') {				
				if(c == '@' && !isType && !isBody) { // a new block										
					isNewEntity = true;	
				} else if(isType) {
					//puts("find a block type.");					
					//begin body
					if(ISOPENCH(c)) {
						//puts("find a block body."); 
						nest = CLOSECH(c);
						nBracket = 0; isBody = true; isType = false;
					} 
				} else if(isBody) {
					//printf("%d", nBracket); putchar(c);
					if(nBracket == 0) {
						//end body
						if(c == nest) {
							isNewEntity = true; isBody = false;
						} else {
							if(c == '{') nBracket++;//'{' and '}' should appear in pairs within an entity.							
						}
					} else if(nBracket > 0) {
						if(c == '{') nBracket++;
						if(c == '}') nBracket--;
					} else if(nBracket <0 ){
						bib_log("{ is too much"); break;
					}		
					
				} 
			}

			// save to buffer
			if(p<flen)	{
				if(!isNewEntity || c == nest){
					buffer[p++] = c;			
				}
			} else {
				bib_log("Overflowed while save to buffer."); break;
			}
		}		
		if(isNewEntity){
			//puts("new a block.");
			if(p<flen) buffer[p] = '\0'; //end buffer as a string while not move the pointer p;
			//ignore empty entity consist of only control chars.
			if(p > 0 && !bib_isempty_entity(buffer)) {//deal with the previous block
				be = (BIB_ENTITY_RAW*)malloc(sizeof(BIB_ENTITY_RAW));
				if(be == NULL) {
					bib_log("Cannot allocate %d bytes memory for BIB_ENTITY_RAW.", sizeof(BIB_ENTITY_RAW)); break;
				}

				be->next = NULL;
				be->len = p+1; //the string length + one byte '\0';
				be->buf = (char *)malloc(be->len);
				if(be->buf == NULL) {
					free(be);
					bib_log("Cannot allocate %d bytes memory for be->buf.", be->len); break;
				}
				memcpy(be->buf, buffer, be->len);
				//strncpy(be->buf, buffer, be->len); 				
				//if strlen(buffer) == be->len, then no '\0' is added by strncpy.
				be->buf[p] = '\0'; //end the string with '\0'
				//printf("%d:%d:%d:%s", nEntity++, flen, p, be->buf);
				


				//head is the first block.
				if(head == NULL) {					
					head = be;					
				}
				//insert at the tail.
				if(tail == NULL) {
					tail = be;
				} else {
					tail->next = be;
					tail = be;
				}
			}
			
			p = 0;				
			isType = false; isBody = false; isNewEntity = false; isRegular = false;
			nBracket = 0; nest = '\0';
			//memset(buffer, 0, flen);
			if(c == '@') {				
				isRegular = true; isType = true; isBody = false;
				buffer[p++] = c;			
			}	
		}
	}

	free(buffer);
	fclose(file);

	return head;
}

void bib_fclose(BIB_FILE file)
{
	BIB_FILE bf, bf0;

	bf = file;
	while(bf) {
		if(bf->buf) free(bf->buf);
		bf0 = bf->next;
		free(bf);	
		bf = bf0;
	}
}

BIB_FILE bib_append(BIB_FILE fa, BIB_FILE fb)
{
	BIB_ENTITY_RAW *be;
	if(fa == NULL) return fb;
	if(fb == NULL) return fa;
	be = fa;
	while(be->next){
		be = be->next;
	}
	be->next = fb;
	return fa;
}

int bib_toupr(const char *src, char *dest, const int maxd)
{
	int i = 0;
	while(*src && i<maxd){		
		if(*src>='a' && *src <='z') {
			*dest++ = (*src++)+'A'-'a';
		} else {
			*dest++ = *src++;
		}
		i++;
	}

	*dest = '\0';

	return i;
}

int bib_tolwr(const char *src, char *dest, const int maxd)
{
	int i = 0;
	while(*src && i<maxd){		
		if(*src>='A' && *src <='Z') {
			*dest++ = (*src++)+'a'-'A';
		} else {
			*dest++ = *src++;
		}
		i++;
	}

	*dest = '\0';

	return i;
}
bool bib_isempty_entity(char *raw)
{	
	if(raw == NULL) return true;	
	while(*raw && ISCTRLCH(*raw)){ raw++;}
	return (*raw == '\0');
}

int bib_itype(const char * type)
{
	static char stype[STR_TYPE_LEN];
	bib_tolwr(type, stype, STR_TYPE_LEN);
	if(strcmp(stype, "comment") == 0) return BTE_COMMENT;
	if(strcmp(stype, "preamble") == 0) return BTE_PREAMBLE;
	if(strcmp(stype, "string") == 0) return BTE_STRING;

	if(strcmp(stype, "article") == 0) return BTE_ARTICLE;
	if(strcmp(stype, "book") == 0) return BTE_BOOK;
	if(strcmp(stype, "booklet") == 0) return BTE_BOOKLET;
	if(strcmp(stype, "inbook") == 0) return BTE_INBOOK;
	if(strcmp(stype, "incollection") == 0) return BTE_INCOLLECTION;
	if(strcmp(stype, "inproceedings") == 0) return BTE_INPROCEEDINGS;
	if(strcmp(stype, "manual") == 0) return BTE_MANUAL;
	if(strcmp(stype, "mastersthesis") == 0) return BTE_MASTERSTHESIS;
	if(strcmp(stype, "misc") == 0) return BTE_MISC;
	if(strcmp(stype, "phdthesis") == 0) return BTE_PHDTHESIS;
	if(strcmp(stype, "proceedings") == 0) return BTE_PROCEEDINGS;
	if(strcmp(stype, "techreport") == 0) return BTE_TECHREPORT;
	if(strcmp(stype, "unpublished") == 0) return BTE_UNPUBLISHED;	

	return BTE_UNKNOWN;
}

/*
#define VT_TYPE (0)
#define VT_KEY (1)
#define VT_NAME (2)
#define VT_VALUE (3)
*/
//make sure that raw MUST have following one of formats:
//1)  ....@type...{...key...,...name...=...value...,...name...=...value...,...}
//2)  ....@type...{...key...,...name...=...value...,...name...=...value...}
//3)  ....@type...{...name...=...value...,...name...=...value...,...}
//4)  ....@type...{...name...=...value...,...name...=...value...}
// where { and } can be replaced by ( and ).
// a type and a key is consist of "a-zA-Z0-9-_"
char *bib_token(char **praw, int type)
{
	static char nest = 0;
	static bool hasValue = false;
	char *raw;
	char *raw0;
	char *raw1;
	char c, c0;
	int nBracket;
	bib_errclr();

	if(praw == NULL) return NULL;
	raw = *praw;

	if(type == VT_TYPE) {
		c=*raw;
		while(c && c != '@'){c=*(++raw);} //find a '@'
		c=*(++raw);	raw0 = raw;
		while(c && ISTYPECH(c)){c=*(++raw);} //find a non-type char.
		if(c) {*raw = '\0'; raw1 = raw; c0 = c;}

		//check if it is followed by an open ch.
		while(ISCTRLCH(c)){c=*(++raw);} //find a non-type char.
		if(!ISOPENCH(c)){
			if(c) {*raw1 = c0;}
			bib_log("Expect a { or ( for VT_TYPE."); return NULL;
		} else {
			nest= CLOSECH(c);
		}
	} else if(type == VT_KEY) {
		c=*raw;
		while(c && !ISKEYCH(c)){c=*(++raw);} //find a key char.
		raw0 = raw;	
		while(c && ISKEYCH(c)){c=*(++raw);} //find a non-key char.
		if(c) {*raw = '\0'; raw1 = raw; c0 = c;}

		//check if it is followed by an , .
		while(ISCTRLCH(c)){c=*(++raw);} //find a non-control char.
		if(!(c==',')){			
			//since key is optional, recover if on key is found.
			if(c) {*raw1 = c0;}
			//bib_log("Expect a , for VT_KEY."); 
			return NULL;
		}	
	} else if(type == VT_NAME) {
		c=*raw;
		while(c && !ISNAMECH(c)){c=*(++raw);} //find a key char.
		if(!c) return NULL;
		raw0 = raw;	
		while(c && ISNAMECH(c)){c=*(++raw);} //find a non-key char.
		if(c) {*raw = '\0'; raw1 = raw; c0 = c;}

		//check if it is followed by an , or '=' .
		//= for name-value.
		//, for ony name.
		while(ISCTRLCH(c)){c=*(++raw);} //find a non-control char.
		if(!(c==',' || c=='=')){
			if(c) {*raw1 = c0;}
			//bib_log("Expect a , or = for VT_NAME."); 
			return NULL;						
		}
		if(c == '=') {hasValue = false;}
		if(c == '=') {hasValue = true;}
	} else if(type == VT_VALUE) {
		if(!hasValue) return NULL;
		hasValue = false;

		c=*raw; nBracket = 0;		
		while(c && ISCTRLCH(c)){c=*(++raw);} //find a value char.
		raw0=raw;

		while(c){
			if(c == '\\') { c=*(++raw); c=*(++raw);} //ignore \ and the one after \ such as \" .
			
			if(nBracket == 0){							
				if((c == nest) || (c == ',')) { //end value
					break;
				}
				if(c == '{') nBracket++;
				if(c == '}') {
					bib_log("Expect less }."); return NULL;	
				}
			} else if(nBracket>0) {
				if(c == '}') nBracket--;
				if(c == '{') nBracket++;
			} else if(nBracket<0) {
				bib_log("Expect less }."); return NULL;	
			}
			c=*(++raw);
		}
		if(!c) {
			bib_log("Expect a , or some %c for VT_VALUE.", nest); return NULL;	
		} 
		raw1 = raw; 
		c=*(--raw);//trim the last control ch
		while(c && ISCTRLCH(c)){c=*(--raw);}
		*(++raw) = '\0';
		raw = raw1;
	}
	*praw = raw+1;
	return raw0;
}
BIB_ENTITY *bib_parse_entity(BIB_ENTITY_RAW *entity)
{	
	char *raw, *raw0;
	char *buffer;
	int tlen;
	BIB_ENTITY *be = NULL;
	BIB_FIELD *one = NULL, *tail = NULL;
	
	bib_errclr();
	if(entity == NULL) return NULL;
	raw = entity->buf;

	be = (BIB_ENTITY *)malloc(sizeof(BIB_ENTITY));
	if(be == NULL) {
		bib_log("Cannot allocate %d bytes memory for BIB_ENTITY.", sizeof(BIB_ENTITY));
		return NULL;
	}

	memset(be, 0, sizeof(BIB_ENTITY));
	be->type = BTE_UNKNOWN;
	be->stype = NULL;
	be->key = NULL;
	be->fields = NULL;
	if(raw == NULL) {
		return be;
	}

	buffer = (char *) malloc(entity->len);
	if(buffer == NULL) {
		free(be);
		bib_log("Cannot allocate %d bytes memory for buffer.", entity->len); 
		return NULL;
	}		

	

	//memcpy(buffer, entity->buf, entity->len);	
	strncpy(buffer, entity->buf, entity->len);	
	raw = buffer;
	
	//block with non-plain type
	if(raw[0] == '@') {	
/*
#define VT_TYPE (0)
#define VT_KEY (1)
#define VT_NAME (2)
#define VT_VALUE (3)
*/
		raw0 = bib_token(&raw, VT_TYPE);		
		be->type = bib_itype(raw0);		
		tlen = strlen(raw0)+1;
		be->stype = (char*)malloc(tlen);
		if(be->stype == NULL) {
			free(be); free(buffer);
			bib_log("Cannot allocate %d bytes memory for BIB_ENTITY.stype.", tlen); 
			return NULL;
		} else {
			strncpy(be->stype, raw0, tlen); //no change
			//bib_toupr(raw0, be->stype, tlen);//update case
			//bib_tolwr(raw0, be->stype, tlen); //lower case
		}

		if((raw0 = bib_token(&raw, VT_KEY)) != NULL) {	
			tlen = strlen(raw0)+1;
			if(tlen > STR_KEY_LEN) {
				free(be); free(buffer);
				bib_log("Key string length %d is too larg while max is %d\n", tlen, STR_KEY_LEN); 
				return NULL;
			}
			be->key = (char*)malloc(tlen);
			if(be->key){
				strncpy(be->key, raw0, tlen);	
			}
		}
		
		while((raw0 = bib_token(&raw, VT_NAME)) != NULL){
			one = (BIB_FIELD*) malloc(sizeof(BIB_FIELD));
			if(one == NULL) {				
				bib_log("Cannot allocate %d bytes memory for BIB_FIELD.", sizeof(BIB_FIELD)); 
				break;
			}
			memset(one, 0, sizeof(BIB_FIELD));
			one->next = NULL;
			one->name = NULL;
			one->value = NULL;

			one->name = (char *) malloc(strlen(raw0)+1);
			if(one->name == NULL) {
				free(one);
				bib_log("Cannot allocate %d bytes memory for BIB_FIELD.name.", strlen(raw0)+1); 
				break;
			}
			strcpy(one->name, raw0);

			if((raw0 = bib_token(&raw, VT_VALUE)) != NULL){
				one->value = (char *) malloc(strlen(raw0)+1);
				if(one->value == NULL) {
					free(one); free(one->name);
					bib_log("Cannot allocate %d bytes memory for BIB_FIELD.value.", strlen(raw0)+1); 
					break;
				}
				strcpy(one->value, raw0);
			}
			if(be->fields == NULL) {be->fields = one;}
			if(tail == NULL) {
				tail = one;
			} else {
				tail->next = one;
				tail = one;
			}

		}		
	} else {		
		be->fields = (BIB_FIELD*) malloc(sizeof(BIB_FIELD));
		if(be->fields == NULL) {
			free(be); free(buffer);
			bib_log("Cannot allocate %d bytes memory for BIB_FIELD.", sizeof(BIB_FIELD)); 
			return NULL;
		}

		be->fields->value = (char*)malloc(entity->len);
		if(be->fields) {
			memcpy(be->fields->value, entity->buf, entity->len);
			be->fields->value[entity->len-1] = '\0';

			be->type = BTE_PLAIN;
			be->fields->next = NULL;
			be->fields->name = NULL;
		}		
	}
	free(buffer);
	return be;
}
void bib_free_entity(BIB_ENTITY *ent)
{
	BIB_FIELD* bf, *bf0;
	if(ent){
		if(ent->key) free(ent->key);
		if(ent->stype) free(ent->stype);
		
		bf = ent->fields;
		while(bf) {
			if(bf->name) free(bf->name);
			if(bf->value) free(bf->value);
			bf0 = bf;
			bf = bf->next;
			free(bf0);
		}
		free(ent);
	}
}


void bib_errclr()
{
	strErr[0] = '\0';
}

char *bib_errstr()
{
	return strErr;
}
bool bib_iserr()
{
	return strErr[0] != '\0';
}

char *bib_log(const char *msg, ...)
{
	 va_list ap;  
	 va_start(ap, msg);  
	 
	 vsnprintf(strErr, STR_ERR_LEN, msg, ap); 
	 va_end(ap); 
	 return strErr;
}

