// license:GPLv3+

#pragma once

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
   vector<Notification> m_notifications;
   unsigned int m_nextNotificationIs = 1;

   float RenderNotification(int index, float posY) const;
};
