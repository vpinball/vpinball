#include "stdafx.h"
#include "Properties/LightseqStatesProperty.h"
#include <WindowsX.h>

LightseqStatesProperty::LightseqStatesProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPLIGHTSEQ_STATE, pvsel)
{
}

void LightseqStatesProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLightSeq))
            continue;
        LightSeq *const lightseq = (LightSeq *)m_pvsel->ElementAt(i);

        PropertyDialog::SetFloatTextbox(m_posXEdit, lightseq->GetX());
        PropertyDialog::SetFloatTextbox(m_posYEdit, lightseq->GetY());
        PropertyDialog::SetIntTextbox(m_updateIntervalEdit, lightseq->GetUpdateInterval());

        char szT[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, lightseq->m_d.m_wzCollection, -1, szT, MAX_PATH, NULL, NULL);

        PropertyDialog::UpdateCollectionComboBox(lightseq->GetPTable(), m_collectionCombo, szT);

        //only show the first element on multi-select
        break;

    }
}

void LightseqStatesProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLightSeq))
            continue;
        LightSeq *const lightseq = (LightSeq *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 9:
                PropertyDialog::StartUndo(lightseq);
                lightseq->SetX(PropertyDialog::GetFloatTextbox(m_posXEdit));
                PropertyDialog::EndUndo(lightseq);
                break;
            case 10:
                PropertyDialog::StartUndo(lightseq);
                lightseq->SetY(PropertyDialog::GetFloatTextbox(m_posYEdit));
                PropertyDialog::EndUndo(lightseq);
                break;
            case IDC_LIGHTSEQ_UPDATE_INTERVAL_EDIT:
                PropertyDialog::StartUndo(lightseq);
                lightseq->SetUpdateInterval(PropertyDialog::GetIntTextbox(m_updateIntervalEdit));
                PropertyDialog::EndUndo(lightseq);
                break;
            case DISPID_Collection:
            {
                PropertyDialog::StartUndo(lightseq);
                char szText[MAX_PATH];
                PropertyDialog::GetComboBoxText(m_collectionCombo, szText);
                UNICODE_FROM_ANSI(lightseq->m_d.m_wzCollection, szText, lstrlen(szText));
                lightseq->m_d.m_wzCollection[lstrlen(szText)] = 0;
                PropertyDialog::EndUndo(lightseq);
                break;
            }
            default:
                break;
        }
    }
    UpdateVisuals();
}
BOOL LightseqStatesProperty::OnInitDialog()
{
    AttachItem(9, m_posXEdit);
    AttachItem(10, m_posYEdit);
    AttachItem(DISPID_Collection, m_collectionCombo);
    AttachItem(IDC_LIGHTSEQ_UPDATE_INTERVAL_EDIT, m_updateIntervalEdit);
    UpdateVisuals();
    return TRUE;
}

