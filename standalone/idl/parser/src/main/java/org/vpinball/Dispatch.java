package org.vpinball;

import java.util.ArrayList;

public class Dispatch {
	private String id;
	private ArrayList<Method> methodList = new ArrayList<Method>();

	public String getId() {
		return id;
	}

	public void setId(String id) {
		if (id.equals("-4")) {
			this.id = "DISPID_NEWENUM";
		}
		else if (id.equals("0")) {
			this.id = "DISPID_VALUE";
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
}
