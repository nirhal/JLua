
#ifndef jlua_h
#define jlua_h

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

/* Uncomment to enable Java Load Lib  */
//#define ENABLELOADLIB

/* Constants that are used to index the JVM and Java version */
#define JLUAJVMTAG      "__JVM"
#define JLUAJVERSIONTAG      "__JVersion"
/* Defines whether the metatable is of a java Object */
#define JLUAOBJECTIND      "__IsJavaObject"
/* Defines the lua State Index Property Name */
#define JLUASTATEINDEX     "JLuaStateIndex"
/* Index metamethod name */
#define LUAINDEXMETAMETHODTAG "__index"
/* New index metamethod name */
#define LUANEWINDEXMETAMETHODTAG "__newindex"
/* Garbage collector metamethod name */
#define LUAGCMETAMETHODTAG    "__gc"
/* Call metamethod name */
#define LUACALLMETAMETHODTAG  "__call"


extern jclass throwable_class;
extern jmethodID get_message_method;
extern jclass java_function_class;
extern jmethodID java_function_method;
extern jclass jlua_api_class;
extern jclass java_lang_class;


/***************************************************************************
*
* $FC Function objectIndex
*
* $ED Description
*    Function to be called by the metamethod __index of the java object
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int objectIndex(lua_State *L);


/***************************************************************************
*
* $FC Function objectIndexReturn
*
* $ED Description
*    Function returned by the metamethod __index of a java Object. It is
*    the actual function that is going to call the java method.
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int objectIndexReturn(lua_State *L);


/***************************************************************************
*
* $FC Function objectNewIndex
*
* $ED Description
*    Function to be called by the metamethod __newindex of the java object
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int objectNewIndex(lua_State *L);


/***************************************************************************
*
* $FC Function arrayIndex
*
* $ED Description
*    Function to be called by the metamethod __index of a java array
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int arrayIndex(lua_State *L);


/***************************************************************************
*
* $FC Function arrayNewIndex
*
* $ED Description
*    Function to be called by the metamethod __newindex of a java array
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int arrayNewIndex(lua_State *L);


/***************************************************************************
*
* $FC Function GC
*
* $ED Description
*    Function to be called by the metamethod __gc of the java object
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int gc(lua_State *L);


/***************************************************************************
*
* $FC Function javaGetClass
*
* $ED Description
*    Implementation of lua function jlua.GetClass
*
* $EP Function Parameters
*    $P L - lua State
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int javaGetClass(lua_State *L);

/***************************************************************************
*
* $FC Function newArray
*
* $ED Description
*    Implementation of lua function jlua.newArray
*
* $EP Function Parameters
*    $P L - lua State
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int newArray(lua_State *L);

/***************************************************************************
*
* $FC Function tryBlock
*
* $ED Description
*    Implementation of lua function jlua.try
*
* $EP Function Parameters
*    $P L - lua State
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int tryBlock(lua_State *L);

/***************************************************************************
*
* $FC Function throwObj
*
* $ED Description
*    Implementation of lua function jlua.throw
*
* $EP Function Parameters
*    $P L - lua State
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int throwObj(lua_State *L);

/***************************************************************************
*
* $FC Function createProxy
*
* $ED Description
*    Implementation of lua function jlua.createProxy.
*    Transform a lua table into a java class that implements a list
*  of interfaces
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int createProxy(lua_State *L);



/***************************************************************************
*
* $FC Function javaLoadLib
*
* $ED Description
*    Implementation of lua function jlua.loadLib
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/
#ifdef ENABLELOADLIB
int javaLoadLib( lua_State * L );
#endif

/***************************************************************************
*
* $FC pushJavaObject
*
* $ED Description
*    Function to create a lua proxy to a java object
*
* $EP Function Parameters
*    $P L - lua State
*    $P javaObject - Java Object to be pushed on the stack
*    $P checkObject - Whether to check the permission to access object
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int pushJavaObject(lua_State *L, jobject javaObject, int checkObject);


/***************************************************************************
*
* $FC pushJavaArray
*
* $ED Description
*    Function to create a lua proxy to a java array
*
* $EP Function Parameters
*    $P L - lua State
*    $P javaObject - Java array to be pushed on the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function
*
*$. **********************************************************************/

int pushJavaArray(lua_State *L, jobject javaObject);


/***************************************************************************
*
* $FC isJavaObject
*
* $ED Description
*    Returns 1 is given index represents a java object
*
* $EP Function Parameters
*    $P L - lua State
*    $P idx - index on the stack
*
* $FV Returned Value
*    int - Boolean.
*
*$. **********************************************************************/

int isJavaObject(lua_State *L, int idx);


/***************************************************************************
*
* $FC getStateFromCPtr
*
* $ED Description
*    Returns the lua_State from the CPtr Java Object
*
* $EP Function Parameters
*    $P L - lua State
*    $P cptr - CPtr object
*
* $FV Returned Value
*    int - Number of values to be returned by the function.
*
*$. **********************************************************************/

lua_State *getStateFromCPtr(JNIEnv *env, jobject cptr);


/***************************************************************************
*
* $FC jluaFunctionCall
*
* $ED Description
*    function called by metamethod __call of instances of JavaFunctionWrapper
*
* $EP Function Parameters
*    $P L - lua State
*    $P Stack - Parameters will be received by the stack
*
* $FV Returned Value
*    int - Number of values to be returned by the function.
*
*$. **********************************************************************/

int jluaFunctionCall(lua_State *L);


/***************************************************************************
*
* $FC pushJNIEnv
*
* $ED Description
*    function that pushes the jni environment into the lua state
*
* $EP Function Parameters
*    $P env - java environment
*    $P L - lua State
*
* $FV Returned Value
*    void
*
*$. **********************************************************************/

void pushJNIEnv(JNIEnv *env, lua_State *L);


/***************************************************************************
*
* $FC getEnvFromState
*
* $ED Description
*    auxiliary function to get the JNIEnv from the lua state
*
* $EP Function Parameters
*    $P L - lua State
*
* $FV Returned Value
*    JNIEnv * - JNI environment
*
*$. **********************************************************************/

JNIEnv *getEnvFromState(lua_State *L);

/***************************************************************************
*
* $FC getEnvFromStateCheck
*
* $ED Description
*    Wrapper for getEnvFromState
*
* $EP Function Parameters
*    $P L - lua State
*
* $FV Returned Value
*    JNIEnv * - JNI environment
*
*$. **********************************************************************/

JNIEnv *getEnvFromStateCheck(lua_State *L);


#endif