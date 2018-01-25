#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include "jlua.h"


jclass throwable_class = NULL;
jmethodID get_message_method = NULL;
jclass java_function_class = NULL;
jmethodID java_function_method = NULL;
jclass jlua_api_class = NULL;
jclass java_lang_class = NULL;


/*
** Assumes the table is on top of the stack.
*/
static void set_info(lua_State *L) {
    lua_pushliteral (L, "_COPYRIGHT");
    lua_pushliteral (L, "Copyright (C) 2003-2007 Kepler Project");
    lua_settable(L, -3);
    lua_pushliteral (L, "_DESCRIPTION");
    lua_pushliteral (L, "JLua is a script tool for Java");
    lua_settable(L, -3);
    lua_pushliteral (L, "_NAME");
    lua_pushliteral (L, "JLua");
    lua_settable(L, -3);
    lua_pushliteral (L, "_VERSION");
    lua_pushliteral (L, "1.0");
    lua_settable(L, -3);
}

/**************************** JNI FUNCTIONS ****************************/

/************************************************************************
*   JNI Called function
*      JLua API Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState_jlua_1open
        (JNIEnv *env, jobject jobj, jobject cptr, jint stateId) {
    lua_State *L;

    jclass tempClass;

    L = getStateFromCPtr(env, cptr);

    lua_pushstring(L, JLUASTATEINDEX);
    lua_pushnumber(L, (lua_Number) stateId);
    lua_settable(L, LUA_REGISTRYINDEX);


    lua_newtable(L);

    lua_setglobal(L, "jlua");

    lua_getglobal(L, "jlua");

    set_info(L);

    lua_pushstring(L, "getClass");
    lua_pushcfunction(L, &javaGetClass);
    lua_settable(L, -3);

#ifdef ENABLELOADLIB
    lua_pushstring(L, "loadLib");
    lua_pushcfunction(L, &javaLoadLib);
    lua_settable(L, -3);
#endif

    lua_pushstring(L, "createProxy");
    lua_pushcfunction(L, &createProxy);
    lua_settable(L, -3);

    lua_pushstring(L, "newArray");
    lua_pushcfunction(L, &newArray);
    lua_settable(L, -3);

    lua_pushstring(L, "try");
    lua_pushcfunction(L, &tryBlock);
    lua_settable(L, -3);

    lua_pushstring(L, "throw");
    lua_pushcfunction(L, &throwObj);
    lua_settable(L, -3);

    lua_pop(L, 1);

    if (jlua_api_class == NULL) {
        tempClass = (*env)->FindClass(env, "org/jlua/JLuaAPI");

        if (tempClass == NULL) {
            fprintf(stderr, "Could not find JLuaAPI class\n");
            exit(1);
        }

        if ((jlua_api_class = (*env)->NewGlobalRef(env, tempClass)) == NULL) {
            fprintf(stderr, "Could not bind to JLuaAPI class\n");
            exit(1);
        }
    }

    if (java_function_class == NULL) {
        tempClass = (*env)->FindClass(env, "org/jlua/JavaFunction");

        if (tempClass == NULL) {
            fprintf(stderr, "Could not find JavaFunction interface\n");
            exit(1);
        }

        if ((java_function_class = (*env)->NewGlobalRef(env, tempClass)) == NULL) {
            fprintf(stderr, "Could not bind to JavaFunction interface\n");
            exit(1);
        }
    }

    if (java_function_method == NULL) {
        java_function_method = (*env)->GetMethodID(env, java_function_class, "execute", "()I");
        if (!java_function_method) {
            fprintf(stderr, "Could not find <execute> method in JavaFunction\n");
            exit(1);
        }
    }

    if (throwable_class == NULL) {
        tempClass = (*env)->FindClass(env, "java/lang/Throwable");

        if (tempClass == NULL) {
            fprintf(stderr, "Error. Couldn't bind java class java.lang.Throwable\n");
            exit(1);
        }

        throwable_class = (*env)->NewGlobalRef(env, tempClass);

        if (throwable_class == NULL) {
            fprintf(stderr, "Error. Couldn't bind java class java.lang.Throwable\n");
            exit(1);
        }
    }

    if (get_message_method == NULL) {
        get_message_method = (*env)->GetMethodID(env, throwable_class, "getMessage",
                                                 "()Ljava/lang/String;");

        if (get_message_method == NULL) {
            fprintf(stderr, "Could not find <getMessage> method in java.lang.Throwable\n");
            exit(1);
        }
    }

    if (java_lang_class == NULL) {
        tempClass = (*env)->FindClass(env, "java/lang/Class");

        if (tempClass == NULL) {
            fprintf(stderr, "Error. Coundn't bind java class java.lang.Class\n");
            exit(1);
        }

        java_lang_class = (*env)->NewGlobalRef(env, tempClass);

        if (java_lang_class == NULL) {
            fprintf(stderr, "Error. Couldn't bind java class java.lang.Throwable\n");
            exit(1);
        }
    }

    pushJNIEnv(env, L);
}

/************************************************************************
*   JNI Called function
*      JLua API Function
************************************************************************/

JNIEXPORT jobject JNICALL Java_org_jlua_LuaState__1getObjectFromUserdata
        (JNIEnv *env, jobject jobj, jobject cptr, jint index) {
/* Get luastate */
    lua_State *L = getStateFromCPtr(env, cptr);
    jobject *obj;

    if (!isJavaObject(L, index)) {
        (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/Exception"),
                         "Index is not a java object");
        return NULL;
    }

    obj = (jobject *) lua_touserdata(L, index);

    return *obj;
}


/************************************************************************
*   JNI Called function
*      JLua API Function
************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_jlua_LuaState__1isObject
        (JNIEnv *env, jobject jobj, jobject cptr, jint index) {
/* Get luastate */
    lua_State *L = getStateFromCPtr(env, cptr);

    return (isJavaObject(L, index) ? JNI_TRUE : JNI_FALSE);
}


/************************************************************************
*   JNI Called function
*      JLua API Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushJavaObject
        (JNIEnv *env, jobject jobj, jobject cptr, jobject obj) {
/* Get luastate */
    lua_State *L = getStateFromCPtr(env, cptr);

    pushJavaObject(L, obj, 0);
}


/************************************************************************
*   JNI Called function
*      JLua API Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushJavaArray
        (JNIEnv *env, jobject jobj, jobject cptr, jobject obj) {
/* Get luastate */
    lua_State *L = getStateFromCPtr(env, cptr);

    pushJavaArray(L, obj);
}


/************************************************************************
*   JNI Called function
*      JLua API Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushJavaFunction
        (JNIEnv *env, jobject jobj, jobject cptr, jobject obj) {
/* Get luastate */
    lua_State *L = getStateFromCPtr(env, cptr);

    jobject *userData, globalRef;

    globalRef = (*env)->NewGlobalRef(env, obj);

    userData = (jobject *) lua_newuserdata(L, sizeof(jobject));
    *userData = globalRef;

/* Creates metatable */
    lua_newtable(L);

/* pushes the __index metamethod */
    lua_pushstring(L, LUACALLMETAMETHODTAG);
    lua_pushcfunction(L, &jluaFunctionCall);
    lua_rawset(L, -3);

/* pusher the __gc metamethod */
    lua_pushstring(L, LUAGCMETAMETHODTAG);
    lua_pushcfunction(L, &gc);
    lua_rawset(L, -3);

    lua_pushstring(L, JLUAOBJECTIND);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3);

    if (lua_setmetatable(L, -2) == 0) {
        (*env)->ThrowNew(env, (*env)->FindClass(env, "org/jlua/LuaException"),
                         "Index is not a java object");
    }
}


/************************************************************************
*   JNI Called function
*      JLua API Function
************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_jlua_LuaState__1isJavaFunction
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
/* Get luastate */
    lua_State *L = getStateFromCPtr(env, cptr);
    jobject *obj;

    if (!isJavaObject(L, idx)) {
        return JNI_FALSE;
    }

    obj = (jobject *) lua_touserdata(L, idx);

    return (*env)->IsInstanceOf(env, *obj, java_function_class);

}


/*********************** LUA API FUNCTIONS ******************************/

/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jobject JNICALL Java_org_jlua_LuaState__1open
        (JNIEnv *env, jobject jobj) {
    lua_State *L = lua_open();

    jobject obj;
    jclass tempClass;

    tempClass = (*env)->FindClass(env, "org/jlua/CPtr");

    obj = (*env)->AllocObject(env, tempClass);
    if (obj) {
        (*env)->SetLongField(env, obj, (*env)->GetFieldID(env, tempClass, "peer", "J"), (jlong) L);
    }
    return obj;

}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openBase
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

//luaopen_base( L );
    lua_pushcfunction(L, luaopen_base);
    lua_pushstring(L, "");
    lua_call(L, 1, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openTable
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

//luaopen_table( L );
    lua_pushcfunction(L, luaopen_table);
    lua_pushstring(L, LUA_TABLIBNAME);
    lua_call(L, 1, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openIo
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

//luaopen_io( L );
    lua_pushcfunction(L, luaopen_io);
    lua_pushstring(L, LUA_IOLIBNAME);
    lua_call(L, 1, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openOs
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

//luaopen_os( L );
    lua_pushcfunction(L, luaopen_os);
    lua_pushstring(L, LUA_OSLIBNAME);
    lua_call(L, 1, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openString
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

//luaopen_string( L );
    lua_pushcfunction(L, luaopen_string);
    lua_pushstring(L, LUA_STRLIBNAME);
    lua_call(L, 1, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openMath
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

//luaopen_math( L );
    lua_pushcfunction(L, luaopen_math);
    lua_pushstring(L, LUA_MATHLIBNAME);
    lua_call(L, 1, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openDebug
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

//luaopen_debug( L );
    lua_pushcfunction(L, luaopen_debug);
    lua_pushstring(L, LUA_DBLIBNAME);
    lua_call(L, 1, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openPackage
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

//luaopen_package( L );
    lua_pushcfunction(L, luaopen_package);
    lua_pushstring(L, LUA_LOADLIBNAME);
    lua_call(L, 1, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1openLibs
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

    luaL_openlibs(L);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1close
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_close(L);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jobject JNICALL Java_org_jlua_LuaState__1newthread
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);
    lua_State *newThread;

    jobject obj;
    jclass tempClass;

    newThread = lua_newthread(L);

    tempClass = (*env)->FindClass(env, "org/jlua/CPtr");
    obj = (*env)->AllocObject(env, tempClass);
    if (obj) {
        (*env)->SetLongField(env, obj, (*env)->GetFieldID(env, tempClass,
                                                          "peer", "J"), (jlong) L);
    }

    return obj;

}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1getTop
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_gettop(L);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1setTop
        (JNIEnv *env, jobject jobj, jobject cptr, jint top) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_settop(L, (int) top);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushValue
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_pushvalue(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1remove
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_remove(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1insert
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_insert(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1replace
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_replace(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1checkStack
        (JNIEnv *env, jobject jobj, jobject cptr, jint sz) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_checkstack(L, (int) sz);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1xmove
        (JNIEnv *env, jobject jobj, jobject from, jobject to, jint n) {
    lua_State *fr = getStateFromCPtr(env, from);
    lua_State *t = getStateFromCPtr(env, to);

    lua_xmove(fr, t, (int) n);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isNumber
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_isnumber(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isString
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_isstring(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isFunction
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_isfunction(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isCFunction
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_iscfunction(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isUserdata
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_isuserdata(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isTable
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_istable(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isBoolean
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_isboolean(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isNil
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_isnil(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isNone
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_isnone(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1isNoneOrNil
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_isnoneornil(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1type
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_type(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jstring JNICALL Java_org_jlua_LuaState__1typeName
        (JNIEnv *env, jobject jobj, jobject cptr, jint tp) {
    lua_State *L = getStateFromCPtr(env, cptr);

    const char *name = lua_typename(L, tp);

    return (*env)->NewStringUTF(env, name);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1equal
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx1, jint idx2) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_equal(L, idx1, idx2);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1rawequal
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx1, jint idx2) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_rawequal(L, idx1, idx2);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1lessthan
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx1, jint idx2) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_lessthan(L, idx1, idx2);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jdouble JNICALL Java_org_jlua_LuaState__1toNumber
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jdouble) lua_tonumber(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1toInteger
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_tointeger(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1toBoolean
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_toboolean(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jstring JNICALL Java_org_jlua_LuaState__1toString
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    const char *str = lua_tostring(L, idx);

    return (*env)->NewStringUTF(env, str);
}

/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jbyteArray JNICALL Java_org_jlua_LuaState__1toByteArray
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    jbyte *str = (jbyte *) lua_tostring(L, idx);
    jint str_len = (jint) lua_strlen(L, idx);

    jbyteArray ret = (*env)->NewByteArray(env, str_len);
    (*env)->SetByteArrayRegion(env, ret, 0, str_len, str);

    return ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1strlen
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_strlen(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1objlen
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_objlen(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jobject JNICALL Java_org_jlua_LuaState__1toThread
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L, *thr;

    jobject obj;
    jclass tempClass;

    L = getStateFromCPtr(env, cptr);

    thr = lua_tothread(L, (int) idx);

    tempClass = (*env)->FindClass(env, "org/jlua/CPtr");

    obj = (*env)->AllocObject(env, tempClass);
    if (obj) {
        (*env)->SetLongField(env, obj, (*env)->GetFieldID(env, tempClass, "peer", "J"),
                             (jlong) thr);
    }
    return obj;

}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushNil
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_pushnil(L);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushNumber
        (JNIEnv *env, jobject jobj, jobject cptr, jdouble number) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_pushnumber(L, (lua_Number) number);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushInteger
        (JNIEnv *env, jobject jobj, jobject cptr, jint number) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_pushinteger(L, (lua_Integer) number);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushString__Lorg_jlua_CPtr_2Ljava_lang_String_2
        (JNIEnv *env, jobject jobj, jobject cptr, jstring str) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *uniStr;

    uniStr = (*env)->GetStringUTFChars(env, str, NULL);

    if (uniStr == NULL)
        return;

    lua_pushstring(L, uniStr);

    (*env)->ReleaseStringUTFChars(env, str, uniStr);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushString__Lorg_jlua_CPtr_2_3BI
        (JNIEnv *env, jobject jobj, jobject cptr, jbyteArray bytes, jint n) {
    lua_State *L = getStateFromCPtr(env, cptr);
    char *cBytes;

    cBytes = (char *) (*env)->GetByteArrayElements(env, bytes, NULL);

    lua_pushlstring(L, cBytes, n);

    (*env)->ReleaseByteArrayElements(env, bytes, cBytes, 0);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pushBoolean
        (JNIEnv *env, jobject jobj, jobject cptr, jint jbool) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_pushboolean(L, (int) jbool);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1getTable
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_gettable(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1getField
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx, jstring k) {
    lua_State *L = getStateFromCPtr(env, cptr);

    const char *uniStr;
    uniStr = (*env)->GetStringUTFChars(env, k, NULL);

    lua_getfield(L, (int) idx, uniStr);

    (*env)->ReleaseStringUTFChars(env, k, uniStr);
}

/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1rawGet
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_rawget(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1rawGetI
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx, jint n) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_rawgeti(L, idx, n);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1createTable
        (JNIEnv *env, jobject jobj, jobject cptr, jint narr, jint nrec) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_createtable(L, (int) narr, (int) nrec);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1newTable
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_newtable(L);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1getMetaTable
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return lua_getmetatable(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1getFEnv
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_getfenv(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1setTable
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_settable(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1setField
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx, jstring k) {
    lua_State *L = getStateFromCPtr(env, cptr);

    const char *uniStr;
    uniStr = (*env)->GetStringUTFChars(env, k, NULL);

    lua_setfield(L, (int) idx, uniStr);

    (*env)->ReleaseStringUTFChars(env, k, uniStr);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1rawSet
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_rawset(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1rawSetI
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx, jint n) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_rawseti(L, idx, n);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1setMetaTable
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return lua_setmetatable(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1setFEnv
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return lua_setfenv(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1call
        (JNIEnv *env, jobject jobj, jobject cptr, jint nArgs, jint nResults) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_call(L, nArgs, nResults);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1pcall
        (JNIEnv *env, jobject jobj, jobject cptr, jint nArgs, jint nResults, jint errFunc) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_pcall(L, nArgs, nResults, errFunc);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1yield
        (JNIEnv *env, jobject jobj, jobject cptr, jint nResults) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_yield(L, nResults);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1resume
        (JNIEnv *env, jobject jobj, jobject cptr, jint nArgs) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_resume(L, nArgs);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1status
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_status(L);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1gc
        (JNIEnv *env, jobject jobj, jobject cptr, jint what, jint data) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_gc(L, what, data);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1getGcCount
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_getgccount(L);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1next
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_next(L, idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1error
        (JNIEnv *env, jobject jobj, jobject cptr) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) lua_error(L);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1concat
        (JNIEnv *env, jobject jobj, jobject cptr, jint n) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_concat(L, n);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1pop
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx) {
    lua_State *L = getStateFromCPtr(env, cptr);

    lua_pop(L, (int) idx);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1setGlobal
        (JNIEnv *env, jobject jobj, jobject cptr, jstring name) {
    lua_State *L = getStateFromCPtr(env, cptr);

    const char *str = (*env)->GetStringUTFChars(env, name, NULL);

    lua_setglobal(L, str);

    (*env)->ReleaseStringUTFChars(env, name, str);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1getGlobal
        (JNIEnv *env, jobject jobj, jobject cptr, jstring name) {
    lua_State *L = getStateFromCPtr(env, cptr);

    const char *str = (*env)->GetStringUTFChars(env, name, NULL);

    lua_getglobal(L, str);

    (*env)->ReleaseStringUTFChars(env, name, str);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LdoFile
        (JNIEnv *env, jobject jobj, jobject cptr, jstring fileName) {
    lua_State *L = getStateFromCPtr(env, cptr);

    const char *file = (*env)->GetStringUTFChars(env, fileName, NULL);

    int ret;

    ret = luaL_dofile(L, file);

    (*env)->ReleaseStringUTFChars(env, fileName, file);

    return (jint) ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LdoString
        (JNIEnv *env, jobject jobj, jobject cptr, jstring str) {
    lua_State *L = getStateFromCPtr(env, cptr);

    const char *utfStr = (*env)->GetStringUTFChars(env, str, NULL);

    int ret;

    ret = luaL_dostring(L, utfStr);

    return (jint) ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LgetMetaField
        (JNIEnv *env, jobject jobj, jobject cptr, jint obj, jstring e) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *str = (*env)->GetStringUTFChars(env, e, NULL);
    int ret;

    ret = luaL_getmetafield(L, (int) obj, str);

    (*env)->ReleaseStringUTFChars(env, e, str);

    return (jint) ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LcallMeta
        (JNIEnv *env, jobject jobj, jobject cptr, jint obj, jstring e) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *str = (*env)->GetStringUTFChars(env, e, NULL);
    int ret;

    ret = luaL_callmeta(L, (int) obj, str);

    (*env)->ReleaseStringUTFChars(env, e, str);

    return (jint) ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1Ltyperror
        (JNIEnv *env, jobject jobj, jobject cptr, jint nArg, jstring tName) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *name = (*env)->GetStringUTFChars(env, tName, NULL);
    int ret;

    ret = luaL_typerror(L, (int) nArg, name);

    (*env)->ReleaseStringUTFChars(env, tName, name);

    return (jint) ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LargError
        (JNIEnv *env, jobject jobj, jobject cptr, jint numArg, jstring extraMsg) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *msg = (*env)->GetStringUTFChars(env, extraMsg, NULL);
    int ret;

    ret = luaL_argerror(L, (int) numArg, msg);

    (*env)->ReleaseStringUTFChars(env, extraMsg, msg);

    return (jint) ret;;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jstring JNICALL Java_org_jlua_LuaState__1LcheckString
        (JNIEnv *env, jobject jobj, jobject cptr, jint numArg) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *res;

    res = luaL_checkstring(L, (int) numArg);

    return (*env)->NewStringUTF(env, res);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jstring JNICALL Java_org_jlua_LuaState__1LoptString
        (JNIEnv *env, jobject jobj, jobject cptr, jint numArg, jstring def) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *d = (*env)->GetStringUTFChars(env, def, NULL);
    const char *res;
    jstring ret;

    res = luaL_optstring(L, (int) numArg, d);

    ret = (*env)->NewStringUTF(env, res);

    (*env)->ReleaseStringUTFChars(env, def, d);

    return ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jdouble JNICALL Java_org_jlua_LuaState__1LcheckNumber
        (JNIEnv *env, jobject jobj, jobject cptr, jint numArg) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jdouble) luaL_checknumber(L, (int) numArg);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jdouble JNICALL Java_org_jlua_LuaState__1LoptNumber
        (JNIEnv *env, jobject jobj, jobject cptr, jint numArg, jdouble def) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jdouble) luaL_optnumber(L, (int) numArg, (lua_Number) def);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LcheckInteger
        (JNIEnv *env, jobject jobj, jobject cptr, jint numArg) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) luaL_checkinteger(L, (int) numArg);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LoptInteger
        (JNIEnv *env, jobject jobj, jobject cptr, jint numArg, jint def) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) luaL_optinteger(L, (int) numArg, (lua_Integer) def);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1LcheckStack
        (JNIEnv *env, jobject jobj, jobject cptr, jint sz, jstring msg) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *m = (*env)->GetStringUTFChars(env, msg, NULL);

    luaL_checkstack(L, (int) sz, m);

    (*env)->ReleaseStringUTFChars(env, msg, m);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1LcheckType
        (JNIEnv *env, jobject jobj, jobject cptr, jint nArg, jint t) {
    lua_State *L = getStateFromCPtr(env, cptr);

    luaL_checktype(L, (int) nArg, (int) t);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1LcheckAny
        (JNIEnv *env, jobject jobj, jobject cptr, jint nArg) {
    lua_State *L = getStateFromCPtr(env, cptr);

    luaL_checkany(L, (int) nArg);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LnewMetatable
        (JNIEnv *env, jobject jobj, jobject cptr, jstring tName) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *name = (*env)->GetStringUTFChars(env, tName, NULL);
    int ret;

    ret = luaL_newmetatable(L, name);

    (*env)->ReleaseStringUTFChars(env, tName, name);

    return (jint) ret;;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1LgetMetatable
        (JNIEnv *env, jobject jobj, jobject cptr, jstring tName) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *name = (*env)->GetStringUTFChars(env, tName, NULL);

    luaL_getmetatable(L, name);

    (*env)->ReleaseStringUTFChars(env, tName, name);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1Lwhere
        (JNIEnv *env, jobject jobj, jobject cptr, jint lvl) {
    lua_State *L = getStateFromCPtr(env, cptr);

    luaL_where(L, (int) lvl);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1Lref
        (JNIEnv *env, jobject jobj, jobject cptr, jint t) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) luaL_ref(L, (int) t);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1LunRef
        (JNIEnv *env, jobject jobj, jobject cptr, jint t, jint ref) {
    lua_State *L = getStateFromCPtr(env, cptr);

    luaL_unref(L, (int) t, (int) ref);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LgetN
        (JNIEnv *env, jobject jobj, jobject cptr, jint t) {
    lua_State *L = getStateFromCPtr(env, cptr);

    return (jint) luaL_getn(L, (int) t);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT void JNICALL Java_org_jlua_LuaState__1LsetN
        (JNIEnv *env, jobject jobj, jobject cptr, jint t, jint n) {
    lua_State *L = getStateFromCPtr(env, cptr);

    luaL_setn(L, (int) t, (int) n);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LloadFile
        (JNIEnv *env, jobject jobj, jobject cptr, jstring fileName) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *fn = (*env)->GetStringUTFChars(env, fileName, NULL);
    int ret;

    ret = luaL_loadfile(L, fn);

    (*env)->ReleaseStringUTFChars(env, fileName, fn);

    return (jint) ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LloadBuffer
        (JNIEnv *env, jobject jobj, jobject cptr, jbyteArray buff, jlong sz, jstring n) {
    lua_State *L = getStateFromCPtr(env, cptr);
    jbyte *cBuff = (*env)->GetByteArrayElements(env, buff, NULL);
    const char *name = (*env)->GetStringUTFChars(env, n, NULL);
    int ret;

    ret = luaL_loadbuffer(L, (const char *) cBuff, (int) sz, name);

    (*env)->ReleaseStringUTFChars(env, n, name);

    (*env)->ReleaseByteArrayElements(env, buff, cBuff, 0);

    return (jint) ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jint JNICALL Java_org_jlua_LuaState__1LloadString
        (JNIEnv *env, jobject jobj, jobject cptr, jstring str) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *fn = (*env)->GetStringUTFChars(env, str, NULL);
    int ret;

    ret = luaL_loadstring(L, fn);

    (*env)->ReleaseStringUTFChars(env, str, fn);

    return (jint) ret;
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jstring JNICALL Java_org_jlua_LuaState__1Lgsub
        (JNIEnv *env, jobject jobj, jobject cptr, jstring s, jstring p, jstring r) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *utS = (*env)->GetStringUTFChars(env, s, NULL);
    const char *utP = (*env)->GetStringUTFChars(env, p, NULL);
    const char *utR = (*env)->GetStringUTFChars(env, r, NULL);

    const char *sub = luaL_gsub(L, utS, utP, utR);

    (*env)->ReleaseStringUTFChars(env, s, utS);
    (*env)->ReleaseStringUTFChars(env, p, utP);
    (*env)->ReleaseStringUTFChars(env, r, utR);

    return (*env)->NewStringUTF(env, sub);
}


/************************************************************************
*   JNI Called function
*      Lua Exported Function
************************************************************************/

JNIEXPORT jstring JNICALL Java_org_jlua_LuaState__1LfindTable
        (JNIEnv *env, jobject jobj, jobject cptr, jint idx, jstring fname, jint szhint) {
    lua_State *L = getStateFromCPtr(env, cptr);
    const char *name = (*env)->GetStringUTFChars(env, fname, NULL);

    const char *sub = luaL_findtable(L, (int) idx, name, (int) szhint);

    (*env)->ReleaseStringUTFChars(env, fname, name);

    return (*env)->NewStringUTF(env, sub);
}
