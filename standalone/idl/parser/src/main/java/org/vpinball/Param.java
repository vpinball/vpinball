package org.vpinball;

public class Param {
	private static String TOKEN_DEFAULT_VALUE = "defaultvalue(";

	public enum ParamType {
		OUT_RETVAL,
		OUT,
		IN,
	}

	private ParamType paramType;
	private String type;
	private String name;
	private String defaultValue;
	private boolean optional = false;

	public Param(String param) {
		if (param.indexOf("[out, retval]") != -1 || param.indexOf("[ out, retval ]") != -1) {
			paramType = ParamType.OUT_RETVAL;
		}
		else if (param.indexOf("[out]") != -1) {
			paramType = ParamType.OUT;
		}
		else if (param.indexOf("[in, optional]") != -1) {
			paramType = ParamType.IN;
			optional = true;
		}
		else {
			paramType = ParamType.IN;
		}

		int defaultValueIndex = param.indexOf(TOKEN_DEFAULT_VALUE);

		if (defaultValueIndex != -1) {
			defaultValue = param.substring(defaultValueIndex + TOKEN_DEFAULT_VALUE.length(), param.indexOf(")", defaultValueIndex));
		}

		param = param.replaceAll("\\[.*?\\]", "");
		param = param.replaceAll("(?:/\\*(?:[^*]|(?:\\*+[^*/]))*\\*+/)|(?://.*)", "");
		param = param.replaceAll("\\* \\*", " **");
		param = param.replaceAll("\\s+", " ");
		param = param.trim();

		String[] tokens = param.split(" ");

		type = tokens[0];

		if (tokens.length > 1) {
			if (tokens.length == 2) {
				name = tokens[1];
			}
			else if (tokens.length == 3) {
				type += " " + tokens[1];
				name = tokens[2];
			}

			if (name.startsWith("**")) {
				type += "**";
			}
			else if (name.startsWith("*")) {
				type += "*";
			}

			name = name.replaceAll("\\*", "");
		}
	}

	public ParamType getParamType() {
		return paramType;
	}

	public String getType() {
		return type;
	}

	public void setKey(String type) {
		this.type = type;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public boolean hasDefaultValue() {
		return defaultValue != null;
	}

	public boolean isOptional() {
		return optional;
	}

	public String getDefaultValue() {
		return defaultValue;
	}
}
