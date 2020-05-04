#include <stdio.h>
#include <string.h>
//use the hashmap implementation from https://github.com/petewarden/c_hashmap
//we modify the source code a bit so that
//1) null data is accepted.
//2) save a copy of key in hashmap.
#include "hashmap.h"
#include "bibtexparser.h"

void OutputFormat(FILE *f, BIB_ENTITY *be);

int main(int argc, char *argv[])
{
	int i, fi;
	int cnt, cnt_dup, cnt_dist, cnt_plain, cnt_wrt;	
	BIB_FILE bf, bf0;
	BIB_ENTITY *be;	
	FILE *fout;
	int err_hash;
	bool isOutputplain;
	bool isOnlyRegular;
	map_t bibmap;

	if(argc  < 2) {
		printf("at least two parameters.\n");
		return -1;
	}
	fi = -1; isOutputplain = false; isOnlyRegular = false;
	for(i=1;i<argc;i++){
		if(strcmp(argv[i], "-O") == 0 || strcmp(argv[i], "--output") == 0) {
			fi = i+1;
			if(fi>=argc) {
				printf("not given output filename.\n");
				return -2;
			}		
		} else if(strcmp(argv[i], "-R") == 0 ||strcmp(argv[i], "--onlyregular") == 0) {
			isOnlyRegular = true;
		} else if(strcmp(argv[i], "-P") == 0 ||strcmp(argv[i], "--outputplain") == 0) {
			isOutputplain = true;
		} else if(argv[i][0] == '-') {			
			printf("The only available options are: \n"
				"\t-O --output if this option is not given, then stdout is used.\n"
				"\t--onlyregular default is false. if onlyregular is true, then outputplain is set to false.\n"
				"\t--outputplain default is false.\n");
			return -3;
		}		
	}

	bf = NULL;
	for(i=1;i<argc;i++) {		
		if(i == fi || argv[i][0] == '-') continue;		

		bf0 = bib_fopen(argv[i]);
		if(bib_iserr()) {
			printf("bib_err:%s\n", bib_errstr());
		}
		bf = bib_append(bf, bf0);
	}

	if(fi >= 0) {
		//fout = freopen(argv[fi], "w", stdout);
		fout = fopen(argv[fi], "w");
		if(fout == NULL) {
			printf("Set output file failed, using stdout instead.\n");
			fout = stdout;
		}				
	} else {
		fout = stdout;
	}


	cnt = 0, cnt_dup = 0, cnt_dist = 0, cnt_plain = 0; cnt_wrt = 0;
	bibmap = hashmap_new();

	while(bf) {				
		be = bib_parse_entity(bf);				
		if(be) {				
			cnt++; 					
			if(be->type >= BTE_REGULAR) {
				cnt_dist++;	
				if(be->key) {
					bib_toupr(be->key, be->key, strlen(be->key));
					err_hash = hashmap_get(bibmap, be->key, NULL); 
					if(err_hash == MAP_MISSING){//check if it is duplicated.	
						err_hash = hashmap_put(bibmap, be->key, NULL);
						if(err_hash == MAP_OK){
							OutputFormat(fout, be);	cnt_wrt++;							
						} else {
							printf("Hashmap put failed with err_hash:%d\n", err_hash); break;
						}
					} else {
						if(err_hash == MAP_OK) {
							cnt_dup++;
							printf("found a duplicated record %s.\n", be->key);
						} else {
							printf("Hashmap get failed with err_hash:%d\n", err_hash); break;
						}
					}
				} else {//always output if no key is related.
					OutputFormat(fout, be);	cnt_wrt++;
				}
			} else if(be->type == BTE_PLAIN) {
				cnt_plain++;
				if(!isOnlyRegular && isOutputplain) {
					fprintf(fout, "%s\n", bf->buf);
				}
			} else {//other type
				if(!isOnlyRegular) {
					fprintf(fout, "%s\n", bf->buf);				
				}
			} 
			bib_free_entity(be);
		} else {
			printf("bib_err:%s\n", bib_errstr());
		}			
		
		bf=bf->next;
	} 
	if(bib_iserr()) {
		printf("bib_err:%s\n", bib_errstr());
	}

	printf("Total: %d items, write %d items\n", cnt, cnt_wrt);
	printf("Total: %d (%d in bibmap) regular items with  %d duplications.\n", cnt_dist, hashmap_length(bibmap), cnt_dup);
	printf("Total: %d non-regular items.\n", cnt_plain);
	hashmap_free(bibmap);
	bib_fclose(bf);
	
	return 0;
}

void OutputFormat(FILE *f, BIB_ENTITY *be)
{
	BIB_FIELD *bef = NULL;
	if(be == NULL) return ;
	
	fprintf(f, "@%s{", be->stype);
	if(be->key){
		fprintf(f, "%s,", be->key);
	}
	fprintf(f, "\n");
	bef = be->fields;
	while(bef) {
		if(bef->name) {
			fprintf(f, "\t%s", bef->name);		
			if(bef->value) {
				fprintf(f, " = %s", bef->value);
			}
			fprintf(f, ",\n");
		}
		bef = bef->next;
	}
	fprintf(f, "}\n");
}
