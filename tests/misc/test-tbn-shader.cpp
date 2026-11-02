// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

// Tangent-space normal maps are mapped to view space through a cotangent frame
// (TBN_trafo). The shader sources are deployed to test-assets/shaders-src by the
// build, so this test anchors the contract that keeps the normal-mapped output
// consistent across render passes (shading, depth prepass, PBR) and backends
// (DX9 / OpenGL / BGFX GLSL, DXBC, DXIL, SPIR-V, Metal):
// - a single implementation for the BGFX shaders (copies once diverged into
//   different tangent frames per backend),
// - built as columns via mtxFromCols: mat3(T,B,N) makes rows on HLSL/SPIR-V/Metal,
//   transposing the frame,
// - differentiating the raw view space position: the derivatives of a normalized
//   direction (view vector) are the true edges projected onto the plane
//   perpendicular to the view ray, skewing the frame at grazing angles,
// - the operand sign of the DX9 shader (-worldPos), preserving the authored
//   normal map convention,
// - on OpenGL only, negated uv derivatives: its window-y axis points up and
//   jointly flips position & uv y derivatives vs the D3D convention, so negating
//   both uv axes cancels the flip (a single-axis flip like vec2(uv.x,-uv.y)
//   mirrors the frame instead — that is the bug this guards),
// - a degenerate-uv fallback: a null frame must not normalize to NaN.
namespace
{
std::string ReadFileToString(const std::filesystem::path& path)
{
   std::ifstream file(path, std::ios::binary);
   std::ostringstream ss;
   ss << file.rdbuf();
   return ss.str();
}

// Extracts the body of a function starting at 'signature' up to its matching close brace.
std::string FunctionBody(const std::string& src, const std::string& signature)
{
   const size_t start = src.find(signature);
   if (start == std::string::npos)
      return {};
   size_t open = src.find('{', start);
   if (open == std::string::npos)
      return {};
   int depth = 1;
   size_t i = open + 1;
   for (; i < src.size() && depth > 0; ++i)
   {
      if (src[i] == '{')
         ++depth;
      else if (src[i] == '}')
         --depth;
   }
   return src.substr(open + 1, i - open - 2);
}

void CheckCallSites(const std::string& name, const std::string& src, const char* callee)
{
   size_t pos = 0;
   while ((pos = src.find(callee, pos)) != std::string::npos)
   {
      const size_t lineEnd = src.find('\n', pos);
      const std::string line = src.substr(pos, lineEnd == std::string::npos ? std::string::npos : lineEnd - pos);
      INFO(name, ": ", line);
      CHECK((line.find("pos") != std::string::npos || line.find("Pos") != std::string::npos));
      ++pos;
   }
}
}

TEST_CASE("TBN shader contract")
{
   const std::filesystem::path dir = GetAssetPath() / "shaders-src";
   REQUIRE(std::filesystem::is_directory(dir));

   std::vector<std::pair<std::string, std::string>> sources;
   for (const auto& entry : std::filesystem::directory_iterator(dir))
      sources.emplace_back(entry.path().filename().string(), ReadFileToString(entry.path()));
   REQUIRE(sources.size() >= 2);

   const auto getSrc = [&sources](const char* file) -> const std::string*
   {
      for (const auto& [name, src] : sources)
         if (name == file)
            return &src;
      return nullptr;
   };

   SUBCASE("single implementation shared by the BGFX shaders")
   {
      size_t defCount = 0;
      std::string defFile;
      for (const auto& [name, src] : sources)
         if (name.size() > 3 && (name.compare(name.size() - 3, 3, ".sc") == 0 || name == "common.sh") && src.find("mat3 TBN_trafo") != std::string::npos)
         {
            ++defCount;
            defFile = name;
         }
      INFO("BGFX TBN_trafo defined in: ", defFile);
      CHECK(defCount == 1);
      CHECK(defFile == "common.sh");
   }

   SUBCASE("BGFX implementation differentiates -position and the authored uv convention")
   {
      const std::string* src = getSrc("common.sh");
      REQUIRE(src != nullptr);
      const std::string body = FunctionBody(*src, "mat3 TBN_trafo");
      REQUIRE(!body.empty());
      CHECK(body.find("dFdx(-pos)") != std::string::npos); // raw position toward camera (DX9 sign convention)
      CHECK(body.find("dFdy(-pos)") != std::string::npos);
      CHECK(body.find("dFdx(-uv)") != std::string::npos); // joint uv negation for GL window-y-up
      CHECK(body.find("mtxFromCols") != std::string::npos); // T,B,N as columns on every backend
      CHECK(body.find("uv.x, -uv.y") == std::string::npos); // a single-axis flip mirrors the frame
      CHECK(body.find("invmax < 1e-12") != std::string::npos); // degenerate-uv NaN guard
   }

   SUBCASE("legacy OpenGL differentiates the raw position")
   {
      const std::string* src = getSrc("BasicShader.glfx");
      REQUIRE(src != nullptr);
      const std::string body = FunctionBody(*src, "mat3 TBN_trafo");
      REQUIRE(!body.empty());
      CHECK(body.find("dFdx(pos)") != std::string::npos); // glfx's frame is already GL-side; the raw position only removes the skew
      CHECK(body.find("dFdy(pos)") != std::string::npos);
      CheckCallSites("BasicShader.glfx", *src, "TBN_trafo(N,");
   }

   SUBCASE("DX9 differentiates the raw position toward the camera")
   {
      const std::string* src = getSrc("BasicShader.hlsl");
      REQUIRE(src != nullptr);
      const std::string body = FunctionBody(*src, "float3 normal_map");
      REQUIRE(!body.empty());
      CHECK(body.find("ddx(pos)") != std::string::npos);
      CHECK(body.find("ddy(pos)") != std::string::npos);
      CHECK(src->find("normal_map(N, -IN.worldPos") != std::string::npos);
   }

   SUBCASE("all callers pass a position, never a normalized view vector")
   {
      for (const auto& [name, src] : sources)
         CheckCallSites(name, src, "TBN_trafo(N,");
   }
}
