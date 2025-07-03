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
        string tmp = "PhysicsSetName" + std::to_string(i);
        if (!g_pvp->m_settings.LoadValue(Settings::Player, tmp, physicsoptions[i]))
            physicsoptions[i] = "Set " + std::to_string(i + 1);
        tmp = std::to_string(i + 1) + ": " + physicsoptions[i];
        const size_t index = ::SendMessage(hwndList, LB_ADDSTRING, 0, (size_t)tmp.c_str());
    }
    ::SendMessage(hwndList, LB_SETCURSEL, physicsselection, 0);
    ::SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);

    const float FlipperPhysicsMass = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsMass" + std::to_string(physicsselection), 1.f);
    SetItemText(DISPID_Flipper_Speed, FlipperPhysicsMass);
    const float FlipperPhysicsStrength = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsStrength" + std::to_string(physicsselection), 2200.f);
    SetItemText(19, FlipperPhysicsStrength);
    const float FlipperPhysicsElasticity = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsElasticity" + std::to_string(physicsselection), 0.8f);
    SetItemText(21, FlipperPhysicsElasticity);
    const float FlipperPhysicsScatter = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsScatter" + std::to_string(physicsselection), 0.f);
    SetItemText(112, FlipperPhysicsScatter);
    const float FlipperPhysicsTorqueDamping = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsEOSTorque" + std::to_string(physicsselection), 0.75f);
    SetItemText(113, FlipperPhysicsTorqueDamping);
    const float FlipperPhysicsTorqueDampingAngle = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsEOSTorqueAngle" + std::to_string(physicsselection), 6.f);
    SetItemText(189, FlipperPhysicsTorqueDampingAngle);
    const float FlipperPhysicsReturnStrength = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsReturnStrength" + std::to_string(physicsselection), 0.058f);
    SetItemText(23, FlipperPhysicsReturnStrength);
    const float FlipperPhysicsElasticityFalloff = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsElasticityFalloff" + std::to_string(physicsselection), 0.43f);
    SetItemText(22, FlipperPhysicsElasticityFalloff);
    const float FlipperPhysicsFriction = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsFriction" + std::to_string(physicsselection), 0.6f);
    SetItemText(109, FlipperPhysicsFriction);
    const float FlipperPhysicsCoilRampUp = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "FlipperPhysicsCoilRampUp" + std::to_string(physicsselection), 3.f);
    SetItemText(110, FlipperPhysicsCoilRampUp);
    const float TablePhysicsGravityConstant = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsGravityConstant" + std::to_string(physicsselection), DEFAULT_TABLE_GRAVITY);
    SetItemText(1100, TablePhysicsGravityConstant);
    const float TablePhysicsContactFriction = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsContactFriction"+std::to_string(physicsselection), DEFAULT_TABLE_CONTACTFRICTION);
    SetItemText(1101, TablePhysicsContactFriction);
    const float TablePhysicsElasticity = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsElasticity" + std::to_string(physicsselection), DEFAULT_TABLE_ELASTICITY);
    SetItemText(1708, TablePhysicsElasticity);
    const float TablePhysicsElasticityFalloff = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsElasticityFalloff"+std::to_string(physicsselection), DEFAULT_TABLE_ELASTICITY_FALLOFF);
    SetItemText(1709, TablePhysicsElasticityFalloff);
    const float TablePhysicsScatterAngle = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsScatterAngle" + std::to_string(physicsselection), DEFAULT_TABLE_PFSCATTERANGLE);
    SetItemText(1710, TablePhysicsScatterAngle);
    const float TablePhysicsContactScatterAngle = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsContactScatterAngle"+std::to_string(physicsselection), DEFAULT_TABLE_SCATTERANGLE);
    SetItemText(1102, TablePhysicsContactScatterAngle);
    const float TablePhysicsMinSlope = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsMinSlope" + std::to_string(physicsselection), DEFAULT_TABLE_MIN_SLOPE);
    SetItemText(1103, TablePhysicsMinSlope);
    const float TablePhysicsMaxSlope = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsMaxSlope" + std::to_string(physicsselection), DEFAULT_TABLE_MAX_SLOPE);
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
               strncpy_s(szFileName, pt->m_filename.c_str(), sizeof(szFileName)-1);
               const size_t idx = pt->m_filename.find_last_of('.');
               if (idx != string::npos && idx < MAXSTRING)
                  szFileName[idx] = '\0';
            }
            else
               szFileName[0] = '\0';*/
            strncpy_s(szFileName, "Physics.vpp", sizeof(szFileName)-1);

            OPENFILENAME ofn = {};
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hInstance = g_pvp->theInstance;
            ofn.hwndOwner = g_pvp->GetHwnd();
            // TEXT
            ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = sizeof(szFileName);
            ofn.lpstrDefExt = "vpp";
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

            string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "PhysicsDir"s, PATH_TABLES);
            ofn.lpstrInitialDir = szInitialDir.c_str();

            const int ret = GetSaveFileName(&ofn);
            if (ret == 0)
                break;

            const string szFilename(ofn.lpstrFile);
            const size_t index = szFilename.find_last_of(PATH_SEPARATOR_CHAR);
            if (index != string::npos)
            {
                const string newInitDir(szFilename.substr(0, index));
                g_pvp->m_settings.SaveValue(Settings::RecentDir, "PhysicsDir"s, newInitDir);
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
    string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "PhysicsDir"s, PATH_TABLES);

    vector<string> szFileName;
    if (!g_pvp->OpenFileDialog(szInitialDir, szFileName, "Visual Pinball Physics (*.vpp)\0*.vpp\0", "vpp", 0))
        return false;

    const size_t index = szFileName[0].find_last_of(PATH_SEPARATOR_CHAR);
    if (index != string::npos)
        g_pvp->m_settings.SaveValue(Settings::RecentDir, "PhysicsDir"s, szFileName[0].substr(0, index));

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
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsMass" + std::to_string(physicsselection), sz2f(GetItemText(DISPID_Flipper_Speed)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsStrength" + std::to_string(physicsselection), sz2f(GetItemText(19)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsElasticity" + std::to_string(physicsselection), sz2f(GetItemText(21)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsScatter" + std::to_string(physicsselection), sz2f(GetItemText(112)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsEOSTorque" + std::to_string(physicsselection), sz2f(GetItemText(113)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsEOSTorqueAngle" + std::to_string(physicsselection), sz2f(GetItemText(189)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsReturnStrength" + std::to_string(physicsselection), sz2f(GetItemText(23)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsElasticityFalloff" + std::to_string(physicsselection), sz2f(GetItemText(22)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsFriction" + std::to_string(physicsselection), sz2f(GetItemText(109)));
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsCoilRampUp" + std::to_string(physicsselection), sz2f(GetItemText(110)));
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsGravityConstant" + std::to_string(physicsselection), sz2f(GetItemText(1100)));
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsContactFriction" + std::to_string(physicsselection), sz2f(GetItemText(1101)));
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsElasticity" + std::to_string(physicsselection), sz2f(GetItemText(1708)));
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsElasticityFalloff" + std::to_string(physicsselection), sz2f(GetItemText(1709)));
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsScatterAngle" + std::to_string(physicsselection), sz2f(GetItemText(1710)));
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsContactScatterAngle" + std::to_string(physicsselection), sz2f(GetItemText(1102)));
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsMinSlope" + std::to_string(physicsselection), sz2f(GetItemText(1103)));
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsMaxSlope" + std::to_string(physicsselection), sz2f(GetItemText(1104)));
    g_pvp->m_settings.SaveValue(Settings::Player, "PhysicsSetName" + std::to_string(physicsselection), GetItemText(1110));
}
