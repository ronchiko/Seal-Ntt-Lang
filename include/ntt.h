#pragma once

#include <stdlib.h>
#include <stdio.h>

#define SEAL_NTT_MAXFNC 10

#define SEAL_NTT_ERROR -1
#define SEAL_NTT_SUCCESS 0

#define SEAL_NTT_NUM 0
#define SEAL_NTT_STR 1

typedef int SealNtt_Type;

/**
 * \brief An object used to store the type & data of a field until its handles by the handler.
 */
typedef struct {
	SealNtt_Type type;
	void* data;
} SealNtt_Object;

typedef enum {
	_NTTE_START_ = -1,

	NTTE_InvalidType,
	NTTE_UnclosedString,
	NTTE_InvalidArgument,
	NTTE_IllegalArgumentCount,
	NTTE_IllegalFunctionCall,
	NTTE_TooManyFunctions,
	NTTE_InvalidNumber,

	_NTTE_END_
} SealNtt_Error;

/**
 * \brief Initializes the Seal Ntt Parser, return -1 on failure
 */
int SealNtt_Init(int maxFunctions);
/**
 * \brief Cleans all the Seal Ntt allocated memory.
 */
void SealNtt_Cleanup(void);

typedef int (*Seal_NttHandler)(char*, const SealNtt_Object*, void*);
typedef int (*SealNtt_Function)(SealNtt_Object*, SealNtt_Object argv[], size_t argc);

/**
 * \brief Registers a new Ntt type
 */
int SealNtt_NewType(void);
/**
 * \brief Registers a new Ntt Func
 */
int SealNtt_NewFunc(const char* name, SealNtt_Function);

/**
 * \brief Reads an ntt file
 */
int SealNtt_Load(FILE* stream, void* ntt, Seal_NttHandler handler);

extern const char* SEAL_NTT_FUNCTION;

// ntte.c file

#define SEAL_NTT_NO_ARG ""

void SealNtt_RaiseError(SealNtt_Error error, const char*);
const char* SealNtt_VerboseErrorCode(SealNtt_Error error);
const char* SealNtt_VerboseError(void);
void SealNtt_ClearError(void);

//nttint.c file

/**
 * \brief Converts a string into a float (or any type of formatted int)
 */
int SealNtt_StringToNum(const char* str, float*);
/**
 * \brief Converts a string into a integer (If given a float will fail)
 */
int SealNtt_StringToInt(const char* str, int*);