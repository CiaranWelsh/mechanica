/*
 * ca_eval.cpp
 *
 *  Created on: Aug 4, 2015
 *      Author: andy
 */

#include "mechanica_private.h"

//static MxModule *builtins = nullptr;
//static MxModule *globals = nullptr;


/**
 * Initialize the runtime eval modules (builtins, globals)
 */
int initEval()
{
	//assert(builtins == nullptr);

	//builtins = new MxModule("__builtins__");






	return 0;

}

/**
 * Shutdown the eval modules
 */
int finalizeEval()
{
	return 0;
}


/// public API

extern "C" {


CObject* MxEval_GetBuiltins()
{
	MX_NOTIMPLEMENTED
}

CObject* MxEval_GetLocals()
{
	MX_NOTIMPLEMENTED
}

CObject* MxEval_GetGlobals()
{
	MX_NOTIMPLEMENTED
}

const char* MxEval_GetFuncName(CObject* func)
{
	MX_NOTIMPLEMENTED
}

const char* MxEval_GetFuncDesc(CObject* func)
{
	MX_NOTIMPLEMENTED
}

}




