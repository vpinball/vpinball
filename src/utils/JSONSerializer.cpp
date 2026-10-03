// license:GPLv3+

#include "core/stdafx.h"

#include "utils/JSONSerializer.h"

#include <miniz/miniz.h>

#include <unordered_set>


///////////////////////////////////////////////////////////////////////////////////////
// Field maps: FID <-> json property name, per node kind

namespace
{

struct FieldDef
{
   int fid;
   const char* name;
};

struct FieldMap
{
   ankerl::unordered_dense::map<int, string> fidToName;
   ankerl::unordered_dense::map<string, int> nameToFid;
   std::vector<string> orderedNames; // Field names in declaration order, used to order the json output
   std::unordered_set<int> repeatable; // Fields made of a repeated list of scalar records
};

// Fields shared by all scene node types (ISelect persisted properties and fields with an identical meaning on every type using them)
static const std::initializer_list<FieldDef> g_sharedPartFields = {
   { FID(PIID), "id" },
   { FID(NAME), "name" },
   { FID(VCEN), "center" },
   { FID(TMON), "timer_enabled" },
   { FID(TMIN), "timer_interval" },
   { FID(LOCK), "ui_locked" },
   { FID(LVIS), "ui_visible" },
   { FID(LAYR), "legacy_layer" },
   { FID(LANR), "named_layer" },
   { FID(GRUP), "part_group" },
   { FID(PNTS), "dragpoints" }, // Legacy empty tag written before drag points (not used anymore but kept for compatibility)
   { FID(DPNT), "dragpoints" },
};

static FieldMap MakePartMap(std::initializer_list<FieldDef> fields, std::initializer_list<int> repeatable = {})
{
   FieldMap map;
   const auto add = [&map](const FieldDef& def)
   {
      map.fidToName[def.fid] = def.name;
      map.nameToFid[def.name] = def.fid;
      map.orderedNames.push_back(def.name);
   };
   for (const FieldDef& def : g_sharedPartFields)
      add(def);
   for (const FieldDef& def : fields)
      add(def);
   map.repeatable = repeatable;
   return map;
}

static FieldMap MakeMap(std::initializer_list<FieldDef> fields, std::initializer_list<int> repeatable = {})
{
   return MakePartMap(fields, repeatable); // Same thing, kept for readability of the table below
}

static const ankerl::unordered_dense::map<int, FieldMap>& GetFieldMaps()
{
   static const ankerl::unordered_dense::map<int, FieldMap> maps = []
   {
      ankerl::unordered_dense::map<int, FieldMap> m;

      m[eItemBall] = MakePartMap({
         { FID(BISC), "bulb_intensity_scale" },
         { FID(COLR), "color" },
         { FID(DCMD), "decal_mode" },
         { FID(DIMG), "image_decal" },
         { FID(FREF), "force_reflection" },
         { FID(IMAG), "image" },
         { FID(MASS), "mass" },
         { FID(PFRF), "playfield_reflection_strength" },
         { FID(RADI), "radius" },
         { FID(REEN), "reflection_enabled" },
         { FID(SPHR), "pinball_env_spherical_mapping" },
      });

      m[eItemBumper] = MakePartMap({
         { FID(BAMA), "base_material" },
         { FID(BSCT), "scatter" },
         { FID(BSVS), "base_visible" },
         { FID(BVIS), "all_visible" },
         { FID(CAVI), "cap_visible" },
         { FID(COLI), "collidable" },
         { FID(FORC), "force" },
         { FID(HAHE), "hit_event" },
         { FID(HISC), "height_scale" },
         { FID(MATR), "material" },
         { FID(ORIN), "orientation" },
         { FID(RADI), "radius" },
         { FID(RDLI), "ring_drop_offset" },
         { FID(REEN), "reflection_enabled" },
         { FID(RIMA), "ring_material" },
         { FID(RISP), "ring_speed" },
         { FID(RIVS), "ring_visible" },
         { FID(SKMA), "skirt_material" },
         { FID(SKVS), "skirt_visible" },
         { FID(SURF), "surface" },
         { FID(THRS), "threshold" },
      });

      m[eItemDecal] = MakePartMap({
         { FID(BGLS), "desktop_backdrop" },
         { FID(COLR), "color" },
         { FID(FONT), "font" },
         { FID(HIGH), "height" },
         { FID(IMAG), "image" },
         { FID(MATR), "material" },
         { FID(ROTA), "rotation" },
         { FID(SIZE), "sizing_type" },
         { FID(SURF), "surface" },
         { FID(TEXT), "text" },
         { FID(TYPE), "type" },
         { FID(VERT), "vertical_text" },
         { FID(WDTH), "width" },
      });

      m[eItemDispReel] = MakePartMap({
         { FID(CLRB), "back_color" },
         { FID(FONT), "font" },
         { FID(GIPR), "images_per_grid_row" },
         { FID(HIGH), "height" },
         { FID(IMAG), "image" },
         { FID(MSTP), "motor_steps" },
         { FID(RANG), "digit_range" },
         { FID(RCNT), "reel_count" },
         { FID(RSPC), "reel_spacing" },
         { FID(SOUN), "sound" },
         { FID(TRNS), "transparent" },
         { FID(UGRD), "use_image_grid" },
         { FID(UPTM), "update_interval" },
         { FID(VER1), "v1" },
         { FID(VER2), "v2" },
         { FID(VISI), "visible" },
         { FID(WDTH), "width" },
      });

      m[eItemFlasher] = MakePartMap({
         { FID(ADDB), "add_blend" },
         { FID(ALGN), "alignment" },
         { FID(BGLS), "desktop_backdrop" },
         { FID(COLR), "color" },
         { FID(DSPT), "ui_show_texture" },
         { FID(FALP), "alpha" },
         { FID(FHEI), "height" },
         { FID(FIAM), "filter_amount" },
         { FID(FILT), "filter_type" },
         { FID(FLAX), "flasher_x" },
         { FID(FLAY), "flasher_y" },
         { FID(FLDB), "depth_bias" },
         { FID(FROX), "rotation_x" },
         { FID(FROY), "rotation_y" },
         { FID(FROZ), "rotation_z" },
         { FID(FVIS), "visible" },
         { FID(GAMB), "glass_ambient" },
         { FID(GBOT), "glass_pad_bottom" },
         { FID(GLFT), "glass_pad_left" },
         { FID(GRGH), "glass_roughness" },
         { FID(GRHT), "glass_pad_right" },
         { FID(GTOP), "glass_pad_top" },
         { FID(IDMD), "dmd" },
         { FID(IMAB), "image_b" },
         { FID(IMAG), "image" },
         { FID(LINK), "image_src_link" },
         { FID(LMAP), "lightmap" },
         { FID(MOVA), "modulate_vs_add" },
         { FID(RDMD), "render_mode" },
         { FID(RSTL), "render_style" },
      });

      m[eItemFlipper] = MakePartMap({
         { FID(ANGE), "end_angle" },
         { FID(ANGS), "start_angle" },
         { FID(BASR), "base_radius" },
         { FID(ELAS), "elasticity" },
         { FID(ELFO), "elasticity_falloff" },
         { FID(ENBL), "enabled" },
         { FID(ENDR), "end_radius" },
         { FID(FHGT), "height" },
         { FID(FLPR), "flipper_radius_max" },
         { FID(FORC), "mass" },
         { FID(FRIC), "friction" },
         { FID(FRMN), "flipper_radius_min" },
         { FID(FRTN), "return" },
         { FID(IMAG), "image" },
         { FID(MATR), "material" },
         { FID(OVRP), "override_physics" },
         { FID(REEN), "reflection_enabled" },
         { FID(RHGF), "rubber_height" },
         { FID(RHGT), "rubber_height_legacy" },
         { FID(RPUP), "ramp_up" },
         { FID(RTHF), "rubber_thickness" },
         { FID(RTHK), "rubber_thickness_legacy" },
         { FID(RUMA), "rubber_material" },
         { FID(RWDF), "rubber_width" },
         { FID(RWDT), "rubber_width_legacy" },
         { FID(SCTR), "scatter" },
         { FID(STRG), "strength" },
         { FID(SURF), "surface" },
         { FID(TDAA), "torque_damping_angle" },
         { FID(TODA), "torque_damping" },
         { FID(VSBL), "visible" },
      });

      m[eItemGate] = MakePartMap({
         { FID(AFRC), "damping" },
         { FID(ELAS), "elasticity" },
         { FID(GAMA), "angle_max" },
         { FID(GAMI), "angle_min" },
         { FID(GATY), "type" },
         { FID(GCOL), "collidable" },
         { FID(GFRC), "friction" },
         { FID(GGFC), "gravity_factor" },
         { FID(GSUP), "show_bracket" },
         { FID(GVSB), "visible" },
         { FID(HGTH), "height" },
         { FID(LGTH), "length" },
         { FID(MATR), "material" },
         { FID(REEN), "reflection_enabled" },
         { FID(ROTA), "rotation" },
         { FID(SURF), "surface" },
         { FID(TWWA), "two_way" },
      });

      m[eItemHitTarget] = MakePartMap({
         { FID(CLDR), "collidable" },
         { FID(DILB), "disable_lighting_below" },
         { FID(DILI), "disable_lighting_legacy" },
         { FID(DILT), "disable_lighting_top" },
         { FID(DRSP), "drop_speed" },
         { FID(ELAS), "elasticity" },
         { FID(ELFO), "elasticity_falloff" },
         { FID(HTEV), "hitEvent" },
         { FID(IMAG), "image" },
         { FID(ISDR), "is_dropped" },
         { FID(LEMO), "legacy_mode" },
         { FID(MAPH), "physics_material" },
         { FID(MATR), "material" },
         { FID(OVPH), "overwrite_physics" },
         { FID(PIDB), "depth_bias" },
         { FID(RADE), "raise_delay" },
         { FID(REEN), "reflection_enabled" },
         { FID(RFCT), "friction" },
         { FID(ROTZ), "rot_z" },
         { FID(RSCT), "scatter" },
         { FID(THRS), "threshold" },
         { FID(TRTY), "target_type" },
         { FID(TVIS), "visible" },
         { FID(VPOS), "position" },
         { FID(VSIZ), "size" },
      });

      m[eItemKicker] = MakePartMap({
         { FID(EBLD), "enabled" },
         { FID(FATH), "fall_through" },
         { FID(KHAC), "hit_accuracy" },
         { FID(KHHI), "hit_height" },
         { FID(KORI), "orientation" },
         { FID(KSCT), "scatter" },
         { FID(LEMO), "legacy_mode" },
         { FID(MATR), "material" },
         { FID(RADI), "radius" },
         { FID(SURF), "surface" },
         { FID(TYPE), "type" },
      });

      m[eItemLight] = MakePartMap({
         { FID(BGLS), "desktop_backdrop" },
         { FID(BHHI), "bulb_halo_height" },
         { FID(BINT), "blink_interval" },
         { FID(BMSC), "mesh_radius" },
         { FID(BMVA), "modulate_vs_add" },
         { FID(BPAT), "blink_pattern" },
         { FID(BULT), "bulb_light" },
         { FID(BWTH), "intensity" },
         { FID(COL2), "color2" },
         { FID(COLR), "color" },
         { FID(FADE), "fader" },
         { FID(FAPO), "falloff_power" },
         { FID(FASD), "fade_speed_down" },
         { FID(FASP), "fade_speed_up" },
         { FID(HGHT), "height" },
         { FID(IMG1), "image" },
         { FID(IMMO), "image_mode" },
         { FID(LIDB), "depth_bias" },
         { FID(RADI), "radius" },
         { FID(SHAP), "shape" },
         { FID(SHBM), "show_bulb_mesh" },
         { FID(SHDW), "shadows" },
         { FID(SHRB), "show_reflection_on_ball" },
         { FID(STAT), "state_legacy" },
         { FID(STBM), "static_bulb_mesh" },
         { FID(STTF), "state" },
         { FID(SURF), "surface" },
         { FID(TRMS), "transmission_scale" },
         { FID(VSBL), "visible" },
      });

      m[eItemLightSeq] = MakePartMap({
         { FID(BGLS), "desktop_backdrop" },
         { FID(COLC), "collection" },
         { FID(CTRX), "center_x" },
         { FID(CTRY), "center_y" },
         { FID(UPTM), "update_interval" },
      });

      m[eItemPartGroup] = MakePartMap({
         { FID(PMSK), "player_mode_visibility_mask" },
         { FID(SPRF), "space_reference" },
      });

      m[eItemPlunger] = MakePartMap({
         { FID(ANFR), "anim_frames" },
         { FID(APLG), "auto_plunger" },
         { FID(HIGH), "height" },
         { FID(HPSL), "stroke" },
         { FID(IMAG), "image" },
         { FID(MATR), "material" },
         { FID(MECH), "mech_plunger" },
         { FID(MEST), "mech_strength" },
         { FID(MOMX), "momentum_transfer" },
         { FID(MPRK), "park_position" },
         { FID(PSCV), "scatter_velocity" },
         { FID(REEN), "reflection_enabled" },
         { FID(RNGD), "ring_diam" },
         { FID(RNGG), "ring_gap" },
         { FID(RNGW), "ring_width" },
         { FID(RODD), "rod_diam" },
         { FID(SPDF), "speed_fire" },
         { FID(SPDP), "speed_pull" },
         { FID(SPRD), "spring_diam" },
         { FID(SPRE), "spring_end_loops" },
         { FID(SPRG), "spring_gauge" },
         { FID(SPRL), "spring_loops" },
         { FID(SURF), "surface" },
         { FID(TIPS), "tip_shape" },
         { FID(TYPE), "type" },
         { FID(VSBL), "visible" },
         { FID(WDTH), "width" },
         { FID(ZADJ), "z_adjust" },
      });

      m[eItemPrimitive] = MakePartMap({
         { FID(ADDB), "add_blend" }, { FID(CLDR), "collidable" }, { FID(COLR), "color" }, { FID(CORF), "collision_reduction_factor" }, { FID(DILB), "disable_lighting_below" },
         { FID(DILI), "disable_lighting_legacy" }, { FID(DILT), "disable_lighting_top" }, { FID(DIPT), "display_texture" }, { FID(DTXI), "draw_textures_inside" },
         { FID(EBFC), "backfaces_enabled" }, { FID(EFUI), "ui_edge_factor" }, { FID(ELAS), "elasticity" }, { FID(ELFO), "elasticity_falloff" }, { FID(FALP), "alpha" },
         { FID(HTEV), "hitEvent" }, { FID(IMAG), "image" }, { FID(ISTO), "toy" }, { FID(LMAP), "lightmap" },
         { FID(M3DN), "mesh_file" }, // Original mesh file name (informational, the mesh is stored under meshes/)
         { FID(MAPH), "physics_material" }, { FID(MATR), "material" }, { FID(NRMA), "normal_map" }, { FID(OSNM), "object_space_normal_map" }, { FID(OVPH), "overwrite_physics" },
         { FID(PIDB), "depth_bias" }, { FID(REEN), "reflection_enabled" }, { FID(REFL), "reflection_probe" }, { FID(REFR), "refraction_probe" }, { FID(RFCT), "friction" },
         { FID(RSCT), "scatter" }, { FID(RSTR), "reflection_strength" }, { FID(RTHI), "refraction_thickness" }, { FID(RTV0), "prim_rot1_x" }, { FID(RTV1), "prim_rot1_y" },
         { FID(RTV2), "prim_rot1_z" }, { FID(RTV3), "prim_trans_x" }, { FID(RTV4), "prim_trans_y" }, { FID(RTV5), "prim_trans_z" }, { FID(RTV6), "prim_rot2_x" },
         { FID(RTV7), "prim_rot2_y" }, { FID(RTV8), "prim_rot2_z" }, { FID(SCOL), "side_color" }, { FID(SIDS), "sides" }, { FID(SIMG), "side_image" }, { FID(STRE), "static_rendering" },
         { FID(THRS), "threshold" }, { FID(TVIS), "visible" }, { FID(U3DM), "use_mesh" }, { FID(VPOS), "position" }, { FID(VSIZ), "size" }, { FID(ZMSK), "use_depth_mask" },
         // The mesh binary fields (M3VN/M3FN/M3CX/M3CY/M3CI/M3CJ/M3DX/M3DI/M3AX/M3AY) are
         // intentionally unmapped: the mesh is stored as a GLB file under meshes/
      });

      m[eItemRamp] = MakePartMap({
         { FID(ALGN), "alignment" },
         { FID(CLDR), "collidable" },
         { FID(ELAS), "elasticity" },
         { FID(HTBT), "height_bottom" },
         { FID(HTEV), "hitEvent" },
         { FID(HTTP), "height_top" },
         { FID(IMAG), "image" },
         { FID(IMGW), "image_walls" },
         { FID(MAPH), "physics_material" },
         { FID(MATR), "material" },
         { FID(OVPH), "overwrite_physics" },
         { FID(RADB), "depth_bias" },
         { FID(RADI), "radius" },
         { FID(RADX), "wire_distance_x" },
         { FID(RADY), "wire_distance_y" },
         { FID(REEN), "reflection_enabled" },
         { FID(RFCT), "friction" },
         { FID(RSCT), "scatter" },
         { FID(RVIS), "visible" },
         { FID(THRS), "threshold" },
         { FID(TYPE), "type" },
         { FID(WDBT), "width_bottom" },
         { FID(WDTP), "width_top" },
         { FID(WLHL), "left_wall_height" },
         { FID(WLHR), "right_wall_height" },
         { FID(WVHL), "left_wall_height_visible" },
         { FID(WVHR), "right_wall_height_visible" },
      });

      m[eItemRubber] = MakePartMap({
         { FID(CLDR), "collidable" },
         { FID(ELAS), "elasticity" },
         { FID(ELFO), "elasticity_falloff" },
         { FID(ESIE), "show_in_editor" },
         { FID(ESTR), "static_rendering" },
         { FID(HTEV), "hitEvent" },
         { FID(HTHI), "hit_height" },
         { FID(HTTP), "height_top" },
         { FID(IMAG), "image" },
         { FID(MAPH), "physics_material" },
         { FID(MATR), "material" },
         { FID(OVPH), "overwrite_physics" },
         { FID(REEN), "reflection_enabled" },
         { FID(RFCT), "friction" },
         { FID(ROTX), "rot_x" },
         { FID(ROTY), "rot_y" },
         { FID(ROTZ), "rot_z" },
         { FID(RSCT), "scatter" },
         { FID(RVIS), "visible" },
         { FID(WDTP), "thickness" },
      });

      m[eItemSpinner] = MakePartMap({
         { FID(AFRC), "damping" },
         { FID(HIGH), "height" },
         { FID(IMGF), "image" },
         { FID(LGTH), "length" },
         { FID(MATR), "material" },
         { FID(REEN), "reflection_enabled" },
         { FID(ROTA), "rotation" },
         { FID(SELA), "elasticity" },
         { FID(SMAX), "angle_max" },
         { FID(SMIN), "angle_min" },
         { FID(SSUP), "show_bracket" },
         { FID(SURF), "surface" },
         { FID(SVIS), "visible" },
      });

      m[eItemSurface] = MakePartMap({
         { FID(CLDW), "collidable" },
         { FID(DILB), "disable_lighting_below" },
         { FID(DILI), "disable_lighting_legacy" },
         { FID(DILT), "disable_lighting_top" },
         { FID(DROP), "droppable" },
         { FID(DSPT), "ui_show_texture" },
         { FID(ELAS), "elasticity" },
         { FID(ELFO), "elasticity_falloff" },
         { FID(FLIP), "flipbook" },
         { FID(HTBT), "height_bottom" },
         { FID(HTEV), "hitEvent" },
         { FID(HTTP), "height_top" },
         { FID(IMAG), "image" },
         { FID(INNR), "inner" },
         { FID(ISBS), "is_bottom_solid" },
         { FID(MAPH), "physics_material" },
         { FID(OVPH), "overwrite_physics" },
         { FID(REEN), "reflection_enabled" },
         { FID(SIMA), "side_material" },
         { FID(SIMG), "side_image" },
         { FID(SLGA), "slingshot_animation" },
         { FID(SLGF), "slingshot_force" },
         { FID(SLMA), "sling_shot_material" },
         { FID(SLTH), "slingshot_threshold" },
         { FID(SVBL), "side_visible" },
         { FID(THRS), "threshold" },
         { FID(TOMA), "top_material" },
         { FID(VSBL), "visible" },
         { FID(WFCT), "friction" },
         { FID(WSCT), "scatter" },
      });

      m[eItemTextbox] = MakePartMap({
         { FID(ALGN), "alignment" },
         { FID(CLRB), "back_color" },
         { FID(CLRF), "font_color" },
         { FID(FONT), "font" },
         { FID(IDMD), "dmd" },
         { FID(INSC), "intensity_scale" },
         { FID(TEXT), "text" },
         { FID(TRNS), "transparent" },
         { FID(VER1), "v1" },
         { FID(VER2), "v2" },
      });

      m[eItemTimer] = MakePartMap({
         { FID(BGLS), "desktop_backdrop" },
      });

      m[eItemTrigger] = MakePartMap({
         { FID(ANSP), "anim_speed" },
         { FID(EBLD), "enabled" },
         { FID(MATR), "material" },
         { FID(RADI), "radius" },
         { FID(REEN), "reflection_enabled" },
         { FID(ROTA), "rotation" },
         { FID(SCAX), "scale_x" },
         { FID(SCAY), "scale_y" },
         { FID(SHAP), "shape" },
         { FID(SURF), "surface" },
         { FID(THOT), "hit_height" },
         { FID(VSBL), "visible" },
         { FID(WITI), "wire_thickness" },
      });

      m[eItemCollection] = MakeMap(
         {
            { FID(NAME), "name" },
            { FID(ITEM), "parts" },
            { FID(EVNT), "fire_events" },
            { FID(SSNG), "stop_single_events" },
            { FID(GREL), "group_elements" },
         },
         { FID(ITEM) });

      m[eItemTable] = MakeMap({
         { FID(LEFT), "left" }, { FID(TOPX), "top" }, { FID(RGHT), "right" }, { FID(BOTM), "bottom" }, { FID(EFSS), "is_fullsinglescreen_view_enabled" }, { FID(VSM0), "desktop_view.mode" },
         { FID(ROTA), "desktop_view.rotation" }, { FID(INCL), "desktop_view.inclination" }, { FID(LAYB), "desktop_view.layback" }, { FID(FOVX), "desktop_view.fov" },
         { FID(XLTX), "desktop_view.view_x" }, { FID(XLTY), "desktop_view.view_y" }, { FID(XLTZ), "desktop_view.view_z" }, { FID(SCLX), "desktop_view.scale_x" },
         { FID(SCLY), "desktop_view.scale_y" }, { FID(SCLZ), "desktop_view.scale_z" }, { FID(HOF0), "desktop_view.horizontal_ofs" }, { FID(VOF0), "desktop_view.vertical_ofs" },
         { FID(WTZ0), "desktop_view.window_top_z_ofs" }, { FID(WBZ0), "desktop_view.window_bot_z_ofs" }, { FID(VSM1), "cabinet_view.mode" }, { FID(ROTF), "cabinet_view.rotation" },
         { FID(INCF), "cabinet_view.inclination" }, { FID(LAYF), "cabinet_view.layback" }, { FID(FOVF), "cabinet_view.fov" }, { FID(XLFX), "cabinet_view.view_x" },
         { FID(XLFY), "cabinet_view.view_y" }, { FID(XLFZ), "cabinet_view.view_z" }, { FID(SCFX), "cabinet_view.scale_x" }, { FID(SCFY), "cabinet_view.scale_y" },
         { FID(SCFZ), "cabinet_view.scale_z" }, { FID(HOF1), "cabinet_view.horizontal_ofs" }, { FID(VOF1), "cabinet_view.vertical_ofs" }, { FID(WTZ1), "cabinet_view.window_top_z_ofs" },
         { FID(WBZ1), "cabinet_view.window_bot_z_ofs" }, { FID(VSM2), "fullsinglescreen_view.mode" }, { FID(ROFS), "fullsinglescreen_view.rotation" },
         { FID(INFS), "fullsinglescreen_view.inclination" }, { FID(LAFS), "fullsinglescreen_view.layback" }, { FID(FOFS), "fullsinglescreen_view.fov" },
         { FID(XLXS), "fullsinglescreen_view.view_x" }, { FID(XLYS), "fullsinglescreen_view.view_y" }, { FID(XLZS), "fullsinglescreen_view.view_z" },
         { FID(SCXS), "fullsinglescreen_view.scale_x" }, { FID(SCYS), "fullsinglescreen_view.scale_y" }, { FID(SCZS), "fullsinglescreen_view.scale_z" },
         { FID(HOF2), "fullsinglescreen_view.horizontal_ofs" }, { FID(VOF2), "fullsinglescreen_view.vertical_ofs" }, { FID(WTZ2), "fullsinglescreen_view.window_top_z_ofs" },
         { FID(WBZ2), "fullsinglescreen_view.window_bot_z_ofs" }, { FID(ORRP), "override_physics" }, { FID(ORPF), "override_physics_flipper" }, { FID(GAVT), "gravity" },
         { FID(FRCT), "friction" }, { FID(ELAS), "elasticity" }, { FID(ELFA), "elasticity_falloff" }, { FID(PFSC), "scatter" }, { FID(SCAT), "default_scatter" }, { FID(NDGT), "nudge_time" },
         { FID(PHML), "physics_max_loops" }, { FID(REEL), "render_EM_reels" }, { FID(DECL), "render_decals" }, { FID(OFFX), "win_editor_view_offset_x" },
         { FID(OFFY), "win_editor_view_offset_y" }, { FID(ZOOM), "win_editor_zoom" }, { FID(SLPX), "angle_tilt_max" }, { FID(SLOP), "angle_tilt_min" }, { FID(BIMG), "backdrop_image_0" },
         { FID(BIMF), "backdrop_image_1" }, { FID(BIMS), "backdrop_image_2" }, { FID(BIMN), "image_backdrop_night_day" }, { FID(IMCG), "image_color_grade" }, { FID(BLIM), "ball_image" },
         { FID(BLSM), "ball_spherical_mapping" }, { FID(BLIF), "ball_image_decal" }, { FID(EIMG), "env_image" }, { FID(IMAG), "image" }, // Playfield image
         { FID(NOTX), "notes_text" }, { FID(SSHT), "screenshot" }, { FID(FBCK), "win_editor_backdrop" }, { FID(GLAS), "glass_top_height" }, { FID(GLAB), "glass_bottom_height" },
         { FID(PLMA), "playfield_material" }, { FID(BCLR), "color_backdrop" }, { FID(TDFT), "difficulty" }, { FID(LZAM), "light_ambient" }, { FID(LZDI), "light_emission" },
         { FID(LZHI), "light_height" }, { FID(LZRA), "light_range" }, { FID(LIES), "light_emission_scale" }, { FID(ENES), "env_emission_scale" }, { FID(GLES), "global_emission_scale" },
         { FID(AOSC), "AO_scale" }, { FID(SSSC), "SSR_scale" }, { FID(CLBH), "ground_to_lockbar_height" }, { FID(SVOL), "table_sound_volume" }, { FID(MVOL), "table_music_volume" },
         { FID(PLST), "playfield_reflection_strength" }, { FID(BDMO), "ball_decal_mode" }, { FID(BPRS), "ball_playfield_reflection_strength" },
         { FID(DBIS), "default_bulb_intensity_scale_on_ball" }, { FID(GDAC), "ui_editor_grid" }, { FID(UAOC), "enable_AO" }, { FID(USSR), "enable_SSR" }, { FID(TMAP), "tonemapper" },
         { FID(EXPO), "exposure" }, { FID(BLST), "bloom_strength" }, { FID(MATR), "materials" }, { FID(RPRB), "renderprobes" }, { FID(NAME), "name" }, { FID(CODE), "vbs_script" },
         { FID(CCUS), "ui_custom_colors" }, { FID(TLCK), "tablelocked" },
         // MASI, SEDT, SSND, SIMG, SFNT, SCOL (stream pre-allocation counts), MATE and PHMA
         // (legacy pre-10.8 material blobs) are intentionally unmapped and skipped
      });

      m[JSONSerializer::kMaterialNode] = MakeMap({
         { FID(TYPE), "type" },
         { FID(NAME), "name" },
         { FID(WLIG), "wrap_lighting" },
         { FID(ROUG), "roughness" },
         { FID(GIML), "glossy_image_lerp" },
         { FID(THCK), "thickness" },
         { FID(EDGE), "edge" },
         { FID(EALP), "edge_alpha" },
         { FID(OPAC), "opacity" },
         { FID(BASE), "base_color" },
         { FID(GLOS), "glossy_color" },
         { FID(COAT), "clearcoat_color" },
         { FID(RTNT), "refraction_tint" },
         { FID(EOPA), "is_opacity_active" },
         { FID(ELAS), "elasticity" },
         { FID(ELFO), "elasticity_falloff" },
         { FID(FRIC), "friction" },
         { FID(SCAT), "scatter_angle" },
      });

      m[JSONSerializer::kRenderProbeNode] = MakeMap({
         { FID(TYPE), "type" },
         { FID(NAME), "name" },
         { FID(RBAS), "roughness" },
         { FID(RPLA), "reflection_plane" },
         { FID(RMOD), "reflection_mode" },
         { FID(RLMP), "disable_light_reflection" },
      });

      m[JSONSerializer::kDragPointNode] = MakeMap({
         { FID(VCEN), "center" },
         { FID(POSZ), "position_z" },
         { FID(SMTH), "smooth" },
         { FID(SLNG), "slingshot" },
         { FID(ATEX), "auto_texture" },
         { FID(TEXC), "texture_coordinate" },
         { FID(LOCK), "ui_locked" },
         { FID(LVIS), "ui_visible" },
         { FID(LAYR), "legacy_layer" },
         { FID(LANR), "named_layer" },
      });

      m[JSONSerializer::kPinBinaryNode] = MakeMap({
         { FID(NAME), "name" }, { FID(PATH), "path" }, { FID(SIZE), "size" }, { FID(DATA), "data" }, // Pack-relative path of the binary file holding the data
      });

      m[JSONSerializer::kTextureNode] = MakeMap({
         { FID(NAME), "name" }, { FID(PATH), "path" }, { FID(WDTH), "width" }, { FID(HGHT), "height" },
         { FID(ALTV), "alpha_test" }, // Alpha test value on a 0..255 scale, negative when disabled
         { FID(MD5H), "md5" }, { FID(OPAQ), "opaque" }, { FID(JPEG), "image" }, // Sub object holding the binary image data (name is historical)
         { FID(LINK), "link" }, { FID(BITS), "bits" }, // Legacy raw bitmap, pre 10.8.1
         { FID(SIGN), "signed" }, // Legacy, unused
      });

      return m;
   }();
   return maps;
}

static const FieldMap& GetFieldMap(const int nodeKind)
{
   const auto& maps = GetFieldMaps();
   const auto it = maps.find(nodeKind);
   if (it == maps.end())
   {
      static const FieldMap empty;
      return empty;
   }
   return it->second;
}

} // anonymous namespace


// Returns the json slot of a field, creating intermediate objects for dotted names
// which group related fields into a sub object (e.g. "desktop_view.rotation")
static nlohmann::ordered_json& FieldSlot(nlohmann::ordered_json& node, const char* name)
{
   nlohmann::ordered_json* slot = &node;
   while (const char* const dot = strchr(name, '.'))
   {
      slot = &(*slot)[string(name, dot - name)];
      name = dot + 1;
   }
   return (*slot)[name];
}

// Assigns a value to a field of a JSON node, collecting repeatable fields in an array
template <typename T> static void AssignField(nlohmann::ordered_json* node, const int nodeKind, const int fieldId, const char* name, T&& value)
{
   auto& field = FieldSlot(*node, name);
   if (JSONSerializer::IsRepeatableField(nodeKind, fieldId))
   {
      if (!field.is_array())
         field = nlohmann::ordered_json::array();
      field.push_back(std::forward<T>(value));
   }
   else
      field = std::forward<T>(value);
}

// Reorders the fields of a json object (and recursively of its sub objects) following the field
// map declaration order, so that the output does not depend on the order in which the writer was
// called and stays stable across code changes (packs are meant to be versioned in git). Fields
// unknown to the map keep their relative order and are placed after the mapped ones.
static void SortNodeFields(nlohmann::ordered_json& node, const int nodeKind, const string& prefix)
{
   if (!node.is_object())
      return;
   const FieldMap& map = GetFieldMap(nodeKind);
   // Embedded objects are sorted with their own map, grouping objects (dotted field names like
   // "desktop_view.rotation") keep the parent map with the extended path prefix
   for (auto& [key, value] : node.items())
   {
      const string path = prefix.empty() ? key : prefix + '.' + key;
      const int subKind = JSONSerializer::GetSubObjectKind(nodeKind, JSONSerializer::GetFieldId(nodeKind, path));
      const string subPrefix = (subKind == nodeKind) ? path : ""s;
      if (value.is_object())
         SortNodeFields(value, subKind, subPrefix);
      else if (value.is_array())
         for (nlohmann::ordered_json& element : value)
            if (element.is_object())
               SortNodeFields(element, subKind, subPrefix);
   }
   if (map.orderedNames.empty())
      return;
   const auto rank = [&map, &prefix](const string& key)
   {
      const string path = prefix.empty() ? key : prefix + '.' + key;
      const string dotted = path + '.';
      for (size_t i = 0; i < map.orderedNames.size(); ++i)
         if (map.orderedNames[i] == path || map.orderedNames[i].starts_with(dotted))
            return i;
      return map.orderedNames.size();
   };
   vector<string> keys;
   keys.reserve(node.size());
   for (const auto& [key, value] : node.items())
      keys.push_back(key);
   std::ranges::stable_sort(keys, [&](const string& a, const string& b) { return rank(a) < rank(b); });
   nlohmann::ordered_json sorted = nlohmann::ordered_json::object();
   // The document type marker always comes first (it is not part of the field maps)
   const auto typeIt = std::ranges::find(keys, "$type"s);
   if (typeIt != keys.end())
   {
      sorted["$type"] = std::move(node["$type"]);
      keys.erase(typeIt);
   }
   for (const string& key : keys)
      sorted[key] = std::move(node[key]);
   node = std::move(sorted);
}

const char* JSONSerializer::GetFieldName(const int nodeKind, const int fieldId)
{
   const auto& map = GetFieldMap(nodeKind).fidToName;
   const auto it = map.find(fieldId);
   return (it == map.end()) ? nullptr : it->second.c_str();
}

int JSONSerializer::GetFieldId(const int nodeKind, const string& fieldName)
{
   const auto& map = GetFieldMap(nodeKind).nameToFid;
   const auto it = map.find(fieldName);
   return (it == map.end()) ? 0 : it->second;
}

int JSONSerializer::GetSubObjectKind(const int parentKind, const int fieldId)
{
   switch (fieldId)
   {
   case FID(DPNT): return kDragPointNode;
   case FID(MATR): return (parentKind == eItemTable) ? kMaterialNode : parentKind;
   case FID(RPRB): return kRenderProbeNode;
   case FID(JPEG): return kPinBinaryNode;
   default: return parentKind;
   }
}

bool JSONSerializer::IsRepeatableField(const int nodeKind, const int fieldId) { return GetFieldMap(nodeKind).repeatable.contains(fieldId); }

string JSONSerializer::SanitizeFileName(const string& name)
{
   string result;
   result.reserve(name.size());
   for (const char c : name)
   {
      const bool invalid = (c < 0x20) || (c == '/') || (c == '\\') || (c == ':') || (c == '*') || (c == '?') || (c == '"') || (c == '<') || (c == '>') || (c == '|');
      result += invalid ? '_' : c;
   }
   while (!result.empty() && (result.back() == '.' || result.back() == ' '))
      result.pop_back();
   if (result.empty() || result == "."sv || result == ".."sv)
      result = "_"s;
   return result;
}

const char* JSONSerializer::GetPartTypeName(const int itemType)
{
   switch (static_cast<ItemTypeEnum>(itemType))
   {
   case eItemSurface: return "surface";
   case eItemFlipper: return "flipper";
   case eItemTimer: return "timer";
   case eItemPlunger: return "plunger";
   case eItemTextbox: return "textbox";
   case eItemBumper: return "bumper";
   case eItemTrigger: return "trigger";
   case eItemLight: return "light";
   case eItemKicker: return "kicker";
   case eItemDecal: return "decal";
   case eItemGate: return "gate";
   case eItemSpinner: return "spinner";
   case eItemRamp: return "ramp";
   case eItemTable: return "table";
   case eItemCollection: return "collection";
   case eItemDispReel: return "dispreel";
   case eItemLightSeq: return "lightseq";
   case eItemPrimitive: return "primitive";
   case eItemFlasher: return "flasher";
   case eItemRubber: return "rubber";
   case eItemHitTarget: return "hittarget";
   case eItemBall: return "ball";
   case eItemPartGroup: return "partgroup";
   default: return nullptr;
   }
}

int JSONSerializer::GetPartTypeFromName(const string& name)
{
   for (int type = eItemSurface; type <= eItemPartGroup; type++)
      if (const char* typeName = GetPartTypeName(type); typeName && name == typeName)
         return type;
   return -1;
}


///////////////////////////////////////////////////////////////////////////////////////
// Pack serializers (folder & zip write side)

namespace
{

class FilesystemSerializer final : public JSONSerializer::Serializer
{
public:
   explicit FilesystemSerializer(const std::filesystem::path& basePath)
      : m_basePath(basePath)
   {
   }

   void AddTextFile(const std::filesystem::path& path, const std::string& content) override
   {
      const std::filesystem::path fullPath = m_basePath / path;
      std::error_code ec;
      std::filesystem::create_directories(fullPath.parent_path(), ec);
      std::ofstream out(fullPath, std::ios::binary | std::ios::trunc);
      if (out)
         out << content;
      else
      {
         PLOGE << "Failed to write pack file: " << PathToUTF8(fullPath);
         m_hasError = true;
      }
   }

   void AddBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data) override
   {
      const std::filesystem::path fullPath = m_basePath / path;
      std::error_code ec;
      std::filesystem::create_directories(fullPath.parent_path(), ec);
      std::ofstream out(fullPath, std::ios::binary | std::ios::trunc);
      if (out)
         out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
      else
      {
         PLOGE << "Failed to write pack file: " << PathToUTF8(fullPath);
         m_hasError = true;
      }
   }

   bool HasError() const override { return m_hasError; }

private:
   const std::filesystem::path m_basePath;
   bool m_hasError = false;
};

class ZipSerializer final : public JSONSerializer::Serializer
{
public:
   explicit ZipSerializer(const std::filesystem::path& zipPath)
      : m_zipPath(zipPath)
   {
      memset(&m_zipArchive, 0, sizeof(m_zipArchive));
   }

   ~ZipSerializer() override { Finalize(); }

   bool Finalize() override
   {
      if (m_zipArchive.m_pState)
      {
         if (!mz_zip_writer_finalize_archive(&m_zipArchive))
            m_hasError = true;
         mz_zip_writer_end(&m_zipArchive);
      }
      return !m_hasError;
   }

   void AddTextFile(const std::filesystem::path& path, const std::string& content) override { AddMem(path, content.data(), content.size()); }

   void AddBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data) override { AddMem(path, data.data(), data.size()); }

private:
   void AddMem(const std::filesystem::path& path, const void* data, const size_t size)
   {
      if (!m_zipArchive.m_pState && !mz_zip_writer_init_file(&m_zipArchive, PathToUTF8(m_zipPath).c_str(), 0))
      {
         PLOGE << "Failed to initialize zip writer for: " << PathToUTF8(m_zipPath);
         m_hasError = true;
         return;
      }
      // Zip entries use '/' separators and UTF-8 names
      const string entryName = MakeString(path.generic_wstring());
      if (!mz_zip_writer_add_mem(&m_zipArchive, entryName.c_str(), data, size, MZ_DEFAULT_COMPRESSION))
      {
         PLOGE << "Failed to add file to zip pack: " << entryName;
         m_hasError = true;
      }
   }

   bool HasError() const override { return m_hasError; }

   const std::filesystem::path m_zipPath;
   mz_zip_archive m_zipArchive;
   bool m_hasError = false;
};

class FolderDeserializer final : public JSONSerializer::Deserializer
{
public:
   explicit FolderDeserializer(const std::filesystem::path& basePath)
      : m_basePath(basePath)
   {
   }

   bool Exists(const std::filesystem::path& path) const override
   {
      std::error_code ec;
      return std::filesystem::is_regular_file(m_basePath / path, ec);
   }

   std::vector<std::filesystem::path> ListFiles() const override
   {
      std::vector<std::filesystem::path> files;
      std::error_code ec;
      for (const auto& entry : std::filesystem::recursive_directory_iterator(m_basePath, ec))
         if (entry.is_regular_file())
            files.push_back(entry.path().lexically_relative(m_basePath));
      std::ranges::sort(files);
      return files;
   }

   bool ReadBinaryFile(const std::filesystem::path& path, std::vector<uint8_t>& data) const override
   {
      std::ifstream in(m_basePath / path, std::ios::binary | std::ios::ate);
      if (!in)
         return false;
      const auto size = in.tellg();
      in.seekg(0, std::ios::beg);
      data.resize(static_cast<size_t>(size));
      return static_cast<bool>(in.read(reinterpret_cast<char*>(data.data()), size));
   }

private:
   const std::filesystem::path m_basePath;
};

class ZipDeserializer final : public JSONSerializer::Deserializer
{
public:
   explicit ZipDeserializer(const std::filesystem::path& zipPath)
   {
      memset(&m_zipArchive, 0, sizeof(m_zipArchive));
      if (!mz_zip_reader_init_file(&m_zipArchive, PathToUTF8(zipPath).c_str(), 0))
      {
         PLOGE << "Failed to open zip pack: " << PathToUTF8(zipPath);
         return;
      }
      const mz_uint count = mz_zip_reader_get_num_files(&m_zipArchive);
      for (mz_uint i = 0; i < count; i++)
      {
         mz_zip_archive_file_stat stat;
         if (!mz_zip_reader_file_stat(&m_zipArchive, i, &stat) || mz_zip_reader_is_file_a_directory(&m_zipArchive, i))
            continue;
         size_t size = 0;
         if (void* const data = mz_zip_reader_extract_to_heap(&m_zipArchive, i, &size, 0); data != nullptr)
         {
            auto& content = m_files[PathFromUTF8(stat.m_filename)];
            content.resize(size);
            memcpy(content.data(), data, size);
            mz_free(data);
         }
      }
      mz_zip_reader_end(&m_zipArchive);
   }

   bool Exists(const std::filesystem::path& path) const override { return m_files.contains(path); }

   std::vector<std::filesystem::path> ListFiles() const override
   {
      std::vector<std::filesystem::path> files;
      files.reserve(m_files.size());
      for (const auto& [name, content] : m_files)
         files.push_back(name);
      std::ranges::sort(files);
      return files;
   }

   bool ReadBinaryFile(const std::filesystem::path& path, std::vector<uint8_t>& data) const override
   {
      const auto it = m_files.find(path);
      if (it == m_files.end())
         return false;
      data = it->second;
      return true;
   }

private:
   mz_zip_archive m_zipArchive;
   std::map<std::filesystem::path, std::vector<uint8_t>> m_files;
};

} // anonymous namespace


std::unique_ptr<JSONSerializer::Serializer> JSONSerializer::CreateWriter(const std::filesystem::path& path)
{
   std::error_code ec;
   if (std::filesystem::is_directory(path, ec) || (!std::filesystem::exists(path, ec) && !path.has_extension()))
      return std::make_unique<FilesystemSerializer>(path);
   return std::make_unique<ZipSerializer>(path);
}

std::unique_ptr<JSONSerializer::Deserializer> JSONSerializer::CreateReader(const std::filesystem::path& path)
{
   std::error_code ec;
   if (std::filesystem::is_directory(path, ec))
      return std::make_unique<FolderDeserializer>(path);
   return std::make_unique<ZipDeserializer>(path);
}

bool JSONSerializer::IsPack(const std::filesystem::path& path)
{
   std::error_code ec;
   if (std::filesystem::is_directory(path, ec))
      return true;
   return lowerCase(PathToUTF8(path.extension())) == ".vpz"s;
}


///////////////////////////////////////////////////////////////////////////////////////
// JSONObjectWriter

JSONObjectWriter::JSONObjectWriter(const int nodeKind, JSONSerializer::Serializer* pack)
   : m_nodeKind(nodeKind)
   , m_pack(pack)
{
   m_stack.push_back({ &m_root, nodeKind });
}

void JSONObjectWriter::BeginObject(const int objectId, const bool isArray, const bool isSkippable)
{
   const Scope& top = m_stack.back();
   const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, objectId) : nullptr;
   if (name == nullptr)
   {
      if (top.node)
         PLOGD << "JSON writer: skipped unmapped field " << std::hex << objectId << std::dec;
      m_stack.push_back({ nullptr, -1 });
      return;
   }
   nlohmann::ordered_json* sub;
   if (isArray)
   {
      auto& arr = FieldSlot(*top.node, name);
      if (!arr.is_array())
         arr = nlohmann::ordered_json::array();
      arr.push_back(nlohmann::ordered_json::object());
      sub = &arr.back();
   }
   else
   {
      FieldSlot(*top.node, name) = nlohmann::ordered_json::object();
      sub = &FieldSlot(*top.node, name);
   }
   m_stack.push_back({ sub, JSONSerializer::GetSubObjectKind(top.nodeKind, objectId) });
}

void JSONObjectWriter::WriteBool(const int fieldId, const bool value)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
      AssignField(top.node, top.nodeKind, fieldId, name, value);
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteInt(const int fieldId, const int value)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
      AssignField(top.node, top.nodeKind, fieldId, name, value);
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteUInt(const int fieldId, const unsigned int value)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
      AssignField(top.node, top.nodeKind, fieldId, name, value);
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteFloat(const int fieldId, const float value)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
      AssignField(top.node, top.nodeKind, fieldId, name, value);
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteString(const int fieldId, const string& value)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
      AssignField(top.node, top.nodeKind, fieldId, name, value);
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteWideString(const int fieldId, const wstring& value)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
      AssignField(top.node, top.nodeKind, fieldId, name, MakeString(value));
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteVector2(const int fieldId, const Vertex2D& vec)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
   {
      auto& obj = FieldSlot(*top.node, name);
      obj["x"] = vec.x;
      obj["y"] = vec.y;
   }
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteVector3(const int fieldId, const vec3& vec)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
   {
      auto& obj = FieldSlot(*top.node, name);
      obj["x"] = vec.x;
      obj["y"] = vec.y;
      obj["z"] = vec.z;
   }
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteVector4(const int fieldId, const vec4& vec)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
   {
      auto& obj = FieldSlot(*top.node, name);
      obj["x"] = vec.x;
      obj["y"] = vec.y;
      obj["z"] = vec.z;
      obj["w"] = vec.w;
   }
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteScript(const int fieldId, const string& value)
{
   const Scope& top = m_stack.back();
   const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr;
   if (name == nullptr)
   {
      if (top.node)
         PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
      return;
   }
   if (m_pack == nullptr)
   {
      // No pack: embed the script (should not happen when saving a pack)
      FieldSlot(*top.node, name) = value;
      return;
   }
   const char* const scriptPath = "script.vbs";
   m_pack->AddTextFile(scriptPath, value);
   FieldSlot(*top.node, name) = scriptPath;
}

void JSONObjectWriter::WriteFontDescriptor(const int fieldId, const FontDesc& value)
{
   const Scope& top = m_stack.back();
   if (const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr)
   {
      auto& obj = FieldSlot(*top.node, name);
      obj["name"] = value.name;
      obj["size"] = value.size;
      obj["weight"] = value.weight;
      obj["charset"] = value.charset;
      obj["italic"] = (value.attributes & 0x02) != 0;
      obj["underline"] = (value.attributes & 0x04) != 0;
      obj["strikethrough"] = (value.attributes & 0x08) != 0;
   }
   else if (top.node)
      PLOGD << "JSON writer: skipped unmapped field " << std::hex << fieldId << std::dec;
}

void JSONObjectWriter::WriteRaw(const int fieldId, const void* pvalue, const int size)
{
   const Scope& top = m_stack.back();
   const char* const name = top.node ? JSONSerializer::GetFieldName(top.nodeKind, fieldId) : nullptr;
   if (name == nullptr)
   {
      if (top.node)
         PLOGD << "JSON writer: skipped unmapped raw field " << std::hex << fieldId << std::dec;
      return;
   }
   auto& value = FieldSlot(*top.node, name);
   if (top.nodeKind == JSONSerializer::kRenderProbeNode && fieldId == FID(RPLA) && size == sizeof(vec4))
   {
      const vec4& vec = *static_cast<const vec4*>(pvalue);
      value["x"] = vec.x;
      value["y"] = vec.y;
      value["z"] = vec.z;
      value["w"] = vec.w;
   }
   else if (size % 4 == 0)
   {
      // Raw blocks are written as an array of 32 bit integers (little endian), e.g. ui_custom_colors
      value = nlohmann::ordered_json::array();
      const uint32_t* const data = static_cast<const uint32_t*>(pvalue);
      for (int i = 0; i < size / 4; i++)
         value.push_back(data[i]);
   }
   else
   {
      value = nlohmann::ordered_json::array();
      const uint8_t* const data = static_cast<const uint8_t*>(pvalue);
      for (int i = 0; i < size; i++)
         value.push_back(data[i]);
   }
}

void JSONObjectWriter::EndObject()
{
   if (m_stack.size() > 1)
      m_stack.pop_back();
   // The last EndObject closes the root object: nothing to do, the document is complete
}

const nlohmann::ordered_json& JSONObjectWriter::Json()
{
   SortNodeFields(m_root, m_nodeKind, ""s);
   return m_root;
}


///////////////////////////////////////////////////////////////////////////////////////
// JSONObjectReader

JSONObjectReader::JSONObjectReader(const nlohmann::ordered_json& node, const int nodeKind, const JSONSerializer::Deserializer* pack, const int fileVersion)
   : m_node(node)
   , m_nodeKind(nodeKind)
   , m_pack(pack)
   , m_fileVersion(fileVersion)
{
}

bool JSONObjectReader::AsBool()
{
   if (m_node.is_boolean())
      return m_node.get<bool>();
   if (m_node.is_number_integer())
      return m_node.get<int64_t>() != 0;
   m_hasError = true;
   return false;
}

int JSONObjectReader::AsInt()
{
   if (m_node.is_number_integer())
      return m_node.get<int>();
   if (m_node.is_number())
      return static_cast<int>(m_node.get<double>());
   if (m_node.is_boolean())
      return m_node.get<bool>() ? 1 : 0;
   m_hasError = true;
   return 0;
}

unsigned int JSONObjectReader::AsUInt()
{
   if (m_node.is_number_unsigned())
      return m_node.get<unsigned int>();
   if (m_node.is_number())
      return static_cast<unsigned int>(m_node.get<double>());
   m_hasError = true;
   return 0;
}

float JSONObjectReader::AsFloat()
{
   if (m_node.is_number())
      return static_cast<float>(m_node.get<double>());
   if (m_node.is_boolean())
      return m_node.get<bool>() ? 1.f : 0.f;
   m_hasError = true;
   return 0.f;
}

string JSONObjectReader::AsString()
{
   if (m_node.is_string())
      return m_node.get<string>();
   if (m_node.is_number_integer())
      return std::to_string(m_node.get<int64_t>());
   m_hasError = true;
   return {};
}

wstring JSONObjectReader::AsWideString()
{
   if (m_node.is_string())
      return MakeWString(m_node.get<string>());
   m_hasError = true;
   return {};
}

Vertex2D JSONObjectReader::AsVector2()
{
   Vertex2D vec;
   if (m_node.is_object())
   {
      vec.x = m_node.value("x", 0.f);
      vec.y = m_node.value("y", 0.f);
   }
   else
      m_hasError = true;
   return vec;
}

vec3 JSONObjectReader::AsVector3()
{
   vec3 vec;
   if (m_node.is_object())
   {
      vec.x = m_node.value("x", 0.f);
      vec.y = m_node.value("y", 0.f);
      vec.z = m_node.value("z", 0.f);
   }
   else
      m_hasError = true;
   return vec;
}

vec4 JSONObjectReader::AsVector4()
{
   vec4 vec;
   if (m_node.is_object())
   {
      vec.x = m_node.value("x", 0.f);
      vec.y = m_node.value("y", 0.f);
      vec.z = m_node.value("z", 0.f);
      vec.w = m_node.value("w", 0.f);
   }
   else
      m_hasError = true;
   return vec;
}

string JSONObjectReader::AsScript(const bool isScriptProtected)
{
   if (!m_node.is_string())
   {
      m_hasError = true;
      return {};
   }
   const string value = m_node.get<string>();
   // The script is stored as a separate file of the pack, referenced by its path
   if (m_pack != nullptr && m_pack->Exists(value))
   {
      string script;
      if (m_pack->ReadTextFile(value, script))
         return script;
      m_hasError = true;
      return {};
   }
   return value; // Fallback: inline script
}

FontDesc JSONObjectReader::AsFontDescriptor()
{
   FontDesc fd;
   if (!m_node.is_object())
   {
      m_hasError = true;
      return fd;
   }
   fd.name = m_node.value("name", ""s);
   fd.size = m_node.value("size", 0u);
   fd.weight = m_node.value("weight", static_cast<uint16_t>(0));
   fd.charset = m_node.value("charset", static_cast<uint16_t>(0));
   fd.attributes = (m_node.value("italic", false) ? 0x02 : 0x00) | (m_node.value("underline", false) ? 0x04 : 0x00) | (m_node.value("strikethrough", false) ? 0x08 : 0x00);
   return fd;
}

void JSONObjectReader::AsRaw(void* pvalue, const int size)
{
   if (m_node.is_string() && m_pack != nullptr)
   {
      // Binary data stored as a file of the pack
      std::vector<uint8_t> data;
      if (!m_pack->ReadBinaryFile(m_node.get<string>(), data) || static_cast<int>(data.size()) != size)
      {
         PLOGE << "JSON reader: missing or size mismatched binary file '" << m_node.get<string>() << '\'';
         m_hasError = true;
         return;
      }
      memcpy(pvalue, data.data(), size);
      return;
   }
   if (m_node.is_object() && size == sizeof(vec4) && m_node.contains("x"))
   {
      // vec4 serialized as an object (e.g. render probe reflection plane)
      const vec4 vec(m_node.value("x", 0.f), m_node.value("y", 0.f), m_node.value("z", 0.f), m_node.value("w", 0.f));
      memcpy(pvalue, &vec, sizeof(vec4));
      return;
   }
   if (m_node.is_array())
   {
      if (size == static_cast<int>(m_node.size() * 4))
      {
         uint32_t* const data = static_cast<uint32_t*>(pvalue);
         for (size_t i = 0; i < m_node.size(); i++)
            data[i] = m_node[i].get<uint32_t>();
         return;
      }
      if (size == static_cast<int>(m_node.size()))
      {
         uint8_t* const data = static_cast<uint8_t*>(pvalue);
         for (size_t i = 0; i < m_node.size(); i++)
            data[i] = m_node[i].get<uint8_t>();
         return;
      }
   }
   m_hasError = true;
}

void JSONObjectReader::AsObject(const std::function<bool(const int fieldTag, IObjectReader& fieldReader)>& processField, const bool isSkippable)
{
   if (!m_node.is_object())
   {
      m_hasError = true;
      return;
   }
   const std::function<bool(const nlohmann::ordered_json& node, const string& prefix)> dispatchFields
      = [this, &processField, &dispatchFields](const nlohmann::ordered_json& node, const string& prefix) -> bool
   {
      for (const auto& [key, value] : node.items())
      {
         const string path = prefix.empty() ? key : prefix + '.' + key;
         const int fid = JSONSerializer::GetFieldId(m_nodeKind, path);
         if (fid == 0)
         {
            // Unmapped object: it may be a grouping object holding mapped sub fields
            // (e.g. "desktop_view.rotation"), otherwise it is skipped (forward compatibility)
            if (value.is_object() && !dispatchFields(value, path))
               return false;
            continue;
         }
         const int subKind = JSONSerializer::GetSubObjectKind(m_nodeKind, fid);
         if (value.is_array() && (JSONSerializer::IsRepeatableField(m_nodeKind, fid) || (!value.empty() && value[0].is_object())))
         {
            // Repeated records: one callback per element
            for (const auto& element : value)
            {
               JSONObjectReader elementReader(element, subKind, m_pack, m_fileVersion);
               if (!processField(fid, elementReader))
                  return false;
            }
         }
         else
         {
            JSONObjectReader fieldReader(value, subKind, m_pack, m_fileVersion);
            if (!processField(fid, fieldReader))
               return false;
         }
      }
      return true;
   };
   dispatchFields(m_node, ""s);
}
