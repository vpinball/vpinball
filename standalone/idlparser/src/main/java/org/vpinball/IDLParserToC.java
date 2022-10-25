package org.vpinball;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.vpinball.Param.ParamType;

public class IDLParserToC {
	public static Map<String, String> INTERFACE_CLASS_MAP  = new HashMap<String, String>() {{	    
	    put("IFileSystem", "filesys");
	    put("IFileSystem3", "filesys");
	    put("IDictionary", "dictionary");
	}};

	public enum ParseMode {
		NONE,
		METHODS,
	}

	public void parse(String in, String out) throws Exception {
		LinkedHashMap<String, Dispatch> dispatchMap = new LinkedHashMap<String, Dispatch>();
		
		BufferedReader bufferedReader = new BufferedReader(new FileReader(in));
		
		FileOutputStream outputStream = new FileOutputStream(out);

		int lineNo = 0;
		
		ParseMode mode = ParseMode.NONE;
		String className = "";
		
		boolean foundMethod = false;
		int methodLineNo = 0;
		String methodBuffer = "";
		
		String line = bufferedReader.readLine();

		while (line != null) {
			lineNo++;

			line = line.trim();

			if (line.startsWith("interface ")) {
				if (line.endsWith("IDispatch")) {
					String interfaceName = line.split(":")[0];
					interfaceName = interfaceName.substring("interface ".length()).trim();

					if (INTERFACE_CLASS_MAP.containsKey(interfaceName)) {
						mode = ParseMode.METHODS;
						className = INTERFACE_CLASS_MAP.get(interfaceName);
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
					outputStream.write(generateDispatch(className, dispatchMap).getBytes());
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

	private String generateDispatch(String className, LinkedHashMap<String, Dispatch> dispatchMap) throws Exception {
		StringBuffer buffer = new StringBuffer();

		String interfaceName = null;
		
		for (String key : INTERFACE_CLASS_MAP.keySet()) {
		    if (INTERFACE_CLASS_MAP.get(key).equals(className)) {
		        interfaceName = key;
		        break;
		    }
		}
		
		buffer.append("static HRESULT WINAPI " + className + "_GetIDsOfNames(" + interfaceName + " *iface, REFIID riid, LPOLESTR *rgszNames,\n");
		buffer.append("                UINT cNames, LCID lcid, DISPID *rgDispId)\n");
		buffer.append("{\n"
		        + "#ifndef __STANDALONE__\n"
		        + "    ITypeInfo *typeinfo;\n"
		        + "    HRESULT hr;\n"
		        + "\n"
		        + "    TRACE(\"%p, %s, %p, %u, %lx, %p.\\n\", iface, debugstr_guid(riid), rgszNames, cNames, lcid, rgDispId);\n"
		        + "\n"
		        + "    hr = get_typeinfo(" + interfaceName + "_tid, &typeinfo);\n"
		        + "    if(SUCCEEDED(hr))\n"
		        + "    {\n"
		        + "        hr = ITypeInfo_GetIDsOfNames(typeinfo, rgszNames, cNames, rgDispId);\n"
		        + "        ITypeInfo_Release(typeinfo);\n"
		        + "    }\n"
		        + "\n"
		        + "    return hr;\n"
		        + "#else\n");
		
		StringBuffer fragment = new StringBuffer();
		
		fragment.append("*rgDispId = DISPID_UNKNOWN;\n");
		fragment.append("\n");
		
		int index = 0;

		for (String key : dispatchMap.keySet()) {
			Dispatch dispatch = dispatchMap.get(key);

			if ("DISPID_NEWENUM".equals(dispatch.getId())) {
				continue;
			}

			if (index > 0) {
			    fragment.append("else ");
			}
			else {
			    fragment.append("");
			}
			
			fragment.append("if (!wcsicmp(*rgszNames, L\"" + key + "\")) {\n");
			fragment.append("*rgDispId = " + dispatch.getId() + ";\n");
			fragment.append("}\n");
			
			index++;
		}
		
		fragment.append("\n");
		fragment.append("return (*rgDispId != DISPID_UNKNOWN) ? S_OK : DISP_E_UNKNOWNNAME;\n");
		
		buffer.append(indent(fragment.toString(), 1));
		buffer.append("#endif\n");
		buffer.append("}\n\n");
		
        buffer.append("static HRESULT WINAPI " + className + "_Invoke(" + interfaceName + " *iface, DISPID dispIdMember,\n"
                + "                                      REFIID riid, LCID lcid, WORD wFlags,\n"
                + "                                      DISPPARAMS *pDispParams, VARIANT *pVarResult,\n"
                + "                                      EXCEPINFO *pExcepInfo, UINT *puArgErr)\n"
                + "{\n"
                + "    ITypeInfo *typeinfo;\n"
                + "    HRESULT hr;\n"
                + "\n"
                + "    TRACE(\"%p, %ld, %s, %lx, %d, %p, %p, %p, %p.\\n\", iface, dispIdMember, debugstr_guid(riid),\n"
                + "           lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);\n"
                + "\n"
                + "#ifndef __STANDALONE__\n"
                + "    hr = get_typeinfo(" + interfaceName + "_tid, &typeinfo);\n"
                + "    if(SUCCEEDED(hr))\n"
                + "    {\n"
                + "        hr = ITypeInfo_Invoke(typeinfo, iface, dispIdMember, wFlags,\n"
                + "                pDispParams, pVarResult, pExcepInfo, puArgErr);\n"
                + "        ITypeInfo_Release(typeinfo);\n"
                + "    }\n"
                + "\n"
                + "    return S_OK;\n"
                + "#else\n");
        
        fragment.setLength(0);

        fragment.append("hr = DISP_E_UNKNOWNNAME;\n");
        fragment.append("switch(dispIdMember) {\n");
        
        for (String key : dispatchMap.keySet()) {
            Dispatch dispatch = dispatchMap.get(key);

            if (dispatch.getMethodCount() == 1) {
                fragment.append("case " + dispatch.getId() + ": {\n");
                fragment.append(generateMethod(dispatch.getMethod(0), className));
                fragment.append("}\n");
                fragment.append("\n");
            }
            else {
                fragment.append("case " + dispatch.getId() + ":\n");
                fragment.append("switch(pDispParams->cArgs) {\n");
                for (Method method:dispatch.getMethodList()) {
                    fragment.append("case " + method.getCArgs() + ": {\n");
                    fragment.append(generateMethod(method, className));
                    fragment.append("}\n");
                    fragment.append("\n");
                }
                fragment.append("default:\n");
                fragment.append("break;\n");
                fragment.append("}\n");

                fragment.append("break;\n");
                fragment.append("\n");
            }
        }

        fragment.append("default:\n");
        fragment.append("break;\n");
        fragment.append("}\n");

        fragment.append("\n");

        fragment.append("return hr;\n");
   	      
        buffer.append(indent(fragment.toString(), 1));
        buffer.append("#endif\n");
        buffer.append("}\n\n");
	        
		return buffer.toString();
	}

	private String generateHeaderVariant(int index, Param param, String type, Method method) {
		StringBuffer buffer = new StringBuffer();

		buffer.append("VARIANT var" + index);
		if (param.hasDefaultValue()) {
			buffer.append("// TODO(" + param.getDefaultValue() + ")");
		}
		buffer.append(";\n");

		if (type != null) {
		    buffer.append("V_VT(&var" + index + ") = VT_EMPTY;\n");
			buffer.append("VariantChangeType(&var" + index + ", ");

			if (param.hasDefaultValue() || param.isOptional()) {
				buffer.append("(index > 0) ? &pDispParams->rgvarg[--index] : &var" + index);
			}
			else {
				if (method.getCArgs() > 1) {
					buffer.append("&pDispParams->rgvarg[--index]");
				}
				else {
					buffer.append("pDispParams->rgvarg");
				}
			}

			buffer.append(", 0, " + type + ");\n");
		}
		else {
		    buffer.append("VariantCopy(&var" + index + ", pDispParams->rgvarg[--index]);\n");
		}

		return buffer.toString();
	}
	
	private String generateMethod(Method method, String className) {
		StringBuffer buffer = new StringBuffer();

		buffer.append("hr = " + className + "_" + method.getPropName() + "(iface, ");
		
		int index = 0;

		StringBuffer header = new StringBuffer();

		if (method.hasDefaultValueParams() || method.hasOptionalParams() || method.getCArgs() > 1) {
			header.append("int index = pDispParams->cArgs;\n");
		}

		for (Param param : method.getParamList()) {
			if (index > 0) {
				buffer.append(", ");
			}

			if (param.getParamType() == ParamType.IN) {
				if ("int".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_I4", method));
					buffer.append("V_I4(&var" + index + ")");
				}
				else if ("long".equals(param.getType())) {
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
					if (method.hasDefaultValueParams() ||method.hasOptionalParams() || method.getCArgs() > 1) {
						header.append("VARIANT* var" + index + " = &pDispParams->rgvarg[--index];\n");

						buffer.append("var" + index);
					}
					else {

						buffer.append("pDispParams->rgvarg");
					}
				}
				else {
					buffer.append(param.getType());
				}
			}
			else if (param.getParamType() == ParamType.OUT_RETVAL) {
				if ("int*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_I4;\n");
					buffer.append("&V_I4(pVarResult)");
				}
				else if ("float*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_R4;\n");
					buffer.append("&V_R4(pVarResult)");
				}
				else if ("long*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_I4;\n");
					buffer.append("(long*)&V_I4(pVarResult)");
				}
				else if ("VARIANT_BOOL*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_BOOL;\n");
					buffer.append("&V_BOOL(pVarResult)");
				}
				else if ("BSTR*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_BSTR;\n");
					buffer.append("&V_BSTR(pVarResult)");
				}
				else if ("OLE_COLOR*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_UI4;\n");
					buffer.append("&V_UI4(pVarResult)");
				}
				else if ("VARIANT*".equals(param.getType())) {
					buffer.append("pVarResult");
				}
				else if ("SIZE_T*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_UI4;\n");
					buffer.append("(SIZE_T*)&V_UI4(pVarResult)");
				}
				else if ("IUnknown**".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_UNKNOWN;\n");
					buffer.append("&V_UNKNOWN(pVarResult)");
				}
				else if ("IFontDisp**".equals(param.getType())
						|| "ITable**".equals(param.getType())
						|| "IDispatch**".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_DISPATCH;\n");
					buffer.append("(" + param.getType() + ")&V_DISPATCH(pVarResult)");
				}
				else if ("SAFEARRAY(VARIANT)*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_SAFEARRAY;\n");
					buffer.append("(SAFEARRAY**)&V_ARRAY(pVarResult)");
				}
				else {
					buffer.append(param.getType());
				}
			}

			index++;
		}

		buffer.append(");\n");
		
		for (int i = 0; i < method.getCArgs(); i++) {
		    buffer.append("VariantClear(&var" + i + ");\n");
		}    
		  
		buffer.append("break;\n");

		if (method.hasOutRetvalParam()) {
			String varResult = "";
			varResult += "if (pVarResult == NULL) {\n";
			varResult += "VARIANT valResult;\n";
			varResult += "VariantInit(&valResult);\n";
			varResult += "pVarResult = &valResult;\n";
			varResult += "}\n";

			header.insert(0, varResult);
		}

		buffer.insert(0, header.toString());

		buffer.insert(0, "// line " + method.getLineNo() + ": " + method.getLine() + "\n");

		return buffer.toString();
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
		parser.parse("scrrun.idl", "scrun.c");
	}
}
