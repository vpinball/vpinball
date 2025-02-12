cmake_minimum_required(VERSION 3.25)

set(PLATFORM "win" CACHE STRING "Platform")
set(ARCH "x64" CACHE STRING "Arch")

option(BUILD_SHARED "Option to build shared library" ON)
option(BUILD_STATIC "Option to build static library" ON)

message(STATUS "PLATFORM: ${PLATFORM}")
message(STATUS "ARCH: ${ARCH}")

message(STATUS "BUILD_SHARED: ${BUILD_SHARED}")
message(STATUS "BUILD_STATIC: ${BUILD_STATIC}")

if(PLATFORM STREQUAL "macos")
   set(CMAKE_OSX_DEPLOYMENT_TARGET 14.0)
elseif(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator")
   set(CMAKE_SYSTEM_NAME iOS)
   if (PLATFORM STREQUAL "ios-simulator")
      set(CMAKE_OSX_SYSROOT iphonesimulator)
   endif()
   set(CMAKE_OSX_DEPLOYMENT_TARGET 17.0)
elseif(PLATFORM STREQUAL "tvos")
   set(CMAKE_SYSTEM_NAME tvOS)
   set(CMAKE_OSX_DEPLOYMENT_TARGET 17.0)
elseif(PLATFORM STREQUAL "android")
   set(CMAKE_SYSTEM_NAME Android)
   set(CMAKE_SYSTEM_VERSION 30)
   set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
endif()

file(READ Source/FreeImage.h version)
string(REGEX MATCH "FREEIMAGE_MAJOR_VERSION[ ]+([0-9]+)" _tmp ${version})
set(VERSION_MAJOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "FREEIMAGE_MINOR_VERSION[ ]+([0-9]+)" _tmp ${version})
set(VERSION_MINOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "FREEIMAGE_RELEASE_SERIAL[ ]+([0-9]+)" _tmp ${version})
set(VERSION_PATCH "${CMAKE_MATCH_1}")

project(freeimage VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

if(PLATFORM STREQUAL "win")
   if(ARCH STREQUAL "x86")
      add_compile_definitions(WIN32)
   endif()
elseif(PLATFORM STREQUAL "macos")
   if (ARCH STREQUAL "arm64")
      set(CMAKE_OSX_ARCHITECTURES arm64)
   elseif(ARCH STREQUAL "x64")
      set(CMAKE_OSX_ARCHITECTURES x86_64)
   endif()
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
   set(CMAKE_INSTALL_RPATH "@executable_path")
elseif(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator" OR PLATFORM STREQUAL "tvos")
   set(CMAKE_OSX_ARCHITECTURES arm64)
elseif(PLATFORM STREQUAL "linux" OR PLATFORM STREQUAL "android")
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
   set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)

set(FREEIMAGE_SOURCES
   Source/FreeImage/BitmapAccess.cpp
   Source/FreeImage/ColorLookup.cpp
   Source/FreeImage/ConversionRGBA16.cpp
   Source/FreeImage/ConversionRGBAF.cpp
   Source/FreeImage/FreeImage.cpp
   Source/FreeImage/FreeImageC.c
   Source/FreeImage/FreeImageIO.cpp
   Source/FreeImage/GetType.cpp
   Source/FreeImage/LFPQuantizer.cpp
   Source/FreeImage/MemoryIO.cpp
   Source/FreeImage/PixelAccess.cpp
   Source/FreeImage/J2KHelper.cpp
   Source/FreeImage/MNGHelper.cpp
   Source/FreeImage/Plugin.cpp
   Source/FreeImage/PluginBMP.cpp
   Source/FreeImage/PluginCUT.cpp
   Source/FreeImage/PluginDDS.cpp
   Source/FreeImage/PluginEXR.cpp
   Source/FreeImage/PluginG3.cpp
   Source/FreeImage/PluginGIF.cpp
   Source/FreeImage/PluginHDR.cpp
   Source/FreeImage/PluginICO.cpp
   Source/FreeImage/PluginIFF.cpp
   Source/FreeImage/PluginJ2K.cpp
   Source/FreeImage/PluginJNG.cpp
   Source/FreeImage/PluginJP2.cpp
   Source/FreeImage/PluginJPEG.cpp
   Source/FreeImage/PluginJXR.cpp
   Source/FreeImage/PluginKOALA.cpp
   Source/FreeImage/PluginMNG.cpp
   Source/FreeImage/PluginPCD.cpp
   Source/FreeImage/PluginPCX.cpp
   Source/FreeImage/PluginPFM.cpp
   Source/FreeImage/PluginPICT.cpp
   Source/FreeImage/PluginPNG.cpp
   Source/FreeImage/PluginPNM.cpp
   Source/FreeImage/PluginPSD.cpp
   Source/FreeImage/PluginRAS.cpp
   Source/FreeImage/PluginRAW.cpp
   Source/FreeImage/PluginSGI.cpp
   Source/FreeImage/PluginTARGA.cpp
   Source/FreeImage/PluginTIFF.cpp
   Source/FreeImage/PluginWBMP.cpp
   Source/FreeImage/PluginWebP.cpp
   Source/FreeImage/PluginXBM.cpp
   Source/FreeImage/PluginXPM.cpp
   Source/FreeImage/PSDParser.cpp
   Source/FreeImage/TIFFLogLuv.cpp
   Source/FreeImage/Conversion.cpp
   Source/FreeImage/Conversion16_555.cpp
   Source/FreeImage/Conversion16_565.cpp
   Source/FreeImage/Conversion24.cpp
   Source/FreeImage/Conversion32.cpp
   Source/FreeImage/Conversion4.cpp
   Source/FreeImage/Conversion8.cpp
   Source/FreeImage/ConversionFloat.cpp
   Source/FreeImage/ConversionRGB16.cpp
   Source/FreeImage/ConversionRGBF.cpp
   Source/FreeImage/ConversionType.cpp
   Source/FreeImage/ConversionUINT16.cpp
   Source/FreeImage/Halftoning.cpp
   Source/FreeImage/tmoColorConvert.cpp
   Source/FreeImage/tmoDrago03.cpp
   Source/FreeImage/tmoFattal02.cpp
   Source/FreeImage/tmoReinhard05.cpp
   Source/FreeImage/ToneMapping.cpp
   Source/FreeImage/NNQuantizer.cpp
   Source/FreeImage/WuQuantizer.cpp
   Source/FreeImage/CacheFile.cpp
   Source/FreeImage/MultiPage.cpp
   Source/FreeImage/ZLibInterface.cpp
   Source/Metadata/Exif.cpp
   Source/Metadata/FIRational.cpp
   Source/Metadata/FreeImageTag.cpp
   Source/Metadata/IPTC.cpp
   Source/Metadata/TagConversion.cpp
   Source/Metadata/TagLib.cpp
   Source/Metadata/XTIFF.cpp
   Source/FreeImageToolkit/Background.cpp
   Source/FreeImageToolkit/BSplineRotate.cpp
   Source/FreeImageToolkit/Channels.cpp
   Source/FreeImageToolkit/ClassicRotate.cpp
   Source/FreeImageToolkit/Colors.cpp
   Source/FreeImageToolkit/CopyPaste.cpp
   Source/FreeImageToolkit/Display.cpp
   Source/FreeImageToolkit/Flip.cpp
   Source/FreeImageToolkit/JPEGTransform.cpp
   Source/FreeImageToolkit/MultigridPoissonSolver.cpp
   Source/FreeImageToolkit/Rescale.cpp
   Source/FreeImageToolkit/Resize.cpp
   Source/LibJPEG/jaricom.c
   Source/LibJPEG/jcapimin.c
   Source/LibJPEG/jcapistd.c
   Source/LibJPEG/jcarith.c
   Source/LibJPEG/jccoefct.c
   Source/LibJPEG/jccolor.c
   Source/LibJPEG/jcdctmgr.c
   Source/LibJPEG/jchuff.c
   Source/LibJPEG/jcinit.c
   Source/LibJPEG/jcmainct.c
   Source/LibJPEG/jcmarker.c
   Source/LibJPEG/jcmaster.c
   Source/LibJPEG/jcomapi.c
   Source/LibJPEG/jcparam.c
   Source/LibJPEG/jcprepct.c
   Source/LibJPEG/jcsample.c
   Source/LibJPEG/jctrans.c
   Source/LibJPEG/jdapimin.c
   Source/LibJPEG/jdapistd.c
   Source/LibJPEG/jdarith.c
   Source/LibJPEG/jdatadst.c
   Source/LibJPEG/jdatasrc.c
   Source/LibJPEG/jdcoefct.c
   Source/LibJPEG/jdcolor.c
   Source/LibJPEG/jddctmgr.c
   Source/LibJPEG/jdhuff.c
   Source/LibJPEG/jdinput.c
   Source/LibJPEG/jdmainct.c
   Source/LibJPEG/jdmarker.c
   Source/LibJPEG/jdmaster.c
   Source/LibJPEG/jdmerge.c
   Source/LibJPEG/jdpostct.c
   Source/LibJPEG/jdsample.c
   Source/LibJPEG/jdtrans.c
   Source/LibJPEG/jerror.c
   Source/LibJPEG/jfdctflt.c
   Source/LibJPEG/jfdctfst.c
   Source/LibJPEG/jfdctint.c
   Source/LibJPEG/jidctflt.c
   Source/LibJPEG/jidctfst.c
   Source/LibJPEG/jidctint.c
   Source/LibJPEG/jmemmgr.c
   Source/LibJPEG/jmemnobs.c
   Source/LibJPEG/jquant1.c
   Source/LibJPEG/jquant2.c
   Source/LibJPEG/jutils.c
   Source/LibJPEG/transupp.c
   Source/LibPNG/png.c
   Source/LibPNG/pngerror.c
   Source/LibPNG/pngget.c
   Source/LibPNG/pngmem.c
   Source/LibPNG/pngpread.c
   Source/LibPNG/pngread.c
   Source/LibPNG/pngrio.c
   Source/LibPNG/pngrtran.c
   Source/LibPNG/pngrutil.c
   Source/LibPNG/pngset.c
   Source/LibPNG/pngtrans.c
   Source/LibPNG/pngwio.c
   Source/LibPNG/pngwrite.c
   Source/LibPNG/pngwtran.c
   Source/LibPNG/pngwutil.c
   Source/LibTIFF4/tif_aux.c
   Source/LibTIFF4/tif_close.c
   Source/LibTIFF4/tif_codec.c
   Source/LibTIFF4/tif_color.c
   Source/LibTIFF4/tif_compress.c
   Source/LibTIFF4/tif_dir.c
   Source/LibTIFF4/tif_dirinfo.c
   Source/LibTIFF4/tif_dirread.c
   Source/LibTIFF4/tif_dirwrite.c
   Source/LibTIFF4/tif_dumpmode.c
   Source/LibTIFF4/tif_error.c
   Source/LibTIFF4/tif_extension.c
   Source/LibTIFF4/tif_fax3.c
   Source/LibTIFF4/tif_fax3sm.c
   Source/LibTIFF4/tif_flush.c
   Source/LibTIFF4/tif_getimage.c
   Source/LibTIFF4/tif_hash_set.c
   Source/LibTIFF4/tif_jpeg.c
   Source/LibTIFF4/tif_lerc.c
   Source/LibTIFF4/tif_luv.c
   Source/LibTIFF4/tif_lzma.c
   Source/LibTIFF4/tif_lzw.c
   Source/LibTIFF4/tif_next.c
   Source/LibTIFF4/tif_ojpeg.c
   Source/LibTIFF4/tif_open.c
   Source/LibTIFF4/tif_packbits.c
   Source/LibTIFF4/tif_pixarlog.c
   Source/LibTIFF4/tif_predict.c
   Source/LibTIFF4/tif_print.c
   Source/LibTIFF4/tif_read.c
   Source/LibTIFF4/tif_strip.c
   Source/LibTIFF4/tif_swab.c
   Source/LibTIFF4/tif_thunder.c
   Source/LibTIFF4/tif_tile.c
   Source/LibTIFF4/tif_version.c
   Source/LibTIFF4/tif_warning.c
   Source/LibTIFF4/tif_webp.c
   Source/LibTIFF4/tif_write.c
   Source/LibTIFF4/tif_zip.c
   Source/ZLib/adler32.c
   Source/ZLib/compress.c
   Source/ZLib/crc32.c
   Source/ZLib/deflate.c
   Source/ZLib/gzclose.c
   Source/ZLib/gzlib.c
   Source/ZLib/gzread.c
   Source/ZLib/gzwrite.c
   Source/ZLib/infback.c
   Source/ZLib/inffast.c
   Source/ZLib/inflate.c
   Source/ZLib/inftrees.c
   Source/ZLib/trees.c
   Source/ZLib/uncompr.c
   Source/ZLib/zutil.c
   Source/LibOpenJPEG/bio.c
   Source/LibOpenJPEG/cio.c
   Source/LibOpenJPEG/dwt.c
   Source/LibOpenJPEG/event.c
   Source/LibOpenJPEG/function_list.c
   Source/LibOpenJPEG/image.c
   Source/LibOpenJPEG/invert.c
   Source/LibOpenJPEG/j2k.c
   Source/LibOpenJPEG/jp2.c
   Source/LibOpenJPEG/mct.c
   Source/LibOpenJPEG/mqc.c
   Source/LibOpenJPEG/openjpeg.c
   Source/LibOpenJPEG/opj_clock.c
   Source/LibOpenJPEG/pi.c
   Source/LibOpenJPEG/raw.c
   Source/LibOpenJPEG/t1.c
   Source/LibOpenJPEG/t2.c
   Source/LibOpenJPEG/tcd.c
   Source/LibOpenJPEG/tgt.c
   Source/OpenEXR/IlmThread/IlmThreadSemaphoreOSX.cpp
   Source/OpenEXR/IlmThread/IlmThreadPool.cpp
   Source/OpenEXR/IlmThread/IlmThreadSemaphorePosixCompat.cpp
   Source/OpenEXR/IlmThread/IlmThread.cpp
   Source/OpenEXR/IlmThread/IlmThreadSemaphore.cpp
   Source/OpenEXR/IlmThread/IlmThreadSemaphorePosix.cpp
   Source/OpenEXR/IlmThread/IlmThreadSemaphoreWin32.cpp
   Source/OpenEXR/Imath/ImathFun.cpp
   Source/OpenEXR/Imath/half.cpp
   Source/OpenEXR/Imath/ImathRandom.cpp
   Source/OpenEXR/Imath/ImathMatrixAlgo.cpp
   Source/OpenEXR/Imath/ImathColorAlgo.cpp
   Source/OpenEXR/OpenEXR/ImfInputFile.cpp
   Source/OpenEXR/OpenEXR/ImfIDManifestAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfFloatAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfPreviewImageAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfBoxAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfGenericInputFile.cpp
   Source/OpenEXR/OpenEXR/ImfPxr24Compressor.cpp
   Source/OpenEXR/OpenEXR/ImfOutputPart.cpp
   Source/OpenEXR/OpenEXR/ImfCompositeDeepScanLine.cpp
   Source/OpenEXR/OpenEXR/ImfDoubleAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfFramesPerSecond.cpp
   Source/OpenEXR/OpenEXR/ImfDeepScanLineOutputPart.cpp
   Source/OpenEXR/OpenEXR/ImfLineOrderAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfMultiPartOutputFile.cpp
   Source/OpenEXR/OpenEXR/ImfConvert.cpp
   Source/OpenEXR/OpenEXR/ImfB44Compressor.cpp
   Source/OpenEXR/OpenEXR/ImfRleCompressor.cpp
   Source/OpenEXR/OpenEXR/ImfCompressionAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfKeyCodeAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfCRgbaFile.cpp
   Source/OpenEXR/OpenEXR/ImfRgbaFile.cpp
   Source/OpenEXR/OpenEXR/ImfDeepScanLineInputFile.cpp
   Source/OpenEXR/OpenEXR/ImfCompressor.cpp
   Source/OpenEXR/OpenEXR/ImfDeepTiledInputFile.cpp
   Source/OpenEXR/OpenEXR/ImfGenericOutputFile.cpp
   Source/OpenEXR/OpenEXR/ImfKeyCode.cpp
   Source/OpenEXR/OpenEXR/ImfVecAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfInputPartData.cpp
   Source/OpenEXR/OpenEXR/ImfVersion.cpp
   Source/OpenEXR/OpenEXR/ImfIO.cpp
   Source/OpenEXR/OpenEXR/ImfRationalAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfMultiPartInputFile.cpp
   Source/OpenEXR/OpenEXR/ImfTimeCodeAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfEnvmap.cpp
   Source/OpenEXR/OpenEXR/ImfTiledOutputFile.cpp
   Source/OpenEXR/OpenEXR/ImfChannelListAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfDeepTiledOutputFile.cpp
   Source/OpenEXR/OpenEXR/ImfTimeCode.cpp
   Source/OpenEXR/OpenEXR/ImfDeepCompositing.cpp
   Source/OpenEXR/OpenEXR/ImfDwaCompressor.cpp
   Source/OpenEXR/OpenEXR/ImfStdIO.cpp
   Source/OpenEXR/OpenEXR/ImfMatrixAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfAcesFile.cpp
   Source/OpenEXR/OpenEXR/ImfZipCompressor.cpp
   Source/OpenEXR/OpenEXR/ImfTiledInputFile.cpp
   Source/OpenEXR/OpenEXR/ImfChannelList.cpp
   Source/OpenEXR/OpenEXR/ImfThreading.cpp
   Source/OpenEXR/OpenEXR/ImfDeepScanLineOutputFile.cpp
   Source/OpenEXR/OpenEXR/ImfSystemSpecific.cpp
   Source/OpenEXR/OpenEXR/ImfTiledRgbaFile.cpp
   Source/OpenEXR/OpenEXR/ImfStandardAttributes.cpp
   Source/OpenEXR/OpenEXR/ImfRgbaYca.cpp
   Source/OpenEXR/OpenEXR/ImfChromaticitiesAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfTestFile.cpp
   Source/OpenEXR/OpenEXR/ImfDeepScanLineInputPart.cpp
   Source/OpenEXR/OpenEXR/ImfDeepTiledInputPart.cpp
   Source/OpenEXR/OpenEXR/ImfDeepFrameBuffer.cpp
   Source/OpenEXR/OpenEXR/ImfScanLineInputFile.cpp
   Source/OpenEXR/OpenEXR/ImfTiledMisc.cpp
   Source/OpenEXR/OpenEXR/ImfStringVectorAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfInputPart.cpp
   Source/OpenEXR/OpenEXR/ImfFloatVectorAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfStringAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfOutputFile.cpp
   Source/OpenEXR/OpenEXR/ImfRle.cpp
   Source/OpenEXR/OpenEXR/ImfMisc.cpp
   Source/OpenEXR/OpenEXR/ImfChromaticities.cpp
   Source/OpenEXR/OpenEXR/ImfOpaqueAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfPartType.cpp
   Source/OpenEXR/OpenEXR/ImfZip.cpp
   Source/OpenEXR/OpenEXR/ImfMultiView.cpp
   Source/OpenEXR/OpenEXR/ImfRational.cpp
   Source/OpenEXR/OpenEXR/ImfDeepTiledOutputPart.cpp
   Source/OpenEXR/OpenEXR/ImfWav.cpp
   Source/OpenEXR/OpenEXR/ImfTiledInputPart.cpp
   Source/OpenEXR/OpenEXR/ImfEnvmapAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfLut.cpp
   Source/OpenEXR/OpenEXR/ImfPizCompressor.cpp
   Source/OpenEXR/OpenEXR/ImfOutputPartData.cpp
   Source/OpenEXR/OpenEXR/ImfFastHuf.cpp
   Source/OpenEXR/OpenEXR/ImfAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfHuf.cpp
   Source/OpenEXR/OpenEXR/ImfDeepImageStateAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfIntAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfHeader.cpp
   Source/OpenEXR/OpenEXR/ImfIDManifest.cpp
   Source/OpenEXR/OpenEXR/ImfTiledOutputPart.cpp
   Source/OpenEXR/OpenEXR/ImfTileOffsets.cpp
   Source/OpenEXR/OpenEXR/ImfTileDescriptionAttribute.cpp
   Source/OpenEXR/OpenEXR/ImfFrameBuffer.cpp
   Source/OpenEXR/OpenEXR/ImfPreviewImage.cpp
   Source/OpenEXR/Iex/IexThrowErrnoExc.cpp
   Source/OpenEXR/Iex/IexBaseExc.cpp
   Source/OpenEXR/Iex/IexMathFpu.cpp
   Source/OpenEXR/Iex/IexMathFloatExc.cpp
   Source/LibRawLite/src/write/tiff_writer.cpp
   Source/LibRawLite/src/write/file_write.cpp
   Source/LibRawLite/src/write/apply_profile.cpp
   Source/LibRawLite/src/tables/cameralist.cpp
   Source/LibRawLite/src/tables/wblists.cpp
   Source/LibRawLite/src/tables/colordata.cpp
   Source/LibRawLite/src/tables/colorconst.cpp
   Source/LibRawLite/src/libraw_datastream.cpp
   Source/LibRawLite/src/libraw_c_api.cpp
   Source/LibRawLite/src/integration/rawspeed_glue.cpp
   Source/LibRawLite/src/integration/dngsdk_glue.cpp
   Source/LibRawLite/src/x3f/x3f_parse_process.cpp
   Source/LibRawLite/src/x3f/x3f_utils_patched.cpp
   Source/LibRawLite/src/utils/open.cpp
   Source/LibRawLite/src/utils/utils_libraw.cpp
   Source/LibRawLite/src/utils/init_close_utils.cpp
   Source/LibRawLite/src/utils/utils_dcraw.cpp
   Source/LibRawLite/src/utils/decoder_info.cpp
   Source/LibRawLite/src/utils/thumb_utils.cpp
   Source/LibRawLite/src/utils/phaseone_processing.cpp
   Source/LibRawLite/src/utils/read_utils.cpp
   Source/LibRawLite/src/utils/curves.cpp
   Source/LibRawLite/src/postprocessing/postprocessing_utils_dcrdefs.cpp
   Source/LibRawLite/src/postprocessing/postprocessing_utils.cpp
   Source/LibRawLite/src/postprocessing/postprocessing_aux.cpp
   Source/LibRawLite/src/postprocessing/aspect_ratio.cpp
   Source/LibRawLite/src/postprocessing/mem_image.cpp
   Source/LibRawLite/src/postprocessing/dcraw_process.cpp
   Source/LibRawLite/src/demosaic/misc_demosaic.cpp
   Source/LibRawLite/src/demosaic/dcb_demosaic.cpp
   Source/LibRawLite/src/demosaic/dht_demosaic.cpp
   Source/LibRawLite/src/demosaic/xtrans_demosaic.cpp
   Source/LibRawLite/src/demosaic/ahd_demosaic.cpp
   Source/LibRawLite/src/demosaic/aahd_demosaic.cpp
   Source/LibRawLite/src/decoders/unpack_thumb.cpp
   Source/LibRawLite/src/decoders/decoders_libraw.cpp
   Source/LibRawLite/src/decoders/kodak_decoders.cpp
   Source/LibRawLite/src/decoders/decoders_libraw_dcrdefs.cpp
   Source/LibRawLite/src/decoders/generic.cpp
   Source/LibRawLite/src/decoders/unpack.cpp
   Source/LibRawLite/src/decoders/canon_600.cpp
   Source/LibRawLite/src/decoders/load_mfbacks.cpp
   Source/LibRawLite/src/decoders/fp_dng.cpp
   Source/LibRawLite/src/decoders/smal.cpp
   Source/LibRawLite/src/decoders/fuji_compressed.cpp
   Source/LibRawLite/src/decoders/dng.cpp
   Source/LibRawLite/src/decoders/crx.cpp
   Source/LibRawLite/src/decoders/decoders_dcraw.cpp
   Source/LibRawLite/src/preprocessing/subtract_black.cpp
   Source/LibRawLite/src/preprocessing/ext_preprocess.cpp
   Source/LibRawLite/src/preprocessing/raw2image.cpp
   Source/LibRawLite/src/metadata/identify.cpp
   Source/LibRawLite/src/metadata/exif_gps.cpp
   Source/LibRawLite/src/metadata/mediumformat.cpp
   Source/LibRawLite/src/metadata/sony.cpp
   Source/LibRawLite/src/metadata/ciff.cpp
   Source/LibRawLite/src/metadata/p1.cpp
   Source/LibRawLite/src/metadata/fuji.cpp
   Source/LibRawLite/src/metadata/nikon.cpp
   Source/LibRawLite/src/metadata/cr3_parser.cpp
   Source/LibRawLite/src/metadata/adobepano.cpp
   Source/LibRawLite/src/metadata/pentax.cpp
   Source/LibRawLite/src/metadata/normalize_model.cpp
   Source/LibRawLite/src/metadata/canon.cpp
   Source/LibRawLite/src/metadata/identify_tools.cpp
   Source/LibRawLite/src/metadata/tiff.cpp
   Source/LibRawLite/src/metadata/kodak.cpp
   Source/LibRawLite/src/metadata/olympus.cpp
   Source/LibRawLite/src/metadata/samsung.cpp
   Source/LibRawLite/src/metadata/minolta.cpp
   Source/LibRawLite/src/metadata/misc_parsers.cpp
   Source/LibRawLite/src/metadata/makernotes.cpp
   Source/LibRawLite/src/metadata/leica.cpp
   Source/LibRawLite/src/metadata/hasselblad_model.cpp
   Source/LibRawLite/src/metadata/epson.cpp
   Source/LibWebP/src/dec/alpha_dec.c
   Source/LibWebP/src/dec/buffer_dec.c
   Source/LibWebP/src/dec/frame_dec.c
   Source/LibWebP/src/dec/idec_dec.c
   Source/LibWebP/src/dec/io_dec.c
   Source/LibWebP/src/dec/quant_dec.c
   Source/LibWebP/src/dec/tree_dec.c
   Source/LibWebP/src/dec/vp8l_dec.c
   Source/LibWebP/src/dec/vp8_dec.c
   Source/LibWebP/src/dec/webp_dec.c
   Source/LibWebP/src/demux/anim_decode.c
   Source/LibWebP/src/demux/demux.c
   Source/LibWebP/src/dsp/alpha_processing.c
   Source/LibWebP/src/dsp/alpha_processing_mips_dsp_r2.c
   Source/LibWebP/src/dsp/alpha_processing_neon.c
   Source/LibWebP/src/dsp/alpha_processing_sse2.c
   Source/LibWebP/src/dsp/alpha_processing_sse41.c
   Source/LibWebP/src/dsp/cost_neon.c
   Source/LibWebP/src/dsp/cost.c
   Source/LibWebP/src/dsp/cost_mips32.c
   Source/LibWebP/src/dsp/cost_mips_dsp_r2.c
   Source/LibWebP/src/dsp/cost_sse2.c
   Source/LibWebP/src/dsp/cpu.c
   Source/LibWebP/src/dsp/dec.c
   Source/LibWebP/src/dsp/dec_clip_tables.c
   Source/LibWebP/src/dsp/dec_mips32.c
   Source/LibWebP/src/dsp/dec_mips_dsp_r2.c
   Source/LibWebP/src/dsp/dec_msa.c
   Source/LibWebP/src/dsp/dec_neon.c
   Source/LibWebP/src/dsp/dec_sse2.c
   Source/LibWebP/src/dsp/dec_sse41.c
   Source/LibWebP/src/dsp/enc.c
   Source/LibWebP/src/dsp/enc_avx2.c
   Source/LibWebP/src/dsp/enc_mips32.c
   Source/LibWebP/src/dsp/enc_mips_dsp_r2.c
   Source/LibWebP/src/dsp/enc_msa.c
   Source/LibWebP/src/dsp/enc_neon.c
   Source/LibWebP/src/dsp/enc_sse2.c
   Source/LibWebP/src/dsp/enc_sse41.c
   Source/LibWebP/src/dsp/filters.c
   Source/LibWebP/src/dsp/filters_mips_dsp_r2.c
   Source/LibWebP/src/dsp/filters_msa.c
   Source/LibWebP/src/dsp/filters_neon.c
   Source/LibWebP/src/dsp/filters_sse2.c
   Source/LibWebP/src/dsp/lossless.c
   Source/LibWebP/src/dsp/lossless_enc.c
   Source/LibWebP/src/dsp/lossless_enc_mips32.c
   Source/LibWebP/src/dsp/lossless_enc_mips_dsp_r2.c
   Source/LibWebP/src/dsp/lossless_enc_msa.c
   Source/LibWebP/src/dsp/lossless_enc_neon.c
   Source/LibWebP/src/dsp/lossless_enc_sse2.c
   Source/LibWebP/src/dsp/lossless_enc_sse41.c
   Source/LibWebP/src/dsp/lossless_mips_dsp_r2.c
   Source/LibWebP/src/dsp/lossless_msa.c
   Source/LibWebP/src/dsp/lossless_neon.c
   Source/LibWebP/src/dsp/lossless_sse2.c
   Source/LibWebP/src/dsp/lossless_sse41.c
   Source/LibWebP/src/dsp/rescaler.c
   Source/LibWebP/src/dsp/rescaler_mips32.c
   Source/LibWebP/src/dsp/rescaler_mips_dsp_r2.c
   Source/LibWebP/src/dsp/rescaler_msa.c
   Source/LibWebP/src/dsp/rescaler_neon.c
   Source/LibWebP/src/dsp/rescaler_sse2.c
   Source/LibWebP/src/dsp/ssim.c
   Source/LibWebP/src/dsp/ssim_sse2.c
   Source/LibWebP/src/dsp/upsampling.c
   Source/LibWebP/src/dsp/upsampling_mips_dsp_r2.c
   Source/LibWebP/src/dsp/upsampling_msa.c
   Source/LibWebP/src/dsp/upsampling_neon.c
   Source/LibWebP/src/dsp/upsampling_sse2.c
   Source/LibWebP/src/dsp/upsampling_sse41.c
   Source/LibWebP/src/dsp/yuv.c
   Source/LibWebP/src/dsp/yuv_mips32.c
   Source/LibWebP/src/dsp/yuv_mips_dsp_r2.c
   Source/LibWebP/src/dsp/yuv_neon.c
   Source/LibWebP/src/dsp/yuv_sse2.c
   Source/LibWebP/src/dsp/yuv_sse41.c
   Source/LibWebP/src/enc/alpha_enc.c
   Source/LibWebP/src/enc/analysis_enc.c
   Source/LibWebP/src/enc/backward_references_cost_enc.c
   Source/LibWebP/src/enc/backward_references_enc.c
   Source/LibWebP/src/enc/config_enc.c
   Source/LibWebP/src/enc/cost_enc.c
   Source/LibWebP/src/enc/filter_enc.c
   Source/LibWebP/src/enc/frame_enc.c
   Source/LibWebP/src/enc/histogram_enc.c
   Source/LibWebP/src/enc/iterator_enc.c
   Source/LibWebP/src/enc/near_lossless_enc.c
   Source/LibWebP/src/enc/picture_csp_enc.c
   Source/LibWebP/src/enc/picture_enc.c
   Source/LibWebP/src/enc/picture_psnr_enc.c
   Source/LibWebP/src/enc/picture_rescale_enc.c
   Source/LibWebP/src/enc/picture_tools_enc.c
   Source/LibWebP/src/enc/predictor_enc.c
   Source/LibWebP/src/enc/quant_enc.c
   Source/LibWebP/src/enc/syntax_enc.c
   Source/LibWebP/src/enc/token_enc.c
   Source/LibWebP/src/enc/tree_enc.c
   Source/LibWebP/src/enc/vp8l_enc.c
   Source/LibWebP/src/enc/webp_enc.c
   Source/LibWebP/src/mux/anim_encode.c
   Source/LibWebP/src/mux/muxedit.c
   Source/LibWebP/src/mux/muxinternal.c
   Source/LibWebP/src/mux/muxread.c
   Source/LibWebP/src/utils/bit_reader_utils.c
   Source/LibWebP/src/utils/bit_writer_utils.c
   Source/LibWebP/src/utils/color_cache_utils.c
   Source/LibWebP/src/utils/filters_utils.c
   Source/LibWebP/src/utils/huffman_encode_utils.c
   Source/LibWebP/src/utils/huffman_utils.c
   Source/LibWebP/src/utils/quant_levels_dec_utils.c
   Source/LibWebP/src/utils/quant_levels_utils.c
   Source/LibWebP/src/utils/random_utils.c
   Source/LibWebP/src/utils/rescaler_utils.c
   Source/LibWebP/src/utils/thread_utils.c
   Source/LibWebP/src/utils/utils.c
   Source/LibJXR/image/decode/decode.c
   Source/LibJXR/image/decode/JXRTranscode.c
   Source/LibJXR/image/decode/postprocess.c
   Source/LibJXR/image/decode/segdec.c
   Source/LibJXR/image/decode/strdec.c
   Source/LibJXR/image/decode/strdec_x86.c
   Source/LibJXR/image/decode/strInvTransform.c
   Source/LibJXR/image/decode/strPredQuantDec.c
   Source/LibJXR/image/encode/encode.c
   Source/LibJXR/image/encode/segenc.c
   Source/LibJXR/image/encode/strenc.c
   Source/LibJXR/image/encode/strenc_x86.c
   Source/LibJXR/image/encode/strFwdTransform.c
   Source/LibJXR/image/encode/strPredQuantEnc.c
   Source/LibJXR/image/sys/adapthuff.c
   Source/LibJXR/image/sys/image.c
   Source/LibJXR/image/sys/strcodec.c
   Source/LibJXR/image/sys/strPredQuant.c
   Source/LibJXR/image/sys/strTransform.c
   Source/LibJXR/jxrgluelib/JXRGlue.c
   Source/LibJXR/jxrgluelib/JXRGlueJxr.c
   Source/LibJXR/jxrgluelib/JXRGluePFC.c
   Source/LibJXR/jxrgluelib/JXRMeta.c
)

set(FREEIMAGE_INCLUDE_DIRS
   Source
   Source/Metadata
   Source/FreeImageToolkit
   Source/LibJPEG
   Source/LibPNG
   Source/ZLib
   Source/LibOpenJPEG
   Source/OpenEXR
   Source/OpenEXR/Iex
   Source/OpenEXR/IlmThread
   Source/OpenEXR/Imath
   Source/OpenEXR/OpenEXR
   Source/LibRawLite
   Source/LibRawLite/dcraw
   Source/LibRawLite/internal
   Source/LibRawLite/libraw
   Source/LibRawLite/src
   Source/LibWebP
   Source/LibJXR
   Source/LibJXR/common/include
   Source/LibJXR/image/sys
   Source/LibJXR/jxrgluelib
)

add_compile_definitions(
   DISABLE_PERF_MEASUREMENT
)

if(PLATFORM STREQUAL "win")
   add_compile_definitions(
      WIN32
      OPJ_STATIC
      LIBRAW_NODLL
      FREEIMAGE_EXPORTS
   )
else()
   add_compile_definitions(
      __ANSI__
      PNG_ARM_NEON_OPT=0
   )
endif()

if(PLATFORM STREQUAL "win")
   add_compile_options(
      -msse4.1
   )
else()
   add_compile_options(
      -Wno-implicit-function-declaration
      -ffast-math
   )
endif()

if(BUILD_SHARED)
   add_library(freeimage_shared SHARED ${FREEIMAGE_SOURCES})

   target_include_directories(freeimage_shared PUBLIC ${FREEIMAGE_INCLUDE_DIRS})

   if(PLATFORM STREQUAL "win" AND ARCH STREQUAL "x64")
      set(FREEIMAGE_OUTPUT_NAME "freeimage64")
   else()
      set(FREEIMAGE_OUTPUT_NAME "freeimage")
   endif()

   set_target_properties(freeimage_shared PROPERTIES
      OUTPUT_NAME ${FREEIMAGE_OUTPUT_NAME}
      VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
   )

   install(TARGETS freeimage_shared
      LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
   )

   install(FILES source/FreeImage.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif()

if(BUILD_STATIC)
   add_library(freeimage_static STATIC ${FREEIMAGE_SOURCES})

   target_include_directories(freeimage_static PUBLIC ${FREEIMAGE_INCLUDE_DIRS})

   if(PLATFORM STREQUAL "win")
      set_target_properties(freeimage_static PROPERTIES
         OUTPUT_NAME "freeimage_static"
      )
   else()
      set_target_properties(freeimage_static PROPERTIES
         OUTPUT_NAME "freeimage"
      )
   endif()

   install(TARGETS freeimage_static
      LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
   )

   install(FILES source/FreeImage.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif()
