package org.vpinball;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class Dispatch {
	private static final Map<String, String> DISPID_MAP = new HashMap<String, String>();
    static {
        DISPID_MAP.put("-8",  "DISPID_COLLECT");
        DISPID_MAP.put("-7",  "DISPID_DESTRUCTOR");
        DISPID_MAP.put("-6",  "DISPID_CONSTRUCTOR");
        DISPID_MAP.put("-5",  "DISPID_EVALUATE");
        DISPID_MAP.put("-4",  "DISPID_NEWENUM");
        DISPID_MAP.put("-3",  "DISPID_PROPERTYPUT");
        DISPID_MAP.put("-1",  "DISPID_UNKNOWN");
        DISPID_MAP.put("0",   "DISPID_VALUE");
    }

	private String id;
	private ArrayList<Method> methodList = new ArrayList<Method>();

	public String getId() {
		return id;
	}

	public void setId(String id) {
        if (DISPID_MAP.containsKey(id)) {
            this.id = DISPID_MAP.get(id);
        }
        else {
            this.id = id;
	    }
	}

	public ArrayList<Method> getMethodList() {
		return methodList;
	}

	public int getMethodCount() {
		return methodList.size();
	}

	public void getMethodList(ArrayList<Method> methodList) {
		this.methodList = methodList;
	}

	public Method getMethod(int index) {
		return methodList.get(index);
	}

	public void addMethod(Method method) {
		methodList.add(method);
	}

	public boolean isDispIdValue() {
		return id.equals("DISPID_VALUE");
	}

	public boolean isDispIdNewEnum() {
		return id.equals("DISPID_NEWENUM");
	}
}
