// license:GPLv3+

#include "core/stdafx.h"
#include "vpx-test.h"
#include "doctest.h"

#include "core/DynamicScript.h"

#include <cstdlib>
#include <cstring>

// Counts how many string elements have had their ScriptString::Release called, so the
// test can verify the host walks the array and releases each element (not just the block).
static int g_stringElemReleaseCount = 0;

// Helper to build a 2D string ScriptArray from a flat array of C strings.
// Layout: [Release][lengths[0]=rows][lengths[1]=cols][ScriptString values...]
// Each element is a ScriptString carrying its own Release hook (heap-allocated copy),
// matching the scalar string convention so the lifecycle is owned per element.
static ScriptArray* MakeStringArray2D(unsigned int rows, unsigned int cols, const char* const* strings)
{
   const size_t dataSize = rows * cols * sizeof(ScriptString);
   ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + 2 * sizeof(unsigned int) + dataSize));
   array->Release = [](ScriptArray* me) { free(me); };
   array->lengths[0] = rows;
   array->lengths[1] = cols;
   ScriptString* pData = reinterpret_cast<ScriptString*>(&array->lengths[2]);
   for (unsigned int i = 0; i < rows * cols; i++)
   {
      const size_t n = strlen(strings[i]) + 1;
      char* s = new char[n];
      memcpy(s, strings[i], n);
      pData[i] = { [](ScriptString* str) { delete[] str->string; ++g_stringElemReleaseCount; }, s };
   }
   return array;
}

// Helper to build a 2D bool ScriptArray from a flat array of chars.
// Layout: [Release][lengths[0]=rows][lengths[1]=cols][char values...]
static ScriptArray* MakeBoolArray2D(unsigned int rows, unsigned int cols, const char* values)
{
   const size_t dataSize = rows * cols * sizeof(char);
   ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + 2 * sizeof(unsigned int) + dataSize));
   array->Release = [](ScriptArray* me) { free(me); };
   array->lengths[0] = rows;
   array->lengths[1] = cols;
   char* pData = reinterpret_cast<char*>(&array->lengths[2]);
   memcpy(pData, values, dataSize);
   return array;
}

// Handlers for the test plugin class
namespace TestPluginArrays {

static void TestAddRef(void*, int, ScriptVariant*, ScriptVariant*) { }
static void TestRelease(void*, int, ScriptVariant*, ScriptVariant*) { }

static void get_StringArray2D(void*, int, ScriptVariant*, ScriptVariant* pRet)
{
   static const char* strings[] = { "l-1", "on", "coil1", "pulse", "gi1", "off" };
   pRet->vArray = MakeStringArray2D(3, 2, strings);
}

static void get_BoolArray2D(void*, int, ScriptVariant*, ScriptVariant* pRet)
{
   static const char values[] = { 1, 0, 0, 1, 1, 1 };
   pRet->vArray = MakeBoolArray2D(3, 2, values);
}

// Array type definitions
static ScriptArrayDef stringArray2DDef = { { "TestStringArray2D" }, { "string" }, 2, { 0, 0 } };
static ScriptArrayDef boolArray2DDef = { { "TestBoolArray2D" }, { "bool" }, 2, { 0, 0 } };

// Class definition — flexible array member requires static storage
static ScriptClassDef testClass = { { "TestArrayClass" }, []() { return static_cast<void*>(new int(0)); }, 4,
   {
      { { "AddRef" },        { "ulong" },              0, {}, TestAddRef },
      { { "Release" },       { "ulong" },              0, {}, TestRelease },
      { { "StringArray2D" }, { "TestStringArray2D" },   0, {}, get_StringArray2D },
      { { "BoolArray2D" },   { "TestBoolArray2D" },     0, {}, get_BoolArray2D },
   } };

} // namespace TestPluginArrays


// Helper: invoke a property getter by name on a DynamicDispatch and return the VARIANT result.
static VARIANT InvokePropertyGet(DynamicDispatch* disp, const wchar_t* name)
{
   DISPID dispId = -1;
   LPOLESTR names[] = { const_cast<LPOLESTR>(name) };
   HRESULT hr = disp->GetIDsOfNames(IID_NULL, names, 1, 0, &dispId);
   assert(hr == S_OK);
   (void)hr;

   VARIANT result;
   VariantInit(&result);
   DISPPARAMS params = { nullptr, nullptr, 0, 0 };
   UINT argErr = 0;
   disp->Invoke(dispId, IID_NULL, 0, DISPATCH_PROPERTYGET, &params, &result, nullptr, &argErr);
   return result;
}

// Helper: extract a VARIANT element from a 2D SAFEARRAY
static VARIANT SafeArrayGet2D(SAFEARRAY* psa, LONG row, LONG col)
{
   VARIANT val;
   VariantInit(&val);
   LONG ix[2] = { row, col };
   SafeArrayGetElement(psa, ix, &val);
   return val;
}


TEST_CASE("Plugin 2D string and bool array marshalling")
{
   using namespace TestPluginArrays;

   // Set up a type library with string/bool 2D array types and a class
   DynamicTypeLibrary typeLib;
   typeLib.RegisterScriptArray(&stringArray2DDef);
   typeLib.RegisterScriptArray(&boolArray2DDef);
   typeLib.RegisterScriptClass(&testClass);
   typeLib.ResolveAllClasses();

   // Create a DynamicDispatch wrapping a dummy object
   void* obj = testClass.CreateObject();
   DynamicDispatch* disp = new DynamicDispatch(&typeLib, &testClass, obj);

   SUBCASE("2D string array")
   {
      g_stringElemReleaseCount = 0;
      VARIANT result = InvokePropertyGet(disp, L"StringArray2D");
      CHECK(V_VT(&result) == (VT_ARRAY | VT_VARIANT));

      SAFEARRAY* psa = V_ARRAY(&result);
      REQUIRE(psa != nullptr);
      CHECK(SafeArrayGetDim(psa) == 2);

      // Check dimensions: 3 rows x 2 cols
      LONG lBound0, uBound0, lBound1, uBound1;
      SafeArrayGetLBound(psa, 1, &lBound0);
      SafeArrayGetUBound(psa, 1, &uBound0);
      SafeArrayGetLBound(psa, 2, &lBound1);
      SafeArrayGetUBound(psa, 2, &uBound1);
      CHECK(lBound0 == 0);
      CHECK(uBound0 == 2);
      CHECK(lBound1 == 0);
      CHECK(uBound1 == 1);

      // Check element values — each should be VT_BSTR
      auto checkStr = [&](LONG row, LONG col, const char* expected)
      {
         VARIANT val = SafeArrayGet2D(psa, row, col);
         CHECK(V_VT(&val) == VT_BSTR);
         if (V_VT(&val) == VT_BSTR)
         {
            string actual = MakeString(V_BSTR(&val));
            CHECK(actual == expected);
         }
         VariantClear(&val);
      };

      checkStr(0, 0, "l-1");
      checkStr(0, 1, "on");
      checkStr(1, 0, "coil1");
      checkStr(1, 1, "pulse");
      checkStr(2, 0, "gi1");
      checkStr(2, 1, "off");

      VariantClear(&result);

      // The marshaller copies each element to a BSTR owned by the SAFEARRAY, then the host
      // releases the source ScriptArray. Each element's ScriptString::Release must run so
      // plugin-owned strings are freed through the documented per-element lifecycle.
      CHECK(g_stringElemReleaseCount == 6);
   }

   SUBCASE("2D bool array")
   {
      VARIANT result = InvokePropertyGet(disp, L"BoolArray2D");
      CHECK(V_VT(&result) == (VT_ARRAY | VT_VARIANT));

      SAFEARRAY* psa = V_ARRAY(&result);
      REQUIRE(psa != nullptr);
      CHECK(SafeArrayGetDim(psa) == 2);

      // Check dimensions: 3 rows x 2 cols
      LONG lBound0, uBound0, lBound1, uBound1;
      SafeArrayGetLBound(psa, 1, &lBound0);
      SafeArrayGetUBound(psa, 1, &uBound0);
      SafeArrayGetLBound(psa, 2, &lBound1);
      SafeArrayGetUBound(psa, 2, &uBound1);
      CHECK(lBound0 == 0);
      CHECK(uBound0 == 2);
      CHECK(lBound1 == 0);
      CHECK(uBound1 == 1);

      // Check element values — each should be VT_BOOL
      auto checkBool = [&](LONG row, LONG col, bool expected)
      {
         VARIANT val = SafeArrayGet2D(psa, row, col);
         CHECK(V_VT(&val) == VT_BOOL);
         if (V_VT(&val) == VT_BOOL)
            CHECK(V_BOOL(&val) == (expected ? VARIANT_TRUE : VARIANT_FALSE));
         VariantClear(&val);
      };

      checkBool(0, 0, true);
      checkBool(0, 1, false);
      checkBool(1, 0, false);
      checkBool(1, 1, true);
      checkBool(2, 0, true);
      checkBool(2, 1, true);

      VariantClear(&result);
   }

   disp->Release();
   // Free the object allocated by CreateObject (matches new int(0))
   delete static_cast<int*>(obj);

   // Clean up type registrations
   typeLib.UnregisterScriptClass(&testClass);
   typeLib.UnregisterScriptArray(&stringArray2DDef);
   typeLib.UnregisterScriptArray(&boolArray2DDef);
}
