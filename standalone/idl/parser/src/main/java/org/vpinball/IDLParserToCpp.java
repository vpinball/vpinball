package org.vpinball;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;

import org.vpinball.Param.ParamType;

public class IDLParserToCpp {
	public enum ParseMode {
		NONE,
		METHODS,
		EVENTS,
		ENUM,
	}

	private HashMap<String, IDLEnum> getEnums(String in, List<String> enumList) throws Exception {
		LinkedHashMap<String, IDLEnum> enumMap = new LinkedHashMap<String, IDLEnum>();

		BufferedReader bufferedReader = new BufferedReader(new FileReader(in));

		ParseMode mode = ParseMode.NONE;

		StringBuffer buffer = new StringBuffer();
		buffer.append("\n");

		StringBuffer buffer2 = new StringBuffer();
		buffer2.append("\n");

		String line = bufferedReader.readLine();

		int id = 2000;

		while (line != null) {
			line = line.trim();

			if (line.startsWith("enum ")) {
				String token[] = line.split(" ");

				String enumName = token[1];

				if (enumList.contains(enumName)) {
					mode = ParseMode.ENUM;
				}
			}

			if (mode == ParseMode.NONE) {
				line = bufferedReader.readLine();

				continue;
			}
			else if (line.startsWith("}")) {
				mode = ParseMode.NONE;
			}
			else if (mode == ParseMode.ENUM) {
				line = line.replaceAll(" ", "");
				line = line.replaceAll(",", "");

				if (line.contains("=")) {
					String token[] = line.split("=");

					IDLEnum enumType = new IDLEnum();
					enumType.setName(token[0]);
					enumType.setValue(Integer.parseInt(token[1]));
					enumType.setId(id++);
					enumMap.put(enumType.getName(), enumType);
				}
			}

			line = bufferedReader.readLine();

		}

		bufferedReader.close();

		return enumMap;
	}

	public void parse(String in, String out, List<IDLInterface> interfaceList) throws Exception {
		Method.resetUnknownId();

		HashMap<String, IDLInterface> interfaceMap = new HashMap<String, IDLInterface>();
		for (IDLInterface idlClass:interfaceList) {
			interfaceMap.put(idlClass.getInterfaceName(), idlClass);
		}

		LinkedHashMap<String, Dispatch> dispatchMap = new LinkedHashMap<String, Dispatch>();
		ArrayList<Event> eventList = new ArrayList<Event>();

		BufferedReader bufferedReader = new BufferedReader(new FileReader(in));

		FileOutputStream outputStream = new FileOutputStream(out);

		outputStream.write("#include \"stdafx.h\"\n".getBytes());
		outputStream.write("#include \"olectl.h\"\n".getBytes());
		outputStream.write("\n".getBytes());

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
					else {
						System.out.println("Interface not in map: " + interfaceName);
					}
				}
			}
			else if (line.startsWith("dispinterface ")) {
				String interfaceName = line.split(" ")[1].trim();

				if (interfaceMap.containsKey(interfaceName)) {
					mode = ParseMode.EVENTS;
					idlInterface = interfaceMap.get(interfaceName);
					eventList.clear();
				}
			}

			if (mode == ParseMode.NONE) {
				line = bufferedReader.readLine();

				continue;
			}
			else if (line.startsWith("}")) {
				if (mode == ParseMode.METHODS) {
					HashMap<String, IDLEnum> enumMap = null;

					if (idlInterface.getEnumList() != null) {
						enumMap = getEnums(in, idlInterface.getEnumList());
					}

					outputStream.write(generateDispatch(idlInterface, dispatchMap, interfaceMap, enumMap).getBytes());
				}
				else if (mode == ParseMode.EVENTS) {
					outputStream.write(generateEvents(idlInterface, eventList).getBytes());
				}

				mode = ParseMode.NONE;
			}
			else if (mode == ParseMode.METHODS) {
				if (!foundMethod && line.startsWith("[")) {
					if (!line.contains("[restricted]")) {
						foundMethod = true;
						methodLineNo = lineNo;
					}
				}
				else if (!foundMethod && line.startsWith("/* ! */")) {
					foundMethod = true;
					methodLineNo = lineNo;
				}

				if (foundMethod) {
					methodBuffer += line;

					if (line.endsWith(";") || line.indexOf("); //") != -1) {
						Method method = new Method(methodBuffer, methodLineNo);

						boolean ignore = false;

						if ("JoltAmount".equals(method.getName()) ||
								"TiltAmount".equals(method.getName()) ||
								"JoltTriggerTime".equals(method.getName()) ||
								"TiltTriggerTime".equals(method.getName())) {
							ignore = true;
						}

						if ("GetPlayerHWnd".equals(method.getName())) {
							if ("long*".equals(method.getParamList().get(0).getType())) {
								ignore = true;
							}
						}

						if (!ignore) {
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
						}

						foundMethod = false;
						methodBuffer = "";
					}
				}
			}
			else if (mode == ParseMode.EVENTS) {
				if (line.startsWith("[id(")) {
					eventList.add(new Event(line, lineNo));
				}
			}

			line = bufferedReader.readLine();
		}

		bufferedReader.close();

		outputStream.close();
	}

	private String generateDispatch(IDLInterface idlInterface, LinkedHashMap<String, Dispatch> dispatchMap,
			HashMap<String, IDLInterface> interfaceMap, HashMap<String, IDLEnum> enumMap) throws Exception {
		StringBuffer buffer = new StringBuffer();
		
		buffer.append("STDMETHODIMP " + idlInterface.getClassName() + "::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {\n");
		
		buffer.append("static struct {\n");
		buffer.append("const WCHAR *name;\n");
		buffer.append("DISPID dispId;\n");
		buffer.append("} namesIdsList[] = {\n");
		buffer.append("{ NULL }");
        
		ArrayList<String> keys = new ArrayList<>(dispatchMap.keySet());

		if (enumMap != null) {
			keys.addAll(enumMap.keySet());
		}

		Collections.sort(keys, String.CASE_INSENSITIVE_ORDER);

		for (String key : keys) {
			if (dispatchMap.containsKey(key)) {
				Dispatch dispatch = dispatchMap.get(key);

				if ("DISPID_NEWENUM".equals(dispatch.getId()) || 
						"DISPID_VALUE".equals(dispatch.getId())) {
					continue;
				}

				buffer.append(",\n");
				buffer.append("{ L\"" + key + "\", " + dispatch.getId() + " }");
			}
			else if (enumMap != null && enumMap.containsKey(key)) {
				IDLEnum idlEnum = enumMap.get(key);

				buffer.append(",\n");
				buffer.append("{ L\"" + key + "\", " + idlEnum.getId() + " }");
			}
			else {
				System.out.println("\"" + key + "\" not found");
			}
		}

		buffer.append("\n");
		buffer.append("};\n");
		buffer.append("\n");
		
		buffer.append("size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;\n");
		buffer.append("int r;\n");
		buffer.append("while(min <= max) {\n");
		buffer.append("i = (min + max) / 2;\n");
		buffer.append("r = wcsicmp(namesIdsList[i].name, *rgszNames);\n");
		buffer.append("if(!r) {\n");
		buffer.append("*rgDispId = namesIdsList[i].dispId;\n");
	    buffer.append("return S_OK;\n");
	    buffer.append("}\n");
	    buffer.append("if(r < 0)\n");
	    buffer.append("   min = i+1;\n");
	    buffer.append("else\n");
	    buffer.append("   max = i-1;\n");
	    buffer.append("}\n");
	    buffer.append("return DISP_E_MEMBERNOTFOUND;\n");

		buffer.append("}\n");
		buffer.append("\n");

		buffer.append("STDMETHODIMP " + idlInterface.getClassName() + "::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {\n");
		buffer.append("int index = pDispParams->cArgs;\n");
		buffer.append("VARIANT res;\n");
		buffer.append("HRESULT hres = DISP_E_UNKNOWNNAME;\n");
		buffer.append("\n");
		buffer.append("V_VT(&res) = VT_EMPTY;\n");
		buffer.append("\n");
		buffer.append("switch(dispIdMember) {\n");
		buffer.append("case DISPID_VALUE: {\n");
		buffer.append("if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {\n");
		buffer.append("V_VT(&res) = VT_DISPATCH;\n");
		buffer.append("V_DISPATCH(&res) = this;\n");
		buffer.append("hres = S_OK;\n");
		buffer.append("}\n");
		buffer.append("break;\n");
		buffer.append("}\n");
		
		for (String key : dispatchMap.keySet()) {
			Dispatch dispatch = dispatchMap.get(key);

			buffer.append("case " + dispatch.getId() + ": {\n"); 

			int index = 0;

			for (Method method:dispatch.getMethodList()) {
				if (index > 0) {
					buffer.append("else ");
				}
				buffer.append(generateMethod(method));
				index++;
			}
			buffer.append("break;\n"); 
			buffer.append("}\n"); 
		}

        if (enumMap != null) {
	        for (String key : enumMap.keySet()) {
	            IDLEnum idlEnum = enumMap.get(key);

	            buffer.append("case " + idlEnum.getId() + ": {\n");
				buffer.append("if (wFlags & DISPATCH_PROPERTYGET) {\n");
				buffer.append("V_VT(&res) = VT_I2;\n");
				buffer.append("V_I2(&res) = " + idlEnum.getValue() + ";\n");
				buffer.append("hres = S_OK;\n");
				buffer.append("}\n");
				buffer.append("break;\n");
				buffer.append("}\n");
	        }
        }

		buffer.append("default:\n");
		buffer.append("break;\n");
		buffer.append("}\n");
		buffer.append("if (hres == S_OK) {\n");
		buffer.append("if (pVarResult)\n");
		buffer.append("\t*pVarResult = res;\n");
		buffer.append("else\n");
		buffer.append("\tVariantClear(&res);\n");
		buffer.append("}\n");
		buffer.append("else if (hres != S_FALSE) {\n");
		buffer.append("PLOGI.printf(\"dispId=%d (0x%08x), wFlags=%d, hres=%d\", dispIdMember, dispIdMember, wFlags, hres);\n");
		buffer.append("}\n");
		buffer.append("return hres;\n");
		buffer.append("}\n");
		buffer.append("\n");

		buffer.append("STDMETHODIMP " + idlInterface.getClassName() + "::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {\n");
		buffer.append("if (index == MEMBERID_NIL) {\n");
		buffer.append("*pBstrName = SysAllocString(L\"" + idlInterface.getDocumentationName() + "\");\n");
		buffer.append("return S_OK;\n");
		buffer.append("}\n");
		buffer.append("return E_NOTIMPL;\n");
		buffer.append("}\n");
		buffer.append("\n");

		return indent(buffer.toString(), 0);
	}

	private String generateEvents(IDLInterface idlInterface, List<Event> eventList) throws Exception {
		HashMap<String, Integer> eventDispIdMap = new HashMap<String, Integer>();
		eventDispIdMap.put("DISPID_GameEvents_KeyDown", 1000);
		eventDispIdMap.put("DISPID_GameEvents_KeyUp", 1001);
		eventDispIdMap.put("DISPID_GameEvents_Init", 1002);
		eventDispIdMap.put("DISPID_GameEvents_MusicDone", 1003);
		eventDispIdMap.put("DISPID_GameEvents_Exit", 1004);
		eventDispIdMap.put("DISPID_GameEvents_Paused", 1005);
		eventDispIdMap.put("DISPID_GameEvents_UnPaused", 1006);
		eventDispIdMap.put("DISPID_GameEvents_OptionEvent", 1007);
		eventDispIdMap.put("DISPID_SurfaceEvents_Slingshot", 1101);
		eventDispIdMap.put("DISPID_FlipperEvents_Collide", 1200);
		eventDispIdMap.put("DISPID_TimerEvents_Timer", 1300);
		eventDispIdMap.put("DISPID_SpinnerEvents_Spin", 1301);
		eventDispIdMap.put("DISPID_TargetEvents_Dropped", 1302);
		eventDispIdMap.put("DISPID_TargetEvents_Raised", 1303);
		eventDispIdMap.put("DISPID_LightSeqEvents_PlayDone", 1320);
		eventDispIdMap.put("DISPID_HitEvents_Hit", 1400);
		eventDispIdMap.put("DISPID_HitEvents_Unhit", 1401);
		eventDispIdMap.put("DISPID_LimitEvents_EOS", 1402);
		eventDispIdMap.put("DISPID_LimitEvents_BOS", 1403);
		eventDispIdMap.put("DISPID_AnimateEvents_Animate", 1404);

		StringBuffer buffer = new StringBuffer();
		
		buffer.append("HRESULT " + idlInterface.getClassName() + "::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {\n");
		
		buffer.append("static struct {\n");
		buffer.append("DISPID dispId;\n");
		buffer.append("const WCHAR *name;\n");
		buffer.append("} idsNamesList[] = {\n");
		buffer.append("{ NULL }");

		HashMap<String, Event> eventMap = new HashMap<String, Event>();
		for (Event event : eventList) {
			eventMap.put(event.getId(), event);
		}

		ArrayList<String> keys = new ArrayList<>(eventMap.keySet());
		Collections.sort(keys, new Comparator<String>() {
			@Override
			public int compare(String o1, String o2) {
				int id1 = 0;
				int id2 = 0;
				if (eventDispIdMap.containsKey(o1)) {
					id1 = eventDispIdMap.get(o1);
				}
				else {
					try {
						id1 = Integer.parseInt(o1);
					}
					catch(Exception e) {
						System.out.println(o1 + " is not a sort key");
					}
				}
				if (eventDispIdMap.containsKey(o2)) {
					id2 = eventDispIdMap.get(o2);
				}
				else {
					try {
						id2 = Integer.parseInt(o2);
					}
					catch(Exception e) {
						System.out.println(o2 + " is not a sort key");
					}
				}
				return Integer.compare(id1, id2);
			}
		});

		for (String key : keys) {
			Event event = eventMap.get(key);
			buffer.append(",\n");
			buffer.append("{ " + event.getId() + ", L\"_" + event.getName() + "\" }");
		}
		buffer.append("\n");
		buffer.append("};\n");
		buffer.append("\n");
		
		buffer.append("static WCHAR wzName[MAXSTRING];\n");
		buffer.append("size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;\n");
		buffer.append("int r;\n");
		buffer.append("while(min <= max) {\n");
		buffer.append("i = (min + max) / 2;\n");
		buffer.append("if (idsNamesList[i].dispId == dispid) {\n");
		buffer.append("wcscpy(wzName, m_wzName);\n");
		buffer.append("wcscat(wzName, idsNamesList[i].name);\n");
		buffer.append("LPOLESTR fnNames = (LPOLESTR)wzName;\n");
		buffer.append("DISPID tDispid;\n");
		buffer.append("CComPtr<IDispatch> disp;\n");
		buffer.append("g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);\n");
		buffer.append("if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {\n");
		buffer.append("return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);\n");
		buffer.append("}\n");
		buffer.append("return DISP_E_MEMBERNOTFOUND;\n");
		buffer.append("}\n");
		buffer.append("else if (idsNamesList[i].dispId < dispid)\n");
		buffer.append("   min = i+1;\n");
		buffer.append("else\n");
		buffer.append("   max = i-1;\n");
		buffer.append("}\n");
		buffer.append("return DISP_E_MEMBERNOTFOUND;\n");

		buffer.append("}\n");
		buffer.append("\n");

		return indent(buffer.toString(), 0);
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
			else if ("single".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_R4;\n");
				buffer.append("V_R4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			}
			else if ("double".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_R8;\n");
				buffer.append("V_R8(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			}
			else if ("VARIANT_BOOL".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_BOOL;\n");
				buffer.append("V_BOOL(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			}
			else if ("LONG_PTR".equals(param.getType())) {
				buffer.append("V_VT(&var" + index + ") = VT_UI4;\n");
				buffer.append("V_UI4(&var" + index + ") = " + param.getDefaultValue() + ";\n");
			}
			else if ("BSTR".equals(param.getType())) {
				buffer.append("OLECHAR* pszDefault = L" + param.getDefaultValue() + ";\n");
				buffer.append("V_VT(&var" + index + ") = VT_BSTR;\n");
				buffer.append("V_BSTR(&var" + index + ") = SysAllocString(pszDefault);\n");
			}
			else {
				buffer.append("\nUNSUPPORTED DEFAULT\n");
			}
		}
		else {
			buffer.append("V_VT(&var" + index + ") = VT_EMPTY;\n");
		}

		if (type != null) {
			if (!param.getType().equals("VARIANT")) {
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
				buffer.append("VariantCopyInd(&var" + index + ", ");

				if (param.hasDefaultValue() || param.isOptional()) {
					buffer.append("(index > 0) ? &pDispParams->rgvarg[--index] : &var" + index);
				}
				else {
					buffer.append("&pDispParams->rgvarg[--index]");
				}

				buffer.append(");\n");
			}
		}
		else {
			buffer.append("VariantCopyInd(&var" + index + ", &pDispParams->rgvarg[--index]);\n");
		}

		return buffer.toString();
	}

	private String generateMethod(Method method) {
		StringBuffer buffer = new StringBuffer();

		buffer.append("hres = " + method.getPrefix() + method.getName() + "(");

		int index = 0;

		StringBuffer header = new StringBuffer();

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
				else if ("unsigned short".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_UI2", method));
					buffer.append("V_UI2(&var" + index + ")");
				}
				else if ("short".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_I2", method));
					buffer.append("V_I2(&var" + index + ")");
				}
				else if ("float".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_R4", method));
					buffer.append("V_R4(&var" + index + ")");
				}
				else if ("single".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_R4", method));
					buffer.append("V_R4(&var" + index + ")");
				}
				else if ("double".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_R8", method));
					buffer.append("V_R8(&var" + index + ")");
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
				else if ("IUnknown*".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, null, method));
					buffer.append("V_UNKNOWN(&var" + index + ")");
				}
				else if ("IWMPMedia*".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, null, method));
					buffer.append("(IWMPMedia*)&var" + index );
				}
				else if ("IWMPPlaylist*".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, null, method));
					buffer.append("(IWMPPlaylist*)&var" + index );
				}
				else if ("IGroupActor*".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, null, method));
					buffer.append("(IGroupActor*)&var" + index );
				}
				else if ("_Bitmap*".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, null, method));
					buffer.append("(_Bitmap*)&var" + index );
				}
				else if ("IFontDisp*".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, null, method));
					buffer.append("(IFontDisp*)&var" + index );
				}
				else if ("OLE_COLOR".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_UI4", method));
					buffer.append("(OLE_COLOR)V_UI4(&var" + index + ")");
				}
				else if ("int**".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_INT_PTR", method));
					buffer.append("(int**)V_INT_PTR(&var" + index + ")");
				}
				else if ("LONG_PTR".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_UI4", method));
					buffer.append("(LONG_PTR)V_UI4(&var" + index + ")");
				}   
				else if ("UserDefaultOnOff".equals(param.getType())
						|| "FXAASettings".equals(param.getType())
						|| "PhysicsSet".equals(param.getType())
						|| "BackglassIndex".equals(param.getType())
						|| "ImageAlignment".equals(param.getType())
						|| "PlungerType".equals(param.getType()) 
						|| "TextAlignment".equals(param.getType()) 
						|| "TriggerShape".equals(param.getType())
						|| "LightState".equals(param.getType()) 
						|| "KickerType".equals(param.getType()) 
						|| "DecalType".equals(param.getType()) 
						|| "SizingType".equals(param.getType()) 
						|| "TargetType".equals(param.getType()) 
						|| "GateType".equals(param.getType()) 
						|| "RampType".equals(param.getType()) 
						|| "RampImageAlignment".equals(param.getType())
						|| "SequencerState".equals(param.getType())
						|| "RenderMode".equals(param.getType())
						|| "Interpolation".equals(param.getType())
						|| "Alignment".equals(param.getType())) {
					header.append(generateHeaderVariant(index, param, "VT_I4", method));
					buffer.append("(" + param.getType() + ")V_I4(&var" + index + ")");
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
					buffer.append("V_VARIANTREF(var" + index + ")");
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
				else if ("unsigned short*".equals(param.getType())) {
					buffer.insert(0, "V_VT(&res) = VT_UI2;\n");
					buffer.append("(unsigned short*)&V_UI2(&res)");
				}
				else if ("short*".equals(param.getType())) {
					buffer.insert(0, "V_VT(&res) = VT_I2;\n");
					buffer.append("(short*)&V_I2(&res)");
				}
				else if ("single*".equals(param.getType())) {
					buffer.insert(0, "V_VT(&res) = VT_R4;\n");
					buffer.append("&V_R4(&res)");
				}
				else if ("float*".equals(param.getType())) {
					buffer.insert(0, "V_VT(&res) = VT_R4;\n");
					buffer.append("&V_R4(&res)");
				}
				else if ("double*".equals(param.getType())) {
					buffer.insert(0, "V_VT(&res) = VT_R8;\n");
					buffer.append("(double*)&V_R8(&res)");
				}
				else if ("long*".equals(param.getType())) {
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
				else if ("IBall**".equals(param.getType())
						|| "IFontDisp**".equals(param.getType())
						|| "ITable**".equals(param.getType())
						|| "IRom**".equals(param.getType())
						|| "IRoms**".equals(param.getType())
						|| "IGame**".equals(param.getType())
						|| "IGames**".equals(param.getType())
						|| "IGameSettings**".equals(param.getType())
						|| "IControllerSettings**".equals(param.getType())
						|| "IWMPMedia**".equals(param.getType())
						|| "IWMPMediaCollection**".equals(param.getType())
						|| "IWMPControls**".equals(param.getType())
						|| "IWMPSettings**".equals(param.getType())
						|| "IWMPPlaylistCollection**".equals(param.getType())
						|| "IWMPNetwork**".equals(param.getType())
						|| "IWMPPlaylist**".equals(param.getType())
						|| "IWMPCdromCollection**".equals(param.getType())
						|| "IWMPClosedCaption**".equals(param.getType())
						|| "IWMPError**".equals(param.getType())
						|| "IGroupActor**".equals(param.getType())
						|| "IFrameActor**".equals(param.getType())
						|| "ILabelActor**".equals(param.getType())
						|| "IVideoActor**".equals(param.getType())
						|| "IImageActor**".equals(param.getType())
						|| "IUltraDMD**".equals(param.getType())
						|| "IActionFactory**".equals(param.getType())
						|| "ICompositeAction**".equals(param.getType())
						|| "ITweenAction**".equals(param.getType())
						|| "_Bitmap**".equals(param.getType())
						|| "IDispatch**".equals(param.getType())) {
					buffer.insert(0, "V_VT(&res) = VT_DISPATCH;\n");
					buffer.append("(" + param.getType() + ")&V_DISPATCH(&res)");
				}
				else if ("SAFEARRAY(VARIANT)*".equals(param.getType())) {
					buffer.insert(0, "V_VT(&res) = VT_VARIANT|VT_ARRAY;\n");
					buffer.append("(SAFEARRAY**)&V_ARRAY(&res)");
				}
				else if ("UserDefaultOnOff*".equals(param.getType())
						|| "FXAASettings*".equals(param.getType())
						|| "PhysicsSet*".equals(param.getType())
						|| "BackglassIndex*".equals(param.getType())
						|| "ImageAlignment*".equals(param.getType())
						|| "PlungerType*".equals(param.getType())
						|| "TextAlignment*".equals(param.getType())
						|| "TriggerShape*".equals(param.getType())
						|| "LightState*".equals(param.getType())
						|| "KickerType*".equals(param.getType())
						|| "DecalType*".equals(param.getType())
						|| "SizingType*".equals(param.getType())
						|| "TargetType*".equals(param.getType())
						|| "GateType*".equals(param.getType())
						|| "RampType*".equals(param.getType())
						|| "RampImageAlignment*".equals(param.getType())
						|| "WMPOpenState*".equals(param.getType())
						|| "WMPPlayState*".equals(param.getType())
						|| "Interpolation*".equals(param.getType())
						|| "RenderMode*".equals(param.getType())) {
					buffer.insert(0, "V_VT(&res) = VT_I4;\n");
					buffer.append("(" + param.getType() + ")&V_I4(&res)");
				}
				else {
					System.out.println("Unknown Type: " + param.getType());
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
		IDLParserToCpp parser = new IDLParserToCpp();

		parser.parse(
				"../../inc/b2s/B2S.idl",
				"b2s_i_proxy.cpp",
				Arrays.asList(
						new IDLInterface("_Server", "Server")));
		
		parser.parse(
				"../../inc/ultradmd/UltraDMD.idl",
				"ultradmd_i_proxy.cpp",
				Arrays.asList(
						new IDLInterface("IDMDObject", "UltraDMDDMDObject")));

		parser.parse(
			"../../inc/pup/PUP.idl",
			"pup_i_proxy.cpp",
			Arrays.asList(
				new IDLInterface("IPinDisplay", "PUPPinDisplay", "PinDisplay")));

		parser.parse(
			"../../inc/flexdmd/FlexDMD.idl",
			"flexdmd_i_proxy.cpp",
			Arrays.asList(
				new IDLInterface("IFlexDMD", "FlexDMD"),
				new IDLInterface("IGroupActor", "Group", "Object"),
				new IDLInterface("IFrameActor", "Frame", "Object"),
				new IDLInterface("ICompositeAction", "ParallelAction", "Object"),
                new IDLInterface("ICompositeAction", "SequenceAction", "Object"),
				new IDLInterface("ITweenAction", "TweenAction", "Object"),
				new IDLInterface("IActionFactory", "ActionFactory", "Object"),
				new IDLInterface("IImageActor", "Image", "Object"),
				new IDLInterface("IVideoActor", "Video", "Object"),
				new IDLInterface("ILabelActor", "Label", "Object")));

        parser.parse(
            "../../inc/vpinmame/VPinMAME.idl",
            "vpinmame_i_proxy.cpp",
			Arrays.asList(
				new IDLInterface("IController", "VPinMAMEController", "Controller"),
				new IDLInterface("IControllerSettings", "VPinMAMEControllerSettings", "ControllerSettings"),
				new IDLInterface("IGame", "VPinMAMEGame", "Game"),
				new IDLInterface("IGames", "VPinMAMEGames", "Games"),
				new IDLInterface("IGameSettings", "VPinMAMEGameSettings", "GameSettings"),
				new IDLInterface("IRom", "VPinMAMERom", "Rom"),
				new IDLInterface("IRoms", "VPinMAMERoms", "Roms")));

		parser.parse(
			"../../inc/wmp/wmp.idl",
			"wmp_i_proxy.cpp",
			Arrays.asList(
				new IDLInterface("IWMPCore", "WMPCore"),
				new IDLInterface("IWMPControls", "WMPControls"),
				new IDLInterface("IWMPSettings", "WMPSettings")));

		parser.parse(
			"../../../vpinball.idl",
			"vpinball_standalone_i_proxy.cpp",
			Arrays.asList(
				new IDLInterface("ICollection", "Collection"),
				new IDLInterface("ICollectionEvents", "Collection"),
				new IDLInterface("ITable", "PinTable", "Table"),
				new IDLInterface("ITableEvents", "PinTable"),
				new IDLInterface("ITableGlobal", "ScriptGlobalTable", "TableGlobal",
						Arrays.asList(
								"LightState",
								//"BackglassIndex",
								//"Filters",
								"ImageAlignment",
								"Shape",
								"TriggerShape",
								"RampType",
								"PlungerType",
								//"UserDefaultOnOff",
								//"FXAASettings",
								//"PhysicsSet",
								"TargetType",
								"GateType",
								"TextAlignment",
								"DecalType",
								"SequencerState",
								"SizingType",
								"KickerType",
								"RampImageAlignment")),
				new IDLInterface("IWall", "Surface", "Wall"),
				new IDLInterface("IWallEvents", "Surface"),
				new IDLInterface("IControlPoint", "DragPoint", "ControlPoint"),
				new IDLInterface("IFlipper", "Flipper"),
				new IDLInterface("IFlipperEvents", "Flipper"),
				new IDLInterface("ITimer", "Timer"),
				new IDLInterface("ITimerEvents", "Timer"),
				new IDLInterface("IPlunger", "Plunger"),
				new IDLInterface("IPlungerEvents", "Plunger"),
				new IDLInterface("ITextbox", "Textbox"),
				new IDLInterface("ITextboxEvents", "Textbox"),
				new IDLInterface("IBumper", "Bumper"),
				new IDLInterface("IBumperEvents", "Bumper"),
				new IDLInterface("ITrigger", "Trigger"),
				new IDLInterface("ITriggerEvents", "Trigger"),
				new IDLInterface("ILight", "Light"),
				new IDLInterface("ILightEvents", "Light"),
				new IDLInterface("IKicker", "Kicker"),
				new IDLInterface("IKickerEvents", "Kicker"),
				new IDLInterface("IPrimitive", "Primitive"),
				new IDLInterface("IPrimitiveEvents", "Primitive"),
				new IDLInterface("IHitTarget", "HitTarget"),
				new IDLInterface("IHitTargetEvents", "HitTarget"),
				new IDLInterface("IGate", "Gate"),
				new IDLInterface("IGateEvents", "Gate"),
				new IDLInterface("ISpinner", "Spinner"),
				new IDLInterface("ISpinnerEvents", "Spinner"),
				new IDLInterface("IRamp", "Ramp"),
				new IDLInterface("IRampEvents", "Ramp"),
				new IDLInterface("IFlasher", "Flasher"),
				new IDLInterface("IFlasherEvents", "Flasher"),
				new IDLInterface("IRubber", "Rubber"),
				new IDLInterface("IRubberEvents", "Rubber"),
				new IDLInterface("IDispReel", "DispReel"),
				new IDLInterface("IDispReelEvents", "DispReel"),
				new IDLInterface("ILightSeq", "LightSeq"),
				new IDLInterface("ILightSeqEvents", "LightSeq"),
				new IDLInterface("IVPDebug", "DebuggerModule", "VPDebug"),
				new IDLInterface("IDecal", "Decal"),
				new IDLInterface("IBall", "BallEx", "IBall")));
	}
}
