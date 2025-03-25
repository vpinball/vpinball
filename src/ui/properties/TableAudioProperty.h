// license:GPLv3+

#pragma once

class TableAudioProperty final : public BasePropertyDialog
{
public:
    TableAudioProperty(const VectorProtected<ISelect> *pvsel);
    ~TableAudioProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox     m_soundEffectVolEdit;
    EditBox     m_musicVolEdit;
};
