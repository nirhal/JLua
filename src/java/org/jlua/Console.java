/*
 * $Id: Console.java,v 1.8 2007-09-17 19:28:40 thiago Exp $
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
 */

package org.jlua;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * Simple JLua console.
 * This is also an example on how to use the Java side of JLua and how to startup
 * a JLua application.
 *
 * @author Thiago Ponte
 */
public class Console {

    /**
     * Creates a console for user interaction.
     *
     * @param args names of the lua files to be executed
     */
    public static void main(String[] args) {
        Lua lua = new Lua();

        if (args.length > 0) {


            try {
                for (String arg : args)
                    lua.doFile(arg);

            } catch (Throwable t){
                printError(t, lua.getLastExceptionStackTrace());
            } finally {
                  lua.release();
            }

        } else {

            System.out.println("API JLua - console mode.");

            BufferedReader inp = new BufferedReader(new InputStreamReader(System.in));

            String line;

            System.out.print("> ");

            try {
                while ((line = inp.readLine()) != null && !line.equals("exit")) {
                    try {
                        lua.doString(line);
                    } catch (Throwable t){
                        printError(t, lua.getLastExceptionStackTrace());
                    }

                    System.out.print("> ");
                }

            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                lua.release();
            }
        }





    }

    private static void printError(Throwable t, String stackTrace) {
        if (t instanceof LuaException){
            System.err.println("Error: " + t.getMessage());
        } else {
            System.err.println("Exception occurred: " + t.toString() + stackTrace);
        }
    }
}