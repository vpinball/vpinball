// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/LightseqStatesProperty.h"
#include <WindowsX.h>

LightseqStatesProperty::LightseqStatesProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPLIGHTSEQ_STATE, pvsel)
{
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_updateIntervalEdit.SetDialog(this);
    m_collectionCombo.SetDialog(this);
}

void LightseqStatesProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLightSeq))
            continue;
        LightSeq *const lightseq = (LightSeq *)m_pvsel->ElementAt(i);

        if (dispid == 9 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, lightseq->GetX());
        if (dispid == 10 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, lightseq->GetY());
        if (dispid == IDC_LIGHTSEQ_UPDATE_INTERVAL_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_updateIntervalEdit, lightseq->GetUpdateInterval());

        if (dispid == DISPID_Collection || dispid == -1)
        {
            char szT[MAXSTRING];
            WideCharToMultiByteNull(CP_ACP, 0, lightseq->m_d.m_wzCollection.c_str(), -1, szT, sizeof(szT), nullptr, nullptr);

            PropertyDialog::UpdateCollectionComboBox(lightseq->GetPTable(), m_collectionCombo, szT);
        }

        //only show the first element on multi-select
        break;
    }
}

void LightseqStatesProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLightSeq))
            continue;
        LightSeq *const lightseq = (LightSeq *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 9:
                CHECK_UPDATE_VALUE_SETTER(lightseq->SetX, lightseq->GetX, PropertyDialog::GetFloatTextbox, m_posXEdit, lightseq);
                break;
            case 10:
                CHECK_UPDATE_VALUE_SETTER(lightseq->SetY, lightseq->GetY, PropertyDialog::GetFloatTextbox, m_posYEdit, lightseq);
                break;
            case IDC_LIGHTSEQ_UPDATE_INTERVAL_EDIT:
                CHECK_UPDATE_VALUE_SETTER(lightseq->SetUpdateInterval, lightseq->GetUpdateInterval, PropertyDialog::GetIntTextbox, m_updateIntervalEdit, lightseq);
                break;
            case DISPID_Collection:
            {
                PropertyDialog::StartUndo(lightseq);
                char szText[MAXSTRING];
                PropertyDialog::GetComboBoxText(m_collectionCombo, szText, sizeof(szText));
                WCHAR wzText[MAXSTRING];
                MultiByteToWideCharNull(CP_ACP, 0, szText, -1, wzText, MAXSTRING);
                lightseq->m_d.m_wzCollection = wzText;
                PropertyDialog::EndUndo(lightseq);
                break;
            }
            default:
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL LightseqStatesProperty::OnInitDialog()
{
    m_posXEdit.AttachItem(9);
    m_posYEdit.AttachItem(10);
    m_updateIntervalEdit.AttachItem(IDC_LIGHTSEQ_UPDATE_INTERVAL_EDIT);
    m_collectionCombo.AttachItem(DISPID_Collection);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_updateIntervalEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_collectionCombo, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR LightseqStatesProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
