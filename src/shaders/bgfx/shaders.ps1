

$gen_ball = $true
$gen_basic = $true
$gen_blur = $true
$gen_dmd = $true
$gen_flasher = $true
$gen_light = $true
$gen_motionblur = $true
$gen_postprocess = $true
$gen_stereo = $true
$gen_tonemap = $true
$gen_antialiasing = $true
$gen_imgui = $true

$debug = $false



function Process-Shader {
   Param($Source, $OutputFile, $Header, $Type, $Defines=@())

   $outputs = @('mtl ', 'essl', 'glsl', 'dx11', 'spv ')
   $targets = @(
      '--platform osx     -p metal -O 3', # '--platform ios -p metal'
      '--platform windows -p 320_es    ', # '--platform android -p 320_es'
      '--platform windows -p 440       ', # '--platform linux -p440'
      '--platform windows -p s_5_0 -O 3', # '--platform windows -p s_5_0 --debug -O 0' for debug in Renderdoc & MS Visual Studio (see https://www.intel.com/content/www/us/en/developer/articles/technical/shader-debugging-for-bgfx-rendering-engine.html)
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
      if ($debug)
      {
        $CmdLine = $CmdLine + " --debug -O 0"
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
# Basic material shaders (also 'classic' light, decals, kickers and unshaded variants)
if ($gen_basic)
{
	Write-Host "`n>>>>>>>>>>>>>>>> Base material shader"
	New-Item -Path . -Name "../bgfx_basic.h" -ItemType "File" -Force -Value "// Base material Shaders`n"
    Process-Shader "vs_vr_mask.sc" "basic.h" "vs_vr_mask_" "vertex"
	foreach ($variant2 in @("CLIP", "NOCLIP"))
	{
	  for($k = 0; $k -lt 2; $k++)
	  {
		Process-Shader "vs_basic.sc" "basic.h" ("vs_kicker_" + $variant2.ToLower() + $stOutput[$k]) "vertex" @($stereo[$k], $variant2, "KICKER")
		Process-Shader "fs_basic.sc" "basic.h" ("fs_basic_refl_" + $variant2.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant2, "REFL")
		foreach ($variant in @("TEX", "NOTEX"))
		{
		  Process-Shader "vs_basic.sc"         "basic.h" ("vs_basic_" + $variant.ToLower() + "_" + $variant2.ToLower() + $stOutput[$k]) "vertex" @($stereo[$k], $variant, $variant2)
		  Process-Shader "fs_basic.sc"         "basic.h" ("fs_basic_" + $variant.ToLower() + "_noat_" + $variant2.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2, "NOAT")
		  Process-Shader "fs_basic.sc"         "basic.h" ("fs_basic_" + $variant.ToLower() + "_at_" + $variant2.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2, "AT") # FIXME useless for notex
		  Process-Shader "fs_decal.sc"         "basic.h" ("fs_decal_" + $variant.ToLower() + "_" + $variant2.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2)
		  Process-Shader "vs_basic.sc"         "basic.h" ("vs_classic_light_" + $variant.ToLower() + "_" + $variant2.ToLower() + $stOutput[$k]) "vertex" @($stereo[$k], "CLASSIC_LIGHT", $variant, $variant2)
		  Process-Shader "fs_classic_light.sc" "basic.h" ("fs_classic_light_" + $variant.ToLower() + "_" + $variant2.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2, "NOSHADOW")
		  Process-Shader "fs_unshaded.sc"      "basic.h" ("fs_unshaded_" + $variant.ToLower() + "_" + $variant2.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2, "NOSHADOW")
		  Process-Shader "fs_unshaded.sc"      "basic.h" ("fs_unshaded_" + $variant.ToLower() + "_ballshadow_" + $variant2.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2, "SHADOW")
		}
	  }
	}
}


################################
# Ball shaders
if ($gen_ball)
{
	Write-Host "`n>>>>>>>>>>>>>>>> Ball shader"
	New-Item -Path . -Name "../bgfx_ball.h" -ItemType "File" -Force -Value "// Ball Shaders`n"
	for($k = 0; $k -lt 2; $k++)
	{
	  foreach ($variant3 in @("CLIP", "NOCLIP"))
	  {
		Process-Shader "vs_ball.sc" "ball.h" ("vs_ball_" + $variant3.ToLower() + $stOutput[$k]) "vertex" @($stereo[$k], $variant3)
		Process-Shader "vs_ball_trail.sc" "ball.h" ("vs_ball_trail_" + $variant3.ToLower() + $stOutput[$k]) "vertex" @($stereo[$k], $variant3)
		Process-Shader "fs_ball_trail.sc" "ball.h" ("fs_ball_trail_" + $variant3.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant3)
		Process-Shader "fs_ball_debug.sc" "ball.h" ("fs_ball_debug_" + $variant3.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant3)
		foreach ($variant in @("EQUIRECTANGULAR", "SPHERICAL"))
		{
		  foreach ($variant2 in @("DECAL", "NODECAL"))
		  {
			Process-Shader "fs_ball.sc" "ball.h" ("fs_ball_" + $variant.ToLower() + "_" + $variant2.ToLower() + "_" + $variant3.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2, $variant3)
		  }
		}
	  }
	}
}


################################
# DMD/Sprite shaders
if ($gen_dmd)
{
	Write-Host "`n>>>>>>>>>>>>>>>> DMD & sprite shaders"
	New-Item -Path . -Name "../bgfx_dmd.h" -ItemType "File" -Force -Value "// DMD Shaders`n"
	Process-Shader "vs_dmd.sc" "dmd.h" "vs_dmd_noworld_"  "vertex"
	Process-Shader "vs_dmd.sc" "dmd.h" "vs_dmd_noworld_st_"  "vertex" @("STEREO")
	foreach ($variant3 in @("CLIP", "NOCLIP"))
	{
		Process-Shader "vs_dmd.sc" "dmd.h"  ("vs_dmd_world_" + $variant3.ToLower() + "_")    "vertex" @("WORLD", $variant3)
		Process-Shader "vs_dmd.sc" "dmd.h"  ("vs_dmd_world_" + $variant3.ToLower() + "_st_") "vertex" @("WORLD", "STEREO", $variant3)
		Process-Shader "fs_dmd.sc" "dmd.h"  ("fs_dmd_"  + $variant3.ToLower() + "_") "fragment" @("DMD", $variant3)
		foreach ($variant2 in @("DMD", "SEG", "CRT"))
		{
			Process-Shader "fs_display.sc" "dmd.h" ("fs_display_" + $variant2.ToLower() + "_" + $variant3.ToLower() + "_") "fragment" @($variant2, $variant3)
		}	
		foreach ($variant2 in @("TEX", "NOTEX"))
		{
			Process-Shader "fs_dmd.sc" "dmd.h" ("fs_sprite_" + $variant2.ToLower() + "_" + $variant3.ToLower() + "_") "fragment" @("SPRITE", $variant2, $variant3)
		}
	}
}


################################
# Flasher shaders
if ($gen_flasher)
{
	Write-Host "`n>>>>>>>>>>>>>>>> Flasher shader"
	New-Item -Path . -Name "../bgfx_flasher.h" -ItemType "File" -Force -Value "// Flasher Shaders`n"
	foreach ($variant in @("CLIP", "NOCLIP"))
	{
	  Process-Shader "vs_flasher.sc" "flasher.h" ("vs_flasher_" + $variant.ToLower() + "_")    "vertex" @($variant)
	  Process-Shader "vs_flasher.sc" "flasher.h" ("vs_flasher_" + $variant.ToLower() + "_st_") "vertex" @("STEREO", $variant)
	  Process-Shader "fs_flasher.sc" "flasher.h" ("fs_flasher_" + $variant.ToLower() + "_")    "fragment" @($variant)
	}
}


################################
# Light shaders
if ($gen_light)
{
	Write-Host "`n>>>>>>>>>>>>>>>> Light shader"
	New-Item -Path . -Name "../bgfx_light.h" -ItemType "File" -Force -Value "// Light Shaders`n"
	foreach ($variant in @("CLIP", "NOCLIP"))
	{
	  Process-Shader "vs_light.sc" "light.h" ("vs_light_" + $variant.ToLower() + "_")    "vertex" @($variant)
	  Process-Shader "vs_light.sc" "light.h" ("vs_light_" + $variant.ToLower() + "_st_") "vertex" @($variant, "STEREO")
	  foreach ($variant2 in @("NOSHADOW", "BALLSHADOW"))
	  {
		Process-Shader "fs_light.sc" "light.h" ("fs_light_" + $variant2.ToLower() + "_" + $variant.ToLower() + "_") "fragment" @($variant, $variant2)
	  }
	}
}

################################
# Stereo shaders
if ($gen_stereo)
{
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
}


################################
# Post process shaders
if ($gen_postprocess)
{
	Write-Host "`n>>>>>>>>>>>>>>>> Post process shaders"
	New-Item -Path . -Name "../bgfx_postprocess.h" -ItemType "File" -Force -Value "// Postprocess Shaders`n"
	for($k = 0; $k -lt 2; $k++)
	{
	  Process-Shader "vs_postprocess.sc" "postprocess.h" ("vs_postprocess" + $stOutput[$k]) "vertex" @($stereo[$k])
	  Process-Shader "fs_pp_mirror.sc" "postprocess.h" ("fs_pp_mirror" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_copy.sc" "postprocess.h" ("fs_pp_copy" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_bloom.sc" "postprocess.h" ("fs_pp_bloom" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_irradiance.sc" "postprocess.h" ("fs_pp_irradiance" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_ssao.sc" "postprocess.h" ("fs_pp_ssao" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_ssr.sc" "postprocess.h" ("fs_pp_ssr" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_ao_display.sc" "postprocess.h" ("fs_pp_ao_display" + $stOutput[$k]) "fragment" @($stereo[$k])
	  foreach ($variant in @("FILTER", "NOFILTER"))
	  {
		 Process-Shader "fs_pp_ao.sc" "postprocess.h" ("fs_pp_ao_" + $variant.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant)
	  }
	}
}


################################
# Antialiasing shaders
if ($gen_antialiasing)
{
	New-Item -Path . -Name "../bgfx_antialiasing.h" -ItemType "File" -Force -Value "// Antialiasing Shaders`n"
	for($k = 0; $k -lt 2; $k++)
	{
	  Process-Shader "fs_pp_cas.sc" "antialiasing.h" ("fs_pp_cas" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_bilateral_cas.sc" "antialiasing.h" ("fs_pp_bilateral_cas" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_nfaa.sc" "antialiasing.h" ("fs_pp_nfaa" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_dlaa_edge.sc" "antialiasing.h" ("fs_pp_dlaa_edge" + $stOutput[$k]) "fragment" @($stereo[$k])
	  Process-Shader "fs_pp_dlaa.sc" "antialiasing.h" ("fs_pp_dlaa" + $stOutput[$k]) "fragment" @($stereo[$k])
	  foreach ($variant in @("FXAA1", "FXAA2", "FXAA3"))
	  {
		 Process-Shader "fs_pp_fxaa.sc" "antialiasing.h" ("fs_pp_" + $variant.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant)
	  }
	}

	# TODO add stereo support to SMAA
	Process-Shader "vs_pp_smaa_BlendingWeightCalculation.sc" "antialiasing.h" "vs_pp_smaa_blendweightcalculation_" "vertex"
	Process-Shader "vs_pp_smaa_EdgeDetection.sc"             "antialiasing.h" "vs_pp_smaa_edgedetection_"          "vertex"
	Process-Shader "vs_pp_smaa_NeighborhoodBlending.sc"      "antialiasing.h" "vs_pp_smaa_neighborhoodblending_"   "vertex"
	Process-Shader "fs_pp_smaa_BlendingWeightCalculation.sc" "antialiasing.h" "fs_pp_smaa_blendweightcalculation_" "fragment"
	Process-Shader "fs_pp_smaa_EdgeDetection.sc"             "antialiasing.h" "fs_pp_smaa_edgedetection_"          "fragment"
	Process-Shader "fs_pp_smaa_NeighborhoodBlending.sc"      "antialiasing.h" "fs_pp_smaa_neighborhoodblending_"   "fragment"
}


################################
# Tonemapping shaders
if ($gen_tonemap)
{
	New-Item -Path . -Name "../bgfx_tonemap.h" -ItemType "File" -Force -Value "// Tonemap Shaders`n"
	for($k = 0; $k -lt 2; $k++)
	{
	  #foreach ($variant in @("FILMIC", "TONY", "NEUTRAL", "AGX", "AGX_PUNCHY", "WCG_REINHARD"))
	  foreach ($variant in @("FILMIC", "NEUTRAL", "AGX", "AGX_PUNCHY", "WCG"))
	  {
		 foreach ($variant2 in @("AO", "NOAO"))
		 {
			foreach ($variant3 in @("FILTER", "NOFILTER"))
			{
				Process-Shader "fs_pp_tonemap.sc" "tonemap.h" ("fs_pp_tonemap_"  + $variant.ToLower() + "_" + $variant2.ToLower() + "_" + $variant3.ToLower() + $stOutput[$k]) "fragment" @($stereo[$k], $variant, $variant2, $variant3)
			}
		 }
	  }
	  foreach ($variant in @("REINHARD"))
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
	}
}


################################
# Blur shaders
if ($gen_blur)
{
	New-Item -Path . -Name "../bgfx_blur.h" -ItemType "File" -Force -Value "// Blur Kernel Shaders`n"
	for($k = 0; $k -lt 2; $k++)
	{
	  $blurs = @("BLUR_7", "BLUR_9", "BLUR_11", "BLUR_13", "BLUR_15", "BLUR_19", "BLUR_19H", "BLUR_23", "BLUR_27", "BLUR_39")
	  foreach ($blur in $blurs)
	  {
		 $axis = @("HORIZONTAL", "VERTICAL")
		 $axis_suffix = @("_h", "_v")
		 for($j = 0; $j -lt 2; $j++)
		 {
			Process-Shader "fs_pp_blur.sc" "blur.h" ("fs_"  + $blur.ToLower() + $axis_suffix[$j] + $stOutput[$k]) "fragment" @($stereo[$k], $blur, $axis[$j])
		 }
	  }
	}
}


################################
# Motion blur shaders
if ($gen_motionblur)
{
	Write-Host "`n>>>>>>>>>>>>>>>> Motion blur shaders"
	New-Item -Path . -Name "../bgfx_motionblur.h" -ItemType "File" -Force -Value "// Raytraced motion blur Shaders`n"
	for($k = 0; $k -lt 2; $k++)
	{
		Process-Shader "fs_pp_motionblur.sc"      "motionblur.h" ("fs_pp_motionblur" + $stOutput[$k])   "fragment" @($stereo[$k])
	}
}


################################
# ImgUI shaders
if ($gen_imgui)
{
	Write-Host "`n>>>>>>>>>>>>>>>> ImgUI shaders"
	New-Item -Path . -Name "../bgfx_imgui.h" -ItemType "File" -Force -Value "// ImgUI Shaders`n"
	for($k = 0; $k -lt 2; $k++)
	{
		Process-Shader "vs_imgui.sc" "imgui.h" ("vs_imgui" + $stOutput[$k]) "vertex" @($stereo[$k])
		Process-Shader "fs_imgui.sc" "imgui.h" ("fs_imgui" + $stOutput[$k]) "fragment" @($stereo[$k])
	}
}
