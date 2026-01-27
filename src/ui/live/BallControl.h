// license:GPLv3+

#pragma once

class BallControl final
{
public:
   BallControl(LiveUI& liveUI)
      : m_liveUI(liveUI) {}
   ~BallControl() = default;

   enum class Mode
   {
      Disabled,
      
      // - Drag mode: drag ball with left/middle button (release with a flipper button press)
      DragBall,
      
      // - Throw drag ball mode:
      //   . throw existing ball with left/middle click according to dragging
      //   . destroy ball with right click
      ThrowDraggedBall,
      
      // - Throw new ball mode:
      //   . drop/throw new ball according to dragging at playfield (left click) or glass (middle click) level
      //   . destroy ball with right click
      ThrowNewBall
   };

   void LoadSettings(const Settings& settings);
   void SetMode(bool ballControl, bool throwBall);
   void SetMode(Mode mode);
   Mode GetMode() const { return m_mode; }
   Ball* GetDraggedBall() const { return m_mode == Mode::DragBall && m_dragging ? m_draggedBall : nullptr; }
   const Vertex3Ds& GetDraggedBallTarget() const { return m_dragTarget; }
   void SetDraggedBall(Ball* ball) { if (!m_dragging) m_draggedBall = ball; }
   void EndBallDrag() { m_dragging = false; }
   
   void Update(const int width, const int height);
   
private:
   void HandleDragBall(const int width, const int height);
   void HandleDestroyBall(const int width, const int height) const;
   void HandleThrowBalls(const int width, const int height);

   LiveUI& m_liveUI;
   Mode m_mode = Mode::Disabled;
   bool m_dragging = false;
   Ball *m_draggedBall = nullptr;
   Vertex3Ds m_dragTarget;
   InputManager::ActionState m_prevActionState { };
};
