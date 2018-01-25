package org.jlua;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

public class Lua {

    private LuaState L = null;
    private String packagePath = "/?";

    public Lua() {
        L = LuaStateFactory.newLuaState();
        L.openLibs();
        //L.openBase();
        //L.openTable();
        //L.openString();
        //L.openMath();
        //L.openDebug();
    }

    public void release() {
        L.close();
    }

    public LuaState getLuaState() {
        return L;
    }

    public String getPackagePath() {
        return packagePath;
    }

    public void setPackagePath(String packagePath) {
        this.packagePath = packagePath;
    }

    private void handleThrows() throws Throwable {
        Throwable throwable;
        LuaObject exp = L.getLuaObject(-1);
        if (exp.isJavaObject() && exp.getObject() instanceof Throwable) {
            throwable = (Throwable) exp.getObject();
        } else {
            throwable = new LuaException(exp.toString());
        }
        L.pop(1);
        throw throwable;
    }

    public void doString(String script) throws Throwable {
        if (packagePath != null) {
            LuaObject pck = getGlobal("package");
            setTableField(pck, "path", packagePath);
        }
        int res = L.LdoString(script);
        if (res != 0)
            handleThrows();
    }

    public void doFile(String path) throws Throwable {
        if (packagePath != null) {
            String parent = new File(path).getParent();
            LuaObject pck = getGlobal("package");
            setTableField(pck, "path", packagePath + ";" + parent + "/?.lua" + ";" + parent + "/?");
        }
        int res = L.LdoFile(path);
        if (res != 0)
            handleThrows();
    }

    public JavaFunction getMethod(final Object obj, String javaName, final Class... argsTypes) throws NoSuchMethodException, LuaException {
        final Method m = obj.getClass().getMethod(javaName, argsTypes);
        JavaFunction jf = new JavaFunction(L) {
            @Override
            public int execute() throws Throwable {
                Object[] args = new Object[argsTypes.length];
                for (int i = 0; i < argsTypes.length; i++) {
                    args[i] = JLuaAPI.compareTypes(L, argsTypes[i], -argsTypes.length + i);
                }
                L.pop(argsTypes.length);
                try {
                    Object ret = m.invoke(obj, args);
                    if (!m.getReturnType().equals(Void.TYPE)) {
                        if (ret.getClass().isArray()) {
                            L.pushJavaArray(ret);
                        } else {
                            L.pushObjectValue(ret);
                        }
                    }
                } catch (IllegalAccessException e) {
                    throw new LuaException("IllegalAccessException: " + e.toString());
                } catch (InvocationTargetException e) {
                    if (e.getCause() != null)
                        throw e.getCause();
                }
                return m.getReturnType().equals(Void.TYPE) ? 0 : 1;
            }
        };
        return jf;
    }

    public void registerMethod(String luaName, final Object obj, String javaName, final Class... argsTypes) throws NoSuchMethodException, LuaException {
        JavaFunction jf = getMethod(obj, javaName, argsTypes);

        jf.register(luaName);
    }

    public JavaFunction getStaticMethod(final Class clazz, String javaName, final Class... argsTypes) throws NoSuchMethodException, LuaException {
        final Method m = clazz.getMethod(javaName, argsTypes);
        JavaFunction jf = new JavaFunction(L) {
            @Override
            public int execute() throws Throwable {
                Object[] args = new Object[argsTypes.length];
                for (int i = 0; i < argsTypes.length; i++) {
                    args[i] = JLuaAPI.compareTypes(L, argsTypes[i], -argsTypes.length + i);
                }
                L.pop(argsTypes.length);
                try {
                    Object ret = m.invoke(null, args);
                    if (!m.getReturnType().equals(Void.TYPE)) {
                        if (ret.getClass().isArray()) {
                            L.pushJavaArray(ret);
                        } else {
                            L.pushObjectValue(ret);
                        }
                    }
                } catch (IllegalAccessException e) {
                    throw new LuaException("IllegalAccessException: " + e.toString());
                } catch (InvocationTargetException e) {
                    if (e.getCause() != null)
                        throw e.getCause();
                }
                return m.getReturnType().equals(Void.TYPE) ? 0 : 1;
            }
        };

        return jf;
    }

    public void registerStaticMethod(String luaName, final Class clazz, String javaName, final Class... argsTypes) throws NoSuchMethodException, LuaException {
        JavaFunction jf = getStaticMethod(clazz, javaName, argsTypes);

        jf.register(luaName);
    }

    public String getTraceback() throws LuaException {
        LuaObject tracebackfunc = getGlobalTableField("debug", "traceback");
        L.pushObjectValue(tracebackfunc);
        int err = L.pcall(0, 1, 0);
        if (err != 0) {
            LuaObject exp = L.getLuaObject(-1);
            L.pop(1);
            throw new LuaException(exp.toString());
        }
        LuaObject ret = L.getLuaObject(-1);
        L.pop(1);
        if (ret.isString())
            return ret.getString();
        return null;
    }

    public void registerGlobal(String name, Object obj) throws LuaException {
        L.pushObjectValue(obj);
        L.setGlobal(name);
    }

    public LuaObject createTable() {
        L.newTable();
        LuaObject ret = L.getLuaObject(-1);
        L.pop(1);
        return ret;
    }

    public void setTableField(LuaObject table, Object key, Object value) throws LuaException {
        if (!table.isTable())
            throw new LuaException("First parameter is not a table");
        L.pushObjectValue(table);
        L.pushObjectValue(key);
        L.pushObjectValue(value);
        L.setTable(-3);
        L.pop(1);
    }

    public LuaObject getGlobal(String name) {
        L.getGlobal(name);
        LuaObject ret = L.getLuaObject(-1);
        L.pop(1);
        return ret;
    }

    public String getGlobalString(String name) throws LuaException {
        LuaObject obj = getGlobal(name);
        if (!obj.isString())
            throw new LuaException(name + " is not a string");
        return obj.getString();
    }

    public byte[] getGlobalByteArray(String name) throws LuaException {
        LuaObject obj = getGlobal(name);
        if (!obj.isString())
            throw new LuaException(name + " is not a string");
        return obj.getByteArray();
    }

    public double getGlobalNumber(String name) throws LuaException {
        LuaObject obj = getGlobal(name);
        if (!obj.isNumber())
            throw new LuaException(name + " is not a number");
        return obj.getNumber();
    }

    public boolean getGlobalBoolean(String name) throws LuaException {
        LuaObject obj = getGlobal(name);
        if (!obj.isBoolean())
            throw new LuaException(name + " is not a boolean");
        return obj.getBoolean();
    }

    public Object getGlobalObject(String name) throws LuaException {
        LuaObject obj = getGlobal(name);
        if (!obj.isJavaObject() && !obj.isNil())
            throw new LuaException(name + " is not an object");
        return obj.isNil() ? null : obj.getObject();
    }

    public LuaObject getGlobalTableField(String tableName, Object key) throws LuaException {
        L.getGlobal(tableName);
        if (!L.isTable(-1)) {
            L.pop(1);
            throw new LuaException(tableName + " is not a table");
        }
        L.pushObjectValue(key);
        L.getTable(-2);
        LuaObject ret = L.getLuaObject(-1);
        L.pop(2);
        return ret;
    }

    public String getGlobalTableString(String tableName, Object key) throws LuaException {
        LuaObject obj = getGlobalTableField(tableName, key);
        if (!obj.isString())
            throw new LuaException("Field " + key.toString() + " is not a string");
        return obj.getString();
    }

    public double getGlobalTableNumber(String tableName, Object key) throws LuaException {
        LuaObject obj = getGlobalTableField(tableName, key);
        if (!obj.isNumber())
            throw new LuaException("Field " + key.toString() + " is not a number");
        return obj.getNumber();
    }

    public boolean getGlobalTableBoolean(String tableName, Object key) throws LuaException {
        LuaObject obj = getGlobalTableField(tableName, key);
        if (!obj.isBoolean())
            throw new LuaException("Field " + key.toString() + " is not a boolean");
        return obj.getBoolean();
    }

    public Object getGlobalTableObject(String tableName, Object key) throws LuaException {
        LuaObject obj = getGlobalTableField(tableName, key);
        if (!obj.isJavaObject() && !obj.isNil())
            throw new LuaException("Field " + key.toString() + " is not an object");
        return obj.isNil() ? null : obj.getObject();
    }


    public List<LuaObject> callFunctionEx(String name, int numOfResults, Object... args) throws Throwable {
        L.getGlobal(name);
        for (Object arg : args)
            L.pushObjectValue(arg);
        int res = L.pcall(args.length, numOfResults, 0);
        if (res != 0)
            handleThrows();
        List<LuaObject> ret = new ArrayList<>(numOfResults);
        for (int i = -numOfResults; i < 0; i++)
            ret.add(L.getLuaObject(i));
        L.pop(numOfResults);
        return ret;
    }

    public LuaObject callFunction(String name, Object... args) throws Throwable {
        return callFunctionEx(name, 1, args).get(0);
    }

    public Object createProxy(String tableName, String interfaces) throws LuaException, ClassNotFoundException {
        LuaObject tableObj = getGlobal(tableName);
        return tableObj.createProxy(interfaces);
    }

    public void setJavaObjectChecker(JavaObjectChecker objectChecker) {
        L.setJavaObjectChecker(objectChecker);
    }

    public String getLastExceptionStackTrace() {
        return L.getLastExceptionStackTrace();
    }

    public void clearLastExceptionStackTrace() {
        L.clearLastExceptionStackTrace();
    }


}
