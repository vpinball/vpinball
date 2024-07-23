package org.vpinball;

import java.util.ArrayList;
import java.util.List;

import org.vpinball.Param.ParamType;

public class Method {
    public enum InvokeType {
        FUNC,
        PROPERTYGET,
        PROPERTYPUT,
        PROPERTYPUTREF,
    }

    static private int unknownId = 8000;

    private int lineNo;
    private String line;
    private String name;
    private String id;
    private InvokeType type;

    private ArrayList<Param> paramList = new ArrayList<Param>();

    public Method(String line, int lineNo) throws Exception {
        this.line = line;
        this.lineNo = lineNo;

        int specEndIndex = line.indexOf("]");
        String spec = line.substring(1, specEndIndex).trim();

        int specIdStartIndex = spec.indexOf("id(");

        if (specIdStartIndex != -1) {
            int specIdEndIndex = spec.indexOf(")", specIdStartIndex);
            id = spec.substring(specIdStartIndex + "id(".length(), specIdEndIndex);
        }
        else {
            id = Integer.toString(unknownId++);
        }

        String method = line.substring(specEndIndex+1).trim();

        int paramStartIndex = method.indexOf("(");
        int nameIndex = method.indexOf(" ");

        name = method.substring(nameIndex, paramStartIndex).trim();

        if (spec.contains("propget")) {
            type = InvokeType.PROPERTYGET;
        }
        else if (spec.contains("propputref")) {
            type = InvokeType.PROPERTYPUTREF;
        }
        else if (spec.contains("propput")) {
            type = InvokeType.PROPERTYPUT;
        }
        else {
            type = InvokeType.FUNC;
        }

        String params = method.substring(paramStartIndex + 1, method.indexOf(");")).trim();

        String param = "";
        boolean inBracket = false;

        for (int i = 0; i < params.length(); i++) {
            char character = params.charAt(i);

            if (character == ',') {
                if (!inBracket) {
                    paramList.add(new Param(param));
                    param = "";

                    continue;
                }
            }
            else if (character == '[') {
                inBracket = true;
            }
            else if (character == ']') {
                inBracket = false;
            }

            param += character;
        }

        if (param.length() > 0) {
            paramList.add(new Param(param));
        }
    }

    public String getLine() {
        return line;
    }

    public int getLineNo() {
        return lineNo;
    }

    public String getName() {
        return name;
    } 

    public InvokeType getType() {
        return type;
    }

    public String getId() {
        return id;
    }

    public List<Param> getParamList() {
        return paramList;
    }

    public int getCArgs() {
        int cArgs = 0;
        for (Param param:paramList) {
            if (param.getParamType() != ParamType.OUT_RETVAL) {
                cArgs++;
            }
        }
        return cArgs;
    }

    public boolean hasDefaultValueParams() {
        for (Param param:paramList) {
            if (param.hasDefaultValue()) {
                return true;
            }
        }
        return false;
    }

    public boolean hasOptionalParams() {
        for (Param param:paramList) {
            if (param.isOptional()) {
                return true;
            }
        }
        return false;
    }

    public boolean hasOutRetvalParam() {
        for (Param param:paramList) {
            if (param.getParamType() == ParamType.OUT_RETVAL) {
                return true;
            }
        }
        return false;
    }

    public String getDispatchType() {
        String dispatchType = "";
        if (type == InvokeType.PROPERTYPUTREF) {
            dispatchType = "DISPATCH_PROPERTYPUTREF";
        }
        else if (type == InvokeType.PROPERTYPUT) {
            dispatchType = "DISPATCH_PROPERTYPUT";
        }
        else if (type == InvokeType.PROPERTYGET) {
            dispatchType = "DISPATCH_PROPERTYGET";
        }
        else if (type == InvokeType.FUNC) {
            dispatchType = "DISPATCH_METHOD";
        }  
        return dispatchType;
    }

    public String getPrefix() {
        String prefix = "";
        if (type == InvokeType.PROPERTYPUTREF) {
            prefix = "putref_";
        }
        else if (type == InvokeType.PROPERTYPUT) {
            prefix = "put_";
        }
        else if (type == InvokeType.PROPERTYGET) {
            prefix = "get_";
        }
        return prefix;
    }

    public static void resetUnknownId() {
        unknownId = 8000;
    }
}