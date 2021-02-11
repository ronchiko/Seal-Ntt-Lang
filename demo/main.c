
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "seal.ntt.h"

#define TYPE(name) " \033[34m("#name")\033[0m"
#define ERR(msg) "\033[31m"msg"\033[0m"

#define FOR_CHECK_TYPE(argv, argc, t) 	\
		for(int i = 0; i < argc; i++) 	\
			if(argv[i].type != t)		\
				{ SealNtt_RaiseError(NTTE_InvalidArgument, SEAL_NTT_FUNCTION); return SEAL_NTT_ERROR; }

typedef struct {
	char* name;
	float x, y;
} Object_t;

static char* TYPES[3];
static int PointType;

int NttHandleProperties(char* property, const SealNtt_Object* value, void* out){
	Object_t* o = (Object_t*)out;
	
	if(SEAL_NTT_ARG_IS("name", SEAL_NTT_STR)){
		free(o->name);
		o->name = strdup(value->data);
		return SEAL_NTT_SUCCESS;
	}else if(SEAL_NTT_ARG_IS("xy", PointType)){
		o->x = ((float*)value->data)[0];
		o->y = ((float*)value->data)[1];
		return SEAL_NTT_SUCCESS;
	}else if(SEAL_NTT_ARG_IS("x", SEAL_NTT_NUM)){
		return SealNtt_StringToNum(value->data, &o->x);
	}else if(SEAL_NTT_ARG_IS("y", SEAL_NTT_NUM)){
		return SealNtt_StringToNum(value->data, &o->y);
	}

	SealNtt_RaiseError(NTTE_InvalidProperty, property);
	return SEAL_NTT_ERROR;
}

int Ntt_MakePoint(SealNtt_Object* out, SealNtt_Object* argv, size_t argc){

	if(argv[0].type != SEAL_NTT_NUM || argv[1].type != SEAL_NTT_NUM){
		SealNtt_RaiseError(NTTE_InvalidArgument, SEAL_NTT_FUNCTION);
		return SEAL_NTT_ERROR;
	}

	out->type = PointType;
	float* p = out->data = malloc(sizeof(float) * 2);
	SEAL_NTT_SAFE(SealNtt_StringToNum(argv[0].data, &p[0]));
	SEAL_NTT_SAFE(SealNtt_StringToNum(argv[1].data, &p[1]));

	return SEAL_NTT_SUCCESS;
}

void NttHandleError(const char* msg){
	printf("\033[31mSeal-Ntt error raised\033[0m: %s\n", msg);
}

int main(int argc, char* argv[]){

	if(argc < 2) {
		printf("Command must get a file: ntt [path-to-ntt-file]\n");
		exit(-1);
	}

	if(SealNtt_Init(SEAL_NTT_MAXFNC, NTTF_DontQuitOnError) == -1){
		printf(ERR("Seal NTT Failed to init")": %s\n", SealNtt_VerboseError());
		exit(-1);
	}

	SealNtt_RegisterErrorCallback(&NttHandleError);

	TYPES[SEAL_NTT_NUM] = "(number)";
	TYPES[SEAL_NTT_STR] = "(string)";
	TYPES[PointType = SealNtt_NewType()] = "(point)";

	SealNtt_NewFunc("point", &Ntt_MakePoint, 2);

	for(int i = 1; i < argc; i++){
		char* fileName = argv[i];
		printf("\033[35;1mNTT: '%s'\033[0m\n", fileName);
		
		FILE* nttFile = fopen(fileName, "r");
		if(!nttFile){
			printf(ERR("Couldn't open file '%s'")": %s\n", fileName, strerror(errno));
			exit(-1);
		}		

		Object_t o = {NULL, 0, 0};
		if(SealNtt_Load(nttFile, &o, &NttHandleProperties) == SEAL_NTT_ERROR){
			printf(ERR("NTT Parse error")": %s\n", SealNtt_VerboseError());
			goto cleanup;	
		}

		printf("Object name: %s\nPosition: (%f, %f)\n", o.name, o.x, o.y);
	cleanup:
		free(o.name);
		fclose(nttFile);
	}
	SealNtt_Cleanup();

	return 0;
}