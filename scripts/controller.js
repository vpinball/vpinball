// controller.js
//
// JavaScript port of Controller.vbs (version 1.2 by arngrim): a generic way to define a controller
// for EM and SS tables, with the DOF sound/toy helpers. ES module, import what the table needs:
//
//   import { LoadEM, SoundFX, DOF, DOFContactors, DOFFlippers } from "controller.js";
//
// SS tables import their ROM family module and load VPinMAME (or B2S.Server):
//
//   import { GameOnSolenoid, vpmKeyDown, vpmKeyUp } from "wpc.js";
//   LoadVPM({ gameName: "rom_name", vpmVersion: "01560000" });
//
// EM tables call LoadEM({ gameName: "fake_rom_name" }) in the table init.
//
// PlaySound(SoundFX("sound", DOFContactors), ...) plays the sound or "" depending on the DOF toy
// configuration, DOF(DOFevent, State) triggers a pure DOF call.

export const DOFContactors = 1;
export const DOFKnocker = 2;
export const DOFChimes = 3;
export const DOFBell = 4;
export const DOFGear = 5;
export const DOFShaker = 6;
export const DOFFlippers = 7;
export const DOFTargets = 8;
export const DOFDropTargets = 9;
export const DOFOff = 0;
export const DOFOn = 1;
export const DOFPulse = 2;

// Live bindings: importers see the values set by the Load functions
export let B2SController = null;
export let Controller = null;
export const DOFeffects = new Array(10).fill(0);
export let B2SOn = false;
export let B2SOnALT = false;
export let HasTimeFence = false;
export let IsPluginPinMAME = false;

export function LoadEM({ gameName = "" } = {}) {
   LoadController("EM", { gameName });
}

export function LoadPROC(options) {
   LoadController("PROC", options);
}

export function LoadVPM(options) {
   LoadController("VPM", options);
}

// Two controllers: VPM for the game and B2S.Server for the backglass/DOF (see DOFALT and SoundFXDOFALT)
export function LoadVPMALT(options) {
   LoadController("VPMALT", options);
}

function loadVPinMAME({ vpmVersion = "" }) {
   try {
      Controller = CreateObject("VPinMAME.Controller");
   } catch (e) {
      MsgBox("Can't load VPinMAME.\n" + e.message);
      return;
   }
   if (vpmVersion > "" && Controller.Version < vpmVersion)
      MsgBox("VPinMAME ver " + vpmVersion + " required.");
}

function tryCreateObject(classId) {
   try {
      return CreateObject(classId);
   } catch {
      return null;
   }
}

function controllerSetting(name) {
   try {
      return parseInt(Setting("Controller", name)) || 0;
   } catch {
      return 0;
   }
}

// Try to load B2S.Server and if not possible, load VPinMAME.Controller instead.
// ForceDisableB2S = 1 in the settings forces VPinMAME (or no controller for EM tables), so does
// disableB2S: true in the options. Also fills the array of toy categories that either play the
// sound or trigger the DOF effect.
export function LoadController(tableType, options = {}) {
   const { gameName = "", b2sGameName = gameName, disableB2S: forceDisableB2S = false } = options;
   B2SOn = false;
   B2SOnALT = false;

   let disableB2S = controllerSetting("ForceDisableB2S");
   DOFeffects[DOFContactors] = controllerSetting("DOFContactors");
   DOFeffects[DOFKnocker] = controllerSetting("DOFKnocker");
   DOFeffects[DOFChimes] = controllerSetting("DOFChimes");
   DOFeffects[DOFBell] = controllerSetting("DOFBell");
   DOFeffects[DOFGear] = controllerSetting("DOFGear");
   DOFeffects[DOFShaker] = controllerSetting("DOFShaker");
   DOFeffects[DOFFlippers] = controllerSetting("DOFFlippers");
   DOFeffects[DOFTargets] = controllerSetting("DOFTargets");
   DOFeffects[DOFDropTargets] = controllerSetting("DOFDropTargets");

   if (forceDisableB2S)
      disableB2S = 1;

   if (tableType === "PROC" || tableType === "VPMALT") {
      if (tableType === "PROC") {
         Controller = tryCreateObject("VPROC.Controller");
         if (Controller === null)
            MsgBox("Can't load PROC");
      } else {
         loadVPinMAME(options);
      }
      if (disableB2S === 0 && Controller !== null) {
         B2SController = tryCreateObject("B2S.Server");
         if (B2SController !== null) {
            B2SController.B2SName = b2sGameName;
            B2SController.TableName = ActiveTable.FileName;
            B2SController.Run();
            B2SOn = true;
            B2SOnALT = true;
         }
      }
   } else {
      if (disableB2S === 0) {
         Controller = tryCreateObject("B2S.Server");
         if (Controller === null) {
            if (tableType === "VPM")
               loadVPinMAME(options);
         } else {
            Controller.B2SName = gameName;
            Controller.TableName = ActiveTable.FileName;
            if (tableType === "EM")
               Controller.Run();
            B2SOn = true;
         }
      } else if (tableType === "VPM") {
         loadVPinMAME(options);
      }
   }

   // Check if the controller supports syncing by an external clock (latest B2S and PinMame do)
   HasTimeFence = false;
   IsPluginPinMAME = false;
   if (Controller !== null) {
      try {
         Controller.TimeFence = 0;
         HasTimeFence = true;
      } catch {
      }
      try {
         IsPluginPinMAME = Controller.IsPlugin;
      } catch {
      }
   }
}

// Returns the sound to play, or "" when the toy category is configured to replace the sound by a DOF effect
export function SoundFX(sound, effect) {
   if ((effect === 0 && B2SOn) || DOFeffects[effect] === 1)
      return "";
   return sound;
}

// EM tables: sound and DOF trigger combined, e.g. SoundFXDOF("flipperup", 101, DOFOn, DOFContactors)
export function SoundFXDOF(sound, dofEvent, state, effect) {
   if (DOFeffects[effect] === 1) {
      DOF(dofEvent, state);
      return "";
   }
   if (DOFeffects[effect] === 2)
      DOF(dofEvent, state);
   return sound;
}

// Same as SoundFXDOF, but communicates with B2SController instead of Controller
export function SoundFXDOFALT(sound, dofEvent, state, effect) {
   if (DOFeffects[effect] === 1) {
      DOFALT(dofEvent, state);
      return "";
   }
   if (DOFeffects[effect] === 2)
      DOFALT(dofEvent, state);
   return sound;
}

// Pure DOF call, e.g. DOF(123, DOFOn) where 123 refers to E123 in the DOF ini
export function DOF(dofEvent, state) {
   if (!B2SOn)
      return;
   if (state === DOFPulse) {
      Controller.B2SSetData(dofEvent, 1);
      Controller.B2SSetData(dofEvent, 0);
   } else {
      Controller.B2SSetData(dofEvent, state);
   }
}

// DOF call through B2SController when PROC or VPMALT is used
export function DOFALT(dofEvent, state) {
   if (!B2SOnALT)
      return;
   if (state === DOFPulse) {
      B2SController.B2SSetData(dofEvent, 1);
      B2SController.B2SSetData(dofEvent, 0);
   } else {
      B2SController.B2SSetData(dofEvent, state);
   }
}
