// license:GPLv3+

#pragma once

#include <mutex>

class NotificationOverlay final
{
public:
   NotificationOverlay() = default;
   ~NotificationOverlay() = default;

   unsigned int PushNotification(const string &message, const int lengthMs, const unsigned int reuseId = 0);

   void Update(bool showNotification, ImFont * font);

private:
   struct Notification
   {
      unsigned int id;
      string message;
      uint32_t disappearTick;
   };
   std::mutex m_mutex; // Notifications may be pushed from any thread (e.g. worker threads reporting load errors)
   vector<Notification> m_notifications;
   unsigned int m_nextNotificationIs = 1;

   float RenderNotification(int index, float posY) const;
};
