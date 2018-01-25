
/******************************************************************************
* $Id$
* Copyright (C) 2003-2007 Kepler Project.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include "jlua.h"

/********************* Implementations ***************************/


lua_Number getLuaStateIndex(lua_State *L){
    lua_Number stateIndex;

    lua_pushstring(L, JLUASTATEINDEX);
    lua_rawget(L, LUA_REGISTRYINDEX);

    if (!lua_isnumber(L, -1)) {
        lua_pushstring(L, "Impossible to identify luaState id.");
        lua_error(L);
    }

    stateIndex = lua_tonumber(L, -1);
    lua_pop(L, 1);

    return stateIndex;
}

void setExceptionStackTrace(lua_State *L, JNIEnv *javaEnv) {
    const char *ret;
    jmethodID method;
    jstring str;
    lua_Number stateIndex;
    int is_ok = 1;

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        is_ok = 0;
    } else {
        lua_getfield(L, -1, "traceback");
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 2);
            is_ok = 0;
        } else {
            lua_pushstring(L, "");  /* pass error message */
            lua_pushinteger(L, 2);  /* skip this function and traceback */
            lua_call(L, 2, 1);
            if (!lua_isstring(L, -1)) {
                lua_pop(L, 1);
                is_ok = 0;
            } else {
                ret = lua_tostring(L, -1);
            }
        }
    }

    method = (*javaEnv)->GetStaticMethodID(javaEnv, jlua_api_class, "setExceptionStackTrace",
                                           "(ILjava/lang/String;)V");

    if (is_ok) {
        str = (*javaEnv)->NewStringUTF(javaEnv, ret);
    } else {
        str = NULL;
    }

    (*javaEnv)->CallStaticVoidMethod(javaEnv, jlua_api_class, method,
                                                 (jint) stateIndex, str);

    if (is_ok)
        (*javaEnv)->DeleteLocalRef(javaEnv, str);

    lua_pop(L, 1);
}

void handleException(lua_State *L, JNIEnv *javaEnv){
    jthrowable exp;
    const char *stackTrace;

    exp = (*javaEnv)->ExceptionOccurred(javaEnv);

    /* Handles exception */
    if (exp != NULL) {

        (*javaEnv)->ExceptionClear(javaEnv);

        setExceptionStackTrace(L, javaEnv);

        pushJavaObject(L, exp, 0);

        lua_error(L);
    }
}

/***************************************************************************
*
*  Function: objectIndex
*  ****/

int objectIndex(lua_State *L) {
    lua_Number stateIndex;
    const char *key;
    jmethodID method;
    jint checkField;
    jobject *obj;
    jstring str;
    JNIEnv *javaEnv;

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    if (!lua_isstring(L, -1)) {
        lua_pushstring(L, "Invalid object index. Must be string.");
        lua_error(L);
    }

    key = lua_tostring(L, -1);

    if (!isJavaObject(L, 1)) {
        lua_pushstring(L, "Not a valid Java Object.");
        lua_error(L);
    }

    javaEnv = getEnvFromStateCheck(L);

    obj = (jobject *) lua_touserdata(L, 1);

    method = (*javaEnv)->GetStaticMethodID(javaEnv, jlua_api_class, "checkField",
                                           "(ILjava/lang/Object;Ljava/lang/String;)I");

    str = (*javaEnv)->NewStringUTF(javaEnv, key);

    checkField = (*javaEnv)->CallStaticIntMethod(javaEnv, jlua_api_class, method,
                                                 (jint) stateIndex, *obj, str);


    (*javaEnv)->DeleteLocalRef(javaEnv, str);

    handleException(L, javaEnv);

    if (checkField != 0) {
        return checkField;
    }

    lua_pushstring(L, key);
    lua_pushcclosure(L, &objectIndexReturn, 1);

    return 1;
}


/***************************************************************************
*
*  Function: objectIndexReturn
*  ****/

int objectIndexReturn(lua_State *L) {
    lua_Number stateIndex;
    jobject *pObject;
    jmethodID method;
    const char *methodName;
    jint ret;
    jstring str;
    JNIEnv *javaEnv;

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    if (!isJavaObject(L, 1)) {
        lua_pushstring(L, "Not a valid OO function call.");
        lua_error(L);
    }

    methodName = lua_tostring(L, lua_upvalueindex(1));


    /* Gets the object reference */
    pObject = (jobject *) lua_touserdata(L, 1);

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    /* Gets method */
    method = (*javaEnv)->GetStaticMethodID(javaEnv, jlua_api_class, "objectIndex",
                                           "(ILjava/lang/Object;Ljava/lang/String;)I");

    str = (*javaEnv)->NewStringUTF(javaEnv, methodName);

    ret = (*javaEnv)->CallStaticIntMethod(javaEnv, jlua_api_class, method, (jint) stateIndex,
                                          *pObject, str);

    (*javaEnv)->DeleteLocalRef(javaEnv, str);

    handleException(L, javaEnv);

    return ret;
}


/***************************************************************************
*
*  Function: objectNewIndex
*  ****/

int objectNewIndex(lua_State *L) {
    lua_Number stateIndex;
    jobject *obj;
    jmethodID method;
    const char *fieldName;
    jstring str;
    jint ret;
    JNIEnv *javaEnv;

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    if (!isJavaObject(L, 1)) {
        lua_pushstring(L, "Not a valid java class.");
        lua_error(L);
    }

    /* Gets the field Name */

    if (!lua_isstring(L, 2)) {
        lua_pushstring(L, "Not a valid field call.");
        lua_error(L);
    }

    fieldName = lua_tostring(L, 2);

    /* Gets the object reference */
    obj = (jobject *) lua_touserdata(L, 1);

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    method = (*javaEnv)->GetStaticMethodID(javaEnv, jlua_api_class, "objectNewIndex",
                                           "(ILjava/lang/Object;Ljava/lang/String;)I");

    str = (*javaEnv)->NewStringUTF(javaEnv, fieldName);

    ret = (*javaEnv)->CallStaticIntMethod(javaEnv, jlua_api_class, method, (jint) stateIndex,
                                          *obj, str);

    (*javaEnv)->DeleteLocalRef(javaEnv, str);

    handleException(L, javaEnv);


    return ret;
}

/***************************************************************************
*
*  Function: arrayIndex
*  ****/

int arrayIndex(lua_State *L) {
    lua_Number stateIndex;
    lua_Integer key;
    jmethodID method;
    jint ret;
    jobject *obj;
    JNIEnv *javaEnv;

    /* Can index as number or string */
    if (!lua_isnumber(L, -1) && !lua_isstring(L, -1)) {
        lua_pushstring(L, "Invalid object index. Must be integer or string.");
        lua_error(L);
    }

    /* Important! If the index is not a number, behave as normal Java object */
    if (!lua_isnumber(L, -1)) {
        return objectIndex(L);
    }

    /* Index is number */

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    // Array index
    key = lua_tointeger(L, -1);

    if (!isJavaObject(L, 1)) {
        lua_pushstring(L, "Not a valid Java Object.");
        lua_error(L);
    }

    javaEnv = getEnvFromStateCheck(L);

    obj = (jobject *) lua_touserdata(L, 1);

    method = (*javaEnv)->GetStaticMethodID(javaEnv, jlua_api_class, "arrayIndex",
                                           "(ILjava/lang/Object;I)I");

    ret = (*javaEnv)->CallStaticIntMethod(javaEnv, jlua_api_class, method,
                                          (jint) stateIndex, *obj, (jlong) key);

    handleException(L, javaEnv);

    return ret;
}


/***************************************************************************
*
*  Function: arrayNewIndex
*  ****/

int arrayNewIndex(lua_State *L) {
    lua_Number stateIndex;
    jobject *obj;
    jmethodID method;
    lua_Integer key;
    jint ret;
    JNIEnv *javaEnv;

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    if (!isJavaObject(L, 1)) {
        lua_pushstring(L, "Not a valid java class.");
        lua_error(L);
    }

    /* Gets the field Name */

    if (!lua_isnumber(L, 2)) {
        lua_pushstring(L, "Not a valid array index.");
        lua_error(L);
    }

    key = lua_tointeger(L, 2);

    /* Gets the object reference */
    obj = (jobject *) lua_touserdata(L, 1);

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    method = (*javaEnv)->GetStaticMethodID(javaEnv, jlua_api_class, "arrayNewIndex",
                                           "(ILjava/lang/Object;I)I");

    ret = (*javaEnv)->CallStaticIntMethod(javaEnv, jlua_api_class, method, (jint) stateIndex,
                                          *obj, (jint) key);

    handleException(L, javaEnv);

    return ret;
}

/***************************************************************************
*
*  Function: newArray
*  ****/
int newArray(lua_State *L) {
    jint ret;
    lua_Number stateIndex;
    const char *className;
    jmethodID method;
    jstring javaClassName;
    int length;
    JNIEnv *javaEnv;

    int args = lua_gettop(L);

    if (args != 2) {
        lua_pushstring(L, "Error. Function newArray expects 2 argument.");
        lua_error(L);
    }

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    /* get the string parameter */
    if (!lua_isstring(L, 1)) {
        lua_pushstring(L, "Invalid parameter type. String expected.");
        lua_error(L);
    }
    className = lua_tostring(L, 1);

    if (!lua_isnumber(L, 2)) {
        lua_pushstring(L, "Invalid parameter type. Number expected.");
        lua_error(L);
    }
    length = lua_tointeger(L, 2);

    method = (*javaEnv)->GetStaticMethodID(javaEnv, jlua_api_class, "newArray",
                                           "(ILjava/lang/String;I)I");

    javaClassName = (*javaEnv)->NewStringUTF(javaEnv, className);

    ret = (*javaEnv)->CallStaticIntMethod(javaEnv, jlua_api_class, method, (jint) stateIndex,
                                          javaClassName, length);

    (*javaEnv)->DeleteLocalRef(javaEnv, javaClassName);

    handleException(L, javaEnv);

    return ret;
}


/***************************************************************************
*
*  Function: throwObj
*  ****/
int throwObj(lua_State *L) {
    lua_Number stateIndex;
    JNIEnv *javaEnv;

    int args = lua_gettop(L);

    if (args != 1) {
        lua_pushstring(L, "Error. Function throw expects 1 argument.");
        lua_error(L);
    }

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    setExceptionStackTrace(L, javaEnv);
    lua_pushvalue(L, 1);
    lua_error(L);

    return 0;

}


/***************************************************************************
*
*  Function: tryBlock
*  ****/
int tryBlock(lua_State *L) {
    int err, err_f;

    int args = lua_gettop(L);

    if (args < 1 || args > 3) {
        lua_pushstring(L, "Error. Function try expects 1, 2 or 3 arguments.");
        lua_error(L);
    }


    if (args == 1 && !lua_isfunction(L, 1)) {
        lua_pushstring(L, "Invalid Argument types. Expected (function()).");
        lua_error(L);
    } else if (args == 2 && (!lua_isfunction(L, 1) || !lua_isfunction(L, 2))) {
        lua_pushstring(L, "Invalid Argument types. Expected (function(), function(e)).");
        lua_error(L);
    } else if (args == 3 && (!lua_isfunction(L, 1) || !lua_isfunction(L, 3) ||
                             !(lua_isnil(L, 2) || lua_isfunction(L, 2)))) {
        lua_pushstring(L,
                       "Invalid Argument types. Expected (function(), function(e), function()) or (function(), nil, function()).");
        lua_error(L);
    }

    lua_pushvalue(L, 1); // push try function
    err = lua_pcall(L, 0, 0, 0);
    if (err != 0) {
        if (lua_isfunction(L, 2)){
            lua_pushvalue(L, 2); // push catch function
            lua_insert(L, -2); // push exception
            err = lua_pcall(L, 1, 0, 0);
        }
    }
    if (lua_isfunction(L, 3)){
        lua_pushvalue(L, 3); // push finally function
        err_f = lua_pcall(L, 0, 0, 0);
        if (err_f) { // ignore finally exception if an exception occurred
            if (err) {
                lua_pop(L, 1);
            } else {
                err = err_f;
            }
        }
    }
    if (err)
        lua_error(L);

    return 0;

}

/***************************************************************************
*
*  Function: gc
*  ****/

int gc(lua_State *L) {
    jobject *pObj;
    JNIEnv *javaEnv;

    if (!isJavaObject(L, 1)) {
        return 0;
    }

    pObj = (jobject *) lua_touserdata(L, 1);

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    (*javaEnv)->DeleteGlobalRef(javaEnv, *pObj);

    return 0;
}


/***************************************************************************
*
*  Function: javaGetClass
*  ****/

int javaGetClass(lua_State *L) {
    int top;
    jmethodID method;
    const char *className;
    jstring javaClassName;
    jobject classInstance;
    JNIEnv *javaEnv;

    top = lua_gettop(L);

    if (top != 1) {
        luaL_error(L, "Error. Function javaGetClass received %d arguments, expected 1.", top);
    }

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    /* get the string parameter */
    if (!lua_isstring(L, 1)) {
        lua_pushstring(L, "Invalid parameter type. String expected.");
        lua_error(L);
    }
    className = lua_tostring(L, 1);

    method = (*javaEnv)->GetStaticMethodID(javaEnv, java_lang_class, "forName",
                                           "(Ljava/lang/String;)Ljava/lang/Class;");

    javaClassName = (*javaEnv)->NewStringUTF(javaEnv, className);

    classInstance = (*javaEnv)->CallStaticObjectMethod(javaEnv, java_lang_class,
                                                       method, javaClassName);

    (*javaEnv)->DeleteLocalRef(javaEnv, javaClassName);

    handleException(L, javaEnv);

    /* pushes new object into lua stack */

    return pushJavaObject(L, classInstance, 1);
}


/***************************************************************************
*
*  Function: createProxy
*  ****/
int createProxy(lua_State *L) {
    jint ret;
    lua_Number stateIndex;
    const char *impl;
    jmethodID method;
    jstring str;
    JNIEnv *javaEnv;

    if (lua_gettop(L) != 2) {
        lua_pushstring(L, "Error. Function createProxy expects 2 arguments.");
        lua_error(L);
    }

    /* Gets the luaState index */
    stateIndex = getLuaStateIndex(L);

    if (!lua_isstring(L, 1) || !lua_istable(L, 2)) {
        lua_pushstring(L, "Invalid Argument types. Expected (string, table).");
        lua_error(L);
    }

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    method = (*javaEnv)->GetStaticMethodID(javaEnv, jlua_api_class, "createProxyObject",
                                           "(ILjava/lang/String;)I");

    impl = lua_tostring(L, 1);

    str = (*javaEnv)->NewStringUTF(javaEnv, impl);

    ret = (*javaEnv)->CallStaticIntMethod(javaEnv, jlua_api_class, method, (jint) stateIndex, str);

    (*javaEnv)->DeleteLocalRef(javaEnv, str);

    handleException(L, javaEnv);

    return ret;
}

/***************************************************************************
*
*  Function: javaLoadLib
*  ****/
#ifdef ENABLELOADLIB
int javaLoadLib( lua_State * L )
{
   jint ret;
   int top;
   const char * className, * methodName;
   lua_Number stateIndex;
   jmethodID method;
   jstring javaClassName , javaMethodName;
   JNIEnv * javaEnv;

   top = lua_gettop( L );

   if ( top != 2 )
   {
      lua_pushstring( L , "Error. Invalid number of parameters." );
      lua_error( L );
   }

   /* Gets the luaState index */
   lua_pushstring( L , JLUASTATEINDEX );
   lua_rawget( L , LUA_REGISTRYINDEX );

   if ( !lua_isnumber( L , -1 ) )
   {
      lua_pushstring( L , "Impossible to identify luaState id." );
      lua_error( L );
   }

   stateIndex = lua_tonumber( L , -1 );
   lua_pop( L , 1 );


   if ( !lua_isstring( L , 1 ) || !lua_isstring( L , 2 ) )
   {
      lua_pushstring( L , "Invalid parameter. Strings expected." );
      lua_error( L );
   }

   className  = lua_tostring( L , 1 );
   methodName = lua_tostring( L , 2 );

   /* Gets the JNI Environment */
   javaEnv = getEnvFromState( L );
   if ( javaEnv == NULL )
   {
      lua_pushstring( L , "Invalid JNI Environment." );
      lua_error( L );
   }

   method = ( *javaEnv )->GetStaticMethodID( javaEnv , jlua_api_class , "javaLoadLib" ,
                                             "(ILjava/lang/String;Ljava/lang/String;)I" );

   javaClassName  = ( *javaEnv )->NewStringUTF( javaEnv , className );
   javaMethodName = ( *javaEnv )->NewStringUTF( javaEnv , methodName );

   ret = ( *javaEnv )->CallStaticIntMethod( javaEnv , jlua_api_class , method, (jint)stateIndex ,
                                            javaClassName , javaMethodName );

   ( *javaEnv )->DeleteLocalRef( javaEnv , javaClassName );
   ( *javaEnv )->DeleteLocalRef( javaEnv , javaMethodName );

   handleException(L, javaEnv);

   return ret;
}
#endif


/***************************************************************************
*
*  Function: checkJavaObj
*  ****/

void checkJavaObj(lua_State *L, jobject javaObject) {
    jmethodID method;
    lua_Number stateIndex;

    /* Gets the JNI Environment */
    JNIEnv *javaEnv = getEnvFromStateCheck(L);

    /* Gets the luaState index */
    lua_pushstring( L , JLUASTATEINDEX );
    lua_rawget( L , LUA_REGISTRYINDEX );

    if ( !lua_isnumber( L , -1 ) )
    {
        lua_pushstring( L , "Impossible to identify luaState id." );
        lua_error( L );
    }

    stateIndex = lua_tonumber( L , -1 );
    lua_pop( L , 1 );

    method = ( *javaEnv )->GetStaticMethodID( javaEnv , jlua_api_class , "checkJavaObj" ,
                                              "(ILjava/lang/Object;)V" );

    ( *javaEnv )->CallStaticVoidMethod( javaEnv , jlua_api_class , method, (jint)stateIndex ,
                                        javaObject );

    handleException(L, javaEnv);

}


/***************************************************************************
*
*  Function: pushJavaObject
*  ****/

int pushJavaObject(lua_State *L, jobject javaObject, int checkObject) {
    jobject *userData, globalRef;

    if (checkObject)
        checkJavaObj(L, javaObject);

    /* Gets the JNI Environment */
    JNIEnv *javaEnv = getEnvFromStateCheck(L);

    globalRef = (*javaEnv)->NewGlobalRef(javaEnv, javaObject);

    userData = (jobject *) lua_newuserdata(L, sizeof(jobject));
    *userData = globalRef;

    /* Creates metatable */
    lua_newtable(L);

    /* pushes the __index metamethod */
    lua_pushstring(L, LUAINDEXMETAMETHODTAG);
    lua_pushcfunction(L, &objectIndex);
    lua_rawset(L, -3);

    /* pushes the __newindex metamethod */
    lua_pushstring(L, LUANEWINDEXMETAMETHODTAG);
    lua_pushcfunction(L, &objectNewIndex);
    lua_rawset(L, -3);

    /* pushes the __gc metamethod */
    lua_pushstring(L, LUAGCMETAMETHODTAG);
    lua_pushcfunction(L, &gc);
    lua_rawset(L, -3);

    /* Is Java Object boolean */
    lua_pushstring(L, JLUAOBJECTIND);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3);

    if (lua_setmetatable(L, -2) == 0) {
        (*javaEnv)->DeleteGlobalRef(javaEnv, globalRef);
        lua_pushstring(L, "Cannot create proxy to java object.");
        lua_error(L);
    }

    return 1;
}


/***************************************************************************
*
*  Function: pushJavaArray
*  ****/

int pushJavaArray(lua_State *L, jobject javaObject) {
    jobject *userData, globalRef;

    /* Gets the JNI Environment */
    JNIEnv *javaEnv = getEnvFromStateCheck(L);

    globalRef = (*javaEnv)->NewGlobalRef(javaEnv, javaObject);

    userData = (jobject *) lua_newuserdata(L, sizeof(jobject));
    *userData = globalRef;

    /* Creates metatable */
    lua_newtable(L);

    /* pushes the __index metamethod */
    lua_pushstring(L, LUAINDEXMETAMETHODTAG);
    lua_pushcfunction(L, &arrayIndex);
    lua_rawset(L, -3);

    /* pushes the __newindex metamethod */
    lua_pushstring(L, LUANEWINDEXMETAMETHODTAG);
    lua_pushcfunction(L, &arrayNewIndex);
    lua_rawset(L, -3);

    /* pushes the __gc metamethod */
    lua_pushstring(L, LUAGCMETAMETHODTAG);
    lua_pushcfunction(L, &gc);
    lua_rawset(L, -3);

    /* Is Java Object boolean */
    lua_pushstring(L, JLUAOBJECTIND);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3);

    if (lua_setmetatable(L, -2) == 0) {
        (*javaEnv)->DeleteGlobalRef(javaEnv, globalRef);
        lua_pushstring(L, "Cannot create proxy to java object.");
        lua_error(L);
    }

    return 1;
}


/***************************************************************************
*
*  Function: isJavaObject
*  ****/

int isJavaObject(lua_State *L, int idx) {
    if (!lua_isuserdata(L, idx))
        return 0;

    if (lua_getmetatable(L, idx) == 0)
        return 0;

    lua_pushstring(L, JLUAOBJECTIND);
    lua_rawget(L, -2);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
        return 0;
    }
    lua_pop(L, 2);
    return 1;
}


/***************************************************************************
*
*  Function: getStateFromCPtr
*  ****/

lua_State *getStateFromCPtr(JNIEnv *env, jobject cptr) {
    lua_State *L;

    jclass classPtr = (*env)->GetObjectClass(env, cptr);
    jfieldID CPtr_peer_ID = (*env)->GetFieldID(env, classPtr, "peer", "J");
    jbyte *peer = (jbyte *) (*env)->GetLongField(env, cptr, CPtr_peer_ID);

    L = (lua_State *) peer;

    pushJNIEnv(env, L);

    return L;
}


/***************************************************************************
*
*  Function: jluaFunctionCall
*  ****/

int jluaFunctionCall(lua_State *L) {
    jobject *obj;
    int ret;
    JNIEnv *javaEnv;

    if (!isJavaObject(L, 1)) {
        lua_pushstring(L, "Not a java Function.");
        lua_error(L);
    }

    obj = lua_touserdata(L, 1);

    /* Gets the JNI Environment */
    javaEnv = getEnvFromStateCheck(L);

    /* the Object must be an instance of the JavaFunction class */
    if ((*javaEnv)->IsInstanceOf(javaEnv, *obj, java_function_class) ==
        JNI_FALSE) {
        fprintf(stderr, "Called Java object is not a JavaFunction\n");
        return 0;
    }

    ret = (*javaEnv)->CallIntMethod(javaEnv, *obj, java_function_method);

    handleException(L, javaEnv);

    return ret;
}

/***************************************************************************
*
*  Function: getEnvFromState
*  ****/

JNIEnv *getEnvFromState(lua_State *L) {
    JavaVM **udJvm;
    jint *udJavaVersion;

    lua_pushstring(L, JLUAJVMTAG);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isuserdata(L, -1)) {
        lua_pop(L, 1);
        return NULL;
    }
    udJvm = (JavaVM **) lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, JLUAJVERSIONTAG);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isuserdata(L, -1)) {
        lua_pop(L, 1);
        return NULL;
    }
    udJavaVersion = (jint *) lua_touserdata(L, -1);
    lua_pop(L, 1);


    JNIEnv *g_env = NULL;
    int getEnvStat = (**udJvm)->GetEnv(*udJvm, (void **) &g_env, *udJavaVersion);
    if (getEnvStat == JNI_EDETACHED) {
        if ((**udJvm)->AttachCurrentThread(*udJvm, &g_env, NULL) != 0)
            return NULL;
    } else if (getEnvStat == JNI_OK) {
        return g_env;
    }

    return NULL;
}

/***************************************************************************
*
*  Function: getEnvFromStateCheck
*  ****/

JNIEnv *getEnvFromStateCheck(lua_State *L) {
    JNIEnv *javaEnv;

    /* Gets the JNI Environment */
    javaEnv = getEnvFromState(L);
    if (javaEnv == NULL) {
        lua_pushstring(L, "Invalid JNI Environment.");
        lua_error(L);
    }

    return javaEnv;
}


/***************************************************************************
*
*  Function: pushJNIEnv
*  ****/

void pushJNIEnv(JNIEnv *env, lua_State *L) {
    JavaVM **udJvm;
    jint *udJavaVersion;


    JavaVM *jvm = NULL;
    (*env)->GetJavaVM(env, &jvm);

    jint javaVersion = (*env)->GetVersion(env);

    udJvm = (JavaVM **) lua_newuserdata(L, sizeof(JavaVM *));
    *udJvm = jvm;

    lua_pushstring(L, JLUAJVMTAG);
    lua_insert(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);

    udJavaVersion = (jint *) lua_newuserdata(L, sizeof(jint));
    *udJavaVersion = javaVersion;

    lua_pushstring(L, JLUAJVERSIONTAG);
    lua_insert(L, -2);
    lua_rawset(L, LUA_REGISTRYINDEX);

}


