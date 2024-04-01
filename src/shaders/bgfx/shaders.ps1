

$outputs   = @('metal', 'essl',    'glsl',    'hlsl',    'spirv')
$targets = @('--platform osx -p metal -O 3', '--platform windows -p 320_es', '--platform windows -p 440', '--platform windows -p s_5_0 -O 3', '--platform windows -p spirv')
#$targets = @('--platform ios -p metal', '--platform android -p 320_es', '--platform linux -p440', '--platform windows -p s_5_0', '--platform linux -p spirv')
$shaderc = ".\shaderc.exe"

for($i = 0; $i -lt 5; $i++)
{
   Write-Host Processing shaders for $outputs[$i] using $targets[$i]
   If(!(test-path -PathType container $outputs[$i]))
   {
      New-Item -Name $outputs[$i] -ItemType "directory" > $null
   }

   Write-Host "> Debug shader"
   $Parms = ("-f vs_debug.sc -o " + $outputs[$i] + "/vs_debug.bin " + $targets[$i] + " --type vertex").Split(" ")
   & "$shaderc" $Parms
   $Parms = ("-f fs_debug.sc -o " + $outputs[$i] + "/fs_debug.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   ################################
   # Flasher shaders

   Write-Host "> Flasher shader"
   $Parms = ("-f vs_flasher.sc -o " + $outputs[$i] + "/vs_flasher.bin " + $targets[$i] + " --type vertex").Split(" ")
   & "$shaderc" $Parms
   $Parms = ("-f fs_flasher.sc -o " + $outputs[$i] + "/fs_flasher.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   ################################
   # Basic material shaders

   Write-Host "> Base material shader"
   $variants = @("TEX", "NOTEX")
   foreach ($variant in $variants)
   {
      $Parms = ("-f vs_basic.sc -o " + $outputs[$i] + "/vs_basic_" + $variant.ToLower() + ".bin " + $targets[$i] + " --type vertex --define " + $variant).Split(" ")
      & "$shaderc" $Parms
      $variants2 = @("AT", "NOAT")
      foreach ($variant2 in $variants2)
      {
        $variants3 = @("REFL", "NOREFL")
        foreach ($variant3 in $variants3)
        {
          $Parms = ("-f fs_basic.sc -o " + $outputs[$i] + "/fs_basic_" + $variant.ToLower() + "_" + $variant2.ToLower() + "_" + $variant3.ToLower() + ".bin " + $targets[$i] + " --type fragment --define " + $variant + ";" + $variant2 + ";" + $variant3).Split(" ")
          & "$shaderc" $Parms
        }
      }
   }

   ################################
   # Ball shaders

   Write-Host "> Ball shader"
   $Parms = ("-f vs_ball.sc -o " + $outputs[$i] + "/vs_ball.bin " + $targets[$i] + " --type vertex").Split(" ")
   & "$shaderc" $Parms
   $Parms = ("-f fs_ball_trail.sc -o " + $outputs[$i] + "/fs_ball_trail.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms
   $variants = @("EQUIRECTANGULAR", "SPHERICAL")
   foreach ($variant in $variants)
   {
      $subvariants = @("DECAL", "NODECAL")
      foreach ($subvariant in $subvariants)
      {
         $Parms = ("-f fs_ball.sc -o " + $outputs[$i] + "/fs_ball_" + $variant.ToLower() + "_" + $subvariant.ToLower() + ".bin " + $targets[$i] + " --type fragment --define " + $variant + ";" + $subvariant).Split(" ")
         & "$shaderc" $Parms
      }
   }

   ################################
   # DMD/Sprite shaders

   Write-Host "> DMD & sprite shaders"
   $variants = @("WORLD", "NOWORLD")
   foreach ($variant in $variants)
   {
      $Parms = ("-f vs_dmd.sc -o " + $outputs[$i] + "/vs_basic_dmd_" + $variant.ToLower() + ".bin " + $targets[$i] + " --type vertex --define " + $variant).Split(" ")
      & "$shaderc" $Parms
   }
   $variants = @("DMD", "SPRITE")
   foreach ($variant in $variants)
   {
       
      $variants2 = @("TEX", "NOTEX")
      foreach ($variant2 in $variants2)
      {
         $Parms = ("-f fs_dmd.sc -o " + $outputs[$i] + "/fs_basic_" + $variant.ToLower() + "_" + $variant2.ToLower() + ".bin " + $targets[$i] + " --type fragment --define " + $variant + ";" + $variant2).Split(" ")
         & "$shaderc" $Parms
      }
   }

   ################################
   # Light shaders

   Write-Host "> Light shader"
   $variants = @("NOSHADOW", "BALLSHADOW")
   foreach ($variant in $variants)
   {
      $Parms = ("-f vs_light.sc -o " + $outputs[$i] + "/vs_light_" + $variant.ToLower() + ".bin " + $targets[$i] + " --type vertex --define " + $variant).Split(" ")
      & "$shaderc" $Parms
      $Parms = ("-f fs_light.sc -o " + $outputs[$i] + "/fs_light_" + $variant.ToLower() + ".bin " + $targets[$i] + " --type fragment --define " + $variant).Split(" ")
      & "$shaderc" $Parms
       
      $variants2 = @("TEX", "NOTEX")
      foreach ($variant2 in $variants2)
      {
         $Parms = ("-f vs_basic.sc -o " + $outputs[$i] + "/vs_classic_light_" + $variant2.ToLower() + "_" + $variant.ToLower() + ".bin " + $targets[$i] + " --type vertex --define CLASSIC_LIGHT;" + $variant + ";" + $variant2).Split(" ")
         & "$shaderc" $Parms
         $Parms = ("-f fs_classic_light.sc -o " + $outputs[$i] + "/fs_classic_light_" + $variant2.ToLower() + "_" + $variant.ToLower() + ".bin " + $targets[$i] + " --type fragment --define " + $variant + ";" + $variant2).Split(" ")
         & "$shaderc" $Parms
      }
   }

   ################################
   # Post process shaders

   Write-Host "> Post process shaders"

   $Parms = ("-f vs_postprocess_offseted.sc -o " + $outputs[$i] + "/vs_postprocess_offseted.bin " + $targets[$i] + " --type vertex").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f vs_postprocess.sc -o " + $outputs[$i] + "/vs_postprocess.bin " + $targets[$i] + " --type vertex").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_mirror.sc -o " + $outputs[$i] + "/fs_pp_mirror.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_copy.sc -o " + $outputs[$i] + "/fs_pp_copy.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_bloom.sc -o " + $outputs[$i] + "/fs_pp_bloom.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_irradiance.sc -o " + $outputs[$i] + "/fs_pp_irradiance.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_ssao.sc -o " + $outputs[$i] + "/fs_pp_ssao.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_ssr.sc -o " + $outputs[$i] + "/fs_pp_ssr.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_cas.sc -o " + $outputs[$i] + "/fs_pp_cas.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_bilateral_cas.sc -o " + $outputs[$i] + "/fs_pp_bilateral_cas.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $Parms = ("-f fs_pp_nfaa.sc -o " + $outputs[$i] + "/fs_pp_nfaa.bin " + $targets[$i] + " --type fragment").Split(" ")
   & "$shaderc" $Parms

   $variants = @("FXAA1", "FXAA2", "FXAA3")
   foreach ($variant in $variants)
   {
      $Parms = ("-f fs_pp_fxaa.sc -o " + $outputs[$i] + "/fs_pp_" + $variant.ToLower() + ".bin " + $targets[$i] + " --type fragment --define " + $variant).Split(" ")
      & "$shaderc" $Parms
   }

   $variants = @("REINHARD", "FILMIC", "TONY")
   foreach ($variant in $variants)
   {
   $variants2 = @("AO", "NOAO")
   foreach ($variant2 in $variants2)
   {
   $variants3 = @("FILTER", "NOFILTER")
   foreach ($variant3 in $variants3)
   {
   $variants4 = @("GRAY", "RG", "RGB")
   foreach ($variant4 in $variants4)
   {
      $Parms = ("-f fs_pp_tonemap.sc -o " + $outputs[$i] + "/fs_pp_" + $variant.ToLower() + "_" + $variant2.ToLower() + "_" + $variant3.ToLower() + "_" + $variant4.ToLower() + ".bin " + $targets[$i] + " --type fragment --define " + $variant + ";" + $variant2 + ";" + $variant3 + ";" + $variant4).Split(" ")
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
         $Parms = ("-f fs_blur.sc -o " + $outputs[$i] + "/fs_" + $blur.ToLower()+$axis_suffix[$j] + ".bin " + $targets[$i] + " --type fragment --define " + $blur + ";" + $axis[$j]).Split(" ")
         & "$shaderc" $Parms
      }
   }

   Write-Host ""

}



