package org.vpinball;

public class Event {
	private int lineNo;
	private String line;
	private String id;
	private String name;
	
	public Event(String line, int lineNo) throws Exception {
		this.line = line;
		this.lineNo = lineNo;
			
		this.id = line.substring(line.indexOf("(") + 1, line.indexOf(")"));
		
		int index = line.indexOf(" void ");
		name = line.substring(index + " void ".length(), line.indexOf("(", index));
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
}
