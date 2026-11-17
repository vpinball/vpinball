package org.vpinball;

import java.util.ArrayList;
import java.util.List;

public class Event {
	private int lineNo;
	private String line;
	private String id;
	private String name;

	private ArrayList<Param> paramList = new ArrayList<Param>();

	public Event(String line, int lineNo) throws Exception {
		this.line = line;
		this.lineNo = lineNo;

		this.id = line.substring(line.indexOf("(") + 1, line.indexOf(")"));

		int index = line.indexOf(" void ");
		name = line.substring(index + " void ".length(), line.indexOf("(", index));

		int paramStartIndex = line.indexOf("(", index);
		String params = line.substring(paramStartIndex + 1, line.indexOf(");", paramStartIndex)).trim();

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

	public int getLineNo() {
		return lineNo;
	}

	public String getLine() {
		return line;
	}

	public String getId() {
		return id;
	}

	public String getName() {
		return name;
	}

	public List<Param> getParamList() {
		return paramList;
	}
}
