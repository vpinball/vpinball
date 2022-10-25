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

public class IDLParserToCpp {
	public static Map<String, String> INTERFACE_CLASS_MAP  = new HashMap<String, String>() {{
		put("ICollection", "Collection");
		put("ICollectionEvents", "Collection");
		put("ITable", "PinTable");
		put("ITableEvents", "PinTable");
		put("ITableGlobal", "ScriptGlobalTable");
		put("IWall", "Surface");
		put("IWallEvents", "Surface");
		put("IControlPoint", "DragPoint");
		put("IFlipper", "Flipper");
		put("IFlipperEvents", "Flipper");
		put("ITimer", "Timer");
		put("ITimerEvents", "Timer");
		put("IPlunger", "Plunger");
		put("IPlungerEvents", "Plunger");
		put("ITextbox", "Textbox");
		put("ITextboxEvents", "Textbox");
		put("IBumper", "Bumper");
		put("IBumperEvents", "Bumper");
		put("ITrigger", "Trigger");
		put("ITriggerEvents", "Trigger");
		put("ILight", "Light");
		put("ILightEvents", "Light");
		put("IKicker", "Kicker");
		put("IKickerEvents", "Kicker");
		put("IPrimitive", "Primitive");
		put("IPrimitiveEvents", "Primitive");
		put("IHitTarget", "HitTarget");
		put("IHitTargetEvents", "HitTarget");
		put("IGate", "Gate");
		put("IGateEvents", "Gate");
		put("ISpinner", "Spinner");
		put("ISpinnerEvents", "Spinner");
		put("IRamp", "Ramp");
		put("IRampEvents", "Ramp");
		put("IFlasher", "Flasher");
		put("IFlasherEvents", "Flasher");
		put("IRubber", "Rubber");
		put("IRubberEvents", "Rubber");
		put("IDispReel", "DispReel");
		put("IDispReelEvents", "DispReel");
		put("ILightSeq", "LightSeq");
		put("ILightSeqEvents", "LightSeq");
		put("IVPDebug", "DebuggerModule");
		put("IDecal", "Decal");
		put("IBall", "BallEx");
		
	    put("IController", "VPinMAMEController");
	    
	    put("IFileSystem", "FileSystemController");
	}};

	public enum ParseMode {
		NONE,
		METHODS,
		EVENTS,
	}

	public void parse(String in, String out) throws Exception {
		LinkedHashMap<String, Dispatch> dispatchMap = new LinkedHashMap<String, Dispatch>();
		ArrayList<Event> eventList = new ArrayList<Event>();
		
		BufferedReader bufferedReader = new BufferedReader(new FileReader(in));
		
		FileOutputStream outputStream = new FileOutputStream(out);
		
		outputStream.write("#include \"stdafx.h\"\n".getBytes());
		outputStream.write("#include \"olectl.h\"\n".getBytes());
		outputStream.write("\n".getBytes());

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
			else if (line.startsWith("dispinterface ")) {
				String interfaceName = line.split(" ")[1].trim();
				
				if (INTERFACE_CLASS_MAP.containsKey(interfaceName)) {
					mode = ParseMode.EVENTS;
					className = INTERFACE_CLASS_MAP.get(interfaceName);
					eventList.clear();
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
				else if (mode == ParseMode.EVENTS) {
					outputStream.write(generateEvents(className, eventList).getBytes());
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

	private String generateDispatch(String className, LinkedHashMap<String, Dispatch> dispatchMap) throws Exception {
		StringBuffer buffer = new StringBuffer();

		buffer.append("robin_hood::unordered_map<wstring, int> " + className + "::m_nameIDMap = {\n");
		
		int index = 0;

		for (String key : dispatchMap.keySet()) {
			Dispatch dispatch = dispatchMap.get(key);

			if ("DISPID_NEWENUM".equals(dispatch.getId())) {
				continue;
			}

			if (index > 0) {
				buffer.append(",\n");
			}
			
			buffer.append("robin_hood::pair<wstring, int> { wstring(L\"" + key.toLowerCase() + "\"), " + dispatch.getId() + " }");

			index++;
		}
		
		buffer.append("\n");
		buffer.append("};\n");
		buffer.append("\n");
		buffer.append("STDMETHODIMP " + className + "::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {\n");
		buffer.append("wstring name = wstring(*rgszNames);\n");
		buffer.append("std::transform(name.begin(), name.end(), name.begin(), tolower);\n");
		buffer.append("const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);\n");
		buffer.append("if (it != m_nameIDMap.end()) {\n");
		buffer.append("*rgDispId = it->second;\n");
		buffer.append("return S_OK;\n");
		buffer.append("}\n");
		buffer.append("return DISP_E_UNKNOWNNAME;\n");
		buffer.append("}\n");
		buffer.append("\n");


		buffer.append("STDMETHODIMP " + className + "::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {\n");
		buffer.append("switch(dispIdMember) {\n");

		for (String key : dispatchMap.keySet()) {
			Dispatch dispatch = dispatchMap.get(key);

			if (dispatch.getMethodCount() == 1) {
				buffer.append("case " + dispatch.getId() + ": {\n");
				buffer.append(generateMethod(dispatch.getMethod(0)));
				buffer.append("}\n");
				buffer.append("\n");
			}
			else {
				buffer.append("case " + dispatch.getId() + ":\n");
				buffer.append("switch(pDispParams->cArgs) {\n");
				for (Method method:dispatch.getMethodList()) {
					buffer.append("case " + method.getCArgs() + ": {\n");
					buffer.append(generateMethod(method));
					buffer.append("}\n");
					buffer.append("\n");
				}
				buffer.append("default:\n");
				buffer.append("break;\n");
				buffer.append("}\n");

				buffer.append("break;\n");
				buffer.append("\n");
			}
		}

		buffer.append("default:\n");
		buffer.append("break;\n");
		buffer.append("}\n");

		buffer.append("\n");

		buffer.append("return DISP_E_UNKNOWNNAME;\n");
		buffer.append("}\n");

		buffer.append("\n");		

		return indent(buffer.toString(), 0);
	}

	private String generateEvents(String className, List<Event> eventList) throws Exception {
		StringBuffer buffer = new StringBuffer();

		buffer.append("robin_hood::unordered_map<int, wstring> " + className + "::m_idNameMap = {\n");

		int index = 0;

		for (Event event : eventList) {
			if (index > 0) {
				buffer.append(",\n");
			}

			buffer.append("robin_hood::pair<int, wstring> { " + event.getId() + ", wstring(L\"_" + event.getName() + "\") }");

			index++;
		}

		buffer.append("\n");
		buffer.append("};\n");
		buffer.append("\n");

		buffer.append("HRESULT " + className + "::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {\n");
		buffer.append("CComPtr<IDispatch> disp;\n");
		buffer.append("g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);\n");
		buffer.append("\n");
		buffer.append("const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);\n");
		buffer.append("if (it != m_idNameMap.end()) {\n");
		buffer.append("wstring name = wstring(m_wzName) + it->second;\n");
		buffer.append("LPOLESTR fnNames = (LPOLESTR)name.c_str();\n");
		buffer.append("\n");
		buffer.append("DISPID tDispid;\n");
		buffer.append("const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);\n");
		buffer.append("\n");
		buffer.append("if (hr == S_OK) {\n");
		buffer.append("disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);\n");
		buffer.append("}\n");
		buffer.append("}\n");
		buffer.append("\n");
		buffer.append("return S_OK;\n");
		buffer.append("}\n");
		buffer.append("\n");

		return indent(buffer.toString(), 0);
	}

	private String generateHeaderVariant(int index, Param param, String type, Method method) {
		StringBuffer buffer = new StringBuffer();

		buffer.append("CComVariant var" + index);
		if (param.hasDefaultValue()) {
			buffer.append("(" + param.getDefaultValue() + ")");
		}
		buffer.append(";\n");

		if (type != null) {
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
			if (param.hasDefaultValue() || param.isOptional()) {
				buffer.append("if (index > 0) {\n");
				buffer.append("VariantCopy(&var" + index + ",  &pDispParams->rgvarg[--index]);\n");
				buffer.append("}\n");
			}
			else {
				buffer.append("VariantCopy(&var" + index + ", pDispParams->rgvarg);\n");
			}
		}

		return buffer.toString();
	}

	private String generateMethod(Method method) {
		StringBuffer buffer = new StringBuffer();

		buffer.append("return " + method.getPropName() + "(");

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
						|| "SequencerState".equals(param.getType())) {
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
				else if ("IBall**".equals(param.getType())
						|| "IFontDisp**".equals(param.getType())
						|| "ITable**".equals(param.getType())
						|| "IDispatch**".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_DISPATCH;\n");
					buffer.append("(" + param.getType() + ")&V_DISPATCH(pVarResult)");
				}
				else if ("SAFEARRAY(VARIANT)*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_SAFEARRAY;\n");
					buffer.append("(SAFEARRAY**)&V_ARRAY(pVarResult)");
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
						|| "RampImageAlignment*".equals(param.getType())) {
					buffer.insert(0, "V_VT(pVarResult) = VT_I4;\n");
					buffer.append("(" + param.getType() + ")&V_I4(pVarResult)");
				}
				else {
					buffer.append(param.getType());
				}
			}

			index++;
		}

		buffer.append(");\n");

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
		//parser.parse("/Users/jmillard/vpvr/inc/vpinmame/VPinMAME.idl", "/Users/jmillard/vpvr/inc/vpinmame/vpinmame_controller.cpp");
		parser.parse("/Users/jmillard/vpvr/vpinball.idl", "/Users/jmillard/vpvr/vpinball_standalone.cpp");
	}
}
