package org.jlua;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;


public class LuaInvocationHandler implements InvocationHandler {
    private LuaObject obj;

    public LuaInvocationHandler(LuaObject obj) {
        this.obj = obj;
    }

    /**
     * Function called when a proxy object function is invoked.
     */
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        if (obj == null)
            return null;
        synchronized (obj.getLuaState()) {
            String methodName = method.getName();
            LuaObject func = obj.getField(methodName);

            if (func.isNil()) {
                return null;
            }

            try {
                List<Object> argsList;
                if (args == null) {
                    argsList = new LinkedList<>();
                } else {
                    argsList = new LinkedList<>(Arrays.asList(args));
                }
                argsList.add(0, obj);
                args = argsList.toArray();
            } catch (Exception e) {
                e.printStackTrace();
            }


            Class retType = method.getReturnType();
            Object ret;

            // Checks if returned type is void. if it is returns null.
            if (retType.equals(Void.class) || retType.equals(void.class)) {
                func.call(args, 0);
                ret = null;
            } else {
                ret = func.call(args, 1)[0];
                if (ret != null && ret instanceof Double) {
                    ret = LuaState.convertLuaNumber((Double) ret, retType);
                }
            }

            return ret;
        }
    }
}