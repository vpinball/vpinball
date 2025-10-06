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
   void RemoveMDIChild(CMDIChild* pChild) {
       auto it = std::find(m_children.begin(), m_children.end(), pChild);
       if (it != m_children.end())
           m_children.erase(it);
   }
private:
   vector<CMDIChild*> m_children;
};

class CResizer { };
class CProgressBar : public CWnd { };

inline int GetWinVersion() { return 2601; }
