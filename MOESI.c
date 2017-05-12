#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

//constants that dictate the number of caches and the number of caches lines in each cache
#define NUM_LINES 4
#define NUM_CACHES 3

#define getState(cacheNum, lineNum) \
	(cacheSystem->caches[(cacheNum)].lines[(lineNum)].state)

#define BAD_COMMAND 666
#define HIT 111
#define MISS 222

//define macros for our states
#define INVALID 1
#define EXCLUSIVE 2
#define SHARED 4
#define DIRTY 8
#define MODIFIED (DIRTY | EXCLUSIVE)
#define OWNER (DIRTY | SHARED)

#define PROC_READ 1
#define PROC_WRITE 2
#define BUS_READ 4
#define BUS_WRITE 8
#define PROC_EVICT 16
#define BUS_EVICT 32

#define NO_CACHE_DISPLAY 100
#define DISPLAY_CACHE 250

typedef struct cacheGroup cacheGroup;
typedef struct cache cache;
typedef struct cacheLine cacheLine; 

struct cacheLine{
	int address;
	int state;	
};

struct cache{
	cacheLine lines[NUM_LINES];
	int cacheNum;
};

struct cacheGroup{
	cache caches[NUM_CACHES];
};

cacheGroup* createCacheSystem(){
	cacheGroup* cacheSystem = malloc(sizeof(cacheGroup));
		
	int i, j;
	for(i = 0; i < NUM_CACHES; i++){
		cacheSystem->caches[i].cacheNum = i;
		for(j = 0; j < NUM_LINES; j++){
			cacheSystem->caches[i].lines[j].state = INVALID;
		}
	}
	return cacheSystem;
}

int checkValidState(int state){
	if(state == INVALID || state == EXCLUSIVE || state == SHARED || state == MODIFIED || state == OWNER) 		return 1;
	return 0;
}

char stateChr(int state){
	if(!checkValidState(state)){
		printf("Invalid state detected: %d. terminating program\n", state); exit(-1);
	}
	switch(state){
		case INVALID: return 'I';
		case EXCLUSIVE: return 'E';
		case SHARED: return 'S';
		case MODIFIED: return 'M';
		case OWNER: return 'O';
	}
}

void printCacheSystem(cacheGroup* cacheSystem){
	printf("-------------------------------------------------------------------------\n");
	int i, j;
	for(i = 0; i < NUM_CACHES; i++)
		(i == (NUM_CACHES - 1)) ? printf("\tCACHE %d \t|\n", i): printf("\tCACHE %d \t|", i);
	for(i = 0; i < NUM_LINES; i++)
	for(j = 0; j < NUM_CACHES; j++)
		(j == (NUM_CACHES - 1)) ? printf("line %d\t  %c\t\t|\n", i, stateChr(getState(j, i))) : printf("line %d\t  %c\t\t|", i, stateChr(getState(j, i)));
	printf("\n");
	printf("-------------------------------------------------------------------------\n");
}

void freeCacheSystem(cacheGroup* cacheSystem, FILE* in){
	free(cacheSystem);
	if(in){
		fclose(in);
	}
}

FILE* determineMode(int argc, char** argv){
	FILE* in = NULL;
	int mode = NO_CACHE_DISPLAY;
	int fileIdx = 0;
	if(argc == 1){
		 printf("************ENTERING INTERACTIVE MODE************\n\n");
		 return in;
	}
	if(strcmp(argv[1], "-t") == 0){
			mode = DISPLAY_CACHE;
	}
	if((argc > 2 && mode == DISPLAY_CACHE) || (argc > 1 && mode == NO_CACHE_DISPLAY)){
	if(mode == NO_CACHE_DISPLAY) fileIdx = 1;
	else fileIdx = 2;
		in = fopen(argv[fileIdx], "r");
		if(!in){
			printf("The file \"%s\" does not exist. Terminating program.\n", argv[fileIdx]);	printf("usage: %s [[-t] file name]\n", argv[0]);
			printf("Specfied file: read file line by line for cache commands.\nSpecify -t option trace cache changes in a graphical display.\n");
			printf("No arguments specified: user types in cache commands interactively\n");exit(-2);
		}
	}
	if(mode == DISPLAY_CACHE) printf("************ENTERING INTERACTIVE MODE************\n\n");
	if(in) printf("************READING FROM: \"%s\"***************\n\n", argv[fileIdx]);;
	return in;
}

void clear(){
	while(getchar() != '\n');
}

int parseCommand(FILE* const in, int* const cacheNum, int* const lineNum, char* const command, int count, int argc, char** argv){
	//get info from user
	if(in == NULL){
		printf("Enter command <cacheNum><command><lineNum> : ");
		if(scanf("%d%c%d", cacheNum, command, lineNum) != 3) {
			printf("Bad command\n");
			clear();
			return BAD_COMMAND;
		}
		if(*cacheNum < 0 || *cacheNum >= NUM_CACHES){
			printf("Invalid cacheNum. \t0 <= cacheNum < %d\n", NUM_CACHES); 	
			return BAD_COMMAND;
		}
		if(*command != 'r' && *command != 'w' && *command != 'e'){
			printf("Invalid command. read: 'r'\twrite: 'w'\tevict: 'e'\n"); 	
			return BAD_COMMAND;	
		}
		if(*lineNum < 0 || *lineNum >= NUM_LINES){
			printf("Invalid lineNum. \t0 <= lineNum < %d.\n", NUM_LINES); 	
			return BAD_COMMAND;
		}
		return 1;
	}
	//parse textFile
	else{
		char check = fgetc(in);
		if(check == EOF){
			//printf("Reached EOF.\n");
			return 0;	
		}
		ungetc(check, in);
		if(fscanf(in, "%d%c%d\n", cacheNum, command, lineNum) != 3){
			printf("Bad input at line %d in \"%s\"\n.", count, argv[argc-1]);
			return 0;
		}
		if(*cacheNum < 0 || *cacheNum >= NUM_CACHES){
			printf("Invalid cacheNum at line %d in \"%s\".\n", count, argv[argc -1]); 
			return BAD_COMMAND;	
		}
		if(*command != 'r' && *command != 'w' && *command != 'e'){
			printf("Invalid command at line %d in \"%s\".\n", count, argv[argc - 1]); 		
			return BAD_COMMAND;
		}
		if(*lineNum < 0 || *lineNum >= NUM_LINES){
			printf("Invalid lineNum at line %d in \"%s\".\n", count, argv[1]); 	
			return BAD_COMMAND;
		}
		return 1;
	}	
}

int cacheLookUp(cacheGroup* cacheSystem, int cacheNum, int lineNum){
	if(getState(cacheNum,lineNum) == INVALID) return MISS;
	return HIT;		
}
//the command will be either PROB_READ OR BUS_WRITE (bus reads and writes)
void changeOtherState(cacheGroup* cacheSystem, int cacheNum, int lineNum, int lookup, char command){
	int state = getState(cacheNum, lineNum);
    if(command == BUS_WRITE && lookup == HIT){
		getState(cacheNum, lineNum) = INVALID;	
	}
	else if(state == EXCLUSIVE && command == BUS_READ && lookup == HIT){
		getState(cacheNum, lineNum) = SHARED;	
	} 
	else if(state == SHARED && command == BUS_READ && lookup == HIT){
		getState(cacheNum, lineNum) = SHARED;	
	}
	else if(state == MODIFIED && command == BUS_READ && lookup == HIT){
		getState(cacheNum, lineNum) = OWNER;	
	}
	else if(state == OWNER && command == BUS_READ && lookup == HIT){
		getState(cacheNum, lineNum) = OWNER;	
	}
	else if(command == BUS_EVICT){
		if(state == OWNER || state == MODIFIED) printf("FLUSH\n");
		getState(cacheNum, lineNum) = INVALID;	
	}	
	else{	
	}
}

//signal will either be shared or exclusive
//the commands here will either be PROC_READ or PROC_WRITE
void changeProcState(cacheGroup* cacheSystem, int thisCacheNum, int thisLineNum, int lookup, int signal, char command){
	int state = getState(thisCacheNum, thisLineNum);
	//printf("state: %d\ncacheNum:%d\nlineNum:%d\n", state, thisCacheNum, thisLineNum);
	//invalid transitions
	if(state == INVALID && signal == EXCLUSIVE && lookup == MISS && command == PROC_READ){
		getState(thisCacheNum, thisLineNum) = EXCLUSIVE;
	}	
	else if(state == INVALID && signal == SHARED && lookup == MISS && command == PROC_READ){
		getState(thisCacheNum, thisLineNum) = SHARED;	
	}
	else if(state == INVALID && lookup == MISS && command == PROC_WRITE ){
		getState(thisCacheNum, thisLineNum) = MODIFIED;
	}
	//exclusive transitions
	else if(state == EXCLUSIVE && lookup == HIT && command == PROC_READ){
		getState(thisCacheNum, thisLineNum) = EXCLUSIVE;	
	}
	else if(state == EXCLUSIVE && lookup == HIT && command == PROC_WRITE){
		getState(thisCacheNum, thisLineNum) = MODIFIED;	
	}
	//shared transitions
	else if(state == SHARED && lookup == HIT && command == PROC_READ){
		getState(thisCacheNum, thisLineNum) = SHARED;	
	}
	else if(state == SHARED && lookup == MISS && command == PROC_READ){
		getState(thisCacheNum, thisLineNum) = INVALID;	
	}
	else if(state == SHARED && lookup == HIT && command == PROC_WRITE){
		getState(thisCacheNum, thisLineNum) = MODIFIED;	
	}
	//modified transitions
	else if(state== MODIFIED && lookup == HIT && (command == PROC_WRITE || command == PROC_READ)){
		getState(thisCacheNum, thisLineNum) = MODIFIED;	
	}
	//owned transitions
	else if(state == OWNER && lookup == HIT && (command == PROC_WRITE || command == PROC_READ)){
		getState(thisCacheNum, thisLineNum) = OWNER;	
	}
	else if(state == EXCLUSIVE && command == PROC_EVICT){
		getState(thisCacheNum, thisLineNum) = INVALID;	
	}
	else if(state == SHARED && command == PROC_EVICT){
		getState(thisCacheNum, thisLineNum) = INVALID;	
	} 
	else if(state == MODIFIED && command == PROC_EVICT){
		printf("FLUSH\n");	
		getState(thisCacheNum, thisLineNum) = INVALID;	
	}
	else if(state == OWNER && command == PROC_EVICT){
		printf("FLUSH\n");
		getState(thisCacheNum, thisLineNum) = INVALID;	
	}
	//should never get in here, if we do we have to debug!
	else{
	}
}

//TODO: implement
int bus_write(cacheGroup *cacheSystem, int cacheNum, int lineNum){
		int lookup = cacheLookUp(cacheSystem, cacheNum, lineNum);
		printf("Cache %d, Bus Write %d\n", cacheNum, lineNum);
		if(lookup == HIT){
			int state = getState(cacheNum, lineNum);
			if(state == MODIFIED || state == OWNER) printf("Hit Dirty\nFlush\n");
			else printf("Hit\n");
		}
		if(lookup == MISS) printf("Miss\n");
		char oldState = stateChr(getState(cacheNum, lineNum));
		changeOtherState(cacheSystem, cacheNum, lineNum, lookup, BUS_WRITE);
		printf("%c->%c\n", oldState, stateChr(getState(cacheNum, lineNum)));
		printf("End Bus Write\n\n");
		return lookup;
}
//TODO: implement
int bus_evict(cacheGroup *cacheSystem, int cacheNum, int lineNum){
		int lookup = cacheLookUp(cacheSystem, cacheNum, lineNum);
		char oldState = stateChr(getState(cacheNum, lineNum));
		changeOtherState(cacheSystem, cacheNum, lineNum, lookup, BUS_EVICT);
		printf("%c->%c\n", oldState, stateChr(getState(cacheNum, lineNum)));
		return lookup;
}


//TODO: implement
int bus_read(cacheGroup *cacheSystem, int cacheNum , int lineNum){
		int lookup = cacheLookUp(cacheSystem, cacheNum, lineNum);
		printf("Cache %d, Bus Read %d\n", cacheNum, lineNum);
		if(lookup == HIT){
			int state = getState(cacheNum, lineNum);
			if(state == MODIFIED || state == OWNER) printf("Hit Dirty\n");
			else printf("Hit\n");
		}
		if(lookup == MISS) printf("Miss\n");
		char oldState = stateChr(getState(cacheNum, lineNum));
		changeOtherState(cacheSystem, cacheNum, lineNum, lookup, BUS_READ);
		printf("%c->%c\n", oldState, stateChr(getState(cacheNum, lineNum)));
		printf("End Bus Read\n\n");
		return lookup;
}

//TODO: implement
void readCommand(cacheGroup *cacheSystem, int cacheNum, int lineNum, char command){
		int lookup = cacheLookUp(cacheSystem, cacheNum, lineNum);
		int busAlookup, busBlookup, otherA, otherB;
		if(lookup == HIT){
			int state = getState(cacheNum, lineNum);
			if(state == MODIFIED || state == OWNER){
			printf("HIT DIRTY\n", cacheNum, lineNum);
			printf("%c -> %c\n\n", stateChr(getState(cacheNum, lineNum)), stateChr(getState(cacheNum, lineNum)));
			}else{
			printf("Hit\n");
			printf("%c -> %c\n\n", stateChr(getState(cacheNum, lineNum)), stateChr(getState(cacheNum, lineNum)));
			}
		}
		if(lookup == MISS) {
			printf("Cache %d, Miss %d\n\n", cacheNum, lineNum);
			if(cacheNum == 0){
				otherA = 1;
				otherB = 2;	
			}
			else if(cacheNum == 1){
				otherA = 0;
				otherB = 2;	
			}
			else{
				otherA = 0;
				otherB = 1;	
			}
			busAlookup = bus_read(cacheSystem, otherA , lineNum);
			busBlookup = bus_read(cacheSystem, otherB, lineNum);
			int signal;
			if(busAlookup == HIT || busBlookup == HIT) signal = SHARED;
			else signal = EXCLUSIVE;
			char oldState = stateChr(getState(cacheNum, lineNum));
			changeProcState(cacheSystem, cacheNum, lineNum, lookup, signal, PROC_READ);
			printf("Cache %d\n%c -> %c\n", cacheNum, oldState, stateChr(getState(cacheNum, lineNum)));
			if(getState(otherA, lineNum) == INVALID && getState(otherB, lineNum) == INVALID)
				printf("MEMORY READ\n");
			printf("\n");
		}
	
}

//TODO: implement
void writeCommand(cacheGroup *cacheSystem, int cacheNum, int lineNum, char command){
		int lookup = cacheLookUp(cacheSystem, cacheNum, lineNum);
		int busA, busB, otherA, otherB;
		if(lookup == HIT){
			int state = getState(cacheNum, lineNum);
			if(state == MODIFIED || state == OWNER){
			printf("Hit Dirty\n");
			}else{
			printf("Hit\n");
			}
		}
		if(lookup == MISS) printf("Miss\n\n");
		if(cacheNum == 0){
				otherA = 1;
				otherB = 2;	
			}
			else if(cacheNum == 1){
				otherA = 0;
				otherB = 2;	
			}
			else{
				otherA = 0;
				otherB = 1;	
			}
			//dummy signal
			int signal = 5555;
			if(getState(cacheNum, lineNum) != EXCLUSIVE && getState(cacheNum, lineNum) != MODIFIED){
				busA = bus_write(cacheSystem, otherA , lineNum);
				busB = bus_write(cacheSystem, otherB, lineNum);
			}
			char oldState = stateChr(getState(cacheNum, lineNum));
			changeProcState(cacheSystem, cacheNum, lineNum, lookup, signal, PROC_WRITE);
				if(getState(cacheNum, lineNum) != MODIFIED) printf("Cache %d\n", cacheNum);
					printf("%c -> %c\n\n", oldState, stateChr(getState(cacheNum, lineNum)));
}

//TODO: implement
void evictCommand(cacheGroup *cacheSystem, int cacheNum, int lineNum, char command){
			int lookup = cacheLookUp(cacheSystem, cacheNum, lineNum);
			int busA, busB, otherA, otherB;
			char oldState = stateChr(getState(cacheNum, lineNum));
		if(cacheNum == 0){
				otherA = 1;
				otherB = 2;	
			}
			else if(cacheNum == 1){
				otherA = 0;
				otherB = 2;	
			}
			else{
				otherA = 0;
				otherB = 1;	
			}
			int signal;
			if(busA == HIT || busB == HIT) signal = HIT;
			else signal = MISS;
			busA = bus_evict(cacheSystem, otherA , lineNum);
			busB = bus_evict(cacheSystem, otherB, lineNum);
			changeProcState(cacheSystem, cacheNum, lineNum, lookup, signal, PROC_EVICT);
			printf("Evict Cache %d\n%c -> %c\n\n", cacheNum, oldState, stateChr(getState(cacheNum, lineNum)));
}

void doCommand(cacheGroup *cacheSystem, int cacheNum, int lineNum, char command){
	if(command == 'r') readCommand(cacheSystem, cacheNum, lineNum, command);
	else if(command == 'w') writeCommand(cacheSystem, cacheNum, lineNum, command);
	else if(command == 'e') evictCommand(cacheSystem, cacheNum, lineNum, command);
	//should never reach this else block...
	else{
		printf("Bad command, terminating the program.\n");
		exit(-8);
	}
}

int main(int argc, char** argv){
	FILE* in = determineMode(argc, argv); 
	cacheGroup *cacheSystem = createCacheSystem();
	if(argc > 1 && strcmp(argv[1], "-t") == 0) printCacheSystem(cacheSystem);
	int cacheNum, lineNum, cond, fileCount = 1; char command;
	while((cond = parseCommand(in, &cacheNum, &lineNum, &command,fileCount++, argc, argv)) != 0){
		if(cond == BAD_COMMAND) continue;
		if(argc > 1 && strcmp(argv[1], "-t") == 0) printf("Command: %d%c%d\n", cacheNum, command, lineNum);
		else printf("%d%c%d\n", cacheNum, command, lineNum);
		doCommand(cacheSystem, cacheNum, lineNum, command);
		if(argc > 1 && strcmp(argv[1], "-t") == 0) printCacheSystem(cacheSystem);
	}
	
	freeCacheSystem(cacheSystem, in);
	return 0;
}
