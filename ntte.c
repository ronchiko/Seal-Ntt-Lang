#include "seal.ntt.h"

#define SEAL_NTTE_NONE _NTTE_START_

static int currentError = SEAL_NTTE_NONE;
static char* errorMsg = NULL;
static void (*callback)(const char*) = NULL;

static const char* ERROR_MSG_TABLE[] = {
	[NTTE_InvalidType] = "Invalid input type '%s'.",
	[NTTE_UnclosedString] = "Open string while parsing.",
	[NTTE_InvalidArgument] = "Invalid argument given to function '%s'.",
	[NTTE_IllegalArgumentCount] = "Too few/many arguments given to function '%s'.",
	[NTTE_IllegalFunctionCall] = "No function '%s'.",
	[NTTE_TooManyFunctions] = "Too many defined functions.",
	[NTTE_InvalidNumber] = "Invalid number '%s'.",
	[NTTE_InvalidProperty] = "Invalid property '%s'.",
};

void SealNtt_RaiseError(SealNtt_Error error, const char* arg){
	currentError = error;
	if(errorMsg) free(errorMsg);
	errorMsg = malloc(256);

	sprintf(errorMsg, ERROR_MSG_TABLE[error], arg);
	if(callback) (*callback)(errorMsg);
}
void SealNtt_RegisterErrorCallback(void (*cbf)(const char*)){
	callback = cbf;
}
void SealNtt_ClearError(void){
	free(errorMsg);
	errorMsg = NULL;
	currentError = SEAL_NTTE_NONE;
}
const char* SealNtt_VerboseErrorCode(SealNtt_Error error){
	if(_NTTE_START_ >= error || error >= _NTTE_END_) return "Unknown error.";
	return ERROR_MSG_TABLE[error];
}
const char* SealNtt_VerboseError(void){
	if(_NTTE_START_ >= currentError || currentError >= _NTTE_END_) return "Unknown error.";
	return errorMsg;
}
