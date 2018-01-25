package org.jlua;

public class JavaPackage {

    private String name;

    public JavaPackage(String name) throws LuaException {
        if (name == null || name.isEmpty())
            throw new LuaException("Iligal java package name");
        this.name = name;
    }

    public String getName() {
        return name;
    }

    @Override
    public String toString() {
        return name;
    }
}
