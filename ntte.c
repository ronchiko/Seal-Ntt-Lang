#include "ntt.h"

#define SEAL_NTTE_NONE _NTTE_START_

static int currentError = SEAL_NTTE_NONE;
static char* errorArg = "";

static const char* ERROR_MSG_TABLE[] = {
	[NTTE_InvalidType] = "Invalid input type.",
	[NTTE_UnclosedString] = "Open string while parsing.",
	[NTTE_InvalidArgument] = "Invalid argument given to function.",
	[NTTE_IllegalArgumentCount] = "Too few/many arguments given to function.",
	[NTTE_IllegalFunctionCall] = "No such function.",
	[NTTE_TooManyFunctions] = "Too many defined functions.",
	[NTTE_InvalidNumber] = "Invalid number."
};

void SealNtt_RaiseError(SealNtt_Error error){
	currentError = error;
}
void SealNtt_ClearError(void){
	currentError = SEAL_NTTE_NONE;
}
const char* SealNtt_VerboseErrorCode(SealNtt_Error error){
	if(_NTTE_START_ >= error || error >= _NTTE_END_) return "Unknown error.";
	return ERROR_MSG_TABLE[error];
}
const char* SealNtt_VerboseError(void){
	return SealNtt_VerboseErrorCode(currentError);
}
