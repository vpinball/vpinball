package org.vpinball;

import java.util.List;

public class IDLInterface {
	String interfaceName;
	String className;
	List<String> enumList;
	String documentationName;
	
	public IDLInterface(String interfaceName, String className) {
		this.interfaceName = interfaceName;
		this.className = className;
	}
	
	public IDLInterface(String interfaceName, String className, String documentationName) {
		this.interfaceName = interfaceName;
		this.className = className;
		this.documentationName = documentationName;
	}

	public IDLInterface(String interfaceName, String className, List<String> enumList) {
		this.interfaceName = interfaceName;
		this.className = className;
		this.enumList = enumList;
	}
	
	public IDLInterface(String interfaceName, String className, String documentationName, List<String> enumList) {
		this.interfaceName = interfaceName;
		this.className = className;
		this.documentationName = documentationName;
		this.enumList = enumList;
	}

	public String getInterfaceName() {
		return interfaceName;
	}

	public void setInterfaceName(String interfaceName) {
		this.interfaceName = interfaceName;
	}

	public String getClassName() {
		return className;
	}

	public void setClassName(String className) {
		this.className = className;
	}

	public List<String> getEnumList() {
		return enumList;
	}

	public void setEnumList(List<String> enumList) {
		this.enumList = enumList;
	}

	public String getDocumentationName() {
		return documentationName != null ? documentationName : className;
	}

	public void setDocumentationName(String documentationName) {
		this.documentationName = documentationName;
	}
}
