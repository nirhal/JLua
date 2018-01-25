# Jlua #

JLua is a scripting tool for Java. 
This tool allows scripts written in Lua to manipulate components developed in Java and vice versa. 


JLua is a fork of "LuaJava". 
It offers the following improvements:

* Added LuaState wrapper to ease manipulation of lua objects.
* Simpler creation of Java objects from Lua.
* Lua error cause automatically Java exception and vice versa.
* Added "try ... cache ... finally" mechanism to Lua.
* Allows simpler implementation of Java interfaces in Lua.
* Simple Java array creation from lua.
* Fixed some multithreading issues.
* Plenty of other bug fixes.


### Building JLua ###

First, change your current working directory to the build directory.
Then, run cmake:

```
cmake /path/to/source/
```
For 64-bit build run:
```
cmake /path/to/source/ -DCMAKE_GENERATOR_PLATFORM=x64
```

On **Linux/Mac** the default generator is Unix Makefiles, so the build can be started by invoking the make executable: 
```
make
```

On **Windows** the default generator is a Visual Studio project file. You can open the file and build manually or execute the following command from the VS Native Tools command prompt:
```
devenv Project.sln /build Debug
```
 
Before running the Java program, make sure you have the .so (UNIX) or .dll (Windows) file path in your "LD_LIBRARY_PATH" (UNIX) or "PATH" (Windows) environmental variable.


### Usage from Lua ###

This library offers 5 functions:

* **jlua.getClass ( class_name )**

    Get a Java class to lua code. 
    Examples: 
    ```
    System = jlua.getClass("java.lang.System")
    System.out:println("Hello world")
    ```
    ```
    ArrayList = jlua.getClass("java.util.ArrayList")
    a = ArrayList:new(2)
    a:add("Hello")
    a:add("World")
    print(a:get(0))
    print(a:get(1))
    ```
    
* **jlua.newArray ( class_name, num_of_elements )**

    Create now Java array from Lua. 
    Examples: 
    ```
    a = jlua.newArray("int", 3)
    a[1] = 7
    a[2] = 5
    a[3] = 8
    print(a[1] + a[2] + a[3])
    ```
    ```
    a = jlua.newArray("java.lang.String", 2)
    a[1] = "aa"
    a[2] = "bb"
    print(a[1] .. a[2])
    ```
    
* **jlua.throw ( trowable_or_string )**

    Throws Java exception or string
    Examples: 
    ```
    jlua.throw("Error!!")
    ```
    ```
    Exception = jlua.getClass("java.lang.Exception")
    jlua.throw(Exception:new("some exception"))
    ```
    
* **jlua.try ( try_block, catch_block )** or <br/>
    **jlua.try ( try_block, catch_block, finally_block )** or <br/>
     **jlua.try ( try_block, nil, finally_block )**

    Create now Java array from Lua. 
    Examples: 
    ```
    jlua.try(function()
                print("aa")
                jlua.throw(jlua.getClass("java.lang.Exception"):new("hey"))
                print("bb")
            end, function(e)
                print(e:toString())
             end)
    ```
    ```
    jlua.try(function()
        jlua.try(function()
            print("aa")
            jlua.throw(jlua.getClass("java.lang.Exception"):new("hey"))
            print("bb")
        end, nil , function()
            print("ff")
        end)
    end, function(e)
        print(e:toString())
        end)
    ```
    
* **jlua.createProxy ( java_interface, lua_table )**
    
    Creating lua implementation of Java interface.
    Example:
    ```
    runnable = {}
    function runnable:run()
        print("hey")
    end
    runnableJava = jlua.createProxy("java.lang.Runnable", runnable)
    thread = jlua.getClass("java.lang.Thread"):new(runnableJava)
    thread:start()
    ```
    
    Actually, in this example there is no need to use jlua.createProxy.
    JLua automatically convert lua table to Java interface as it passes as a parameter.
    Simplified example:
    ```
    runnable = {}
    function runnable:run()
        print("hey")
    end
    thread = jlua.getClass("java.lang.Thread"):new(runnable)
    thread:start()
    ```
    

### Usage from Java ###

The "Lua" object offer many methods to manipulate lua objects.
See src/java/org/jlua/Console.java for simple usage example.