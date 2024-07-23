#pragma once

class Page {
public:
   Page();
   ~Page();

   int GetId() const { return m_id; }
   void SetId(int id) { m_id = id; }
   const string& GetFilename() { return m_szFilename; }
   void SetFilename(const string& filename) { m_szFilename = filename; }

private:
   int m_id;
   string m_szFilename;
};