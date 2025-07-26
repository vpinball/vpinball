## IDL Parsers

We provide two custom Java-based IDL parsers to generate code for Visual Pinball Standalone.

- **IDLParserToCpp.java**  
  Parses COM IDL files from Visual Pinball and related components, emitting C++ stubs that implement `IDispatch::GetIDsOfNames` and `IDispatch::Invoke`. Generates code for:  
  - `vpinball.idl`  
  - `B2S.idl`  

- **IDLParserToC.java**  
  Parses COM IDL files from Wine, emitting C stubs that implement `IDispatch::GetIDsOfNames` and `IDispatch::Invoke`. Generates code for:  
  - `vbsregexp55.idl` (VBScript RegExp)  
  - `scrrun.idl` (Dictionary, FileSystemObject, and filesystem APIs)

Both parsers reverse-engineer the name-to-DISPID mappings and invocation logic normally provided by a Windows Type Library.

> [!NOTE]
> Wine’s TypeLib engine has not be ported for Visual Pinball Standalone. 

> [!IMPORTANT]
> These parsers were developed through reverse engineering and extensive trial-and-error, so there are files that require manual edits.

---

## Files Requiring **No** Changes

- `standalone/vpinball_standalone_i_proxy.cpp`
- `standalone/inc/b2s/b2s_i_proxy.cpp`
- `standalone/inc/wine/dlls/scrrun/dictionary_proxy.c`
- `standalone/inc/wine/dlls/vbscript/regexp_proxy.c`

---

## Files **With** Manual Edits

### 1. `standalone/inc/wine/dlls/scrrun/filesystem_proxy.c`

Copy the following entries from the `IFileSystem3` `GetIDsOfNames` and `Invoke` interface functions into the `IFileSystem` interface:

```
GetFileVersion
GetStandardStream
```

> [!NOTE]
> `GetIDsOfNames` has to be in case-insensitive order.

Delete the `IFileSystem3` `GetIDsOfNames` and `Invoke` interface functions.

Replace:

```
static HRESULT WINAPI filesys_GetIDsOfNames(IFileSystem *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)

static HRESULT WINAPI filesys_Invoke(IFileSystem *iface, DISPID dispIdMember,
                REFIID riid, LCID lcid, WORD wFlags,
                DISPPARAMS *pDispParams, VARIANT *pVarResult,
                EXCEPINFO *pExcepInfo, UINT *puArgErr)
```

with:

```
static HRESULT WINAPI filesys_GetIDsOfNames(IFileSystem3 *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)

static HRESULT WINAPI filesys_Invoke(IFileSystem3 *iface, DISPID dispIdMember,
                REFIID riid, LCID lcid, WORD wFlags,
                DISPPARAMS *pDispParams, VARIANT *pVarResult,
                EXCEPINFO *pExcepInfo, UINT *puArgErr)
```

---

### Generate IDL for projects without IDL:

B2SServer does not supply IDL files.

To generate an IDL file from a DLL or EXE:

- Open `Windows Developer Command Prompt`

```
tlbexp C:\Users\jsm17\OneDrive\Desktop\B2SBackglassServer.dll 
```

- Open `oleview` (x64) in `"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\oleview.exe"`

- `File` > `View TypeLib...` > Select `B2SBackglassServer.tlb`

- `File` > `Save As...` > Enter `B2S.idl`

- The saved IDL file will be encoded in UTF-16. On MacOS or Linux convert the file to UTF-8:

```
iconv -f utf-16 -t utf-8 B2S.idl > B2S.idl_tmp
mv B2S.idl_tmp B2S.idl
```

### Build widl:

Wine’s widl tool is an IDL compiler that parses COM IDL files and automatically generates C headers and stubs needed to implement IDispatch interfaces.

```
brew install llvm lld
git clone https://gitlab.winehq.org/wine/wine.git
cd wine
export PATH="/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/bison/bin:$PATH"
./configure --without-freetype --enable-win64
make -j10
```

### Generating IDispatch interface files:

```
cd standalone/scripts
./widlgen
```


