

function Process-Shader {
   Param($Source, $OutputFile, $Header, $Type, $Defines=@())

   $outputs = @('mtl ', 'essl', 'glsl', 'dx11', 'spv ')
   $targets = @(
      '--platform osx     -p metal -O 3', # '--platform ios -p metal'
      '--platform windows -p 320_es    ', # '--platform android -p 320_es'
      '--platform windows -p 440       ', # '--platform linux -p440'
      '--platform windows -p s_5_0 -O 3', 
      '--platform windows -p spirv     ')
   $shaderc = ".\shaderc.exe"

   $OutputPath = ("../bgfx_" + $OutputFile)
   $ShortName = $Header.Substring(0, $Header.Length - 1)

   Add-Content -Path $OutputPath -Value ("`n//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////`n// Build of " + $Type + " shader from " + $Source + " to " + $ShortName + " with options: " + $Defines)
   Write-Host ("> " + $Type + " shader from " + $Source + " to " + $ShortName + " with " + $Defines)
   for($i = 0; $i -lt 5; $i++)
   {
      $CmdLine = "-f " + $Source + " " + $targets[$i] + " --bin2c " + $Header + $outputs[$i] + " --type " + $Type
      #$CmdLine = "-f " + $Source + " " + $Target + " -o shaders/" + $Header + ".bin --type " + $Type
      If($Defines.count -ne 0)
      {
         $CmdLine = $CmdLine + " --define " + ($Defines -join ';')
      }
      $CmdLine = $CmdLine + " -o tmp.h"
      $Parms = $CmdLine.Split(" ")
      & "$shaderc" $Parms
      Get-Content -Path "tmp.h" | Add-Content -Path $OutputPath
      Remove-Item "tmp.h"
   }
   Add-Content -Path $OutputPath -Value ("static const bgfx::EmbeddedShader " + $ShortName + " = BGFX_EMBEDDED_SHADER(" + $ShortName + ");")
}

$stereo = @("NOSTEREO", "STEREO")
$stOutput = @(("_"), ("_st_"))


################################
# Flasher shaders
Write-Host "`n>>>>>>>>>>>>>>>> Flasher shader"
New-Item -Path . -Name "../bgfx_flasher.h" -ItemType "File" -Force -Value "// Flasher Shaders`n"
Process-Shader "vs_flasher.sc" "flasher.h" "vs_flasher_"    "vertex"
Process-Shader "vs_flasher.sc" "flasher.h" "vs_flasher_st_" "vertex" @("STEREO")
Process-Shader "fs_flasher.sc" "flasher.h" "fs_flasher_"    "fragment"

################################
# Basic material shaders (also 'classic' light)
Write-Host "`n>>>>>>>>>>>>>>>> Base material shader"
New-Item -Path . -Name "../bgfx_basic.h" -ItemType "File" -Force -Value "// Base material Shaders`n"
foreach ($variant in @("TEX", "NOTEX"))
{
  Process-Shader "vs_basic.sc" "basic.h" ("vs_basic_" + $variant.ToLower() + "_")    "vertex" @($variant)
  Process-Shader "vs_basic.sc" "basic.h" ("vs_basic_" + $variant.ToLower() + "_st_") "vertex" @("STEREO", $variant)
  foreach ($variant2 in @("AT", "NOAT"))
  {
	 foreach ($variant3 in @("REFL", "NOREFL"))
	 {
		Process-Shader "fs_basic.sc" "basic.h" ("fs_basic_" + $variant.ToLower() + "_" + $variant2.ToLower() + "_" + $variant3.ToLower() + "_") "fragment" @($variant, $variant2, $variant3)
		Process-Shader "fs_basic.sc" "basic.h" ("fs_basic_" + $variant.ToLower() + "_" + $variant2.ToLower() + "_" + $variant3.ToLower() + "_st_") "fragment" @("STEREO", $variant, $variant2, $variant3)
	 }
  }
  Process-Shader "vs_basic.sc" "basic.h" ("vs_classic_light_" + $variant.ToLower() + "_") "vertex" @("CLASSIC_LIGHT", $variant)
  Process-Shader "vs_basic.sc" "basic.h" ("vs_classic_light_" + $variant.ToLower() + "_st_") "vertex" @("STEREO", "CLASSIC_LIGHT", $variant)
  foreach ($variant2 in @("NOSHADOW", "BALLSHADOW"))
  {
	 Process-Shader "fs_classic_light.sc" "basic.h" ("fs_classic_light_" + $variant.ToLower() + "_" + $variant2.ToLower() + "_") "fragment" @($variant, $variant2)
  }
}

################################
# Ball shaders
Write-Host "`n>>>>>>>>>>>>>>>> Ball shader"
New-Item -Path . -Name "../bgfx_ball.h" -ItemType "File" -Force -Value "// Ball Shaders`n"
for($k = 0; $k -lt 2; $k++)
{
  Process-Shader "vs_ball.sc" "ball.h" ("vs_ball" + $stOutput[$k]) "vertex" @($stereo[$k])
  foreach ($variant in @("EQUIRECTANGULAR", "SPHERICAL"))
  {
	 foreach ($subvariant in @("DECAL", "NODECAL"))
	 {
		Process-Shader "fs_ball.sc" "ball.h" ("fs_ball_" + $variant.ToLower() + "_" + $subvariant.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $subvariant)
	 }
  }
}
Process-Shader "fs_ball_trail.sc" "ball.h" "fs_ball_trail_" "fragment"

################################
# DMD/Sprite shaders
Write-Host "`n>>>>>>>>>>>>>>>> DMD & sprite shaders"
New-Item -Path . -Name "../bgfx_dmd.h" -ItemType "File" -Force -Value "// DMD Shaders`n"
Process-Shader "vs_dmd.sc" "dmd.h" "vs_basic_dmd_world_"    "vertex" @("WORLD")
Process-Shader "vs_dmd.sc" "dmd.h" "vs_basic_dmd_world_st_" "vertex" @("WORLD", "STEREO")
Process-Shader "vs_dmd.sc" "dmd.h" "vs_basic_dmd_noworld_"  "vertex"
foreach ($variant in @("DMD", "SPRITE"))
{
  foreach ($variant2 in @("TEX", "NOTEX"))
  {
	 Process-Shader "fs_dmd.sc" "dmd.h" ("fs_basic_" + $variant.ToLower() + "_" + $variant2.ToLower() + "_") "fragment" @($variant, $variant2)
  }
}

################################
# Light shaders
Write-Host "`n>>>>>>>>>>>>>>>> Light shader"
New-Item -Path . -Name "../bgfx_light.h" -ItemType "File" -Force -Value "// Light Shaders`n"
Process-Shader "vs_light.sc" "light.h" "vs_light_"    "vertex"
Process-Shader "vs_light.sc" "light.h" "vs_light_st_" "vertex" @("STEREO")
foreach ($variant in @("NOSHADOW", "BALLSHADOW"))
{
  Process-Shader "fs_light.sc" "light.h" ("fs_light_" + $variant.ToLower() + "_") "fragment" @($variant)
}

################################
# Stereo shaders
Write-Host "`n>>>>>>>>>>>>>>>> Stereo shaders"
New-Item -Path . -Name "../bgfx_stereo.h" -ItemType "File" -Force -Value "// Stereo Shaders`n"
for($k = 0; $k -lt 2; $k++)
{
   Process-Shader "fs_pp_stereo.sc" "stereo.h" ("fs_pp_stereo_sbs" + $stOutput[$k]) "fragment" @("SBS", $stereo[$k])
   Process-Shader "fs_pp_stereo.sc" "stereo.h" ("fs_pp_stereo_tb" + $stOutput[$k]) "fragment" @("TB", $stereo[$k])
   Process-Shader "fs_pp_stereo.sc" "stereo.h" ("fs_pp_stereo_int" + $stOutput[$k]) "fragment" @("INT", $stereo[$k])
   Process-Shader "fs_pp_stereo.sc" "stereo.h" ("fs_pp_stereo_flipped_int" + $stOutput[$k]) "fragment" @("FLIPPED_INT", $stereo[$k])
   Process-Shader "fs_pp_stereo.sc" "stereo.h" ("fs_pp_stereo_anaglyph_deghost" + $stOutput[$k]) "fragment" @("ANAGLYPH", "DEGHOST", $stereo[$k])
   foreach ($colors in @("SRGB", "GAMMA"))
   {
      foreach ($desat in @("NODESAT", "DYNDESAT"))
      {
         Process-Shader "fs_pp_stereo.sc" "stereo.h" ("fs_pp_stereo_anaglyph_lin_" + $colors.ToLower() + "_" + $desat.ToLower() + $stOutput[$k]) "fragment" @("ANAGLYPH", $desat, $colors, $stereo[$k])
      }
   }
}

################################
# Post process shaders
Write-Host "`n>>>>>>>>>>>>>>>> Post process shaders"
New-Item -Path . -Name "../bgfx_postprocess.h" -ItemType "File" -Force -Value "// Postprocess Shaders`n"
New-Item -Path . -Name "../bgfx_antialiasing.h" -ItemType "File" -Force -Value "// Antialiasing Shaders`n"
New-Item -Path . -Name "../bgfx_tonemap.h" -ItemType "File" -Force -Value "// Tonemap Shaders`n"
New-Item -Path . -Name "../bgfx_blur.h" -ItemType "File" -Force -Value "// Blur Kernel Shaders`n"
for($k = 0; $k -lt 2; $k++)
{
  Process-Shader "vs_postprocess_offseted.sc" "postprocess.h" ("vs_postprocess_offseted" + $stOutput[$k]) "vertex" @($stereo[$k])
  Process-Shader "vs_postprocess.sc" "postprocess.h" ("vs_postprocess" + $stOutput[$k]) "vertex" @($stereo[$k])
  Process-Shader "fs_pp_mirror.sc" "postprocess.h" ("fs_pp_mirror" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_copy.sc" "postprocess.h" ("fs_pp_copy" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_bloom.sc" "postprocess.h" ("fs_pp_bloom" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_irradiance.sc" "postprocess.h" ("fs_pp_irradiance" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_ssao.sc" "postprocess.h" ("fs_pp_ssao" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_ssr.sc" "postprocess.h" ("fs_pp_ssr" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_cas.sc" "antialiasing.h" ("fs_pp_cas" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_bilateral_cas.sc" "antialiasing.h" ("fs_pp_bilateral_cas" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_nfaa.sc" "antialiasing.h" ("fs_pp_nfaa" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_dlaa_edge.sc" "antialiasing.h" ("fs_pp_dlaa_edge" + $stOutput[$k]) "fragment" @($stereo[$k])
  Process-Shader "fs_pp_dlaa.sc" "antialiasing.h" ("fs_pp_dlaa" + $stOutput[$k]) "fragment" @($stereo[$k])
  foreach ($variant in @("FXAA1", "FXAA2", "FXAA3"))
  {
	 Process-Shader "fs_pp_fxaa.sc" "antialiasing.h" ("fs_pp_" + $variant.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant)
  }
  foreach ($variant in @("REINHARD", "FILMIC", "TONY"))
  {
	 foreach ($variant2 in @("AO", "NOAO"))
	 {
		foreach ($variant3 in @("FILTER", "NOFILTER"))
		{
		   foreach ($variant4 in @("GRAY", "RG", "RGB"))
		   {
			  Process-Shader "fs_pp_tonemap.sc" "tonemap.h" ("fs_pp_tonemap_"  + $variant.ToLower() + "_" + $variant2.ToLower() + "_" + $variant3.ToLower() + "_" + $variant4.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2, $variant3, $variant4)
		   }
		}
	 }
  }
  $blurs = @("BLUR_7", "BLUR_9", "BLUR_11", "BLUR_13", "BLUR_15", "BLUR_19", "BLUR_19H", "BLUR_23", "BLUR_27", "BLUR_39")
  foreach ($blur in $blurs)
  {
	 $axis = @("HORIZONTAL", "VERTICAL")
	 $axis_suffix = @("_h", "_v")
	 for($j = 0; $j -lt 2; $j++)
	 {
		Process-Shader "fs_blur.sc" "blur.h" ("fs_"  + $blur.ToLower() + $axis_suffix[$j] + $stOutput[$k]) "fragment" @($stereo[$k], $blur, $axis[$j])
	 }
  }
}

