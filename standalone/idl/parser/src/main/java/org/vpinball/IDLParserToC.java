package org.vpinball;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;

import org.vpinball.Param.ParamType;

public class IDLParserToC {

    public enum ParseMode {
        NONE,
        METHODS,
    }

    public void parse(String in, String out, List<IDLInterface> interfaceList) throws Exception {
		HashMap<String, IDLInterface> interfaceMap = new HashMap<String, IDLInterface>();
		for (IDLInterface idlClass:interfaceList) {
			interfaceMap.put(idlClass.getInterfaceName(), idlClass);
		}
    	
    	LinkedHashMap<String, Dispatch> dispatchMap = new LinkedHashMap<String, Dispatch>();

        BufferedReader bufferedReader = new BufferedReader(new FileReader(in));

        FileOutputStream outputStream = new FileOutputStream(out);

        int lineNo = 0;

        ParseMode mode = ParseMode.NONE;
        IDLInterface idlInterface = null;

        boolean foundMethod = false;
        int methodLineNo = 0;
        String methodBuffer = "";

        String line = bufferedReader.readLine();

        while (line != null) {
            lineNo++;

            line = line.trim();

            if (line.startsWith("interface ")) {
                if (line.contains(":")) {
                    String interfaceName = line.split(":")[0];
                    interfaceName = interfaceName.substring("interface ".length()).trim();

                    if (interfaceMap.containsKey(interfaceName)) {
                        mode = ParseMode.METHODS;
                        idlInterface = interfaceMap.get(interfaceName);
                        dispatchMap.clear();
                    }
                }
            }

            if (mode == ParseMode.NONE) {
                line = bufferedReader.readLine();

                continue;
            }
            else if (line.startsWith("}")) {
                if (mode == ParseMode.METHODS) {
                    outputStream.write(generateDispatch(idlInterface, dispatchMap, interfaceMap).getBytes());
                }

                mode = ParseMode.NONE;
            }
            else if (mode == ParseMode.METHODS) {
                if (!foundMethod && line.startsWith("[")) {
                    foundMethod = true;
                    methodLineNo = lineNo;
                }

                if (foundMethod) {
                    methodBuffer += line;

                    if (line.endsWith(";")) {
                        Method method = new Method(methodBuffer, methodLineNo);

                        if (dispatchMap.containsKey(method.getName())) {
                            Dispatch dispatch = dispatchMap.get(method.getName());
                            dispatch.addMethod(method);
                        }
                        else {
                            Dispatch dispatch = new Dispatch();
                            dispatch.setId(method.getId());
                            dispatch.addMethod(method);

                            dispatchMap.put(method.getName(), dispatch);
                        }

                        foundMethod = false;
                        methodBuffer = "";
                    }
                }
            }

            line = bufferedReader.readLine();
        }

        bufferedReader.close();

        outputStream.close();
    }

    private String generateDispatch(IDLInterface idlInterface, LinkedHashMap<String, Dispatch> dispatchMap, HashMap<String, IDLInterface> interfaceMap) throws Exception {
        StringBuffer buffer = new StringBuffer();
   
        buffer.append("static HRESULT WINAPI " + idlInterface.getClassName() + "_GetIDsOfNames(" + idlInterface.getInterfaceName() + " *iface, REFIID riid, LPOLESTR *rgszNames,\n");
        buffer.append("                UINT cNames, LCID lcid, DISPID *rgDispId)\n");
        buffer.append("{\n");
               
        StringBuffer fragment = new StringBuffer();

        fragment.append("static struct {\n");
        fragment.append("const WCHAR *name;\n");
        fragment.append("DISPID dispId;\n");
        fragment.append("} names_ids_list[] = {\n");
        fragment.append("{ NULL }");

        ArrayList<String> keys = new ArrayList<>(dispatchMap.keySet());
        Collections.sort(keys, String.CASE_INSENSITIVE_ORDER);
     
        for (String key : keys) {
			if (dispatchMap.containsKey(key)) {
				Dispatch dispatch = dispatchMap.get(key);

				if ("DISPID_NEWENUM".equals(dispatch.getId()) || 
						"DISPID_VALUE".equals(dispatch.getId())) {
					continue;
				}

				fragment.append(",\n");
				fragment.append("{ L\"" + key + "\", " + dispatch.getId() + " }");
			}
			else {
				System.out.println("\"" + key + "\" not found");
			}
		}
        
        fragment.append("\n");
        fragment.append("};\n");
        fragment.append("\n");
		
        fragment.append("size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;\n");
        fragment.append("int r;\n");
        fragment.append("while(min <= max) {\n");
        fragment.append("i = (min + max) / 2;\n");
        fragment.append("r = wcsicmp(names_ids_list[i].name, *rgszNames);\n");
        fragment.append("if(!r) {\n");
        fragment.append("*rgDispId = names_ids_list[i].dispId;\n");
        fragment.append("return S_OK;\n");
        fragment.append("}\n");
        fragment.append("if(r < 0)\n");
        fragment.append("   min = i+1;\n");
        fragment.append("else\n");
        fragment.append("   max = i-1;\n");
        fragment.append("}\n");
        fragment.append("return DISP_E_MEMBERNOTFOUND;\n");
		
        buffer.append(indent(fragment.toString(), 1));
        buffer.append("}\n\n");

        buffer.append("static HRESULT WINAPI " + idlInterface.getClassName() + "_Invoke(" + idlInterface.getInterfaceName() + " *iface, DISPID dispIdMember,\n"
                + "                                      REFIID riid, LCID lcid, WORD wFlags,\n"
                + "                                      DISPPARAMS *pDispParams, VARIANT *pVarResult,\n"
                + "                                      EXCEPINFO *pExcepInfo, UINT *puArgErr)\n"
                + "{\n");
               
        fragment.setLength(0);

        fragment.append("int index = pDispParams->cArgs;\n");
        fragment.append("VARIANT res;\n");
        fragment.append("HRESULT hres = DISP_E_UNKNOWNNAME;\n");		
        fragment.append("\n");
        fragment.append("V_VT(&res) = VT_EMPTY;\n");
        fragment.append("\n");
        fragment.append("switch(dispIdMember) {\n");
        fragment.append("case DISPID_VALUE: {\n");
        fragment.append("if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {\n");
        fragment.append("V_VT(&res) = VT_DISPATCH;\n");
        fragment.append("V_DISPATCH(&res) = (IDispatch*)iface;\n");
        fragment.append("hres = S_OK;\n");
        fragment.append("}\n");
        fragment.append("break;\n");
        fragment.append("}\n");
		
        int index = 0;
        
        for (String key : dispatchMap.keySet()) {
            Dispatch dispatch = dispatchMap.get(key);

            fragment.append("case " + dispatch.getId() + ": {\n"); 

            index = 0;

            for (Method method:dispatch.getMethodList()) {
                if (index > 0) {
                    fragment.append("else ");
                }
                fragment.append(generateMethod(method, idlInterface.getClassName()));
                index++;
            }
            fragment.append("break;\n"); 
            fragment.append("}\n"); 
        }
        
        fragment.append("default:\n");
        fragment.append("break;\n");
        fragment.append("}\n");
        fragment.append("if (hres == S_OK) {\n");
        fragment.append("if (pVarResult)\n");
        fragment.append("\t*pVarResult = res;\n");
        fragment.append("else\n");
        fragment.append("\tVariantClear(&res);\n");
        fragment.append("}\n");
        fragment.append("else if (hres != S_FALSE) {\n");
        fragment.append("printf(\"" + idlInterface.getClassName() + "_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d\\n\", dispIdMember, dispIdMember, wFlags, hres);\n");
        fragment.append("}\n");
        fragment.append("return hres;\n");
        
        buffer.append(indent(fragment.toString(), 1));
        buffer.append("}\n\n");

        return buffer.toString();
    }

    private String generateHeaderVariant(int index, Param param, String type, Method method) {
        StringBuffer buffer = new StringBuffer();

        buffer.append("VARIANT var" + index + ";\n");
       
        if (param.hasDefaultValue()) {
            if ("int".equals(param.getType())) {
                buffer.append("V_VT(&var" + index + ") = VT_I4;\n");
                buffer.append("V_I4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
            }
            else if ("long".equals(param.getType())) {
                buffer.append("V_VT(&var" + index + ") = VT_I4;\n");
                buffer.append("V_I4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
            }
            else if ("float".equals(param.getType())) {
                buffer.append("V_VT(&var" + index + ") = VT_R4;\n");
                buffer.append("V_R4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
            }
            else if ("VARIANT_BOOL".equals(param.getType())) {
                buffer.append("V_VT(&var" + index + ") = VT_BOOL;\n");
                buffer.append("V_BOOL(&var" + index + ") = " + param.getDefaultValue() + ";\n");
            }
            else if ("BSTR".equals(param.getType())) {
                buffer.append("OLECHAR* pszDefault = L" + param.getDefaultValue() + ";\n");
                buffer.append("V_VT(&var" + index + ") = VT_BSTR;\n");
                buffer.append("V_BSTR(&var" + index + ") = SysAllocString(pszDefault);\n");
            }
            else if ("CompareMethod".equals(param.getType())
                    || "Tristate".equals(param.getType())
                    || "IOMode".equals(param.getType())
                    || "StandardStreamTypes".equals(param.getType())
                    || "FileAttribute".equals(param.getType())
                    || "SpecialFolderConst".equals(param.getType())
                    || "DriveTypeConst".equals(param.getType())) {
                buffer.append("V_VT(&var" + index + ") = VT_I4;\n");
                buffer.append("V_I4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
            }
            else {
                buffer.append("\nUNSUPPORTED DEFAULT\n");
            }
        }
        else {
            buffer.append("V_VT(&var" + index + ") = VT_EMPTY;\n");
        }

        if (type != null) {
            buffer.append("VariantChangeType(&var" + index + ", ");

            if (param.hasDefaultValue() || param.isOptional()) {
                buffer.append("(index > 0) ? &pDispParams->rgvarg[--index] : &var" + index);
            }
            else {
                buffer.append("&pDispParams->rgvarg[--index]");
            }

            buffer.append(", 0, " + type + ");\n");
        }
        else {
            buffer.append("VariantCopyInd(&var" + index + ", &pDispParams->rgvarg[--index]);\n");
        }

        return buffer.toString();
    }

    private String generateMethod(Method method, String className) {
        StringBuffer buffer = new StringBuffer();

        buffer.append("hres = " + className + "_" + method.getPrefix() + method.getName() + "(iface");

        int index = 0;

        StringBuffer header = new StringBuffer();

        for (Param param : method.getParamList()) {
            buffer.append(", ");
          
            if (param.getParamType() == ParamType.IN) {
                if ("int".equals(param.getType())) {
                    header.append(generateHeaderVariant(index, param, "VT_I4", method));
                    buffer.append("V_I4(&var" + index + ")");
                }
                else if ("long".equalsIgnoreCase(param.getType())) {
                    header.append(generateHeaderVariant(index, param, "VT_I4", method));
                    buffer.append("V_I4(&var" + index + ")");
                }
                else if ("float".equals(param.getType())) {
                    header.append(generateHeaderVariant(index, param, "VT_R4", method));
                    buffer.append("V_R4(&var" + index + ")");
                }
                else if ("VARIANT_BOOL".equals(param.getType())) {
                    header.append(generateHeaderVariant(index, param, "VT_BOOL", method));
                    buffer.append("V_BOOL(&var" + index + ")");
                }
                else if ("BSTR".equals(param.getType())) {
                    header.append(generateHeaderVariant(index, param, "VT_BSTR", method));
                    buffer.append("V_BSTR(&var" + index + ")");
                }
                else if ("DATE".equals(param.getType())) {
                    header.append(generateHeaderVariant(index, param, "VT_DATE", method));
                    buffer.append("V_DATE(&var" + index + ")");
                }
                else if ("VARIANT*".equals(param.getType())) { 
                    header.append(generateHeaderVariant(index, param, null, method));
                    buffer.append("&var" + index );
                }
                else if ("VARIANT".equals(param.getType())) { 
                    header.append(generateHeaderVariant(index, param, "VT_VARIANT", method));
                    buffer.append("var" + index);
                }
                else if ("OLE_COLOR".equals(param.getType())) {
                    header.append(generateHeaderVariant(index, param, "VT_UI4", method));
                    buffer.append("(OLE_COLOR)V_UI4(&var" + index + ")");
                }
                else if ("CompareMethod".equals(param.getType())
                        || "Tristate".equals(param.getType())
                        || "IOMode".equals(param.getType())
                        || "StandardStreamTypes".equals(param.getType())
                        || "FileAttribute".equals(param.getType())
                        || "SpecialFolderConst".equals(param.getType())
                        || "DriveTypeConst".equals(param.getType())) {
                    header.append(generateHeaderVariant(index, param, "VT_I4", method));
                    buffer.append("(" + param.getType() + ")V_I4(&var" + index + ")");
                }
                else if ("IFontDisp*".equals(param.getType())) {
                    // FIX ME
                    buffer.append("(IFontDisp*)pDispParams->rgvarg");
                }
                else if ("void".equals(param.getType())) {
                }
                else {
                    buffer.append(param.getType());
                }
            }
            else if (param.getParamType() == ParamType.OUT) {
                if ("VARIANT*".equals(param.getType())) { 
                        header.append("VARIANT* var" + index + " = &pDispParams->rgvarg[--index];\n");
                        buffer.append("var" + index);
                }
                else {
                    buffer.append(param.getType());
                }
            }
            else if (param.getParamType() == ParamType.OUT_RETVAL) {
                if ("int*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_I4;\n");
                    buffer.append("(int*)&V_I4(&res)");
                }
                else if ("float*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_R4;\n");
                    buffer.append("&V_R4(&res)");
                }
                else if ("long*".equalsIgnoreCase(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_I4;\n");
                    buffer.append("(LONG*)&V_I4(&res)");
                }
                else if ("VARIANT_BOOL*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_BOOL;\n");
                    buffer.append("&V_BOOL(&res)");
                }
                else if ("BSTR*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_BSTR;\n");
                    buffer.append("&V_BSTR(&res)");
                }
                else if ("DATE*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_DATE;\n");
                    buffer.append("&V_DATE(&res)");
                }
                else if ("OLE_COLOR*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_UI4;\n");
                    buffer.append("&V_UI4(&res)");
                }
                else if ("VARIANT*".equals(param.getType())) {
                    buffer.append("&res");
                }
                else if ("SIZE_T*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_UI4;\n");
                    buffer.append("(SIZE_T*)&V_UI4(&res)");
                }
                else if ("IUnknown**".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_UNKNOWN;\n");
                    buffer.append("&V_UNKNOWN(&res)");
                }
                else if ("IFontDisp**".equals(param.getType())
                        || "ITable**".equals(param.getType())
                        || "IDrive**".equals(param.getType())
                        || "IDriveCollection**".equals(param.getType())
                        || "IFolderCollection**".equals(param.getType())
                        || "IFileCollection**".equals(param.getType())
                        || "IFile**".equals(param.getType())
                        || "IFolder**".equals(param.getType())
                        || "ITextStream**".equals(param.getType())
                        || "IDispatch**".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_DISPATCH;\n");
                    buffer.append("(" + param.getType() + ")&V_DISPATCH(&res)");
                }
                else if ("SAFEARRAY(VARIANT)*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_VARIANT|VT_ARRAY;\n");
                    buffer.append("(SAFEARRAY**)&V_ARRAY(&res)");
                }
                else if ("CompareMethod*".equals(param.getType())
                        || "Tristate*".equals(param.getType())
                        || "IOMode*".equals(param.getType())
                        || "StandardStreamTypes*".equals(param.getType())
                        || "FileAttribute*".equals(param.getType())
                        || "SpecialFolderConst*".equals(param.getType())
                        || "DriveTypeConst*".equals(param.getType())) {
                    buffer.insert(0, "V_VT(&res) = VT_I4;\n");
                    buffer.append("(" + param.getType() + ")&V_I4(&res)");
                }
                else {
                    buffer.append(param.getType());
                }
            }

            index++;
        }

        buffer.append(");\n");

        index = 0;
        
        for (Param param : method.getParamList()) {
            if (param.getParamType() != ParamType.OUT && 
                param.getParamType() != ParamType.OUT_RETVAL) {
                buffer.append("VariantClear(&var" + index + ");\n");
            }
            index++;
        }

        buffer.insert(0, header.toString());

        buffer.insert(0, "// line " + method.getLineNo() + ": " + method.getLine() + "\n");

        String wrap = "if (wFlags & " + method.getDispatchType() +") {\n";
        wrap += buffer.toString();
        wrap += "}\n";

        return wrap;
    }

    private String indent(String code, int indent) throws Exception {
        StringBuffer buffer = new StringBuffer();

        for (int index = 0; index < indent; index++) {
            buffer.append("\t");
        }

        boolean newLine = false;

        for (int i = 0; i < code.length(); i++) {
            char character = code.charAt(i);

            if (character == '\n') {
                newLine = true;
                buffer.append(character);
            }
            else {
                if (character == '{') {
                    indent++;
                }
                else if (character == '}') {
                    indent--;
                }

                if (newLine) {
                    newLine = false;
                    for (int index = 0; index < indent; index++) {
                        buffer.append("\t");
                    }
                }

                buffer.append(character);
            }
        }

        return buffer.toString();       
    }

    public static void main(String[] args) throws Exception {
        IDLParserToC parser = new IDLParserToC();

        parser.parse(
                "../../inc/wine/dlls/scrrun/scrrun.idl", 
                "dictionary_proxy.c",
                Arrays.asList(
                	new IDLInterface("IDictionary", "dictionary")));

        parser.parse(
                "../../inc/wine/dlls/scrrun/scrrun.idl", 
                "filesystem_proxy.c",
                Arrays.asList(   
                	new IDLInterface("IDriveCollection", "drivecoll"),
                	new IDLInterface("IFolderCollection", "foldercoll"),
                	new IDLInterface("IFolder", "folder"),
                	new IDLInterface("IDrive", "drive"),
                	new IDLInterface("ITextStream", "textstream"),
                	new IDLInterface("IFile", "file"),
                	new IDLInterface("IFileSystem3", "filesys"),
                	new IDLInterface("IFileSystem", "filesys")
                	));
    }
}
