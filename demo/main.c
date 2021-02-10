
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "ntt.h"

#define TYPE(name) " \033[34m("#name")\033[0m"
#define ERR(msg) "\033[31m"msg"\033[0m"

#define FOR_CHECK_TYPE(argv, argc, t) 	\
		for(int i = 0; i < argc; i++) 	\
			if(argv[i].type != t)		\
				{ SealNtt_RaiseError(NTTE_InvalidArgument, (char*)argv[i].data); return SEAL_NTT_ERROR; }

typedef struct {
	char *name;
	char *value;
} Property;

typedef struct {
	size_t count;
	Property* props;
} NttProperties;

static char* TYPES[6];
static int Vec3Type, Vec4Type, FileType, ShaderType;

int PushProperty(char* name, const SealNtt_Object* value, void* object){
	NttProperties* props = (NttProperties*)object;
	
	props->props = realloc(props->props, sizeof(Property) * (props->count + 1));
	size_t i = props->count;
	props->props[i].name = strdup(name);

	int z = 0;
	char buffer[200];
	size_t len = strlen(value->data);
	for(int i = 0; i < len && z < 200; i++) buffer[z++] = ((const char*)value->data)[i];
	len = strlen(TYPES[value->type]);
	for(int i = 0; i < len && z < 200; i++) buffer[z++] = ((const char*)TYPES[value->type])[i];
	
	if(z >= 200) return -1;
	buffer[z++] = '\0';
	
	props->props[i].value = strdup(buffer);
	props->count++;

	return 0;
}

int Seal_FuncMakeFile(SealNtt_Object* o, SealNtt_Object argv[], size_t argc){
	if(argc < 1){
		SealNtt_RaiseError(NTTE_IllegalArgumentCount, SEAL_NTT_FUNCTION);
		return SEAL_NTT_ERROR;
	}

	FOR_CHECK_TYPE(argv, argc, SEAL_NTT_STR);

	char str[350];
	snprintf(str, 350, "\033[31;1m'%s'\033[0m", argv[0].data);
	o->data = strdup(str);
	o->type = FileType;

	return SEAL_NTT_SUCCESS; 
}

int Seal_FuncMakeShader(SealNtt_Object* o, SealNtt_Object argv[], size_t argc){
	if(argc < 2){
		SealNtt_RaiseError(NTTE_IllegalArgumentCount, SEAL_NTT_FUNCTION);
		return SEAL_NTT_ERROR;
	}

	FOR_CHECK_TYPE(argv, argc, SEAL_NTT_STR);

	char str[350];
	sprintf(str, "GL(\033[31;1m'%s'\033[0m, \033[31;1m'%s'\033[0m)", argv[0].data, argv[1].data);

	o->data = strdup(str);
	o->type = ShaderType;

	return SEAL_NTT_SUCCESS; 
}

int Seal_FuncMakeVector3(SealNtt_Object* o, SealNtt_Object argv[], size_t argc){
	if(argc < 3){
		SealNtt_RaiseError(NTTE_IllegalArgumentCount, SEAL_NTT_FUNCTION);
		return SEAL_NTT_ERROR;
	}
	
	FOR_CHECK_TYPE(argv, argc, SEAL_NTT_NUM);

	
	float buffer[3];
	for(int i = 0; i < 3; i++)
		buffer[i] = strtof(argv[i].data, NULL);

	char str[200];
	sprintf(str, "(%f, %f, %f)", buffer[0], buffer[1], buffer[2]);

	o->type = Vec3Type;
	o->data = strdup(str);

	return SEAL_NTT_SUCCESS;
}

int Seal_FuncMakeVector4(SealNtt_Object* o, SealNtt_Object argv[], size_t argc){
	if(argc < 4){
		SealNtt_RaiseError(NTTE_IllegalArgumentCount, SEAL_NTT_FUNCTION);
		return SEAL_NTT_ERROR;
	}
	
	FOR_CHECK_TYPE(argv, argc, SEAL_NTT_NUM);
	
	float buffer[4];
	for(int i = 0; i < 4; i++)
		buffer[i] = strtof(argv[i].data, NULL);

	char str[200];
	sprintf(str, "Q(%f, %f, %f, %f)", buffer[0], buffer[1], buffer[2], buffer[3]);

	o->type = Vec4Type;
	o->data = strdup(str);

	return SEAL_NTT_SUCCESS;
}

int main(int argc, char* argv[]){

	if(argc < 2) {
		printf("Command must get a file: ntt [path-to-ntt-file]\n");
		exit(-1);
	}

	if(SealNtt_Init(SEAL_NTT_MAXFNC) == -1){
		printf(ERR("Seal NTT Failed to init")": %s\n", SealNtt_VerboseError());
		exit(-1);
	}

	int h, i;
	float f;
	SealNtt_StringToInt("0x1234", &h);
	SealNtt_StringToInt("1234", &i);
	SealNtt_StringToNum("123.4124", &f);
	
	printf("N: 0x%X, %d, %f\n", h, i, f);

	TYPES[SEAL_NTT_NUM] = TYPE(int);
	TYPES[SEAL_NTT_STR] = TYPE(string);
	TYPES[(Vec3Type = SealNtt_NewType())] = TYPE(vec3); 
	TYPES[(Vec4Type = SealNtt_NewType())] = TYPE(quaternion);
	TYPES[(FileType = SealNtt_NewType())] = TYPE(file); 
	TYPES[(ShaderType = SealNtt_NewType())] = TYPE(GL Shader); 

	SealNtt_NewFunc("v3", &Seal_FuncMakeVector3);
	SealNtt_NewFunc("qt", &Seal_FuncMakeVector4);
	SealNtt_NewFunc("path", &Seal_FuncMakeFile);
	SealNtt_NewFunc("shader", &Seal_FuncMakeShader);

	for(int i = 1; i < argc; i++){
		char* fileName = argv[i];
		printf("\033[35;1mNTT: '%s'\033[0m\n", fileName);
		NttProperties props = {0, NULL};
		FILE* nttFile = fopen(fileName, "r");
		if(!nttFile){
			printf(ERR("Couldn't open file '%s'")": %s\n", fileName, strerror(errno));
			exit(-1);
		}		

		if(SealNtt_Load(nttFile, &props, &PushProperty) == SEAL_NTT_ERROR){
			printf(ERR("NTT Parse error")": %s\n", SealNtt_VerboseError());
			goto cleanup;	
		}

		for(size_t i = 0; i < props.count; i++){
			printf("\t\033[32m%s\033[0m: %s\n", props.props[i].name, props.props[i].value);
			free(props.props[i].name);
			free(props.props[i].value);
		}
	cleanup:
		free(props.props);

		fclose(nttFile);
	}
	SealNtt_Cleanup();

	return 0;
}