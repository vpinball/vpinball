// license:GPLv3+

#include <algorithm>
#include <cmath>

#include "B2SProPhysics.h"

namespace B2S {

static constexpr float ShallowSurfaceSlopeLimit = 0.35f;
static constexpr float ShallowSurfaceRollMultiplier = 2.f;
static constexpr float BoundaryScatterDegrees = 10.f;
static constexpr float BoundaryScatterMinimumImpactSpeed = 150.f;
static constexpr double Pi = 3.14159265358979323846;

static float NormalizeSwitchAngle(float angle)
{
   float normalized = std::fmod(angle, 360.f);
   if (normalized > 180.f)
      normalized -= 360.f;
   if (normalized <= -180.f)
      normalized += 360.f;
   return normalized;
}

B2SProPhysicsBall::B2SProPhysicsBall(B2SBulb& ball, B2SBulb* flipper)
   : m_ball(ball)
   , m_flipper(ball.m_pro.motionPathGravityDrop ? nullptr : flipper) // Gravity trough balls have no flipper or launcher
   , m_bounds(ball.m_pro.physicsBounds)
   , m_gravity(std::clamp(ball.m_pro.physicsGravity, 0.f, 10000.f))
   , m_flipperStrength(std::clamp(ball.m_pro.physicsFlipperStrength, 0.f, 5.f))
   , m_boundaryBounce(std::clamp(ball.m_pro.physicsBoundaryBounce, 0.f, 1.f))
   , m_hasLauncher(ball.m_pro.physicsLauncher.enabled && !ball.m_pro.motionPathGravityDrop)
   , m_launcher(ball.m_pro.physicsLauncher)
   , m_ballWidth(static_cast<float>(ball.m_width))
   , m_ballHeight(static_cast<float>(ball.m_height))
   , m_authoredCenter { static_cast<float>(ball.m_locationX) + static_cast<float>(ball.m_width) * 0.5f, static_cast<float>(ball.m_locationY) + static_cast<float>(ball.m_height) * 0.5f }
{
   const B2SProBulb& pro = ball.m_pro;
   for (size_t pathIndex = 0; pathIndex < pro.physicsBoundaries.size(); pathIndex++)
   {
      const vector<B2SProPoint>& path = pro.physicsBoundaries[pathIndex];
      if (path.size() < 2)
         continue;
      m_boundaryPaths.push_back(path);
      vector<float> values;
      const vector<float>* saved = pathIndex < pro.physicsBoundarySegmentBounces.size() ? &pro.physicsBoundarySegmentBounces[pathIndex] : nullptr;
      for (size_t segment = 0; segment + 1 < path.size(); segment++)
      {
         const float value = (saved != nullptr && segment < saved->size()) ? (*saved)[segment] : -1.f;
         values.push_back(std::clamp(value, -1.f, 3.f));
      }
      m_boundarySegmentBounces.push_back(std::move(values));
   }
   for (size_t index = 0; index < pro.physicsObstacles.size(); index++)
   {
      const B2SProRect& obstacle = pro.physicsObstacles[index];
      if (obstacle.w <= 0.f || obstacle.h <= 0.f)
         continue;
      m_obstacles.push_back(obstacle);
      const float value = index < pro.physicsObstacleBounces.size() ? pro.physicsObstacleBounces[index] : -1.f;
      m_obstacleBounces.push_back((std::isnan(value) || std::isinf(value) || value < 0.f) ? m_boundaryBounce : std::min(3.f, value));
   }
   for (const B2SProBulb::SwitchZone& zone : pro.physicsSwitchZones)
   {
      if (zone.bounds.w <= 0.f || zone.bounds.h <= 0.f || zone.switchId <= 0)
         continue;
      B2SProBulb::SwitchZone copy = zone;
      copy.angle = NormalizeSwitchAngle(zone.angle);
      m_switchZones.push_back(copy);
      m_switchZoneInside.push_back(false);
   }
   if (m_hasLauncher)
   {
      m_launcher.captureRadius = std::max(5.f, m_launcher.captureRadius);
      m_launcherHolding = true;
   }
   m_center = m_authoredCenter;
   m_presented = m_center;
}

B2SProPhysicsBall::~B2SProPhysicsBall()
{
   DeleteTexture(m_frameTexture);
}

void B2SProPhysicsBall::Start()
{
   m_active = true;
   m_ball.m_brightness = 1.f;
}

void B2SProPhysicsBall::LoadMask()
{
   // The mask was decoded with the other images on the loader thread; only its pixels are looked up here
   m_maskLoaded = true;
   if (m_ball.m_pro.physicsSelectionMask == nullptr)
      return;
   const VPXTextureInfo* const info = GetTextureInfo(m_ball.m_pro.physicsSelectionMask);
   if (info == nullptr || info->data == nullptr || info->width == 0 || info->height == 0)
      return;
   // Only an RGBA mask carries alpha; other formats hide nothing
   if (info->format != VPXTEXFMT_sRGBA8)
      return;
   m_maskPixels = static_cast<const uint8_t*>(info->data);
   m_maskBpp = 4;
   m_maskWidth = static_cast<int>(info->width);
   m_maskHeight = static_cast<int>(info->height);
}

VPXTexture B2SProPhysicsBall::MaskedFrame(float& x, float& y, float& w, float& h)
{
   if (!m_maskLoaded)
      LoadMask();
   if (m_maskPixels == nullptr || m_ball.m_image == nullptr || !m_ball.m_proRt.posValid)
      return nullptr;
   const B2SBulb::ProRuntime& rt = m_ball.m_proRt;
   if (!m_frameValid || rt.posX != m_framePosX || rt.posY != m_framePosY || rt.rollAngle != m_frameAngle)
   {
      const VPXTextureInfo* const src = GetTextureInfo(m_ball.m_image);
      if (src == nullptr || src->data == nullptr || src->width == 0 || src->height == 0 || (src->format != VPXTEXFMT_sRGBA8 && src->format != VPXTEXFMT_sRGB8))
         return nullptr;
      const int bpp = src->format == VPXTEXFMT_sRGBA8 ? 4 : 3;
      const uint8_t* const pixels = static_cast<const uint8_t*>(src->data);
      // Axis aligned bounds of the ball rectangle turned by the rolling angle around its centre
      const double radians = static_cast<double>(rt.rollAngle) * Pi / 180.0;
      const double cosine = std::cos(radians), sine = std::sin(radians);
      const double halfW = m_ballWidth * 0.5, halfH = m_ballHeight * 0.5;
      const double extentX = std::abs(halfW * cosine) + std::abs(halfH * sine);
      const double extentY = std::abs(halfW * sine) + std::abs(halfH * cosine);
      const int left = static_cast<int>(std::floor(rt.posX - extentX));
      const int top = static_cast<int>(std::floor(rt.posY - extentY));
      const int right = static_cast<int>(std::ceil(rt.posX + extentX));
      const int bottom = static_cast<int>(std::ceil(rt.posY + extentY));
      m_frameW = std::max(1, right - left);
      m_frameH = std::max(1, bottom - top);
      m_frameX = static_cast<float>(left);
      m_frameY = static_cast<float>(top);
      m_frameBuffer.assign(static_cast<size_t>(m_frameW) * m_frameH * 4, 0);
      // Source pixels per authored unit (the artwork may be a higher resolution than its rectangle)
      const double scaleU = static_cast<double>(src->width) / m_ballWidth;
      const double scaleV = static_cast<double>(src->height) / m_ballHeight;
      const int sw = static_cast<int>(src->width), sh = static_cast<int>(src->height);
      for (int py = 0; py < m_frameH; py++)
      {
         const int maskY = top + py;
         for (int px = 0; px < m_frameW; px++)
         {
            const int maskX = left + px;
            const int mask = (maskX >= 0 && maskY >= 0 && maskX < m_maskWidth && maskY < m_maskHeight) ? m_maskPixels[(static_cast<size_t>(maskY) * m_maskWidth + maskX) * m_maskBpp + 3] : 0;
            if (mask == 0)
               continue;
            // Inverse rotation of the pixel centre into the unrotated ball rectangle, then into source pixels
            const double dx = (maskX + 0.5) - rt.posX, dy = (maskY + 0.5) - rt.posY;
            const double ux = dx * cosine + dy * sine + halfW;
            const double uy = -dx * sine + dy * cosine + halfH;
            if (ux < 0.0 || uy < 0.0 || ux >= m_ballWidth || uy >= m_ballHeight)
               continue;
            const double sx = ux * scaleU - 0.5, sy = uy * scaleV - 0.5;
            const int x0 = static_cast<int>(std::floor(sx)), y0 = static_cast<int>(std::floor(sy));
            const double fx = sx - x0, fy = sy - y0;
            double acc[4] = { 0.0, 0.0, 0.0, 0.0 };
            for (int j = 0; j < 2; j++)
               for (int i = 0; i < 2; i++)
               {
                  const int sxi = std::clamp(x0 + i, 0, sw - 1), syi = std::clamp(y0 + j, 0, sh - 1);
                  const double weight = (i == 0 ? 1.0 - fx : fx) * (j == 0 ? 1.0 - fy : fy);
                  const uint8_t* const p = pixels + (static_cast<size_t>(syi) * sw + sxi) * bpp;
                  acc[0] += weight * p[0];
                  acc[1] += weight * p[1];
                  acc[2] += weight * p[2];
                  acc[3] += weight * (bpp == 4 ? p[3] : 255);
               }
            uint8_t* const dst = m_frameBuffer.data() + (static_cast<size_t>(py) * m_frameW + px) * 4;
            dst[0] = static_cast<uint8_t>(std::clamp(acc[0] + 0.5, 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(acc[1] + 0.5, 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(acc[2] + 0.5, 0.0, 255.0));
            dst[3] = static_cast<uint8_t>((static_cast<int>(std::clamp(acc[3] + 0.5, 0.0, 255.0)) * mask + 127) / 255);
         }
      }
      UpdateTexture(&m_frameTexture, m_frameW, m_frameH, VPXTEXFMT_sRGBA8, m_frameBuffer.data());
      m_framePosX = rt.posX;
      m_framePosY = rt.posY;
      m_frameAngle = rt.rollAngle;
      m_frameValid = m_frameTexture != nullptr;
   }
   if (!m_frameValid)
      return nullptr;
   x = m_frameX;
   y = m_frameY;
   w = static_cast<float>(m_frameW);
   h = static_cast<float>(m_frameH);
   return m_frameTexture;
}

void B2SProPhysicsBall::ActivateAtCurrentPosition()
{
   const B2SProPoint center = m_ball.ProMotionCenter();
   m_center = center;
   m_presented = center;
   m_velocity = {};
   m_launcherHolding = false;
   m_launcherArmed = false;
   m_hasLastFlipperAngle = false;
   m_active = true;
   m_ball.m_brightness = 1.f;
   m_ball.m_proRt.posX = center.x;
   m_ball.m_proRt.posY = center.y;
   m_ball.m_proRt.posValid = true;
}

void B2SProPhysicsBall::DeactivateForMotionPath()
{
   m_active = false;
   m_velocity = {};
   m_hasLastFlipperAngle = false;
   m_launcherHolding = false;
   m_launcherArmed = false;
}

void B2SProPhysicsBall::Launch()
{
   if (!m_hasLauncher || !m_launcherArmed)
      return;
   const float randomAngleOffset = (Random() * 2.f - 1.f) * m_launcher.randomAngle;
   const float randomStrengthFactor = 1.f + (Random() * 2.f - 1.f) * m_launcher.randomStrength / 100.f;
   B2SProPoint origin;
   float angle = 0.f;
   GetLauncherPose(origin, angle);
   const double radians = static_cast<double>(angle + randomAngleOffset) * Pi / 180.0;
   const float strength = std::max(0.f, m_launcher.strength * randomStrengthFactor);
   m_center = origin;
   m_presented = origin;
   m_velocity = { static_cast<float>(std::cos(radians)) * strength, static_cast<float>(std::sin(radians)) * strength };
   m_launcherArmed = false;
   m_launcherHolding = false;
   m_launcherExitedCapture = false;
   std::fill(m_switchZoneInside.begin(), m_switchZoneInside.end(), false);
}

void B2SProPhysicsBall::Step(const float dt)
{
   if (!m_active || m_ballWidth <= 0.f || m_ballHeight <= 0.f)
      return;
   float angularVelocity = 0.f;
   if (m_flipper != nullptr)
   {
      const float angle = m_flipper->m_proRt.angle;
      if (m_hasLastFlipperAngle)
         angularVelocity = static_cast<float>(static_cast<double>(angle - m_lastFlipperAngle) * Pi / 180.0 / static_cast<double>(dt));
      m_lastFlipperAngle = angle;
      m_hasLastFlipperAngle = true;
   }
   StepPhysics(dt, angularVelocity);
}

void B2SProPhysicsBall::Present()
{
   if (!m_active)
      return;
   B2SBulb::ProRuntime& rt = m_ball.m_proRt;
   // Rolling: the artwork turns by the travelled distance over its radius (AdvanceMotionPathRoll)
   if (m_ball.m_pro.motionPathRollEnabled)
   {
      const float dx = m_center.x - m_presented.x;
      const float dy = m_center.y - m_presented.y;
      const double distance = std::sqrt(static_cast<double>(dx) * dx + static_cast<double>(dy) * dy);
      if (distance > 0.0001)
      {
         const float direction = std::abs(dx) >= std::abs(dy) ? (dx > 0.f ? 1.f : (dx < 0.f ? -1.f : 0.f)) : (dy > 0.f ? 1.f : (dy < 0.f ? -1.f : 0.f));
         if (direction != 0.f)
         {
            const float radius = std::max(1.f, std::min(m_ballWidth, m_ballHeight) * 0.5f);
            rt.rollAngle = static_cast<float>(std::fmod(static_cast<double>(rt.rollAngle) + direction * distance / radius * 180.0 / Pi, 360.0));
         }
      }
   }
   m_presented = m_center;
   rt.posX = m_center.x;
   rt.posY = m_center.y;
   rt.posValid = true;
}

void B2SProPhysicsBall::StepPhysics(const float dt, const float flipperAngularVelocity)
{
   B2SProPoint center = m_center;
   if (m_launcherHolding && m_hasLauncher)
   {
      B2SProPoint holdCenter;
      float angle = 0.f;
      GetLauncherPose(holdCenter, angle);
      m_center = holdCenter;
      m_velocity = {};
      return;
   }
   const B2SProPoint previousCenter = center;
   m_velocity.y += m_gravity * dt;
   center.x += m_velocity.x * dt;
   center.y += m_velocity.y * dt;
   const float radius = std::max(2.f, std::min(m_ballWidth, m_ballHeight) * 0.43f);
   const float left = m_bounds.x, top = m_bounds.y, right = m_bounds.x + m_bounds.w, bottom = m_bounds.y + m_bounds.h;
   if (center.x - radius < left)
   {
      center.x = left + radius;
      m_velocity.x = std::abs(m_velocity.x) * 0.55f;
   }
   if (center.x + radius > right)
   {
      center.x = right - radius;
      m_velocity.x = -std::abs(m_velocity.x) * 0.55f;
   }
   if (center.y - radius < top)
   {
      center.y = top + radius;
      m_velocity.y = std::abs(m_velocity.y) * 0.55f;
   }
   if (center.y + radius > bottom)
   {
      center.y = bottom - radius;
      m_velocity.y = -std::abs(m_velocity.y) * 0.55f;
   }
   // A launcher that follows its pivot uses a plain rotating snippit, not a flipper surface
   if (m_flipper != nullptr && (!m_hasLauncher || !m_launcher.followPivot))
      ResolveFlipperCollision(center, radius, m_velocity, flipperAngularVelocity);
   for (size_t pathIndex = 0; pathIndex < m_boundaryPaths.size(); pathIndex++)
   {
      const vector<B2SProPoint>& path = m_boundaryPaths[pathIndex];
      for (size_t segment = 0; segment + 1 < path.size(); segment++)
      {
         float segmentBounce = m_boundaryBounce;
         if (pathIndex < m_boundarySegmentBounces.size() && segment < m_boundarySegmentBounces[pathIndex].size() && m_boundarySegmentBounces[pathIndex][segment] >= 0.f)
            segmentBounce = m_boundarySegmentBounces[pathIndex][segment];
         const B2SProPoint beforeCenter = center;
         const B2SProPoint beforeVelocity = m_velocity;
         ResolveStaticFloorCollision(center, previousCenter, radius, m_velocity, path[segment], path[segment + 1], segmentBounce);
         const bool resolved = std::abs(center.x - beforeCenter.x) > 0.0001f || std::abs(center.y - beforeCenter.y) > 0.0001f || std::abs(m_velocity.x - beforeVelocity.x) > 0.0001f
            || std::abs(m_velocity.y - beforeVelocity.y) > 0.0001f;
         if (resolved)
            ApplyShallowSurfaceRollingBoost(m_velocity, path[segment], path[segment + 1], dt, m_gravity);
      }
   }
   for (size_t index = 0; index < m_obstacles.size(); index++)
      ResolveCircularObstacleCollision(center, radius, m_velocity, m_obstacles[index], m_obstacleBounces[index]);
   CheckLauncherCapture(center);
   CheckSwitchZones(previousCenter, center);
   m_center = center;
}

void B2SProPhysicsBall::ApplyShallowSurfaceRollingBoost(B2SProPoint& velocity, const B2SProPoint& from, const B2SProPoint& to, const float dt, const float gravity)
{
   const float segmentX = to.x - from.x;
   const float segmentY = to.y - from.y;
   const float length = std::sqrt(segmentX * segmentX + segmentY * segmentY);
   if (length <= 0.01f)
      return;
   const float tangentX = segmentX / length;
   const float tangentY = segmentY / length;
   const float absoluteSlope = std::abs(tangentY);
   if (absoluteSlope <= 0.001f || absoluteSlope > ShallowSurfaceSlopeLimit)
      return;
   const float extraProjectedGravity = gravity * tangentY * (ShallowSurfaceRollMultiplier - 1.f);
   velocity.x += tangentX * extraProjectedGravity * dt;
   velocity.y += tangentY * extraProjectedGravity * dt;
}

void B2SProPhysicsBall::GetLauncherPose(B2SProPoint& origin, float& angle) const
{
   origin = m_launcher.origin;
   angle = m_launcher.angle;
   if (!m_launcher.followPivot || m_flipper == nullptr)
      return;
   const B2SProBulb& pivot = m_flipper->m_pro;
   const bool oscillating = pivot.pivotAutomaticOscillation || pivot.pivotTriggerType == static_cast<int>(B2SProTriggerType::Startup);
   if (oscillating)
      origin = m_authoredCenter;
   const B2SProPoint hinge { static_cast<float>(m_flipper->m_locationX) + static_cast<float>(m_flipper->m_width) * pivot.pivotX,
      static_cast<float>(m_flipper->m_locationY) + static_cast<float>(m_flipper->m_height) * pivot.pivotY };
   const float restAngle = oscillating ? 0.f : pivot.pivotDownAngle;
   const float delta = m_flipper->m_proRt.angle - restAngle;
   const double radians = static_cast<double>(delta) * Pi / 180.0;
   const float dx = origin.x - hinge.x, dy = origin.y - hinge.y;
   origin = { hinge.x + static_cast<float>(dx * std::cos(radians) - dy * std::sin(radians)), hinge.y + static_cast<float>(dx * std::sin(radians) + dy * std::cos(radians)) };
   angle += delta;
}

void B2SProPhysicsBall::CheckLauncherCapture(B2SProPoint& center)
{
   if (!m_hasLauncher || m_launcherArmed)
      return;
   B2SProPoint origin;
   float angle = 0.f;
   GetLauncherPose(origin, angle);
   const float radius = m_launcher.captureRadius;
   const float dx = center.x - origin.x;
   const float dy = center.y - origin.y;
   const bool inside = dx * dx + dy * dy <= radius * radius;
   if (!m_launcherExitedCapture)
      m_launcherExitedCapture = !inside;
   else if (inside)
   {
      center = origin;
      m_velocity = {};
      m_launcherArmed = true;
      m_launcherHolding = true;
   }
}

void B2SProPhysicsBall::CheckSwitchZones(const B2SProPoint& previousCenter, const B2SProPoint& center)
{
   for (size_t index = 0; index < m_switchZones.size(); index++)
   {
      const B2SProBulb::SwitchZone& zone = m_switchZones[index];
      const bool inside = PointInSwitchZone(center, zone);
      const bool crossed = !inside && SegmentIntersectsSwitchZone(previousCenter, center, zone);
      if (!m_switchZoneInside[index] && (inside || crossed))
         pulseSwitch(zone.switchId);
      m_switchZoneInside[index] = inside;
   }
}

bool B2SProPhysicsBall::PointInSwitchZone(const B2SProPoint& point, const B2SProBulb::SwitchZone& zone)
{
   const B2SProPoint local = SwitchZoneLocalPoint(point, zone);
   return local.x >= zone.bounds.x && local.x < zone.bounds.x + zone.bounds.w && local.y >= zone.bounds.y && local.y < zone.bounds.y + zone.bounds.h;
}

bool B2SProPhysicsBall::SegmentIntersectsSwitchZone(const B2SProPoint& from, const B2SProPoint& to, const B2SProBulb::SwitchZone& zone)
{
   const B2SProPoint localFrom = SwitchZoneLocalPoint(from, zone);
   const B2SProPoint localTo = SwitchZoneLocalPoint(to, zone);
   const auto contains = [&zone](const B2SProPoint& p) { return p.x >= zone.bounds.x && p.x < zone.bounds.x + zone.bounds.w && p.y >= zone.bounds.y && p.y < zone.bounds.y + zone.bounds.h; };
   if (contains(localFrom) || contains(localTo))
      return true;
   const double deltaX = localTo.x - localFrom.x, deltaY = localTo.y - localFrom.y;
   double first = 0.0, last = 1.0;
   return ClipSwitchSegment(-deltaX, localFrom.x - zone.bounds.x, first, last) && ClipSwitchSegment(deltaX, zone.bounds.x + zone.bounds.w - localFrom.x, first, last)
      && ClipSwitchSegment(-deltaY, localFrom.y - zone.bounds.y, first, last) && ClipSwitchSegment(deltaY, zone.bounds.y + zone.bounds.h - localFrom.y, first, last);
}

bool B2SProPhysicsBall::ClipSwitchSegment(const double direction, const double distance, double& first, double& last)
{
   if (std::abs(direction) < 0.0000001)
      return distance >= 0.0;
   const double ratio = distance / direction;
   if (direction < 0.0)
   {
      if (ratio > last)
         return false;
      if (ratio > first)
         first = ratio;
   }
   else
   {
      if (ratio < first)
         return false;
      if (ratio < last)
         last = ratio;
   }
   return true;
}

B2SProPoint B2SProPhysicsBall::SwitchZoneLocalPoint(const B2SProPoint& point, const B2SProBulb::SwitchZone& zone)
{
   const float angle = NormalizeSwitchAngle(zone.angle);
   if (std::abs(angle) < 0.001f)
      return point;
   const float centerX = zone.bounds.x + zone.bounds.w * 0.5f;
   const float centerY = zone.bounds.y + zone.bounds.h * 0.5f;
   const double radians = -static_cast<double>(angle) * Pi / 180.0;
   const double cosine = std::cos(radians), sine = std::sin(radians);
   const double x = point.x - centerX, y = point.y - centerY;
   return { static_cast<float>(centerX + x * cosine - y * sine), static_cast<float>(centerY + x * sine + y * cosine) };
}

void B2SProPhysicsBall::ResolveCircularObstacleCollision(B2SProPoint& center, const float ballRadius, B2SProPoint& velocity, const B2SProRect& obstacle, const float bounce)
{
   const float obstacleCenterX = obstacle.x + obstacle.w * 0.5f;
   const float obstacleCenterY = obstacle.y + obstacle.h * 0.5f;
   const float obstacleRadius = std::max(1.f, (obstacle.w + obstacle.h) * 0.25f);
   const float dx = center.x - obstacleCenterX;
   const float dy = center.y - obstacleCenterY;
   const float minimumDistance = ballRadius + obstacleRadius;
   const float distanceSquared = dx * dx + dy * dy;
   if (distanceSquared >= minimumDistance * minimumDistance)
      return;
   float distance = std::sqrt(distanceSquared);
   float normalX, normalY;
   if (distance < 0.001f)
   {
      normalX = 0.f;
      normalY = -1.f;
      distance = 0.f;
   }
   else
   {
      normalX = dx / distance;
      normalY = dy / distance;
   }
   center.x += normalX * (minimumDistance - distance);
   center.y += normalY * (minimumDistance - distance);
   const float normalSpeed = velocity.x * normalX + velocity.y * normalY;
   if (normalSpeed < 0.f)
   {
      velocity.x -= (1.f + bounce) * normalSpeed * normalX;
      velocity.y -= (1.f + bounce) * normalSpeed * normalY;
   }
}

void B2SProPhysicsBall::ResolveStaticFloorCollision(B2SProPoint& center, const B2SProPoint& previousCenter, const float radius, B2SProPoint& velocity, const B2SProPoint& from, const B2SProPoint& to, const float bounce)
{
   const float segmentX = to.x - from.x;
   const float segmentY = to.y - from.y;
   const float lengthSquared = segmentX * segmentX + segmentY * segmentY;
   if (lengthSquared <= 0.01f)
      return;
   const float rawFraction = ((center.x - from.x) * segmentX + (center.y - from.y) * segmentY) / lengthSquared;
   const float fraction = std::clamp(rawFraction, 0.f, 1.f);
   const float contactX = from.x + segmentX * fraction;
   const float contactY = from.y + segmentY * fraction;
   const float offsetX = center.x - contactX;
   const float offsetY = center.y - contactY;
   const float distanceSquared = offsetX * offsetX + offsetY * offsetY;
   if (distanceSquared >= radius * radius)
   {
      // A fast ball can cross the whole boundary within a step and end beyond it without overlapping:
      // sweep its path against the segment capsule before treating this as no collision
      B2SProPoint sweptCenter, sweptNormal;
      if (!TryGetSweptStaticFloorCollision(previousCenter, center, radius, from, to, sweptCenter, sweptNormal))
         return;
      center.x = sweptCenter.x + sweptNormal.x * 0.01f;
      center.y = sweptCenter.y + sweptNormal.y * 0.01f;
      const float sweptNormalSpeed = velocity.x * sweptNormal.x + velocity.y * sweptNormal.y;
      if (sweptNormalSpeed < 0.f)
         ReflectBoundaryVelocity(velocity, sweptNormal.x, sweptNormal.y, bounce, sweptNormalSpeed);
      return;
   }
   const float distance = std::sqrt(distanceSquared);
   float normalX, normalY;
   if (rawFraction < 0.f || rawFraction > 1.f)
   {
      const float previousOffsetX = previousCenter.x - contactX;
      const float previousOffsetY = previousCenter.y - contactY;
      const float previousDistance = std::sqrt(previousOffsetX * previousOffsetX + previousOffsetY * previousOffsetY);
      // Resolve against the current radial direction so a slow contact rolls around a segment end
      if (distance >= 0.001f)
      {
         normalX = offsetX / distance;
         normalY = offsetY / distance;
      }
      else if (previousDistance >= 0.001f)
      {
         normalX = previousOffsetX / previousDistance;
         normalY = previousOffsetY / previousDistance;
      }
      else
         return;
   }
   else
   {
      const float segmentLength = std::sqrt(lengthSquared);
      normalX = segmentY / segmentLength;
      normalY = -segmentX / segmentLength;
      float previousSide = (previousCenter.x - contactX) * normalX + (previousCenter.y - contactY) * normalY;
      if (std::abs(previousSide) < 0.001f)
         previousSide = offsetX * normalX + offsetY * normalY;
      if (std::abs(previousSide) < 0.001f)
      {
         if (velocity.x * normalX + velocity.y * normalY > 0.f)
         {
            normalX = -normalX;
            normalY = -normalY;
         }
      }
      else if (previousSide < 0.f)
      {
         normalX = -normalX;
         normalY = -normalY;
      }
   }
   center.x = contactX + normalX * radius;
   center.y = contactY + normalY * radius;
   const float normalSpeed = velocity.x * normalX + velocity.y * normalY;
   if (normalSpeed < 0.f)
      ReflectBoundaryVelocity(velocity, normalX, normalY, bounce, normalSpeed);
}

void B2SProPhysicsBall::ReflectBoundaryVelocity(B2SProPoint& velocity, const float normalX, const float normalY, const float bounce, const float incomingNormalSpeed)
{
   velocity.x -= (1.f + bounce) * incomingNormalSpeed * normalX;
   velocity.y -= (1.f + bounce) * incomingNormalSpeed * normalY;
   if (-incomingNormalSpeed < BoundaryScatterMinimumImpactSpeed)
      return;
   // Hard impacts scatter a little so a ball does not retrace the same path forever
   const float degrees = (Random() * 2.f - 1.f) * BoundaryScatterDegrees;
   const double radians = static_cast<double>(degrees) * Pi / 180.0;
   const float cosine = static_cast<float>(std::cos(radians));
   const float sine = static_cast<float>(std::sin(radians));
   const float rotatedX = velocity.x * cosine - velocity.y * sine;
   const float rotatedY = velocity.x * sine + velocity.y * cosine;
   if (rotatedX * normalX + rotatedY * normalY <= 0.f)
      return;
   velocity.x = rotatedX;
   velocity.y = rotatedY;
}

bool B2SProPhysicsBall::TryGetSweptStaticFloorCollision(const B2SProPoint& previousCenter, const B2SProPoint& currentCenter, const float radius, const B2SProPoint& from, const B2SProPoint& to,
   B2SProPoint& impactCenter, B2SProPoint& impactNormal)
{
   const float motionX = currentCenter.x - previousCenter.x;
   const float motionY = currentCenter.y - previousCenter.y;
   const float motionLengthSquared = motionX * motionX + motionY * motionY;
   if (motionLengthSquared <= 0.000001f)
      return false;
   const float segmentX = to.x - from.x;
   const float segmentY = to.y - from.y;
   const float segmentLengthSquared = segmentX * segmentX + segmentY * segmentY;
   if (segmentLengthSquared <= 0.01f)
      return false;
   const float segmentLength = std::sqrt(segmentLengthSquared);
   const float tangentX = segmentX / segmentLength;
   const float tangentY = segmentY / segmentLength;
   const float baseNormalX = tangentY;
   const float baseNormalY = -tangentX;
   const float previousSignedDistance = (previousCenter.x - from.x) * baseNormalX + (previousCenter.y - from.y) * baseNormalY;
   const float currentSignedDistance = (currentCenter.x - from.x) * baseNormalX + (currentCenter.y - from.y) * baseNormalY;
   const float signedDistanceChange = currentSignedDistance - previousSignedDistance;
   float bestTime = std::numeric_limits<float>::max();
   bool found = false;

   // A neighbouring corner segment can push the ball inside this one after it was processed for the
   // step; if the next motion then crosses to the other side, keep the side it occupied before
   const float previousProjection = (previousCenter.x - from.x) * tangentX + (previousCenter.y - from.y) * tangentY;
   const float currentProjection = (currentCenter.x - from.x) * tangentX + (currentCenter.y - from.y) * tangentY;
   if (previousProjection >= 0.f && previousProjection <= segmentLength && currentProjection >= 0.f && currentProjection <= segmentLength && std::abs(previousSignedDistance) < radius
      && previousSignedDistance * currentSignedDistance <= 0.f && std::abs(currentSignedDistance) >= radius)
   {
      const float retainedSide = std::abs(previousSignedDistance) >= 0.000001f ? (previousSignedDistance > 0.f ? 1.f : -1.f) : (signedDistanceChange < 0.f ? 1.f : -1.f);
      impactCenter = { from.x + tangentX * previousProjection + baseNormalX * retainedSide * radius, from.y + tangentY * previousProjection + baseNormalY * retainedSide * radius };
      impactNormal = { baseNormalX * retainedSide, baseNormalY * retainedSide };
      return true;
   }

   // The two parallel sides of the radius expanded capsule
   for (int side = 0; side < 2; side++)
   {
      const float sideSign = side == 0 ? 1.f : -1.f;
      if (sideSign * signedDistanceChange < -0.000001f)
      {
         const float hitTime = (sideSign * radius - previousSignedDistance) / signedDistanceChange;
         if (hitTime >= 0.f && hitTime <= 1.f && hitTime < bestTime)
         {
            const float hitX = previousCenter.x + motionX * hitTime;
            const float hitY = previousCenter.y + motionY * hitTime;
            const float projection = (hitX - from.x) * tangentX + (hitY - from.y) * tangentY;
            if (projection >= 0.f && projection <= segmentLength)
            {
               bestTime = hitTime;
               impactCenter = { hitX, hitY };
               impactNormal = { baseNormalX * sideSign, baseNormalY * sideSign };
               found = true;
            }
         }
      }
   }

   // The round caps, so a fast ball cannot skip a boundary end
   for (int endpointIndex = 0; endpointIndex < 2; endpointIndex++)
   {
      const B2SProPoint& endpoint = endpointIndex == 0 ? from : to;
      const float relativeX = previousCenter.x - endpoint.x;
      const float relativeY = previousCenter.y - endpoint.y;
      const float circleConstant = relativeX * relativeX + relativeY * relativeY - radius * radius;
      if (circleConstant < 0.f)
         continue;
      const float circleLinear = 2.f * (relativeX * motionX + relativeY * motionY);
      const double discriminant = static_cast<double>(circleLinear) * circleLinear - 4.0 * static_cast<double>(motionLengthSquared) * circleConstant;
      if (discriminant < 0.0)
         continue;
      const float hitTime = static_cast<float>((-static_cast<double>(circleLinear) - std::sqrt(discriminant)) / (2.0 * static_cast<double>(motionLengthSquared)));
      if (hitTime >= 0.f && hitTime <= 1.f && hitTime < bestTime)
      {
         const float hitX = previousCenter.x + motionX * hitTime;
         const float hitY = previousCenter.y + motionY * hitTime;
         const float normalX = hitX - endpoint.x;
         const float normalY = hitY - endpoint.y;
         const float normalLength = std::sqrt(normalX * normalX + normalY * normalY);
         if (normalLength >= 0.001f)
         {
            bestTime = hitTime;
            impactCenter = { hitX, hitY };
            impactNormal = { normalX / normalLength, normalY / normalLength };
            found = true;
         }
      }
   }
   return found;
}

void B2SProPhysicsBall::ResolveFlipperCollision(B2SProPoint& center, const float ballRadius, B2SProPoint& velocity, const float angularVelocity)
{
   const B2SProBulb& pro = m_flipper->m_pro;
   const float fx = static_cast<float>(m_flipper->m_locationX), fy = static_cast<float>(m_flipper->m_locationY);
   const float fw = static_cast<float>(m_flipper->m_width), fh = static_cast<float>(m_flipper->m_height);
   const B2SProPoint hinge { fx + fw * pro.pivotX, fy + fh * pro.pivotY };
   const float baseTipX = fx + fw * pro.pivotTipX;
   const float baseTipY = fy + fh * pro.pivotTipY;
   const double radians = static_cast<double>(m_flipper->m_proRt.angle) * Pi / 180.0;
   const double sourceX = baseTipX - hinge.x;
   const double sourceY = baseTipY - hinge.y;
   const B2SProPoint tip { static_cast<float>(hinge.x + sourceX * std::cos(radians) - sourceY * std::sin(radians)), static_cast<float>(hinge.y + sourceX * std::sin(radians) + sourceY * std::cos(radians)) };
   const float segmentX = tip.x - hinge.x;
   const float segmentY = tip.y - hinge.y;
   const float lengthSquared = segmentX * segmentX + segmentY * segmentY;
   if (lengthSquared <= 0.01f)
      return;
   const float rawFraction = ((center.x - hinge.x) * segmentX + (center.y - hinge.y) * segmentY) / lengthSquared;
   const float fraction = std::clamp(rawFraction, 0.f, 1.f);
   const B2SProPoint contact { hinge.x + segmentX * fraction, hinge.y + segmentY * fraction };
   const float segmentLength = std::sqrt(lengthSquared);
   float normalX = segmentY / segmentLength;
   float normalY = -segmentX / segmentLength;
   if (normalY > 0.f)
   {
      normalX = -normalX;
      normalY = -normalY;
   }
   const float flipperRadius = std::max(3.f, std::min(fw, fh) * 0.07f);
   const float minimumDistance = ballRadius + flipperRadius;
   if (rawFraction < 0.f || rawFraction > 1.f)
   {
      const float endX = center.x - contact.x;
      const float endY = center.y - contact.y;
      const float endDistance = std::sqrt(endX * endX + endY * endY);
      if (endDistance >= minimumDistance || endDistance < 0.001f)
         return;
      normalX = endX / endDistance;
      normalY = endY / endDistance;
      if (normalY > 0.f)
         return;
      center.x += normalX * (minimumDistance - endDistance);
      center.y += normalY * (minimumDistance - endDistance);
   }
   else
   {
      const float signedDistance = (center.x - contact.x) * normalX + (center.y - contact.y) * normalY;
      if (signedDistance >= minimumDistance)
         return;
      center.x += normalX * (minimumDistance - signedDistance);
      center.y += normalY * (minimumDistance - signedDistance);
   }
   const float armX = contact.x - hinge.x;
   const float armY = contact.y - hinge.y;
   float impactStrength = m_flipperStrength;
   float variation = pro.pivotRandomStrength;
   if (angularVelocity != 0.f && variation > 0.f && !std::isinf(variation))
   {
      variation = std::min(100.f, variation);
      impactStrength *= 1.f + (Random() * 2.f - 1.f) * variation / 100.f;
   }
   const float surfaceX = -angularVelocity * armY * impactStrength;
   const float surfaceY = angularVelocity * armX * impactStrength;
   float relativeX = velocity.x - surfaceX;
   float relativeY = velocity.y - surfaceY;
   const float towardSurface = relativeX * normalX + relativeY * normalY;
   if (towardSurface < 0.f)
   {
      constexpr float restitution = 0.58f;
      relativeX -= (1.f + restitution) * towardSurface * normalX;
      relativeY -= (1.f + restitution) * towardSurface * normalY;
      velocity.x = surfaceX + relativeX;
      velocity.y = surfaceY + relativeY;
   }
}

}
