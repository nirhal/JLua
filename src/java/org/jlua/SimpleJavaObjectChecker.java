package org.jlua;



public class SimpleJavaObjectChecker implements JavaObjectChecker {

    private String[] allowedClasses;
    private String[] disallowedClasses;

    public SimpleJavaObjectChecker(String[] allowedClasses, String[] disallowedClasses){
        this.allowedClasses = allowedClasses;
        this.disallowedClasses = disallowedClasses;
    }

    private static boolean isItemClass(String className, String item){
        return className.equals(item) || className.startsWith(item+".") || className.startsWith(item+"$");
    }

    private static boolean isClassInArray(String[] array, Class clazz){
        while (clazz != null && !clazz.getName().equals(Object.class.getName())) {
            for (String item : array) {
                if (isItemClass(clazz.getName(), item))
                    return true;
                for (Class inter : clazz.getInterfaces()){
                    if (isItemClass(inter.getName(), item))
                        return true;
                }
            }
            clazz = clazz.getSuperclass();
        }
        return false;
    }

    @Override
    public boolean checkJavaObject(Object obj) {
        Class clazz;

        if (obj instanceof Class){
            clazz = (Class) obj;
        } else {
            clazz = obj.getClass();
        }

        if (clazz.getName().equals(Object.class.getName()))
            return true;

        return isClassInArray(allowedClasses, clazz) && !isClassInArray(disallowedClasses, clazz);
    }
}
