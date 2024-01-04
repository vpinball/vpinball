class CDocker : public CWnd { };
class CDockProperty { };
class CDockNotes { };
class CDockLayers { };
class CDockToolbar { };

class CMDIChild : public CWnd { };

class CMDIDockFrame : public CDocker { 
public:
   CMDIChild* GetActiveMDIChild() { return (!m_children.empty()) ? m_children.front() : nullptr; }
   void AddMDIChild(CMDIChild* pChild) { m_children.push_back(pChild); }
private:
   vector<CMDIChild*> m_children;
};

class CResizer { };
class CProgressBar : public CWnd { };

inline int GetWinVersion() { return 2601; }
