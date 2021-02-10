#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ntt.h"

#define DELIMS " \n\t"

#define NTT_CALLSTACK_SIZE 200
#define BASE_FUNC_NAME "<source>"

static struct {
	char* callstack[NTT_CALLSTACK_SIZE];
	int used;
} Ntt_CallStack = {{0}, 0};
const char* SEAL_NTT_FUNCTION = BASE_FUNC_NAME;

void Ntt_StackPush(char* c){
	if(Ntt_CallStack.used < NTT_CALLSTACK_SIZE - 1){
		Ntt_CallStack.used++;
		SEAL_NTT_FUNCTION = Ntt_CallStack.callstack[Ntt_CallStack.used] = c;
	}
}

void Ntt_StackPop(void){
	if(Ntt_CallStack.used > 0) {
		Ntt_CallStack.used--;
		SEAL_NTT_FUNCTION = Ntt_CallStack.callstack[Ntt_CallStack.used];
	}else{
		SEAL_NTT_FUNCTION = BASE_FUNC_NAME;
	}
}

typedef struct {
	char* name;
	SealNtt_Function handler;
} Ntt_Function;

static size_t functionsCount;
static Ntt_Function* functions;
static int _LastType;

typedef int SealNtt_Result;

int SealNtt_Init(int c){
	functionsCount = c;
	functions = calloc(c, sizeof(Ntt_Function));
	_LastType = SEAL_NTT_STR;
	return 0;
}

void SealNtt_Cleanup(void){
	for(int i = 0; i < functionsCount; i++)
		free(functions[i].name);
	free(functions);
	SealNtt_ClearError();
}

int SealNtt_NewType(void){
	return ++_LastType;
}

int SealNtt_NewFunc(const char* name, SealNtt_Function handler){
	for(int i = 0; i < functionsCount; i++){
		if(functions[i].name == NULL){
			functions[i].name = strdup(name);
			functions[i].handler = handler;
			return 0;
		}
	}

	SealNtt_RaiseError(NTTE_TooManyFunctions, "");
	return SEAL_NTT_ERROR;
}

static SealNtt_Function Ntt_GetFunction(const char* name){
	if(name == NULL) return NULL;
	
	for(int i = 0; i < functionsCount; i++)
		if(functions[i].name != NULL && strcmp(functions[i].name, name) == 0)
			return functions[i].handler;

	return NULL;
}

// Parsing
typedef struct _TokenNode_s {
	char* value;
	size_t size;
	struct _TokenNode_s* next;
} TokenNode;

static TokenNode* Ntt_AdvanceFree(TokenNode* token) {
	TokenNode* next = token->next;

	if(token->value) free(token->value);
	free(token);

	return next;
}

static int Ntt_HandleArg(TokenNode** start, Seal_NttHandler handler, void* o, SealNtt_Object* nttl){
	
	if(*start == NULL){
		SealNtt_RaiseError(NTTE_InvalidArgument, "");
		return SEAL_NTT_ERROR;
	}
	assert(*start != NULL);
	SealNtt_Result result = SEAL_NTT_SUCCESS; 
	
	size_t tokenLength = (*start)->size;
	char* token = strdup((*start)->value);
	*start = Ntt_AdvanceFree(*start);

	if('0' <= token[0] && token[0] <= '9'){
		nttl->type = SEAL_NTT_NUM;
		nttl->data = strdup(token);
	}else if(token[0] == '"'){
		nttl->type = SEAL_NTT_STR;
		nttl->data = strndup(token + 1, tokenLength - 2);
	}else {
		assert(token != NULL);
		
		SealNtt_Function fnc = Ntt_GetFunction(token);
		if(fnc == NULL){
			result = SEAL_NTT_ERROR;
			SealNtt_RaiseError(NTTE_IllegalFunctionCall, token);
			goto cleanup;
		}
		assert(fnc != NULL);
		Ntt_StackPush(token);
		
		size_t argc = 0;
		SealNtt_Object* argv = NULL;
		while(*start != NULL){
			argv = realloc(argv, sizeof(SealNtt_Object) * (++argc));
			
			if(Ntt_HandleArg(start, handler, o, &argv[argc - 1]) == SEAL_NTT_ERROR){
				result = SEAL_NTT_ERROR;
				goto func_argv_clear;
			}
		}
		
		if(fnc(nttl, argv, argc) == SEAL_NTT_ERROR) result = SEAL_NTT_ERROR;
func_argv_clear:
		Ntt_StackPop();
		for(int i = 0; i < argc; i++)
			free(argv[i].data);
		free(argv);

		if(result != SEAL_NTT_SUCCESS) goto cleanup;
	}

cleanup:
	free(token);
	return result;
}

static int Ntt_HandleProps(TokenNode** start, Seal_NttHandler handler, void* o){

	if(*start == NULL){
		// Empty line
		return SEAL_NTT_SUCCESS;
	}
	
	SealNtt_Result result = SEAL_NTT_SUCCESS;

	char* prop = strdup((*start)->value);
	*start = Ntt_AdvanceFree(*start);
	SealNtt_Object obj = { -1, NULL };
	
	if(Ntt_HandleArg(start, handler, o, &obj) == SEAL_NTT_ERROR){
		result = SEAL_NTT_ERROR;
		goto cleanup;
	}

	if(handler(prop, &obj, o) == SEAL_NTT_ERROR){
		result =  SEAL_NTT_ERROR;
		goto cleanup;
	}

cleanup:
	free(prop);
	free(obj.data);
	return result;
}

int SealNtt_Load(FILE* file, void* o, Seal_NttHandler handler){
	char* buffer = NULL;
	size_t size;
	Ntt_CallStack.used = 0;
	Ntt_StackPush(BASE_FUNC_NAME);
	while(1){
		// Read the line from the stream		
		if(getline(&buffer, &size, file) == -1){
			break;
		}

		// Allocate token nodes
		TokenNode* head = malloc(sizeof(TokenNode)), *tail = head;
		head->next = NULL;
		head->value = NULL;
		
		char* line = strdup(buffer);
		char* rest = line;
		char* token = strtok_r(line, DELIMS, &rest);
		
		

		while(token){
			char* value = strdup(token);
			if(value[0] == '\"'){	// Explictly parse for string
				if(value[strlen(value) - 1] != '\"'){
					size_t i = 0;
					while(rest[i] != '"'){
						if(rest[i] == '\0'){	// Unclosed string
							free(buffer);
							return -1;
						}
						i++;
					}
					value = strncat(value, " ", 1);
					value = strncat(value, rest, i + 1);
					// Move rest
					rest += i + 1;
				}
			}
			tail->next = malloc(sizeof(TokenNode));
			tail = tail->next;
			tail->next = NULL;
			tail->size = strlen(value);
			tail->value = value;

			token = strtok_r(rest, DELIMS, &rest);
		}

		// Read token nodes
		head = Ntt_AdvanceFree(head);
		if(head && head->value[0] != '#'){	// Comment 
			if(Ntt_HandleProps(&head, handler, o) == -1){
				free(buffer);
				return -1;
			}
		}

		while(head) head = Ntt_AdvanceFree(head);	// Make sure to free the linked list
		tail = NULL;
		free(line);
	}
	Ntt_StackPop();
	free(buffer);
	return 0;
}