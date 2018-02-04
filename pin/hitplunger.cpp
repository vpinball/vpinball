#include "stdafx.h"

#define PLUNGERHEIGHT 50.0f

const float PlungerMoverObject::m_mass = 30.0f;

HitPlunger::HitPlunger(const float x, const float y, const float x2, const float zheight,
   const float frameTop, const float frameBottom,
   Plunger * const pPlunger)
{
   m_plungerMover.m_plunger = pPlunger;
   m_plungerMover.m_x = x;
   m_plungerMover.m_x2 = x2;
   m_plungerMover.m_y = y;

   m_plungerMover.m_frameEnd = frameTop;
   m_plungerMover.m_frameStart = frameBottom;
   const float frameLen = m_plungerMover.m_frameLen = frameBottom - frameTop;

   m_plungerMover.m_pullForce = 0.0f;
   m_plungerMover.m_reverseImpulse = 0.0f;
   m_plungerMover.m_fireTimer = 0;
   m_plungerMover.m_autoFireTimer = 0;

   m_plungerMover.m_fStrokeEventsArmed = false;
   m_plungerMover.m_speed = 0.0f;
   m_plungerMover.m_travelLimit = frameTop;
   m_plungerMover.m_scatterVelocity = pPlunger->m_d.m_scatterVelocity;

   const float restPos = pPlunger->m_d.m_parkPosition; // The rest position is taken from the "park position" property

   // start at the rest position
   m_plungerMover.m_restPos = restPos;
   m_plungerMover.m_pos = frameTop + (restPos * frameLen);

   m_hitBBox.zlow = zheight;
   m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;

   m_plungerMover.m_linesegBase.m_hitBBox.zlow = zheight;
   m_plungerMover.m_linesegBase.m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;
   m_plungerMover.m_linesegSide[0].m_hitBBox.zlow = zheight;
   m_plungerMover.m_linesegSide[0].m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;
   m_plungerMover.m_linesegSide[1].m_hitBBox.zlow = zheight;
   m_plungerMover.m_linesegSide[1].m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;
   m_plungerMover.m_linesegEnd.m_hitBBox.zlow = zheight;
   m_plungerMover.m_linesegEnd.m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;

   m_plungerMover.m_jointBase[0].m_hitBBox.zlow = zheight;
   m_plungerMover.m_jointBase[0].m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;
   m_plungerMover.m_jointBase[1].m_hitBBox.zlow = zheight;
   m_plungerMover.m_jointBase[1].m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;
   m_plungerMover.m_jointEnd[0].m_hitBBox.zlow = zheight;
   m_plungerMover.m_jointEnd[0].m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;
   m_plungerMover.m_jointEnd[1].m_hitBBox.zlow = zheight;
   m_plungerMover.m_jointEnd[1].m_hitBBox.zhigh = zheight + PLUNGERHEIGHT;

   m_plungerMover.SetObjects(m_plungerMover.m_pos);
}

void HitPlunger::CalcHitBBox()
{
   // Allow roundoff
   m_hitBBox.left = m_plungerMover.m_x - 0.1f;
   m_hitBBox.right = m_plungerMover.m_x2 + 0.1f;
   m_hitBBox.top = m_plungerMover.m_frameEnd - 0.1f;
   m_hitBBox.bottom = m_plungerMover.m_y + 0.1f;

   // zlow & zhigh gets set in constructor
}

void PlungerMoverObject::SetObjects(const float len)
{
   m_linesegBase.v1.x = m_x;
   m_linesegBase.v1.y = m_y;
   m_linesegBase.v2.x = m_x2;
   m_linesegBase.v2.y = m_y;// + 0.0001f;

   m_jointBase[0].m_xy.x = m_x;
   m_jointBase[0].m_xy.y = m_y;
   m_jointBase[1].m_xy.x = m_x2;
   m_jointBase[1].m_xy.y = m_y;// + 0.0001f;

   m_linesegSide[0].v2.x = m_x;
   m_linesegSide[0].v2.y = m_y;
   m_linesegSide[0].v1.x = m_x + 0.0001f;
   m_linesegSide[0].v1.y = len;

   m_linesegSide[1].v1.x = m_x2;
   m_linesegSide[1].v1.y = m_y;
   m_linesegSide[1].v2.x = m_x2 + 0.0001f;
   m_linesegSide[1].v2.y = len;

   m_linesegEnd.v2.x = m_x;
   m_linesegEnd.v2.y = len;
   m_linesegEnd.v1.x = m_x2;
   m_linesegEnd.v1.y = len;// + 0.0001f;

   m_jointEnd[0].m_xy.x = m_x;
   m_jointEnd[0].m_xy.y = len;
   m_jointEnd[1].m_xy.x = m_x2;
   m_jointEnd[1].m_xy.y = len;// + 0.0001f;

   m_linesegBase.CalcNormal();
   m_linesegEnd.CalcNormal();

   m_linesegSide[0].CalcNormal();
   m_linesegSide[1].CalcNormal();
}

void PlungerMoverObject::UpdateDisplacements(const float dtime)
{
   // figure the travel distance
   const float dx = dtime * m_speed;

   // figure the position change
   m_pos += dx;

   // apply the travel limit
   if (m_pos < m_travelLimit)
      m_pos = m_travelLimit;

   // if we're in firing mode and we've crossed the bounce position, reverse course
   const float relPos = (m_pos - m_frameEnd) / m_frameLen;
   const float bouncePos = m_restPos + m_fireBounce;
   if (m_fireTimer != 0 && dtime != 0.0f
      && ((m_fireSpeed < 0.0f ? relPos <= bouncePos : relPos >= bouncePos)))
   {
      // stop at the bounce position
      m_pos = m_frameEnd + bouncePos*m_frameLen;

      // reverse course at reduced speed
      m_fireSpeed = -m_fireSpeed * 0.4f;

      // figure the new bounce as a fraction of the previous bounce
      m_fireBounce *= -0.4f;
   }

   // apply the travel limit (again)
   if (m_pos < m_travelLimit)
      m_pos = m_travelLimit;

   // limit motion to the valid range
   if (dtime != 0.0f)
   {
      if (m_pos < m_frameEnd)
      {
         m_speed = 0.0f;
         m_pos = m_frameEnd;
      }
      else if (m_pos > m_frameStart)
      {
         m_speed = 0.0f;
         m_pos = m_frameStart;
      }

      // apply the travel limit (yet again)
      if (m_pos < m_travelLimit)
         m_pos = m_travelLimit;
   }

   // the travel limit applies to one displacement update only - reset it
   m_travelLimit = m_frameEnd;

   // fire an Start/End of Stroke events, as appropriate
   const float strokeEventLimit = m_frameLen / 50.0f;
   const float strokeEventHysteresis = strokeEventLimit*2.0f;
   if (m_fStrokeEventsArmed && m_pos + dx > m_frameStart - strokeEventLimit)
   {
      m_plunger->FireVoidEventParm(DISPID_LimitEvents_BOS, fabsf(m_speed));
      m_fStrokeEventsArmed = false;
   }
   else if (m_fStrokeEventsArmed && m_pos + dx < m_frameEnd + strokeEventLimit)
   {
      m_plunger->FireVoidEventParm(DISPID_LimitEvents_EOS, fabsf(m_speed));
      m_fStrokeEventsArmed = false;
   }
   else if (m_pos > m_frameEnd + strokeEventHysteresis
      && m_pos < m_frameStart - strokeEventHysteresis)
   {
      // away from the limits - arm the stroke events
      m_fStrokeEventsArmed = true;
   }

   // update the display
   SetObjects(m_pos);
}

void PlungerMoverObject::PullBack(float speed)
{
   // start the pull by applying the artificial "pull force"
   m_speed = 0.0f;
   m_pullForce = speed;
}

void PlungerMoverObject::Fire(float startPos)
{
   // cancel any pull force
   m_pullForce = 0.0f;

   // make sure the starting point is behind the park position
   if (startPos < m_restPos)
      startPos = m_restPos;

   // move immediately to the starting position
   m_pos = m_frameEnd + (startPos * m_frameLen);

   // Figure the release speed as a fraction of the
   // fire speed property, linearly proportional to the
   // starting distance.  Note that the release motion
   // is upwards, so the speed is negative.
   const float dx = startPos - m_restPos;
   const float normalize = g_pplayer->m_ptable->m_plungerNormalize / 13.0f / 100.0f;
   m_fireSpeed = -m_plunger->m_d.m_speedFire
      * dx * m_frameLen / m_mass
      * normalize;

   // Figure the target stopping position for the
   // bounce off of the barrel spring.  Treat this
   // as proportional to the pull distance, but max
   // out (i.e., go all the way to the forward travel
   // limit, position 0.0) if the pull position is
   // more than about halfway.
   const float maxPull = .5f;
   const float bounceDist = (dx < maxPull ? dx / maxPull : 1.0f);

   // the initial bounce will be negative, since we're moving upwards,
   // and we calculated it as a fraction of the forward travel distance
   // (which is the part between 0 and the rest position)
   m_fireBounce = -bounceDist * m_restPos;

   // enter Fire mode for long enough for the process to complete
   m_fireTimer = 200;
}

void PlungerMoverObject::UpdateVelocities()
{
   // figure our current position in relative coordinates (0.0-1.0,
   // where 0.0 is the maximum forward position and 1.0 is the
   // maximum retracted position)
   const float pos = (m_pos - m_frameEnd) / m_frameLen;

   // If "mech plunger" is enabled, read the mechanical plunger
   // position; otherwise treat it as fixed at 0.
   const float mech = m_plunger->m_d.m_mechPlunger ? MechPlunger() : 0.0f;

   // calculate the delta from the last reading
   const float dmech = m_mech0 - mech;

   // Frame-to-frame mech movement threshold for detecting a release
   // motion.  1.0 is the full range of travel, which corresponds
   // to about 3" on a standard pinball plunger.  We want to choose
   // the value here so that it's faster than the player is likely
   // to move the plunger manually, but slower than the plunger
   // typically moves under spring power when released.  It appears
   // from observation that a real plunger moves at something on the
   // order of 3 m/s.  Figure the fastest USB update interval will
   // be 10ms, typical is probably 25ms, and slowest is maybe 40ms;
   // and figure the bracket speed range down to about 1 m/s.  This
   // gives us a distance per USB interval of from 25mm to 100mm.
   // 25mm translates to .32 of our distance units (0.0-1.0 scale).
   // The lower we make this, the more sensitive we'll be at
   // detecting releases, but if we make it too low we might mistake
   // manual movements for releases.  In practice, it seems safe to
   // lower it to about 0.2 - this doesn't seem to cause false
   // positives and seems reliable at identifying actual releases.
   const float ReleaseThreshold = 0.2f;

   // note if we're acting as an auto plunger
   const int autoPlunger = m_plunger->m_d.m_autoPlunger;

   // check which forces are acting on us
   if (m_fireTimer > 0)
   {
      // Fire mode.  In this mode, we're moving freely under the spring
      // forces at the speed we calculated when we initiated the release.
      // Simply leave the speed unchanged.
      // 
      // Decrement the release mode timer.  The mode ends after the
      // timeout elapses, even if the mech plunger hasn't actually
      // come to rest.  This ensures that we don't get stuck in this
      // mode, and also allows us to sync up again with the real
      // plunger after a respectable pause if the user is just
      // moving it around a lot.
      m_speed = m_fireSpeed;
      --m_fireTimer;
   }
   else if (m_autoFireTimer > 0)
   {
      // The Auto Fire timer is running.  We start this timer when we
      // send a synthetic KeyDown(Return) event to the script to simulate
      // a Launch Ball event when the user pulls back and releases the
      // mechanical plunger and we're operating as an auto plunger.
      // When the timer reaches zero, we'll send the corresponding
      // KeyUp event and cancel the timer.
      if (--m_autoFireTimer == 0)
      {
         if (g_pplayer != 0)
         {
            g_pplayer->m_ptable->FireKeyEvent(
               DISPID_GameEvents_KeyUp,
               g_pplayer->m_rgKeys[ePlungerKey]);
         }
      }
   }
   else if (autoPlunger && dmech > ReleaseThreshold)
   {
      // Release motion detected in Auto Plunger mode.
      //
      // If we're acting as an auto plunger, and the player performs
      // a pull-and-release motion on the mechanical plunger, simulate
      // a Launch Ball event.
      //
      // An Auto Plunger simulates a solenoid-driven ball launcher
      // on a table like Medieval Madness.  On this type of game,
      // the original machine doesn't have a spring-loaded plunger.
      // for the user to operate manually.  The user-operated control
      // is instead a button of some kind (the physical form varies
      // quite a bit, from big round pushbuttons to gun triggers to
      // levers to rotating knobs, but they all amount to momentary
      // on/off switches in different guises).  But on virtual
      // cabinets, the mechanical plunger doesn't just magically
      // disappear when you load Medieval Madness!  So the idea here
      // is that we can use a mech plunger to simulate a button.
      // It's pretty simple and natural: you just perform the normal
      // action that you're accustomed to doing with a plunger,
      // namely pulling it back and letting it go.  The software
      // observes this gesture, and rather than trying to simulate
      // the motion directly on the software plunger, we simply 
      // turn it into a synthetic Launch Ball keyboard event.  This
      // amounts to sending a KeyDown(Return) message to the script,
      // followed a short time later by a KeyUp(Return).  The script
      // will then act exactly like it would if the user had actually
      // pressed the Return key (or, equivalently on a cabinet, the
      // Launch Ball button).

      // Send a KeyDown(Return) to the table script.  This
      // will allow the script to set ROM switch levels or
      // perform any other tasks it normally does when the
      // actual Launch Ball button is pressed.
      if (g_pplayer != 0)
      {
         g_pplayer->m_ptable->FireKeyEvent(
            DISPID_GameEvents_KeyDown,
            g_pplayer->m_rgKeys[ePlungerKey]);
      }

      // start the timer to send the corresponding KeyUp in 100ms
      m_autoFireTimer = 101;
   }
   else if (m_pullForce != 0.0f)
   {
      // A "pull" force is in effect.  This is a *simulated* pull, so
      // it overrides the real physical plunger position.
      //
      // Simply update the model speed by applying the accleration
      // due to the pull force.
      //
      // Force = mass*acceleration -> a = F/m.  Increase the speed
      // by the acceleration.  Technically we're calculating dv = a dt,
      // but we can elide the elapsed time factor because it's
      // effectively a constant that's implicitly folded into the
      // pull force value.
      m_speed += m_pullForce / m_mass;

      // if we're already at the maximum retracted position, stop
      if (m_pos >= m_frameStart)
         m_speed = 0.0f;
   }
   else if (dmech > ReleaseThreshold)
   {
      // Normal mode, fast forward motion detected.  Consider this
      // to be a release event.
      //
      // The release motion of a physical plunger is much faster
      // than our sampling rate can keep up with, so we can't just
      // use the joystick readings directly.  The problem is that a
      // real plunger can shoot all the way forward, bounce all the
      // way back, and shoot forward again in the time between two
      // consecutive samples.  A real plunger moves at around 3-5m/s,
      // which translates to 3-5mm/ms, or 30-50mm per 10ms sampling
      // period.  The whole plunger travel distance is ~65mm.
      // So in one reading, we can travel almost the whole range!
      // This means that samples are effectively random during a
      // release motion.  We might happen to get lucky and have
      // our sample timing align perfectly with a release, so that
      // we get one reading at the retracted position just before
      // a release and the very next reading at the full forward
      // position.  Or we might get unlikely and catch one reading
      // halfway down the initial initial lunge and the next reading
      // at the very apex of the bounce back - and if we took those
      // two readings at face value, we'd be fooled into thinking
      // the plunger was stationary at the halfway point!
      //
      // But there's hope.  A real plunger's barrel spring is pretty
      // inelastic, so the rebounds after a release damp out quickly.  
      // Observationally, each bounce bounces back to less than half
      // of the previous one.  So even with the worst-case aliasing,
      // we can be confident that we'll see a declining trend in the
      // samples during a release-bounce-bounce-bounce sequence.
      //
      // Our detection strategy is simply to consider any rapid
      // forward motion to be a release.  If we see the plunger move
      // forward by more than the threshold distance, we'll consider
      // it a release.  See the comments above for how we chose the
      // threshold value.

      // Go back through the recent history to find the apex of the
      // release.  Our "threshold" calculation is basically attempting
      // to measure the instantaneous speed of the plunger as the
      // difference in position divided by the time interval.  But
      // the time interval is extremely imprecise, because joystick
      // reports aren't synchronized to our clock.  In practice the
      // time between USB reports is in the 10-30ms range, which gives
      // us a considerable range of error in calculating an instantaneous
      // speed.
      //
      // So instead of relying on the instantaneous speed alone, now
      // that we're pretty sure a release motion is under way, go back
      // through our recent history to find out where it really
      // started.  Scan the history for monotonically ascending values,
      // and take the highest one we find.  That's probably where the
      // user actually released the plunger.
      float apex = m_mech0;
      if (m_mech1 > apex)
      {
         apex = m_mech1;
         if (m_mech2 > apex)
            apex = m_mech2;
      }

      // trigger a release from the apex position
      Fire(apex);
   }
   else
   {
      // Normal mode, and NOT firing the plunger.  In this mode, we
      // simply want to make the on-screen plunger sync up with the
      // position of the physical plunger.
      //
      // This isn't as simple as just setting the software plunger's
      // position to magically match that of the physical plunger.  If
      // we did that, we'd break the simulation by making the software
      // plunger move at infinite speed.  This wouldn't rip the fabric
      // of space-time or anything that dire, but it *would* prevent
      // the collision detection code from working properly.
      //
      // So instead, sync up the positions by setting the software
      // plunger in motion on a course for syncing up with the
      // physical plunger, as fast as we can while maintaining a
      // realistic speed in the simulation.

      // for a normal plunger, sync to the mech plunger; otherwise
      // just go to the rest position
      const float target = autoPlunger ? m_restPos : mech;

      // figure the current difference in positions
      const float error = target - pos;

      // Model the software plunger as though it were connected to the
      // mechanical plunger by a spring with spring constant 'mech
      // strength'.  The force from a stretched spring is -kx (spring
      // constant times displacement); in this case, the displacement
      // is the distance between the physical and virtual plunger tip
      // positions ('error').  The force from an acceleration is ma,
      // so the acceleration from the spring force is -kx/m.  Apply
      // this acceleration to the current plunger speed.  While we're
      // at it, apply some damping to the current speed to simulate
      // friction.
      //
      // The 'normalize' factor is the table's normalization constant
      // divided by 1300, for historical reasons.  Old versions applied
      // a 1/13 adjustment factor, which appears to have been empirically
      // chosen to get the speed in the right range.  The m_plungerNormalize
      // factor has default value 100 in this version, so we need to
      // divide it by 100 to get a multipler value.
      //
      // The 'dt' factor represents the amount of time that we're applying
      // this acceleration.  This is in "VP 9 physics frame" units, where
      // 1.0 equals the amount of real time in one VP 9 physics frame.
      // The other normalization factors were originally chosen for VP 9
      // timing, so we need to adjust for the new VP 10 time base.  VP 10
      // runs physics frames at roughly 10x the rate of VP 9, so the time
      // per frame is about 1/10 the VP 9 time.
      const float plungerFriction = 0.95f;
      const float normalize = g_pplayer->m_ptable->m_plungerNormalize / 13.0f / 100.0f;
      const float dt = 0.1f;
      m_speed *= plungerFriction;
      m_speed += error * m_frameLen
         * m_plunger->m_d.m_mechStrength / m_mass
         * normalize * dt;

      // add any reverse impulse to the result
      m_speed += m_reverseImpulse;
   }

   // cancel any reverse impulse
   m_reverseImpulse = 0.0f;

   // Shift the current mech reading into the history list, if it's
   // different from the last reading.  Only keep distinct readings;
   // the physics loop tends to run faster than the USB reporting
   // rate, so we might see the same USB report several times here.
   if (mech != m_mech0)
   {
      m_mech2 = m_mech1;
      m_mech1 = m_mech0;
      m_mech0 = mech;
   }
}

float HitPlunger::HitTest(const Ball * pball_, const float dtime, CollisionEvent& coll) const
{
   Ball * const pball = const_cast<Ball*>(pball_);   // HACK; needed below // evil cast to non-const, but not so expensive as constructor for full copy (and avoids screwing with the ball IDs)

   float hittime = dtime; //start time
   bool fHit = false;

   // If we got here, then the ball is close enough to the plunger
   // to where we should animate the button's light.
   // Save the time so we can tell the button when to turn on/off.  
   g_pplayer->m_LastPlungerHit = g_pplayer->m_time_msec;

   // We are close enable the plunger light.
   CollisionEvent hit;
   float newtime;

   // Check for hits on the non-moving parts, like the side of back
   // of the plunger.  These are just like hitting a wall.
   // Check all and find the nearest collision.

   newtime = m_plungerMover.m_linesegBase.HitTest(pball, dtime, hit);
   if (newtime >= 0.f && newtime <= hittime)
   {
      fHit = true;
      hittime = newtime;
      coll = hit;
      coll.m_hitvel.x = 0.f;
      coll.m_hitvel.y = 0.f;
   }

   for (int i = 0; i < 2; i++)
   {
      newtime = m_plungerMover.m_linesegSide[i].HitTest(pball, hittime, hit);
      if (newtime >= 0.f && newtime <= hittime)
      {
         fHit = true;
         hittime = newtime;
         coll = hit;
         coll.m_hitvel.x = 0.f;
         coll.m_hitvel.y = 0.f;
      }

      newtime = m_plungerMover.m_jointBase[i].HitTest(pball, hittime, hit);
      if (newtime >= 0.f && newtime <= hittime)
      {
         fHit = true;
         hittime = newtime;
         coll = hit;
         coll.m_hitvel.x = 0.f;
         coll.m_hitvel.y = 0.f;
      }
   }

   // Now check for hits on the business end, which might be moving.
   //
   // Our line segments are static, but they're meant to model a moving
   // object (the tip of the plunger).  We need to include the motion of
   // the tip to know if there's going to be a collision within the 
   // interval we're covering, since it's not going to stay in the same
   // place throughout the interval.  Use a little physics trick: do the
   // calculation in an inertial frame where the tip is stationary.  To
   // do this, just adjust the ball speed to what it looks like in the
   // tip's rest frame.
   //
   // Note that we're about to cast the const Ball* to non-const and change
   // the object's contents.  This is bad practice, but we do it intentionally,
   // for speed.  Past versions made a local copy of the Ball instance, which 
   // is technically the correct way to do this, but it takes longer and
   // messes with the ball IDs.  Before changing the speed value in the
   // (nominally) const Ball instance, save the old value so that we can
   // restore it when we're done.
   const float oldvely = pball->m_vel.y;   // save the old velocity value
   pball->m_vel.y -= m_plungerMover.m_speed;     // WARNING! EVIL OVERRIDE OF CONST INSTANCE POINTER!!!

   // Figure the impulse from hitting the moving end.
   // Calculate this as the product of the plunger speed and the
   // momentum transfer factor, which essentially models the plunger's
   // mass in abstract units.  In practical terms, this lets table
   // authors fine-tune the plunger's strength in terms of the amount
   // of energy it transfers when striking a ball.  Note that table
   // authors can also adjust the strength via the release speed,
   // but that's also used for the visual animation, so it's not
   // always possible to get the right combination of visuals and
   // physics purely by adjusting the speed.  The momentum transfer
   // factor provides a way to tweak the physics without affecting
   // the visuals.
   //
   // Further adjust the transfered momentum by the ball's mass
   // (which is likewise in abstract units).  Divide by the ball's
   // mass, since a heavier ball will have less velocity transfered
   // for a given amount of momentum (p=mv -> v=p/m).
   //
   // Note that both the plunger momentum transfer factor and the
   // ball's mass are expressed in relative units, where 1.0f is
   // the baseline and default.  Older tables that were designed
   // before these properties existed won't be affected since we'll
   // multiply the legacy calculation by 1.0/1.0 == 1.0.  (Set an
   // arbitrary lower bound to prevent division by zero and/or crazy
   // physics.)
   const float ballMass = (pball->m_mass > 0.05f ? pball->m_mass : 0.05f);
   const float xferRatio = m_pplunger->m_d.m_momentumXfer / ballMass;
   const float deltay = m_plungerMover.m_speed * xferRatio;

   // check the moving bits
   newtime = m_plungerMover.m_linesegEnd.HitTest(pball, hittime, hit);
   if (newtime >= 0.f && newtime <= hittime)
   {
      fHit = true;
      hittime = newtime;
      coll = hit;
      coll.m_hitvel.x = 0.f;
      coll.m_hitvel.y = deltay;
   }

   for (int i = 0; i < 2; i++)
   {
      newtime = m_plungerMover.m_jointEnd[i].HitTest(pball, hittime, hit);
      if (newtime >= 0.f && newtime <= hittime)
      {
         fHit = true;
         hittime = newtime;
         coll = hit;
         coll.m_hitvel.x = 0.f;
         coll.m_hitvel.y = deltay;
      }
   }

   // restore the original ball velocity (WARNING! CONST POINTER OVERRIDE!)
   pball->m_vel.y = oldvely;

   // check for a hit
   if (fHit)
   {
      // We hit the ball.  Set a travel limit to freeze the plunger at
      // its current position for the next displacement update.  This
      // is necessary in case we have a relatively heavy ball with a
      // relatively light plunger, in which case the ball won't speed
      // up to the plunger's current speed.  Freezing the plunger here
      // prevents the plunger from overtaking the ball.  This serves
      // two purposes, one physically meaningful and the other a bit of
      // a hack for the physics loop.  The physical situation is that we
      // have a slow-moving ball blocking a fast-moving plunger; this
      // momentary travel limit effectively models the blockage.  The
      // hack is that the physics loop can't handle a situation where
      // a moving object is in continuous contact with the ball.  The
      // physics loop is written so that time only advances as far as
      // the next collision.  This means that the loop will get stuck
      // if two objects remain in continuous contact, because the time
      // to the next collision will be exactly 0.0 as long as the contact
      // continues.  We *have* to break the contact for time to progress
      // in the loop.  This has never been a problem for other objects
      // because other collisions always impart enough momentum to send
      // the colliding objects on their separate ways.  With a low
      // momentum transfer ratio in the plunger, though, we can find
      // ourselves pushing the ball along, with the spring keeping the
      // plunger pressed against the ball the whole way.  The plunger
      // freeze here deals with this by breaking contact for just long
      // enough to let the ball move a little bit, so that there's a
      // non-zero time to the next collision with the plunger.  We'll
      // then catch up again and push it along a little further.
      if (m_plungerMover.m_travelLimit < m_plungerMover.m_pos)
         (const_cast<HitPlunger*>(this))->m_plungerMover.m_travelLimit = m_plungerMover.m_pos; // HACK

      // If the distance is negative, it means the objects are
      // overlapping.  Make certain that we give the ball enough
      // of an impulse to get it not to overlap.
      if (coll.m_hitdistance <= 0.0f
         && coll.m_hitvel.y == deltay
         && fabsf(deltay) < fabsf(coll.m_hitdistance))
         coll.m_hitvel.y = -fabsf(coll.m_hitdistance);

      // return the collision time delta
      return hittime;
   }
   else
   {
      // no collision
      return -1.0f;
   }
}

void HitPlunger::Collide(CollisionEvent& coll)
{
   Ball * const pball = coll.m_ball;

   float dot = (pball->m_vel.x - coll.m_hitvel.x)* coll.m_hitnormal.x + (pball->m_vel.y - coll.m_hitvel.y) * coll.m_hitnormal.y;

   if (dot >= -C_LOWNORMVEL)             // nearly receding ... make sure of conditions
   {                                     // otherwise if clearly approaching .. process the collision
      if (dot > C_LOWNORMVEL) return;    // is this velocity clearly receding (i.e must > a minimum)
#ifdef C_EMBEDDED
      if (coll.m_hitdistance < -C_EMBEDDED)
         dot = -C_EMBEDSHOT;             // has ball become embedded???, give it a kick
      else
         return;
#endif
   }

#ifdef C_DISP_GAIN 
   // correct displacements, mostly from low velocity blidness, an alternative to true acceleration processing     
   float hdist = -C_DISP_GAIN * coll.m_hitdistance;         // distance found in hit detection
   if (hdist > 1.0e-4f)
   {                                                                                               // magnitude of jump
      if (hdist > C_DISP_LIMIT)
      {
         hdist = C_DISP_LIMIT;
      }                                         // crossing ramps, delta noise
      pball->m_pos += hdist * coll.m_hitnormal;    // push along norm, back to free area (use the norm, but is not correct)
   }
#endif

   // figure the basic impulse
   const float impulse = dot * -1.45f / (1.0f + 1.0f / m_plungerMover.m_mass);

   // We hit the ball, so attenuate any plunger bounce we have queued up
   // for a Fire event.  Real plungers bounce quite a bit when fired without
   // hitting anything, but bounce much less when they hit something, since
   // most of the momentum gets transfered out of the plunger and to the ball.
   m_plungerMover.m_fireBounce *= 0.6f;

   // Check for a downward collision with the tip.  This is the moving
   // part of the plunger, so it has some special handling.
   if (coll.m_hitvel.y != 0.0f)
   {
      // The tip hit the ball (or vice versa).
      //
      // Figure the reverse impulse to the plunger.  If the ball was moving
      // and the plunger wasn't, a little of the ball's momentum should
      // transfer to the plunger.  (Ideally this would just fall out of the
      // momentum calculations organically, the way it works in real life,
      // but our physics are pretty fake here.  So we add a bit to the
      // fakeness here to make it at least look a little more realistic.)
      //
      // Figure the reverse impulse as the dot product times the ball's
      // y velocity, multiplied by the ratio between the ball's collision
      // mass and the plunger's nominal mass.  In practice this is *almost*
      // satisfyingly realistic, but the bump seems a little too big.  So
      // apply a fudge factor to make it look more real.  The fudge factor
      // isn't entirely unreasonable physically - you could look at it as
      // accounting for the spring tension and friction.
      const float reverseImpulseFudgeFactor = .22f;
      m_plungerMover.m_reverseImpulse = pball->m_vel.y * impulse
         * (pball->m_mass / m_plungerMover.m_mass)
         * reverseImpulseFudgeFactor;
   }

   // update the ball speed for the impulse
   pball->m_vel += impulse * coll.m_hitnormal;

   pball->m_vel *= 0.999f;           //friction all axiz     //!! TODO: fix this

   const float scatter_vel = m_plungerMover.m_scatterVelocity * g_pplayer->m_ptable->m_globalDifficulty;// apply dificulty weighting

   if (scatter_vel > 0.f && fabsf(pball->m_vel.y) > scatter_vel) //skip if low velocity 
   {
      float scatter = rand_mt_m11();                                                          // -1.0f..1.0f
      scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_vel;     // shape quadratic distribution and scale
      pball->m_vel.y += scatter;
   }

#ifdef C_DYNAMIC
   pball->m_dynamic = C_DYNAMIC;
#endif
}
