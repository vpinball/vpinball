// vpx-language: js
//
// JavaScript port of the VPX example table script, kept one on one with the VBScript version so the two
// can be compared. As a sidecar next to exampleTable.vpx it replaces the VBScript embedded in the table
// (the same way a .vbs sidecar does); remove or rename it to play the VBScript version again.
//
// The table script is an ES module. Table parts are globals, event handlers are exported functions named
// <Item>_<Event> (like the VBScript Subs). Callbacks the host looks up by name (OnBallBallCollision) are
// exported as well.
//
// Credits of the original table and script:
// Initial table created by fuzzel, jimmyfingers, jpsalas, toxie & unclewilly (in alphabetical order)
// Flipper primitives by zany
// Ball rolling sound script by jpsalas
// Ball shadow by ninuzzu
// Ball control & ball dropping sound by rothbauerw
// DOF by arngrim
// Positional sound helper functions by djrobx

// controller.js (DOF) helps controlling additional hardware like lights, gears, knockers, bells and chimes.
// This table uses DOF via the SoundFX calls inserted in some of the PlaySound commands, which will then
// fire an additional event, instead of just playing a sample/sound effect.
import { SoundFX, DOFContactors, DOFFlippers } from "controller.js";

// If using Visual PinMAME (VPM), pass the ROM/game name to the controller loader (see controller.js),
// both for VPM, and DOF to load the right DOF config from the Configtool, whether it's a VPM or an Original table

const EnableRetractPlunger = false; // Change to true to enable retracting the plunger at a linear speed; wait for 1 second at the maximum position; move back towards the resting position; nice for button/key plungers

if (!Table1.ShowDT)
   ScoreText.Visible = false;

let EnableBallControl = false; // Change to true to enable manual ball control (or press C in-game) via the arrow keys and B (boost movement) keys

const BallSize = 25; // Ball radius

export function Table1_KeyDown(keycode) {
   if (keycode === PlungerKey) {
      if (EnableRetractPlunger)
         Plunger.PullBackandRetract();
      else
         Plunger.PullBack();
      PlaySound("plungerpull", 0, 1, AudioPan(Plunger), 0.25, 0, false, true, AudioFade(Plunger));
   }

   if (keycode === LeftFlipperKey) {
      LeftFlipper.RotateToEnd();
      PlaySound(SoundFX("fx_flipperup", DOFFlippers), 0, 0.67, AudioPan(LeftFlipper), 0.05, 0, false, true, AudioFade(LeftFlipper));
   }

   if (keycode === RightFlipperKey) {
      RightFlipper.RotateToEnd();
      PlaySound(SoundFX("fx_flipperup", DOFFlippers), 0, 0.67, AudioPan(RightFlipper), 0.05, 0, false, true, AudioFade(RightFlipper));
   }

   if (keycode === LeftTiltKey)
      Nudge(90, 2);

   if (keycode === RightTiltKey)
      Nudge(270, 2);

   if (keycode === CenterTiltKey)
      Nudge(0, 2);

   // Manual Ball Control
   if (keycode === 46) // C Key
      EnableBallControl = !EnableBallControl;
   if (EnableBallControl) {
      if (keycode === 48) // B Key
         BCboost = (BCboost === 1) ? BCboostmulti : 1;
      if (keycode === 203) BCleft = 1;  // Left Arrow
      if (keycode === 200) BCup = 1;    // Up Arrow
      if (keycode === 208) BCdown = 1;  // Down Arrow
      if (keycode === 205) BCright = 1; // Right Arrow
   }
}

export function Table1_KeyUp(keycode) {
   if (keycode === PlungerKey) {
      Plunger.Fire();
      PlaySound("plunger", 0, 1, AudioPan(Plunger), 0.25, 0, false, true, AudioFade(Plunger));
   }

   if (keycode === LeftFlipperKey) {
      LeftFlipper.RotateToStart();
      PlaySound(SoundFX("fx_flipperdown", DOFFlippers), 0, 1, AudioPan(LeftFlipper), 0.05, 0, false, true, AudioFade(LeftFlipper));
   }

   if (keycode === RightFlipperKey) {
      RightFlipper.RotateToStart();
      PlaySound(SoundFX("fx_flipperdown", DOFFlippers), 0, 1, AudioPan(RightFlipper), 0.05, 0, false, true, AudioFade(RightFlipper));
   }

   // Manual Ball Control
   if (EnableBallControl) {
      if (keycode === 203) BCleft = 0;  // Left Arrow
      if (keycode === 200) BCup = 0;    // Up Arrow
      if (keycode === 208) BCdown = 0;  // Down Arrow
      if (keycode === 205) BCright = 0; // Right Arrow
   }
}

export function playfield_mesh_Hit() {
   MsgBox("ball has hit playfield");
}

export function Drain_Hit() {
   PlaySound("drain", 0, 1, AudioPan(Drain), 0.25, 0, false, true, AudioFade(Drain));
   Drain.DestroyBall();
   BIP = BIP - 1;
   if (BIP === 0) {
      BallRelease.CreateBall();
      BallRelease.Kick(90, 7);
      PlaySound(SoundFX("ballrelease", DOFContactors), 0, 1, AudioPan(BallRelease), 0.25, 0, false, true, AudioFade(BallRelease));
      BIP = BIP + 1;
   }
}

let BIP = 0;

export function Plunger_Init() {
   PlaySound(SoundFX("ballrelease", DOFContactors), 0, 1, AudioPan(BallRelease), 0.25, 0, false, true, AudioFade(BallRelease));
   BallRelease.CreateBall();
   BallRelease.Kick(90, 7);
   BIP = BIP + 1;
}

export function Gate_Hit() {
   Kicker1.Kick(190, 10);
}

export function Bumper1_Hit() {
   PlaySound(SoundFX("fx_bumper4", DOFContactors), 0, 1, AudioPan(Bumper1), 0, 0, false, true, AudioFade(Bumper1));
   b1l1.State = 1; b1l2.State = 1;
   Bumper1.TimerEnabled = true;
}

export function Bumper1_Timer() {
   b1l1.State = 0; b1l2.State = 0;
   Bumper1.TimerEnabled = false;
}

export function Bumper2_Hit() {
   PlaySound(SoundFX("fx_bumper4", DOFContactors), 0, 1, AudioPan(Bumper2), 0, 0, false, true, AudioFade(Bumper2));
   B2L1.State = 1; b2l2.State = 1;
   Bumper2.TimerEnabled = true;
}

export function Bumper2_Timer() {
   B2L1.State = 0; b2l2.State = 0;
   Bumper2.TimerEnabled = false;
}

export function Bumper3_Hit() {
   PlaySound(SoundFX("fx_bumper4", DOFContactors), 0, 1, AudioPan(Bumper3), 0, 0, false, true, AudioFade(Bumper3));
   b3l1.State = 1; b3l2.State = 1;
   Bumper3.TimerEnabled = true;
}

export function Bumper3_Timer() {
   b3l1.State = 0; b3l2.State = 0;
   Bumper3.TimerEnabled = false;
}

export function Bumper4_Hit() {
   PlaySound(SoundFX("fx_bumper4", DOFContactors), 0, 1, AudioPan(Bumper4), 0, 0, false, true, AudioFade(Bumper4));
   B4L1.State = 1; B4L2.State = 1;
   Bumper4.TimerEnabled = true;
}

export function Bumper4_Timer() {
   B4L1.State = 0; B4L2.State = 0;
   Bumper4.TimerEnabled = false;
}

export function Bumper5_Hit() {
   PlaySound(SoundFX("fx_bumper4", DOFContactors), 0, 1, AudioPan(Bumper5), 0, 0, false, true, AudioFade(Bumper5));
   b5l1.State = 1; b5l2.State = 1;
   Bumper5.TimerEnabled = true;
}

export function Bumper5_Timer() {
   b5l1.State = 0; b5l2.State = 0;
   Bumper5.TimerEnabled = false;
}

export function sw9_Hit() {
   l9.State = l9.State === 1 ? 0 : 1;
}

export function sw8_Hit() {
   l8.State = l8.State === 1 ? 0 : 1;
}

export function sw7_Hit() {
   l7.State = l7.State === 1 ? 0 : 1;
}

export function sw6_Hit() {
   l6.State = l6.State === 1 ? 0 : 1;
}

//****Targets
export function sw1_Hit() {
   l1.State = l1.State === 1 ? 0 : 1;
   sw1.TimerEnabled = true;
}

export function sw1_Timer() {
   sw1.IsDropped = false;
   sw1.TimerEnabled = false;
}

export function sw2_Hit() {
   l2.State = l2.State === 1 ? 0 : 1;
   sw2.TimerEnabled = true;
}

export function sw2_Timer() {
   sw2.IsDropped = false;
   sw2.TimerEnabled = false;
}

export function sw3_Hit() {
   l3.State = l3.State === 1 ? 0 : 1;
   sw3.TimerEnabled = true;
}

export function sw3_Timer() {
   sw3.IsDropped = false;
   sw3.TimerEnabled = false;
}

export function sw11_Hit() {
   l11.State = l11.State === 1 ? 0 : 1;
}

export function sw12_Hit() {
   l12.State = l12.State === 1 ? 0 : 1;
}

export function sw13_Hit() {
   l13.State = l13.State === 1 ? 0 : 1;
}

export function Kicker1_Hit() {
   PlaySound("kicker_enter_center", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall));
   PlaySound(SoundFX("ballrelease", DOFContactors), 0, 0.5, AudioPan(BallRelease), 0.25, 0, false, true, AudioFade(BallRelease));
   BallRelease.CreateBall();
   BallRelease.Kick(90, 8);
   BIP = BIP + 1;
}

export function Kicker1_Unhit() {
   PlaySound("popper_ball", 0, 0.75, AudioPan(Kicker1), 0.25, 0, false, true, AudioFade(Kicker1));
}

//*****GI Lights On
for (const xx of GI)
   xx.State = 1;

//**********Sling Shot Animations
// Rstep and Lstep  are the variables that increment the animation
//****************
let RStep = 0, LStep = 0;

export function RightSlingShot_Slingshot() {
   PlaySound(SoundFX("right_slingshot", DOFContactors), 0, 1, 0.05, 0.05);
   RSling.Visible = false;
   RSling1.Visible = true;
   Sling1.RotX = 20;
   RStep = 0;
   RightSlingShot.TimerEnabled = true;
   gi1.State = 0; gi2.State = 0;
}

export function RightSlingShot_Timer() {
   switch (RStep) {
   case 3: RSling1.Visible = false; RSling2.Visible = true; Sling1.RotX = 10; break;
   case 4: RSling2.Visible = false; RSling.Visible = true; Sling1.RotX = 0; RightSlingShot.TimerEnabled = false; gi1.State = 1; gi2.State = 1; break;
   }
   RStep = RStep + 1;
}

export function LeftSlingShot_Slingshot() {
   PlaySound(SoundFX("left_slingshot", DOFContactors), 0, 1, -0.05, 0.05);
   LSling.Visible = false;
   LSling1.Visible = true;
   Sling2.RotX = 20;
   LStep = 0;
   LeftSlingShot.TimerEnabled = true;
   gi3.State = 0; gi4.State = 0;
}

export function LeftSlingShot_Timer() {
   switch (LStep) {
   case 3: LSling1.Visible = false; LSling2.Visible = true; Sling2.RotX = 10; break;
   case 4: LSling2.Visible = false; LSling.Visible = true; Sling2.RotX = 0; LeftSlingShot.TimerEnabled = false; gi3.State = 1; gi4.State = 1; break;
   }
   LStep = LStep + 1;
}


//*********************************************************************
//                 Positional Sound Playback Functions
//*********************************************************************

// Play a sound, depending on the X,Y position of the table element (especially cool for surround speaker setups, otherwise stereo panning only)
// parameters (defaults): loopcount (1), volume (1), randompitch (0), pitch (0), useexisting (false), restart (true))
// Note that this will not work (currently) for walls/slingshots as these do not feature a simple, single X,Y position
export function PlayXYSound(soundname, tableobj, loopcount, volume, randompitch, pitch, useexisting, restart) {
   PlaySound(soundname, loopcount, volume, AudioPan(tableobj), randompitch, pitch, useexisting, restart, AudioFade(tableobj));
}

// Similar functions that are less complicated to use (e.g. simply use standard parameters for the PlaySound call)
export function PlaySoundAt(soundname, tableobj) {
   PlaySound(soundname, 1, 1, AudioPan(tableobj), 0, 0, false, true, AudioFade(tableobj));
}

export function PlaySoundAtBall(soundname) {
   PlaySoundAt(soundname, ActiveBall);
}


//*********************************************************************
//                     Supporting Ball & Sound Functions
//*********************************************************************

// Fades between front and back of the table (for surround systems or 2x2 speakers, etc), depending on the Y position on the table. "Table1" is the name of the table
function AudioFade(tableobj) {
   const tmp = tableobj.Y * 2 / Table1.Height - 1;
   if (tmp > 0)
      return Math.pow(tmp, 10);
   else
      return -Math.pow(-tmp, 10);
}

// Calculates the pan for a tableobj based on the X position on the table. "Table1" is the name of the table
function AudioPan(tableobj) {
   const tmp = tableobj.X * 2 / Table1.Width - 1;
   if (tmp > 0)
      return Math.pow(tmp, 10);
   else
      return -Math.pow(-tmp, 10);
}

// Calculates the Volume of the sound based on the ball speed
function Vol(ball) {
   return Math.pow(BallVel(ball), 2) / 2000;
}

// Calculates the pitch of the sound based on the ball speed
function Pitch(ball) {
   return BallVel(ball) * 20;
}

// Calculates the ball speed
function BallVel(ball) {
   return Math.floor(Math.sqrt(Math.pow(ball.VelX, 2) + Math.pow(ball.VelY, 2)));
}


//*****************************************
//   rothbauerw's Manual Ball Control
//*****************************************

let BCup = 0, BCdown = 0, BCleft = 0, BCright = 0;
let ControlBallInPlay = false, ControlActiveBall = null;

let BCboost = 1;             // Do Not Change - default setting
const BCvel = 4;             // Controls the speed of the ball movement
const BCyveloffset = -0.01;  // Offsets the force of gravity to keep the ball from drifting vertically on the table, should be negative
const BCboostmulti = 3;      // Boost multiplier to ball velocity (toggled with the B key)

export function StartBallControl_Hit() {
   ControlActiveBall = ActiveBall;
   ControlBallInPlay = true;
}

export function StopBallControl_Hit() {
   ControlBallInPlay = false;
}

export function BallControlTimer_Timer() {
   if (EnableBallControl && ControlBallInPlay) {
      if (BCright === 1)
         ControlActiveBall.VelX = BCvel * BCboost;
      else if (BCleft === 1)
         ControlActiveBall.VelX = -BCvel * BCboost;
      else
         ControlActiveBall.VelX = 0;

      if (BCup === 1)
         ControlActiveBall.VelY = -BCvel * BCboost;
      else if (BCdown === 1)
         ControlActiveBall.VelY = BCvel * BCboost;
      else
         ControlActiveBall.VelY = BCyveloffset;
   }
}


//********************************************************************
//      JP's VP10 Rolling Sounds (+rothbauerw's Dropping Sounds)
//********************************************************************

const tnob = 5; // total number of balls
const rolling = new Array(tnob + 1).fill(false);

export function RollingTimer_Timer() {
   const BOT = GetBalls();

   // stop the sound of deleted balls
   for (let b = BOT.length; b <= tnob; b++) {
      rolling[b] = false;
      StopSound("fx_ballrolling" + b);
   }

   // exit if no balls on the table
   if (BOT.length === 0)
      return;

   for (let b = 0; b < BOT.length; b++) {
      // play the rolling sound for each ball
      if (BallVel(BOT[b]) > 1 && BOT[b].Z < 30) {
         rolling[b] = true;
         PlaySound("fx_ballrolling" + b, -1, Vol(BOT[b]), AudioPan(BOT[b]), 0, Pitch(BOT[b]), true, false, AudioFade(BOT[b]));
      } else {
         if (rolling[b] === true) {
            StopSound("fx_ballrolling" + b);
            rolling[b] = false;
         }
      }

      // play ball drop sounds
      if (BOT[b].VelZ < -1 && BOT[b].Z < 55 && BOT[b].Z > 27) // height adjust for ball drop sounds
         PlaySound("fx_ball_drop" + b, 0, Math.abs(BOT[b].VelZ) / 17, AudioPan(BOT[b]), 0, Pitch(BOT[b]), true, false, AudioFade(BOT[b]));
   }
}

//**********************
// Ball Collision Sound
//**********************

export function OnBallBallCollision(ball1, ball2, velocity) {
   PlaySound("fx_collide", 0, Math.pow(velocity, 2) / 2000, AudioPan(ball1), 0, Pitch(ball1), false, false, AudioFade(ball1));
}


//*****************************************
//   ninuzzu's   FLIPPER SHADOWS v3 (VPX 10.8)
//*****************************************

export function LeftFlipper_Animate() {
   FlipperLSh.RotZ = LeftFlipper.CurrentAngle;
}

export function RightFlipper_Animate() {
   FlipperRSh.RotZ = RightFlipper.CurrentAngle;
}

//*****************************************
//   ninuzzu's   BALL SHADOW
//*****************************************
const BallShadow = [BallShadow1, BallShadow2, BallShadow3, BallShadow4, BallShadow5];

export function BallShadowUpdate_Timer() {
   const BOT = GetBalls();
   // hide shadow of deleted balls
   if (BOT.length < tnob) {
      for (let b = BOT.length; b < tnob; b++)
         BallShadow[b].Visible = false;
   }
   // exit if no balls on the table
   if (BOT.length === 0)
      return;
   // render the shadow for each ball
   for (let b = 0; b < BOT.length; b++) {
      BallShadow[b].X = BOT[b].X + (BOT[b].X - (Table1.Width / 2)) * 1.25 / BallSize;
      BallShadow[b].Y = BOT[b].Y + 12;
      BallShadow[b].Size_X = 5;
      BallShadow[b].Size_Y = 5;
      BallShadow[b].Visible = BOT[b].Z > 20;
   }
}


//************************************
// What you need to add to your table
//************************************

// a timer called RollingTimer. With a fast interval, like 10
// one collision sound, in this script is called fx_collide
// as many sound files as max number of balls, with names ending with 0, 1, 2, 3, etc
// for ex. as used in this script: fx_ballrolling0, fx_ballrolling1, fx_ballrolling2, fx_ballrolling3, etc


//******************************************
// Explanation of the rolling sound routine
//******************************************

// sounds are played based on the ball speed and position

// the routine checks first for deleted balls and stops the rolling sound.

// The For loop goes through all the balls on the table and checks for the ball speed and
// if the ball is on the table (height lower than 30) then then it plays the sound
// otherwise the sound is stopped, like when the ball has stopped or is on a ramp or flying.

// The sound is played using the VOL, AUDIOPAN, AUDIOFADE and PITCH functions, so the volume and pitch of the sound
// will change according to the ball speed, and the AUDIOPAN & AUDIOFADE functions will change the stereo position
// according to the position of the ball on the table.


//**************************************
// Explanation of the collision routine
//**************************************

// The collision is built in VP.
// You only need to export a function OnBallBallCollision(ball1, ball2, velocity) and when two balls collide they
// will call this routine. What you add in the function is up to you. As an example is a simple PlaySound with volume and panning
// depending of the speed of the collision.


export function Pins_Hit(idx) {
   PlaySound("pinhit_low", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), false, false, AudioFade(ActiveBall));
}

export function Targets_Hit(idx) {
   PlaySound("target", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), false, false, AudioFade(ActiveBall));
}

export function Metals_Thin_Hit(idx) {
   PlaySound("metalhit_thin", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall));
}

export function Metals_Medium_Hit(idx) {
   PlaySound("metalhit_medium", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall));
}

export function Metals2_Hit(idx) {
   PlaySound("metalhit2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall));
}

export function Gates_Hit(idx) {
   PlaySound("gate4", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall));
}

export function Spinner_Spin() {
   PlaySound("fx_spinner", 0, 0.25, AudioPan(Spinner), 0.25, 0, false, true, AudioFade(Spinner));
}

export function Rubbers_Hit(idx) {
   const finalspeed = Math.sqrt(ActiveBall.VelX * ActiveBall.VelX + ActiveBall.VelY * ActiveBall.VelY);
   if (finalspeed > 20)
      PlaySound("fx_rubber2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall));
   if (finalspeed >= 6 && finalspeed <= 20)
      RandomSoundRubber();
}

export function Posts_Hit(idx) {
   const finalspeed = Math.sqrt(ActiveBall.VelX * ActiveBall.VelX + ActiveBall.VelY * ActiveBall.VelY);
   if (finalspeed > 16)
      PlaySound("fx_rubber2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall));
   if (finalspeed >= 6 && finalspeed <= 16)
      RandomSoundRubber();
}

function RandomSoundRubber() {
   switch (Math.floor(Math.random() * 3) + 1) {
   case 1: PlaySound("rubber_hit_1", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall)); break;
   case 2: PlaySound("rubber_hit_2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall)); break;
   case 3: PlaySound("rubber_hit_3", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall)); break;
   }
}

export function LeftFlipper_Collide(parm) {
   RandomSoundFlipper();
}

export function RightFlipper_Collide(parm) {
   RandomSoundFlipper();
}

function RandomSoundFlipper() {
   switch (Math.floor(Math.random() * 3) + 1) {
   case 1: PlaySound("flip_hit_1", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall)); break;
   case 2: PlaySound("flip_hit_2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall)); break;
   case 3: PlaySound("flip_hit_3", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), true, false, AudioFade(ActiveBall)); break;
   }
}
