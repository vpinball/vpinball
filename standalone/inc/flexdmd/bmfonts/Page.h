#pragma once

class Page final {
public:
   Page();
   ~Page();

   int GetId() const { return m_id; }
   void SetId(int id) { m_id = id; }
   const string& GetFilename() { return m_filename; }
   void SetFilename(const string& filename) { m_filename = filename; }

private:
   int m_id;
   string m_filename;
};
