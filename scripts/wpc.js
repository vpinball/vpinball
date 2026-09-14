// wpc.js: port of WPC.vbs, the Williams WPC system data and keyboard handling.
//
//   import { GameOnSolenoid, vpmKeyDown, vpmKeyUp } from "wpc.js";
//   vpmInit(Table1, { useSolenoids: 2, gameOnSolenoid: GameOnSolenoid });
//
// vpmKeyDown/vpmKeyUp return true when the key was handled, call them from the table KeyDown/KeyUp handlers.

import { Controller } from "./controller.js";
import { vpmTimer, vpmNudge, vpmFlips, vpmShowOptions, vpmShowHelp, vpmVol, vpmAddBall } from "./core.js";
import * as keys from "./vpmkeys.js";

// Flipper Solenoid (GameOn)
export const GameOnSolenoid = 31;

// Cabinet switches
export const swCoin1 = 1;
export const swCoin2 = 2;
export const swCoin3 = 3;
export const swCoin4 = 4;
export const swCancel = 5;
export const swDown = 6;
export const swUp = 7;
export const swEnter = 8;
export const swStartButton = 13;
export const swSlamTilt = 21;
export const swCoinDoor = 22;
export const swLRFlip = 112;
export const swLLFlip = 114;
export const swURFlip = 116;
export const swULFlip = 118;

const setSwitch = (no, state) => Controller.$set("Switch", no, !!state);
const insertCoin = (sw) => {
   vpmTimer.addTimer(750, () => vpmTimer.pulseSw(sw));
   if (typeof globalThis.SCoin !== "undefined")
      PlaySound(globalThis.SCoin);
};

export function vpmKeyDown(keycode) {
   switch (keycode) {
   case LeftFlipperKey:
      setSwitch(swLLFlip, true);
      vpmFlips.FlipL(true);
      if (keycode === StagedLeftFlipperKey) {
         vpmFlips.FlipUL(true);
         if (vpmFlips.FlipperSolNumber[2] !== 0)
            setSwitch(swULFlip, true);
      }
      return false;
   case RightFlipperKey:
      setSwitch(swLRFlip, true);
      vpmFlips.FlipR(true);
      if (keycode === StagedRightFlipperKey) {
         vpmFlips.FlipUR(true);
         if (vpmFlips.FlipperSolNumber[3] !== 0)
            setSwitch(swURFlip, true);
      }
      return false;
   case StagedLeftFlipperKey:
      vpmFlips.FlipUL(true);
      if (vpmFlips.FlipperSolNumber[2] !== 0)
         setSwitch(swULFlip, true);
      return true;
   case StagedRightFlipperKey:
      vpmFlips.FlipUR(true);
      if (vpmFlips.FlipperSolNumber[3] !== 0)
         setSwitch(swURFlip, true);
      return true;
   case keys.keyInsertCoin1: insertCoin(swCoin1); return true;
   case keys.keyInsertCoin2: insertCoin(swCoin2); return true;
   case keys.keyInsertCoin3: insertCoin(swCoin3); return true;
   case keys.keyInsertCoin4: insertCoin(swCoin4); return true;
   case StartGameKey: setSwitch(swStartButton, true); return true;
   case keys.keyCancel: setSwitch(swCancel, true); return true;
   case keys.keyDown: setSwitch(swDown, true); return true;
   case keys.keyUp: setSwitch(swUp, true); return true;
   case keys.keyEnter: setSwitch(swEnter, true); return true;
   case keys.keySlamDoorHit: setSwitch(swSlamTilt, true); return true;
   case keys.keyCoinDoor:
      if (keys.toggleKeyCoinDoor)
         setSwitch(swCoinDoor, !Controller.Switch(swCoinDoor));
      else
         setSwitch(swCoinDoor, !keys.inverseKeyCoinDoor);
      return true;
   case keys.keyBangBack: vpmNudge.DoMechTilt(); return true;
   case keys.keyVPMVolume: vpmVol(); return true;
   default:
      return false;
   }
}

export function vpmKeyUp(keycode) {
   switch (keycode) {
   case LeftFlipperKey:
      setSwitch(swLLFlip, false);
      vpmFlips.FlipL(false);
      if (keycode === StagedLeftFlipperKey) {
         vpmFlips.FlipUL(false);
         if (vpmFlips.FlipperSolNumber[2] !== 0)
            setSwitch(swULFlip, false);
      }
      return false;
   case RightFlipperKey:
      setSwitch(swLRFlip, false);
      vpmFlips.FlipR(false);
      if (keycode === StagedRightFlipperKey) {
         vpmFlips.FlipUR(false);
         if (vpmFlips.FlipperSolNumber[3] !== 0)
            setSwitch(swURFlip, false);
      }
      return false;
   case StagedLeftFlipperKey:
      vpmFlips.FlipUL(false);
      if (vpmFlips.FlipperSolNumber[2] !== 0)
         setSwitch(swULFlip, false);
      return true;
   case StagedRightFlipperKey:
      vpmFlips.FlipUR(false);
      if (vpmFlips.FlipperSolNumber[3] !== 0)
         setSwitch(swURFlip, false);
      return true;
   case keys.keyCancel: setSwitch(swCancel, false); return true;
   case keys.keyDown: setSwitch(swDown, false); return true;
   case keys.keyUp: setSwitch(swUp, false); return true;
   case keys.keyEnter: setSwitch(swEnter, false); return true;
   case keys.keySlamDoorHit: setSwitch(swSlamTilt, false); return true;
   case StartGameKey: setSwitch(swStartButton, false); return true;
   case keys.keyCoinDoor:
      if (!keys.toggleKeyCoinDoor)
         setSwitch(swCoinDoor, keys.inverseKeyCoinDoor);
      return true;
   case keys.keyShowOpts: Controller.Pause = true; vpmShowOptions(); Controller.Pause = false; return true;
   case keys.keyShowKeys: Controller.Pause = true; vpmShowHelp(); Controller.Pause = false; return true;
   case keys.keyAddBall: Controller.Pause = true; vpmAddBall(); Controller.Pause = false; return true;
   case keys.keyReset: Controller.Stop(); Controller.Run(); vpmTimer.reset(); return true;
   case keys.keyFrame: Controller.LockDisplay = !Controller.LockDisplay; return true;
   case keys.keyDoubleSize: Controller.DoubleSize = !Controller.DoubleSize; return true;
   default:
      return false;
   }
}
