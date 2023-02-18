

$vprofiles = @('metal','320_es','440','vs_3_0','vs_5_0', 'spirv')
$fprofiles = @('metal','320_es','440','ps_3_0','ps_5_0', 'spirv')
$outputs = @('metal','gles','glsl','dx9','dx11', 'spirv')
$platforms = @('osx','windows','windows','windows','windows','windows')
$shaderc = ".\shaderc.exe"

for($i = 0; $i -lt 6; $i++)
{
   Write-Host Processing shaders for $outputs[$i]
   If(!(test-path -PathType container $outputs[$i]))
   {
      New-Item -Name $outputs[$i] -ItemType "directory" > $null
   }

   Write-Host "> Debug shader"
   $Parms = ("-f vs_debug.sc -o " + $outputs[$i] + "/vs_debug.bin --platform " + $platforms[$i] + " -p " + $vprofiles[$i] + " --type vertex").Split(" ")
   & "$shaderc" $Parms
   $Parms = ("-f fs_debug.sc -o " + $outputs[$i] + "/fs_debug.bin --platform " + $platforms[$i] + " -p " + $fprofiles[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   ################################
   # Flasher shaders

   Write-Host "> Flasher shader"
   $Parms = ("-f vs_flasher.sc -o " + $outputs[$i] + "/vs_flasher.bin --platform " + $platforms[$i] + " -p " + $vprofiles[$i] + " --type vertex").Split(" ")
   & "$shaderc" $Parms
   $Parms = ("-f fs_flasher.sc -o " + $outputs[$i] + "/fs_flasher.bin --platform " + $platforms[$i] + " -p " + $fprofiles[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   ################################
   # Basic material shaders

   Write-Host "> Base material shader"
   $variants = @("TEX", "NOTEX")
   foreach ($variant in $variants)
   {
	   $Parms = ("-f vs_basic.sc -o " + $outputs[$i] + "/vs_basic_" + $variant.ToLower() + ".bin --platform " + $platforms[$i] + " -p " + $vprofiles[$i] + " --type vertex --define " + $variant).Split(" ")
	   & "$shaderc" $Parms
	   $Parms = ("-f fs_basic.sc -o " + $outputs[$i] + "/fs_basic_" + $variant.ToLower() + ".bin --platform " + $platforms[$i] + " -p " + $fprofiles[$i] + " --type fragment --define " + $variant).Split(" ")
	   & "$shaderc" $Parms
   }

   ################################
   # Ball shaders

   Write-Host "> Ball shader"
   $variants = @("DECAL", "NODECAL")
   foreach ($variant in $variants)
   {
      $subvariants = @("CAB", "NOCAB")
	  foreach ($subvariant in $subvariants)
      {
		   $Parms = ("-f vs_ball.sc -o " + $outputs[$i] + "/vs_ball_" + $variant.ToLower() + "_" + $subvariant.ToLower() + ".bin --platform " + $platforms[$i] + " -p " + $vprofiles[$i] + " --type vertex --define " + $variant + " " + $subvariant).Split(" ")
		   & "$shaderc" $Parms
		   $Parms = ("-f fs_ball.sc -o " + $outputs[$i] + "/fs_ball_" + $variant.ToLower() + "_" + $subvariant.ToLower() + ".bin --platform " + $platforms[$i] + " -p " + $fprofiles[$i] + " --type fragment --define " + $variant + " " + $subvariant).Split(" ")
		   & "$shaderc" $Parms
	  }
   }

   ################################
   # Light shaders

   Write-Host "> Light shader"
   $variants = @("NOSHADOW", "BALLSHADOW")
   foreach ($variant in $variants)
   {
	   $Parms = ("-f vs_light.sc -o " + $outputs[$i] + "/vs_light_" + $variant.ToLower() + ".bin --platform " + $platforms[$i] + " -p " + $vprofiles[$i] + " --type vertex --define " + $variant).Split(" ")
	   & "$shaderc" $Parms
	   $Parms = ("-f fs_light.sc -o " + $outputs[$i] + "/fs_light_" + $variant.ToLower() + ".bin --platform " + $platforms[$i] + " -p " + $fprofiles[$i] + " --type fragment --define " + $variant).Split(" ")
	   & "$shaderc" $Parms
   }

   ################################
   # Post process shaders

   Write-Host "> Post process shaders"

   $Parms = ("-f vs_postprocess.sc -o " + $outputs[$i] + "/vs_postprocess.bin --platform " + $platforms[$i] + " -p " + $vprofiles[$i] + " --type vertex").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_mirror.sc -o " + $outputs[$i] + "/fs_pp_mirror.bin --platform " + $platforms[$i] + " -p " + $fprofiles[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $variants = @("TONEMAP", "NOTONEMAP")
   foreach ($variant in $variants)
   {
   $variants2 = @("AO", "NOAO")
   foreach ($variant2 in $variants2)
   {
   $variants3 = @("FILTER", "NOFILTER")
   foreach ($variant3 in $variants3)
   {
   $variants4 = @("GRAY", "BW", "RGB")
   foreach ($variant4 in $variants4)
   {
	   $Parms = ("-f fs_pp_tonemap.sc -o " + $outputs[$i] + "/fs_pp_" + $variant.ToLower() + "_" + $variant2.ToLower() + "_" + $variant3.ToLower() + "_" + $variant4.ToLower() + ".bin --platform " + $platforms[$i] + " -p " + $fprofiles[$i] + " --type fragment --define " + $variant + " " + $variant2 + " " + $variant3 + " " + $variant4).Split(" ")
	   & "$shaderc" $Parms
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
         $Parms = ("-f fs_blur.sc -o " + $outputs[$i] + "/fs_" + $blur.ToLower()+$axis_suffix[$j] + ".bin --platform " + $platforms[$i] + " -p " + $fprofiles[$i] + " --type fragment --define " + $blur + ";" + $axis[$j]).Split(" ")
         & "$shaderc" $Parms
      }
   }

   Write-Host ""

}



