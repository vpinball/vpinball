// core.js: JavaScript port of the parts of core.vbs a WPC table needs (work in progress).
//
// Usage in a table script:
//
//   import { LoadVPM, SoundFX, DOFContactors } from "./controller.js";
//   import { vpmInit, vpmTimer, vpmNudge, cvpmBallStack, SolCallback, hooks, setSwitch } from "./core.js";
//   import { GameOnSolenoid, vpmKeyDown, vpmKeyUp } from "wpc.js";
//   await LoadVPM({ gameName: "pz_f4" });
//
//   export function Table1_Init() {
//      vpmInit(Table1, { useSolenoids: 2, gameOnSolenoid: GameOnSolenoid, ballSize: 50, ballMass: 1 });
//      ...
//   }
//
// Differences with core.vbs: callbacks are functions instead of strings (SolCallback[7] = (enabled) => ...),
// the table options (UseSolenoids, UseLamps, ...) are passed to vpmInit, and the hooks (LampCallback,
// GICallback, ...) are properties of the exported 'hooks' object. Indexed controller properties are set
// with setSwitch(no, state) or Controller.$set("Switch", no, state).

import { Controller } from "./controller.js";

export const VPinMAMEDriverVer = 3.61;
export const PinMAMEInterval = -2; // controller synced timer, runs more than once a frame

const conStackSw = 8;     // Stack switches
const conMaxBalls = 13;   // Because of Apollo 13
const conMaxTimers = 50;  // Some tables want to generate a lot of timers
const conTimerPulse = 40; // Timer runs at 25Hz
const conFastTicks = 4;   // Fast is 4 times per timer pulse
const conMaxSwHit = 5;    // Don't stack up more than 5 events for each switch

export const sLRFlipper = 46;
export const sLLFlipper = 48;
export const sURFlipper = 34;
export const sULFlipper = 36;

// Solenoid callbacks: SolCallback[no] = (enabled) => ..., SolModCallback[no] = (level) => ...
export const SolCallback = new Array(69).fill(null);
export const SolModCallback = new Array(69).fill(null);
const SolPrevState = new Array(69).fill(false);

// Optional hooks, called from the PinMAME timer loop
export const hooks = {
   LampCallback: null,   // called after lamps are updated
   PDLedCallback: null,  // called after leds are updated
   GICallback: null,     // (no, enabled) called for each changed GI string
   GICallback2: null,    // (no, level) called for each changed GI string
   MotorCallback: null,  // called after solenoids are updated
   NVRAMCallback: null,  // (changes) called with the changed NVRAM
};

// Options, set by vpmInit
const opt = {
   useSolenoids: 0,
   useLamps: true,
   useModSol: 0,
   usePdbLeds: false,
   useNVRAM: false,
   ballSize: 50,
   ballMass: 1,
   gameOnSolenoid: 0,
};

export let vpmBallImage = undefined;
export function setBallImage(image) { vpmBallImage = image; }

// Controller helpers for the indexed properties
export function setSwitch(no, state) { Controller.$set("Switch", no, !!state); }
export function getSwitch(no) { return Controller.Switch(no); }

let vpmTrough = null; // Default trough, used to clear up missing balls

//--------------------
//   Timer
//--------------------
class cvpmTimer {
   #now = 0;
   #queue = [];          // { at, state, sw, callback, delay }  state: 0 = timer, 1 = switch pulsed on, 2 = pulse pending
   #slowUpdates = new Set();
   #fastUpdates = new Set();
   #resets = new Set();
   #fastTimer = null;

   initTimer(timerObj, fast) {
      if (fast) {
         this.#fastTimer = timerObj;
         timerObj.TimerInterval = Math.floor(conTimerPulse / conFastTicks);
         timerObj.TimerEnabled = false;
         timerObj.on("Timer", () => this.fastUpdate());
      } else {
         timerObj.Interval = conTimerPulse;
         timerObj.Enabled = true;
         timerObj.on("Timer", () => this.update());
      }
   }

   enableUpdate(obj, fast, enabled) {
      const set = fast ? this.#fastUpdates : this.#slowUpdates;
      if (enabled)
         set.add(obj);
      else
         set.delete(obj);
      if (fast && this.#fastTimer !== null)
         this.#fastTimer.TimerEnabled = this.#fastUpdates.size > 0;
   }

   reset() {
      for (const obj of this.#resets)
         obj.reset();
   }

   fastUpdate() {
      for (const obj of this.#fastUpdates)
         obj.update();
   }

   update() {
      for (const obj of this.#slowUpdates)
         obj.update();
      if (this.#queue.length === 0)
         return;
      this.#now++;
      for (let i = 0; i < this.#queue.length; i++) {
         const q = this.#queue[i];
         if (q.at > this.#now)
            continue;
         if (q.state === 0) {
            this.#queue.splice(i, 1);
            i--;
            if (typeof q.callback === "function")
               q.callback(q.sw);
         } else if (q.state === 1) {
            setSwitch(q.sw, false);
            q.at = this.#now + q.delay;
            q.state = 0;
         } else {
            setSwitch(q.sw, true);
            q.state = 1;
         }
      }
   }

   addResetObj(obj) { this.#resets.add(obj); }

   pulseSw(swNo) { this.pulseSwitch(swNo, 0, null); }

   // Closes the switch for one pulse, then opens it and calls the callback after 'delay' ms
   pulseSwitch(swNo, delay, callback) {
      let count = 0, last = null;
      for (const q of this.#queue)
         if (q.state > 0 && q.sw === swNo) {
            count++;
            last = q;
         }
      if (count >= conMaxSwHit || this.#queue.length >= conMaxTimers)
         return;
      const q = { at: this.#now, state: 2, sw: swNo, callback, delay: Math.floor(delay / conTimerPulse) };
      if (count > 0)
         q.at = last.at + last.state;
      this.#queue.push(q);
   }

   addTimer(delay, callback) {
      if (this.#queue.length >= conMaxTimers)
         return;
      this.#queue.push({ at: this.#now + Math.floor(delay / conTimerPulse), state: 0, sw: 0, callback, delay: 0 });
   }

   addTimer2(delay, callback, id) {
      if (this.#queue.length >= conMaxTimers)
         return;
      this.#queue.push({ at: this.#now + Math.floor(delay / conTimerPulse), state: 0, sw: id, callback, delay: 0 });
   }
}

export const vpmTimer = new cvpmTimer();

//--------------------
//   Ball stack (trough, saucers)
//--------------------
export class cvpmBallStack {
   #sw = new Array(conStackSw + 1).fill(0);
   #entrySw = 0;
   #balls = 0;
   #ballIn = 0;
   #ballPos = new Array(conMaxBalls + 1).fill(0);
   #saucer = false;
   #initKicker = null;
   #exitKicker = null;
   #exitDir = 0;
   #exitForce = 0;
   #exitDir2 = 0;
   #exitForce2 = 0;
   #entrySnd = "ball_stack_entry_default_snd";
   #entrySndBall = "ball_stack_entry_ball_default_snd";
   #exitSnd = "ball_stack_exit_default_snd";
   #exitSndBall = "ball_stack_exit_ball_default_snd";
   #addSnd = "ball_stack_add_default_snd";
   #soundKicker = null;
   KickZ = 0;
   KickBalls = 1;
   KickForceVar = 0;
   KickAngleVar = 0;

   constructor() {
      vpmTimer.addResetObj(this);
   }

   #needUpdate(enabled) { vpmTimer.enableUpdate(this, false, enabled); }

   #hasSw(no) { return no <= conStackSw && this.#sw[no] !== 0; }

   #setSw(no, status) {
      if (!this.#hasSw(no))
         return false;
      setSwitch(this.#sw[no], status);
      return true;
   }

   reset() {
      for (let i = 1; i <= this.#balls; i++)
         this.#setSw(this.#ballPos[i], true);
      if (this.#entrySw && this.#ballIn > 0)
         setSwitch(this.#entrySw, true);
   }

   update() {
      this.#needUpdate(false);
      let ballQue = 1;
      for (let i = 1; i <= this.#balls; i++) {
         if (this.#ballPos[i] > ballQue) { // next slot available
            this.#needUpdate(true);
            if (this.#hasSw(this.#ballPos[i])) { // has switch
               if (getSwitch(this.#sw[this.#ballPos[i]])) {
                  this.#setSw(this.#ballPos[i], false);
               } else {
                  this.#ballPos[i]--;
                  this.#setSw(this.#ballPos[i], true);
               }
            } else { // no switch, move ball to first switch or occupied slot
               do {
                  this.#ballPos[i]--;
               } while (!this.#setSw(this.#ballPos[i], true) && this.#ballPos[i] !== ballQue);
            }
         }
         ballQue = this.#ballPos[i] + 1;
      }
   }

   AddBall(kicker) {
      if (kicker) {
         if (this.#saucer) {
            if (kicker === this.#exitKicker) {
               this.#exitKicker.Enabled = false;
               this.#initKicker = null;
            } else {
               kicker.Enabled = false;
               this.#initKicker = kicker;
            }
         } else {
            kicker.DestroyBall();
         }
      } else if (this.#saucer) {
         this.#exitKicker.Enabled = false;
         this.#initKicker = null;
      }
      if (this.#entrySw) {
         setSwitch(this.#entrySw, true);
         this.#ballIn++;
      } else {
         this.#balls++;
         this.#ballPos[this.#balls] = conStackSw + 1;
         this.#needUpdate(true);
      }
      corePlaySoundAt(this.#addSnd, this.#soundKicker);
   }

   SolIn(enabled) { if (enabled) this.#kickIn(); }
   SolOut(enabled) { if (enabled) this.#kickOut(false); }
   SolOutAlt(enabled) { if (enabled) this.#kickOut(true); }
   EntrySol_On() { this.#kickIn(); }
   ExitSol_On() { this.#kickOut(false); }
   ExitAltSol_On() { this.#kickOut(true); }

   #kickIn() {
      if (this.#ballIn === 0) {
         PlaySound(this.#entrySnd);
         return;
      }
      PlaySound(this.#entrySndBall);
      this.#balls++;
      this.#ballIn--;
      this.#ballPos[this.#balls] = conStackSw + 1;
      this.#needUpdate(true);
      if (this.#entrySw && this.#ballIn === 0)
         setSwitch(this.#entrySw, false);
   }

   #kickOut(altSol) {
      if (this.#balls === 0) {
         corePlaySoundAt(this.#exitSnd, this.#soundKicker);
         return;
      }
      corePlaySoundAt(this.#exitSndBall, this.#soundKicker);
      let kForce = altSol ? this.#exitForce2 : this.#exitForce;
      const kBaseDir = altSol ? this.#exitDir2 : this.#exitDir;
      kForce += (Math.random() - 0.5) * this.KickForceVar;
      if (this.#saucer) {
         this.#setSw(1, false);
         this.#balls = 0;
         const kDir = kBaseDir + (Math.random() - 0.5) * this.KickAngleVar;
         if (this.#initKicker !== null) {
            vpmCreateBall(this.#exitKicker);
            this.#initKicker.DestroyBall();
            this.#initKicker.Enabled = true;
         } else {
            this.#exitKicker.Enabled = true;
         }
         this.#exitKicker.Kick(kDir, kForce, this.KickZ);
      } else {
         for (let i = 1; i <= this.KickBalls; i++) {
            if (this.#balls === 0 || this.#ballPos[1] !== i)
               break; // No more balls
            for (let j = 2; j <= this.#balls; j++) // Move balls in array
               this.#ballPos[j - 1] = this.#ballPos[j];
            this.#ballPos[this.#balls] = 0;
            this.#balls--;
            this.#needUpdate(true);
            this.#setSw(i, false);
            if (this.#exitKicker !== null) {
               const force = Math.max(kForce, 1);
               const kDir = Math.round(kBaseDir + (Math.random() - 0.5) * this.KickAngleVar);
               const kicker = this.#exitKicker, kickZ = this.KickZ;
               vpmTimer.addTimer(200 * (i - 1), () => vpmCreateBall(kicker).Kick(kDir, force, kickZ));
            }
            kForce *= 0.8;
         }
      }
   }

   InitSaucer(kicker, sw, dir, power) {
      this.InitKick(kicker, dir, power);
      this.#saucer = true;
      this.#sw[1] = sw ? sw : kicker.TimerInterval;
   }

   InitNoTrough(kicker, sw, dir, power) {
      this.InitKick(kicker, dir, power);
      this.Balls = 1;
      this.#sw[1] = sw ? sw : kicker.TimerInterval;
      if (vpmTrough === null)
         vpmTrough = this;
   }

   InitSw(entry, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8 = 0) {
      this.#entrySw = entry;
      [this.#sw[1], this.#sw[2], this.#sw[3], this.#sw[4], this.#sw[5], this.#sw[6], this.#sw[7], this.#sw[8]] = [sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8];
      if (vpmTrough === null)
         vpmTrough = this;
   }

   InitSw8(entry, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8) { this.InitSw(entry, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8); }

   InitKick(kicker, dir, force) {
      this.#exitKicker = kicker;
      this.#exitDir = dir;
      this.#exitForce = force;
      this.#soundKicker = kicker;
   }

   InitAltKick(dir, force) {
      this.#exitDir2 = dir;
      this.#exitForce2 = force;
   }

   // Registers the Hit handler of the kickers feeding the stack
   CreateEvents(kickers) {
      for (const kicker of Array.isArray(kickers) ? kickers : [kickers])
         (kicker || this.#exitKicker).on("Hit", function() { this.AddBall(this); }.bind(this));
   }

   set IsTrough(isTrough) {
      if (isTrough)
         vpmTrough = this;
      else if (vpmTrough === this)
         vpmTrough = null;
   }
   get IsTrough() { return vpmTrough === this; }

   InitEntrySnd(ball, noBall) { this.#entrySndBall = ball; this.#entrySnd = noBall; }
   InitExitSnd(ball, noBall) { this.#exitSndBall = ball; this.#exitSnd = noBall; }
   InitAddSnd(snd) { this.#addSnd = snd; }

   set Balls(balls) {
      for (let i = 1; i <= conStackSw; i++) {
         this.#setSw(i, false);
         this.#ballPos[i] = conStackSw + 1;
      }
      if (this.#saucer && balls > 0 && this.#balls === 0)
         vpmCreateBall(this.#exitKicker);
      this.#balls = balls;
      this.#needUpdate(true);
   }
   get Balls() { return this.#balls; }
   get BallsPending() { return this.#ballIn; }

   set BallImage(image) { vpmBallImage = image; }
}

//--------------------
//   Nudge
//--------------------
class cvpmNudge {
   TiltSwitch = 0;
   #slingBump = [];
   #force = [];

   // Mechanical tilt: sent by the keyboard, a physical plumb or VPX's own plumb simulation
   DoMechTilt() {
      if (this.TiltSwitch !== 0)
         vpmTimer.pulseSw(this.TiltSwitch);
   }

   DoNudge(dir, force) {
      Nudge(dir + (Math.random() - 0.5) * 15 * force, (0.6 + Math.random() * 0.8) * force);
   }

   // Legacy, the plumb simulation is in VPX now
   set Sensitivity(sens) { }
   update() { }
   reset() { }

   // Devices wired through the GameOn solenoid (bumpers, slingshots)
   set TiltObj(slingBump) {
      this.#slingBump = Array.isArray(slingBump) ? slingBump : [...slingBump];
      this.#force = this.#slingBump.map((obj) => typeof obj.SlingshotThreshold !== "undefined" ? obj.SlingshotThreshold : obj.Threshold);
   }

   SolGameOn(enabled) {
      this.#slingBump.forEach((obj, i) => {
         const value = enabled ? this.#force[i] : 100;
         if (typeof obj.SlingshotThreshold !== "undefined")
            obj.SlingshotThreshold = value;
         else
            obj.Threshold = value;
      });
   }
}

export const vpmNudge = new cvpmNudge();

//--------------------
//   Flippers (FastFlips): script flips immediately, then defers to the ROM after a short delay
//--------------------
class cvpmFlips2 {
   Delay = 0;
   TiltObjects = true;
   DebugOn = false;
   LagCompensation = false;
   FlipperSolNumber = [sLLFlipper, sLRFlipper, sULFlipper, sURFlipper]; // 0 = left, 1 = right, 2 = upper left, 3 = upper right
   ButtonState = [0, 0, 0, 0];
   SolState = [0, 0, 0, 0];
   FlipperSub = [null, null, null, null];
   FlippersEnabled = false;
   #onOff = true;
   FlipAt = [0, 0, 0, 0];
   RomControlDelay = 100; // ms between a flip and switching to ROM controlled flippers
   Solenoid = 0;
   Initialized = false;

   init() {
      if (this.Initialized)
         throw new Error("Table script bug: vpmFlips.init is called twice, maybe vpmInit is duplicated");
      this.Initialized = true;
      if (opt.useSolenoids < 2)
         return;
      this.Solenoid = opt.useSolenoids > 2 ? opt.useSolenoids : opt.gameOnSolenoid;
      if (!this.Solenoid)
         throw new Error("vpmInit: useSolenoids = 2 needs the gameOnSolenoid option (see the ROM family module)");
      for (let idx = 0; idx < 4; idx++) {
         const sol = Math.abs(this.FlipperSolNumber[idx]);
         if (sol && typeof SolCallback[sol] === "function") {
            this.FlipperSub[idx] = SolCallback[sol];
            SolCallback[sol] = (enabled) => this.romFlip(idx, enabled);
         }
      }
   }

   set Enabled(enabled) {
      enabled = !!enabled;
      if (enabled === this.#onOff)
         return;
      this.#onOff = enabled;
      for (let idx = 0; idx < 4; idx++) {
         const cb = this.FlipperSub[idx];
         if (cb === null)
            continue;
         if (enabled) {
            if (this.SolState[idx] !== this.ButtonState[idx] && this.FlippersEnabled)
               cb(this.ButtonState[idx]);
         } else if (this.ButtonState[idx] !== this.SolState[idx]) {
            cb(this.SolState[idx]);
         }
      }
   }
   get Enabled() { return this.#onOff; }

   flip(idx, enabled) {
      enabled = enabled ? 1 : 0;
      this.ButtonState[idx] = enabled;
      const cb = this.FlipperSub[idx];
      if ((this.#onOff && this.FlippersEnabled) || this.DebugOn) {
         if (cb !== null)
            cb(enabled);
         this.FlipAt[idx] = GameTime;
      }
   }
   FlipL(enabled) { this.flip(0, enabled); }
   FlipR(enabled) { this.flip(1, enabled); }
   FlipUL(enabled) { this.flip(2, enabled); }
   FlipUR(enabled) { this.flip(3, enabled); }

   romFlip(idx, enabled) {
      enabled = enabled ? 1 : 0;
      this.SolState[idx] = enabled;
      const cb = this.FlipperSub[idx];
      if ((!this.#onOff || GameTime >= this.FlipAt[idx] + this.RomControlDelay) && cb !== null)
         cb(enabled);
   }

   tiltSol(enabled) {
      enabled = !!enabled;
      if (this.Delay > 0 && !enabled) {
         vpmTimer.addTimer(this.Delay, () => this.fireDelay());
         this.LagCompensation = true;
      } else {
         if (this.Delay > 0)
            this.LagCompensation = false;
         this.enableFlippers(enabled);
      }
   }

   fireDelay() {
      if (this.LagCompensation)
         this.enableFlippers(false);
   }

   enableFlippers(enabled) {
      if (enabled)
         for (let idx = 0; idx < 4; idx++)
            if (this.FlipperSub[idx] !== null)
               this.FlipperSub[idx](this.ButtonState[idx]);
      this.FlippersEnabled = enabled;
      if (this.TiltObjects)
         vpmNudge.SolGameOn(enabled);
      if (!enabled)
         for (let idx = 0; idx < 4; idx++)
            if (this.FlipperSub[idx] !== null)
               this.FlipperSub[idx](0);
   }
}

export const vpmFlips = new cvpmFlips2();
export function NoUpperLeftFlipper() { vpmFlips.FlipperSolNumber[2] = 0; }
export function NoUpperRightFlipper() { vpmFlips.FlipperSolNumber[3] = 0; }

//--------------------
//   Init and main loop
//--------------------
let hasTimeFence = false;
let lastVisualSync = -1;

// Called from the table Init event. options: useSolenoids (0, 1, 2 or the game on solenoid), useLamps,
// useModSol, usePdbLeds, useNVRAM, ballSize, ballMass, gameOnSolenoid, timeFence, pulseTimer, pinmameTimer
export function vpmInit(table, options = {}) {
   Object.assign(opt, options);
   if (opt.useModSol)
      Controller.$set("SolMask", 2, Number(opt.useModSol)); // 1 for modulated solenoids, 2 for physical (0..1) outputs

   hasTimeFence = !!options.timeFence;
   if (hasTimeFence)
      Controller.TimeFence = PreciseGameTime > 0 ? PreciseGameTime : 0.01;

   table.on("Paused", () => { Controller.Pause = true; });
   table.on("UnPaused", () => { Controller.Pause = false; });
   table.on("Exit", () => { Controller.Pause = false; Controller.Stop(); });

   // The two timers every VPM table has: PulseTimer drives vpmTimer, PinMAMETimer polls the emulation.
   // The table enables PinMAMETimer once the controller runs: PinMAMETimer.Interval = PinMAMEInterval; PinMAMETimer.Enabled = true
   const globals = /** @type {any} */ (globalThis);
   const pulseTimer = options.pulseTimer ?? globals.PulseTimer;
   if (pulseTimer)
      vpmTimer.initTimer(pulseTimer, false);
   const pinmameTimer = options.pinmameTimer ?? globals.PinMAMETimer;
   if (pinmameTimer)
      pinmameTimer.on("Timer", pinmameTimerUpdate);

   vpmFlips.init();
}

export function vpmExit() { }

export function vpmCreateBall(kicker) {
   const ball = kicker.CreateSizedBallWithMass(opt.ballSize / 2, opt.ballMass);
   if (vpmBallImage !== undefined)
      ball.Image = vpmBallImage;
   return kicker;
}

function pinmameTimerUpdate() {
   if (hasTimeFence)
      Controller.TimeFence = PreciseGameTime;

   // Lights are updated at most once per frame
   const updateVisual = FrameIndex !== lastVisualSync;
   if (updateVisual)
      lastVisualSync = FrameIndex;

   let chgLamp = null, chgGI = null, chgSol = null, chgLed = null;
   if (updateVisual) {
      if (opt.useLamps)
         chgLamp = Controller.ChangedLamps;
      else if (hooks.LampCallback)
         hooks.LampCallback();
      if (opt.usePdbLeds)
         chgLed = Controller.ChangedLEDs;
      else if (hooks.PDLedCallback)
         hooks.PDLedCallback();
      if (hooks.GICallback || hooks.GICallback2)
         chgGI = Controller.ChangedGIStrings;
   }
   if (opt.useNVRAM && hooks.NVRAMCallback) {
      const chgNVRAM = Controller.ChangedNVRAM;
      if (chgNVRAM && chgNVRAM.length)
         hooks.NVRAMCallback(chgNVRAM);
   }
   if (opt.useSolenoids)
      chgSol = Controller.ChangedSolenoids;
   if (hooks.MotorCallback)
      hooks.MotorCallback();

   const pwmScale = opt.useModSol >= 2 ? 1 / 255 : 1;

   if (chgSol && chgSol.length) {
      for (const [nsol, rawState] of chgSol) {
         const state = rawState * pwmScale;
         const bstate = state >= 0.5;
         if (bstate !== SolPrevState[nsol]) {
            SolPrevState[nsol] = bstate;
            const cb = SolCallback[nsol];
            if (cb)
               cb(bstate);
         }
         if (opt.useModSol >= 1) {
            const cb = SolModCallback[nsol];
            if (cb)
               cb(state);
         }
         if (opt.useSolenoids > 1 && nsol === vpmFlips.Solenoid)
            vpmFlips.tiltSol(bstate);
      }
   }

   if (chgLamp && chgLamp.length) {
      for (const [no, state] of chgLamp)
         vpmDoLampUpdate(no, state * pwmScale);
      if (hooks.LampCallback)
         hooks.LampCallback();
   }

   if (chgGI && chgGI.length) {
      for (const [no, state] of chgGI) {
         if (hooks.GICallback)
            hooks.GICallback(no, state * pwmScale >= 0.5);
         if (hooks.GICallback2)
            hooks.GICallback2(no, state * pwmScale);
      }
   }

   if (chgLed && chgLed.length) {
      for (const [no, color] of chgLed) {
         const light = Lights[no];
         if (!light)
            continue;
         for (const l of Array.isArray(light) ? light : [light]) {
            l.Color = color;
            l.State = color === 0 ? 0 : 1;
         }
      }
      if (hooks.PDLedCallback)
         hooks.PDLedCallback();
   }
}

//--------------------
//   Lamps
//--------------------
export const Lights = new Array(261).fill(null); // lamp number -> light or array of lights

export function vpmMapLights(lights) {
   for (const light of lights) {
      const no = light.TimerInterval; // the lamp number is stored in the timer interval
      if (Lights[no] === null)
         Lights[no] = light;
      else if (Array.isArray(Lights[no]))
         Lights[no].push(light);
      else
         Lights[no] = [Lights[no], light];
   }
}

export function vpmDoLampUpdate(no, state) {
   const light = Lights[no];
   if (!light)
      return;
   for (const l of Array.isArray(light) ? light : [light])
      l.State = state;
}

export function vpmDoSolCallback(no, enabled) {
   const cb = SolCallback[no];
   if (cb)
      cb(!!enabled);
}

//--------------------
//   Solenoid helpers
//--------------------
export function vpmSolSound(sound, enabled) {
   if (enabled) {
      StopSound(sound);
      PlaySound(sound);
   }
}

export function vpmFlasher(flash, enabled) {
   for (const obj of Array.isArray(flash) ? flash : [flash])
      obj.State = enabled ? 1 : 0;
}

export function vpmSolGate(gate, sound, enabled) {
   gate.Open = enabled;
   vpmPlaySound(enabled, sound);
}

export function vpmSolWall(wall, sound, enabled) {
   wall.IsDropped = enabled;
   vpmPlaySound(enabled, sound);
}

export function vpmSolMagnet(magnet, enabled) {
   magnet.Enabled = enabled;
   if (!enabled)
      magnet.Kick(180, 1);
}

function vpmPlaySound(enabled, sound) {
   if (typeof sound === "string") {
      if (enabled) {
         StopSound(sound);
         PlaySound(sound);
      }
   } else if (sound) {
      PlaySound(enabled ? globalThis.SSolenoidOn ?? "fx_Solenoidon" : globalThis.SSolenoidOff ?? "fx_Solenoidoff");
   }
}

// Registers the switch events of a list of parts, the switch number is the part's timer interval
export function vpmCreateEvents(hitObjs) {
   for (const obj of hitObjs) {
      const sw = obj.TimerInterval;
      if (typeof obj.HasHitEvent !== "undefined" && typeof obj.SlingshotThreshold !== "undefined") { // Wall
         if (obj.HasHitEvent)
            obj.on("Hit", () => vpmTimer.pulseSw(sw));
         else
            obj.on("Slingshot", () => vpmTimer.pulseSw(sw));
      } else if (typeof obj.TriggerShape !== "undefined") { // Trigger
         obj.on("Hit", () => setSwitch(sw, true));
         obj.on("Unhit", () => setSwitch(sw, false));
      } else if (typeof obj.AngleMax !== "undefined") { // Spinner
         obj.on("Spin", () => vpmTimer.pulseSw(sw));
      } else {
         obj.on("Hit", () => vpmTimer.pulseSw(sw));
      }
   }
}

//--------------------
//   Sound position helpers
//--------------------
export function CoreAudioPan(x) {
   const tmp = x * 2 / Table1.Width - 1;
   return tmp > 0 ? Math.pow(tmp, 10) : -Math.pow(-tmp, 10);
}

export function CoreAudioFade(y) {
   const tmp = y * 2 / Table1.Height - 1;
   return tmp > 0 ? Math.pow(tmp, 10) : -Math.pow(-tmp, 10);
}

function corePlaySoundAt(soundName, at) {
   if (!soundName)
      return;
   if (at)
      PlaySound(soundName, 1, 1, CoreAudioPan(at.X), 0, 0, false, false, CoreAudioFade(at.Y));
   else
      PlaySound(soundName);
}

// Debug helpers of the VBScript version, no UI here
export function vpmShowOptions() { console.log("vpmShowOptions: not available"); }
export function vpmShowHelp() { console.log("vpmShowHelp: not available"); }
export function vpmVol() { console.log("vpmVol: not available"); }
export function vpmAddBall() {
   if (vpmTrough !== null)
      vpmTrough.AddBall(null);
}
