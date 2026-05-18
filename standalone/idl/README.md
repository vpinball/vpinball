## IDL Parser

We provide a custom Java-based IDL parser to generate code for Visual Pinball Standalone.

- **IDLParserToCpp.java**  
  Parses COM IDL files from Visual Pinball and related components, emitting C++ stubs that implement `IDispatch::GetIDsOfNames` and `IDispatch::Invoke`. Generates code for:  
  - `vpinball.idl`  

It reverse-engineers the name-to-DISPID mappings and invocation logic normally provided by a Windows Type Library.

Regenerate the proxy stub with:

```
./standalone/scripts/genproxy.sh
```

This runs the parser via `gradle run`, overwriting `standalone/vpinball_standalone_i_proxy.cpp`.

> [!NOTE]
> Wine’s TypeLib engine has not be ported for Visual Pinball Standalone. 

> [!IMPORTANT]
> This parser was developed through reverse engineering and extensive trial-and-error.

### Generating IDispatch interface files:

Regenerate the widl-compiled interface files with:

```
./standalone/scripts/widlgen.sh
```

This runs Wine’s `widl`, overwriting `standalone/vpinball_standalone_i.h` and `standalone/vpinball_standalone_i.c`.
