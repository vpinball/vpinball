// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include <fstream>
#include <sstream>
#include "PhysicsOptionsDialog.h"
#include "tinyxml2/tinyxml2.h"

constexpr unsigned int num_physicsoptions = 8;
static string physicsoptions[num_physicsoptions];
static unsigned int physicsselection = 0;


PhysicsOptionsDialog::PhysicsOptionsDialog() : CDialog(IDD_PHYSICS_OPTIONS)
{
}

BOOL PhysicsOptionsDialog::OnInitDialog()
{
    const HWND hwndList = GetDlgItem(IDC_PhysicsList).GetHwnd();

    ::SendMessage(hwndList, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
    ::SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
    for (unsigned int i = 0; i < num_physicsoptions; i++)
    {
        physicsoptions[i] = g_app->m_settings.GetPlayer_PhysicsSetName(i);
        string tmp = std::to_string(i + 1) + ": " + physicsoptions[i];
        const size_t index = ::SendMessage(hwndList, LB_ADDSTRING, 0, (size_t)tmp.c_str());
    }
    ::SendMessage(hwndList, LB_SETCURSEL, physicsselection, 0);
    ::SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);

    const float FlipperPhysicsMass = g_app->m_settings.GetPlayer_FlipperPhysicsMass(physicsselection);
    SetItemText(DISPID_Flipper_Speed, FlipperPhysicsMass);
    const float FlipperPhysicsStrength = g_app->m_settings.GetPlayer_FlipperPhysicsStrength(physicsselection);
    SetItemText(19, FlipperPhysicsStrength);
    const float FlipperPhysicsElasticity = g_app->m_settings.GetPlayer_FlipperPhysicsElasticity(physicsselection);
    SetItemText(21, FlipperPhysicsElasticity);
    const float FlipperPhysicsScatter = g_app->m_settings.GetPlayer_FlipperPhysicsScatter(physicsselection);
    SetItemText(112, FlipperPhysicsScatter);
    const float FlipperPhysicsTorqueDamping = g_app->m_settings.GetPlayer_FlipperPhysicsEOSTorque(physicsselection);
    SetItemText(113, FlipperPhysicsTorqueDamping);
    const float FlipperPhysicsTorqueDampingAngle = g_app->m_settings.GetPlayer_FlipperPhysicsEOSTorqueAngle(physicsselection);
    SetItemText(189, FlipperPhysicsTorqueDampingAngle);
    const float FlipperPhysicsReturnStrength = g_app->m_settings.GetPlayer_FlipperPhysicsReturnStrength(physicsselection);
    SetItemText(23, FlipperPhysicsReturnStrength);
    const float FlipperPhysicsElasticityFalloff = g_app->m_settings.GetPlayer_FlipperPhysicsElasticityFalloff(physicsselection);
    SetItemText(22, FlipperPhysicsElasticityFalloff);
    const float FlipperPhysicsFriction = g_app->m_settings.GetPlayer_FlipperPhysicsFriction(physicsselection);
    SetItemText(109, FlipperPhysicsFriction);
    const float FlipperPhysicsCoilRampUp = g_app->m_settings.GetPlayer_FlipperPhysicsCoilRampUp(physicsselection);
    SetItemText(110, FlipperPhysicsCoilRampUp);
    const float TablePhysicsGravityConstant = g_app->m_settings.GetPlayer_TablePhysicsGravityConstant(physicsselection);
    SetItemText(1100, TablePhysicsGravityConstant);
    const float TablePhysicsContactFriction = g_app->m_settings.GetPlayer_TablePhysicsContactFriction(physicsselection);
    SetItemText(1101, TablePhysicsContactFriction);
    const float TablePhysicsElasticity = g_app->m_settings.GetPlayer_TablePhysicsElasticity(physicsselection);
    SetItemText(1708, TablePhysicsElasticity);
    const float TablePhysicsElasticityFalloff = g_app->m_settings.GetPlayer_TablePhysicsElasticityFalloff(physicsselection);
    SetItemText(1709, TablePhysicsElasticityFalloff);
    const float TablePhysicsScatterAngle = g_app->m_settings.GetPlayer_TablePhysicsScatterAngle(physicsselection);
    SetItemText(1710, TablePhysicsScatterAngle);
    const float TablePhysicsContactScatterAngle = g_app->m_settings.GetPlayer_TablePhysicsContactScatterAngle(physicsselection);
    SetItemText(1102, TablePhysicsContactScatterAngle);
    const float TablePhysicsMinSlope = g_app->m_settings.GetPlayer_TablePhysicsMinSlope(physicsselection);
    SetItemText(1103, TablePhysicsMinSlope);
    const float TablePhysicsMaxSlope = g_app->m_settings.GetPlayer_TablePhysicsMaxSlope(physicsselection);
    SetItemText(1104, TablePhysicsMaxSlope);

    SetDlgItemText(1110, physicsoptions[physicsselection].c_str());

    return TRUE;
}

INT_PTR PhysicsOptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DialogProcDefault(uMsg, wParam, lParam);
}

struct PhysValues
{
   string gravityConstant;
   string contactFriction;
   string tableElasticity;
   string tableElasticityFalloff;
   string playfieldScatter;
   string defaultElementScatter;
   // Flipper:
   string speed;
   string strength;
   string elasticity;
   string scatter;
   string eosTorque;
   string eosTorqueAngle;
   string returnStrength;
   string elasticityFalloff;
   string friction;
   string coilRampup;

   string minSlope;
   string maxSlope;
   string name;
};

static PhysValues loadValues;

BOOL PhysicsOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch(HIWORD(wParam))
    {
        case LBN_SELCHANGE:
        {
            const HWND hwndList = GetDlgItem(IDC_PhysicsList).GetHwnd();

            const size_t tmp = ::SendMessage(hwndList, LB_GETCURSEL, 0, 0);

            if (tmp != physicsselection)
            {
                int result = g_pvp->MessageBox("Save", "Save current physics set?", MB_YESNOCANCEL | MB_ICONQUESTION);
                if (result == IDYES)
                    SaveCurrentPhysicsSetting();

                if (result != IDCANCEL)
                {
                    physicsselection = (unsigned int)tmp;
                    SendMessage(WM_INITDIALOG, 0, 0); // reinit all boxes
                }
                else
                    ::SendMessage(hwndList, LB_SETCURSEL, physicsselection, 0);
            }
            break;
        }
    }
    switch(LOWORD(wParam))
    {
        case 1111:
        {
            if (LoadSetting() == false)
                break;

            SetDlgItemText(1100, convert_decimal_point_and_trim(loadValues.gravityConstant, true).c_str());
            SetDlgItemText(1101, convert_decimal_point_and_trim(loadValues.contactFriction, true).c_str());
            SetDlgItemText(1708, convert_decimal_point_and_trim(loadValues.tableElasticity, true).c_str());
            SetDlgItemText(1709, convert_decimal_point_and_trim(loadValues.tableElasticityFalloff, true).c_str());
            SetDlgItemText(1710, convert_decimal_point_and_trim(loadValues.playfieldScatter, true).c_str());
            SetDlgItemText(1102, convert_decimal_point_and_trim(loadValues.defaultElementScatter, true).c_str());
            SetDlgItemText(1103, convert_decimal_point_and_trim(loadValues.minSlope, true).c_str());
            SetDlgItemText(1104, convert_decimal_point_and_trim(loadValues.maxSlope, true).c_str());

            SetDlgItemText(DISPID_Flipper_Speed, convert_decimal_point_and_trim(loadValues.speed, true).c_str());
            SetDlgItemText(19, convert_decimal_point_and_trim(loadValues.strength, true).c_str());
            SetDlgItemText(21, convert_decimal_point_and_trim(loadValues.elasticity, true).c_str());
            SetDlgItemText(112, convert_decimal_point_and_trim(loadValues.scatter, true).c_str());
            SetDlgItemText(113, convert_decimal_point_and_trim(loadValues.eosTorque, true).c_str());
            SetDlgItemText(189, convert_decimal_point_and_trim(loadValues.eosTorqueAngle, true).c_str());
            SetDlgItemText(23, convert_decimal_point_and_trim(loadValues.returnStrength, true).c_str());
            SetDlgItemText(22, convert_decimal_point_and_trim(loadValues.elasticityFalloff, true).c_str());
            SetDlgItemText(109, convert_decimal_point_and_trim(loadValues.friction, true).c_str());
            SetDlgItemText(110, convert_decimal_point_and_trim(loadValues.coilRampup, true).c_str());

            SetDlgItemText(1110, loadValues.name.c_str());
            SetFocus();
            break;
        }
        case 1112:
        {
            char szFileName[MAXSTRING];
            /*CComObject<PinTable>* const pt = g_pvp->GetActiveTable();
            if (pt)
            {
               strncpy_s(szFileName, sizeof(szFileName), pt->m_filename.c_str());
               const size_t idx = pt->m_filename.find_last_of('.');
               if (idx != string::npos && idx < MAXSTRING)
                  szFileName[idx] = '\0';
            }
            else
               szFileName[0] = '\0';*/
            strncpy_s(szFileName, sizeof(szFileName), "Physics.vpp");

            OPENFILENAME ofn = {};
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hInstance = g_app->GetInstanceHandle();
            ofn.hwndOwner = g_pvp->GetHwnd();
            // TEXT
            ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = sizeof(szFileName);
            ofn.lpstrDefExt = "vpp";
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

            string szInitialDir = g_app->m_settings.GetRecentDir_PhysicsDir();
            ofn.lpstrInitialDir = szInitialDir.c_str();

            const int ret = GetSaveFileName(&ofn);
            if (ret == 0)
                break;

            const string szFilename(ofn.lpstrFile);
            const size_t index = szFilename.find_last_of(PATH_SEPARATOR_CHAR);
            if (index != string::npos)
            {
                const string newInitDir(szFilename.substr(0, index));
                g_app->m_settings.SetRecentDir_PhysicsDir(newInitDir, false);
            }

            tinyxml2::XMLDocument xmlDoc;

            auto root = xmlDoc.NewElement("physics");
            auto physFlip = xmlDoc.NewElement("flipper");
            auto physTab = xmlDoc.NewElement("table");

            auto node = xmlDoc.NewElement(PinTable::VPPelementNames[0].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(1100), false).c_str());
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[1].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(1101), false).c_str());
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[2].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(1708), false).c_str());
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[3].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(1709), false).c_str());
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[4].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(1710), false).c_str());
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[5].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(1102), false).c_str());
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[6].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(1103), false).c_str());
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[7].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(1104), false).c_str());
            physTab->InsertEndChild(node);

            // flippers
            node = xmlDoc.NewElement(PinTable::VPPelementNames[8].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(DISPID_Flipper_Speed), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[9].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(19), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[10].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(21), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[11].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(112), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[12].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(113), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[13].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(189), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[14].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(23), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[15].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(22), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[16].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(109), false).c_str());
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement(PinTable::VPPelementNames[17].c_str());
            node->SetText(convert_decimal_point_and_trim(GetItemText(110), false).c_str());
            physFlip->InsertEndChild(node);

            auto settingName = xmlDoc.NewElement("name");
            settingName->SetText(GetItemText(1110).c_str());
            root->InsertEndChild(settingName);
            root->InsertEndChild(physTab);
            root->InsertEndChild(physFlip);
            xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());
            xmlDoc.InsertEndChild(root);

            tinyxml2::XMLPrinter prn;
            xmlDoc.Print(&prn);

            std::ofstream myfile(ofn.lpstrFile);
            myfile << prn.CStr();
            myfile.close();

            SetFocus();
            break;
        }

        default:
        return FALSE;
    }
    return TRUE;
}

void PhysicsOptionsDialog::OnOK()
{
    SaveCurrentPhysicsSetting();
    CDialog::OnOK();
}

bool PhysicsOptionsDialog::LoadSetting()
{
    const string& szInitialDir = g_app->m_settings.GetRecentDir_PhysicsDir();

    vector<string> szFileName;
    if (!g_pvp->OpenFileDialog(szInitialDir, szFileName, "Visual Pinball Physics (*.vpp)\0*.vpp\0", "vpp", 0))
        return false;

    const size_t index = szFileName[0].find_last_of(PATH_SEPARATOR_CHAR);
    if (index != string::npos)
        g_app->m_settings.SetRecentDir_PhysicsDir(szFileName[0].substr(0, index), false);

    tinyxml2::XMLDocument xmlDoc;
    try
    {
        std::stringstream buffer;
        std::ifstream myFile(szFileName[0]);
        buffer << myFile.rdbuf();
        myFile.close();
        const string xml = buffer.str();
        if (xmlDoc.Parse(xml.c_str()))
        {
            ShowError("Error parsing VPP XML file");
            return false;
        }
        auto root = xmlDoc.FirstChildElement("physics");
        auto table = root->FirstChildElement("table");
        auto flipper = root->FirstChildElement("flipper");

        loadValues.gravityConstant = table->FirstChildElement(PinTable::VPPelementNames[0].c_str())->GetText();
        loadValues.contactFriction = table->FirstChildElement(PinTable::VPPelementNames[1].c_str())->GetText();
        loadValues.tableElasticity = table->FirstChildElement(PinTable::VPPelementNames[2].c_str())->GetText();
        loadValues.tableElasticityFalloff = table->FirstChildElement(PinTable::VPPelementNames[3].c_str())->GetText();
        loadValues.playfieldScatter = table->FirstChildElement(PinTable::VPPelementNames[4].c_str())->GetText();
        loadValues.defaultElementScatter = table->FirstChildElement(PinTable::VPPelementNames[5].c_str())->GetText();
        auto tmp = table->FirstChildElement(PinTable::VPPelementNames[6].c_str());
        if(tmp)
           loadValues.minSlope = tmp->GetText();
        else
           loadValues.minSlope = f2sz(DEFAULT_TABLE_MIN_SLOPE);
        tmp = table->FirstChildElement(PinTable::VPPelementNames[7].c_str());
        if(tmp)
           loadValues.maxSlope = tmp->GetText();
        else
           loadValues.maxSlope = f2sz(DEFAULT_TABLE_MAX_SLOPE);
        // flippers
        loadValues.speed = flipper->FirstChildElement(PinTable::VPPelementNames[8].c_str())->GetText();
        loadValues.strength = flipper->FirstChildElement(PinTable::VPPelementNames[9].c_str())->GetText();
        loadValues.elasticity = flipper->FirstChildElement(PinTable::VPPelementNames[10].c_str())->GetText();
        loadValues.scatter = flipper->FirstChildElement(PinTable::VPPelementNames[11].c_str())->GetText();
        loadValues.eosTorque = flipper->FirstChildElement(PinTable::VPPelementNames[12].c_str())->GetText();
        loadValues.eosTorqueAngle = flipper->FirstChildElement(PinTable::VPPelementNames[13].c_str())->GetText();
        loadValues.returnStrength = flipper->FirstChildElement(PinTable::VPPelementNames[14].c_str())->GetText();
        loadValues.elasticityFalloff = flipper->FirstChildElement(PinTable::VPPelementNames[15].c_str())->GetText();
        loadValues.friction = flipper->FirstChildElement(PinTable::VPPelementNames[16].c_str())->GetText();
        loadValues.coilRampup = flipper->FirstChildElement(PinTable::VPPelementNames[17].c_str())->GetText();

        loadValues.name = root->FirstChildElement("name")->GetText();
    }
    catch(...)
    {
        ShowError("Error parsing physics settings file");
        xmlDoc.Clear();
        return false;
    }
    xmlDoc.Clear();

    return true;
}

string PhysicsOptionsDialog::GetItemText(int id) const
{
    return GetDlgItemText(id).GetString();
}

void PhysicsOptionsDialog::SetItemText(int id, float value)
{
    SetDlgItemText(id, f2sz(value).c_str());
}


void PhysicsOptionsDialog::SaveCurrentPhysicsSetting()
{
    g_app->m_settings.SetPlayer_FlipperPhysicsMass(physicsselection, sz2f(GetItemText(DISPID_Flipper_Speed)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsStrength(physicsselection, sz2f(GetItemText(19)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsElasticity(physicsselection, sz2f(GetItemText(21)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsScatter(physicsselection, sz2f(GetItemText(112)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsEOSTorque(physicsselection, sz2f(GetItemText(113)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsEOSTorqueAngle(physicsselection, sz2f(GetItemText(189)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsReturnStrength(physicsselection, sz2f(GetItemText(23)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsElasticityFalloff(physicsselection, sz2f(GetItemText(22)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsFriction(physicsselection, sz2f(GetItemText(109)), false);
    g_app->m_settings.SetPlayer_FlipperPhysicsCoilRampUp(physicsselection, sz2f(GetItemText(110)), false);
    g_app->m_settings.SetPlayer_TablePhysicsGravityConstant(physicsselection, sz2f(GetItemText(1100)), false);
    g_app->m_settings.SetPlayer_TablePhysicsContactFriction(physicsselection, sz2f(GetItemText(1101)), false);
    g_app->m_settings.SetPlayer_TablePhysicsElasticity(physicsselection, sz2f(GetItemText(1708)), false);
    g_app->m_settings.SetPlayer_TablePhysicsElasticityFalloff(physicsselection, sz2f(GetItemText(1709)), false);
    g_app->m_settings.SetPlayer_TablePhysicsScatterAngle(physicsselection, sz2f(GetItemText(1710)), false);
    g_app->m_settings.SetPlayer_TablePhysicsContactScatterAngle(physicsselection, sz2f(GetItemText(1102)), false);
    g_app->m_settings.SetPlayer_TablePhysicsMinSlope(physicsselection, sz2f(GetItemText(1103)), false);
    g_app->m_settings.SetPlayer_TablePhysicsMaxSlope(physicsselection, sz2f(GetItemText(1104)), false);
    g_app->m_settings.SetPlayer_PhysicsSetName(physicsselection, GetItemText(1110), false);
}
