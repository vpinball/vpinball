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
   // Called when a ball is destroyed so we never keep a freed ball as the drag target.
   void NotifyBallDestroyed(const Ball* ball) { if (m_draggedBall == ball) { m_draggedBall = nullptr; m_dragging = false; m_ringHighlightBall = nullptr; m_ringHighlight = 0.f; } }
   
   void Update(const int width, const int height);
   
private:
   void HandleDragBall(const int width, const int height);
   void HandleDestroyBall(const int width, const int height) const;
   void HandleThrowBalls(const int width, const int height);
   bool IsSelectedBallDraggable() const;

   LiveUI& m_liveUI;
   Mode m_mode = Mode::Disabled;
   bool m_dragging = false;
   Ball *m_draggedBall = nullptr;
   Vertex3Ds m_dragTarget;
   InputManager::ActionState m_prevActionState { };
   Ball *m_ringHighlightBall = nullptr; // last highlighted ball, to detect a new selection
   float m_ringHighlight = 0.f; // seconds of transient drag-target ring highlight remaining
};
