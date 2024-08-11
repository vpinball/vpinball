// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include <fstream>
#include <sstream>
#include "PhysicsOptionsDialog.h"
#include "tinyxml2/tinyxml2.h"

constexpr unsigned int num_physicsoptions = 8;
static char * physicsoptions[num_physicsoptions] ={ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
static unsigned int physicsselection = 0;


PhysicsOptionsDialog::PhysicsOptionsDialog() : CDialog(IDD_PHYSICS_OPTIONS)
{
}

BOOL PhysicsOptionsDialog::OnInitDialog()
{
    const HWND hwndList = GetDlgItem(IDC_PhysicsList).GetHwnd();

    const size_t size = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
    for (size_t i = 0; i < size; i++)
    {
        if (physicsoptions[i])
        {
            delete[] physicsoptions[i];
            physicsoptions[i] = nullptr;
        }
        const int* sd = (int *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
        delete sd;
    }

    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
    for (unsigned int i = 0; i < num_physicsoptions; i++)
    {
        physicsoptions[i] = new char[256];
        char tmp[256];
        sprintf_s(tmp, sizeof(tmp), "PhysicsSetName%u", i);
        if (!g_pvp->m_settings.LoadValue(Settings::Player, tmp, physicsoptions[i], 256))
            sprintf_s(physicsoptions[i], 256, "Set %u", i + 1);
        sprintf_s(tmp, sizeof(tmp), "%u: %s", i + 1, physicsoptions[i]);
        const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (size_t)tmp);
        int * const sd = new int;
        *sd = i;
        SendMessage(hwndList, LB_SETITEMDATA, index, (LPARAM)sd);
    }
    SendMessage(hwndList, LB_SETCURSEL, physicsselection, 0);
    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);

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

    SetDlgItemText(1110, physicsoptions[physicsselection]);

    return TRUE;
}

INT_PTR PhysicsOptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DialogProcDefault(uMsg, wParam, lParam);
}

struct PhysValues
{
    char gravityConstant[16];
    char contactFriction[16];
    char tableElasticity[16];
    char tableElasticityFalloff[16];
    char playfieldScatter[16];
    char defaultElementScatter[16];
    // Flipper:
    char speed[16];
    char strength[16];
    char elasticity[16];
    char scatter[16];
    char eosTorque[16];
    char eosTorqueAngle[16];
    char returnStrength[16];
    char elasticityFalloff[16];
    char friction[16];
    char coilRampup[16];

    char minSlope[16];
    char maxSlope[16];
    char name[MAXNAMEBUFFER];
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

            const size_t tmp = SendMessage(hwndList, LB_GETCURSEL, 0, 0);

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

            SetDlgItemText(1100, CString(loadValues.gravityConstant));
            SetDlgItemText(1101, CString(loadValues.contactFriction));
            SetDlgItemText(1708, CString(loadValues.tableElasticity));
            SetDlgItemText(1709, CString(loadValues.tableElasticityFalloff));
            SetDlgItemText(1710, CString(loadValues.playfieldScatter));
            SetDlgItemText(1102, CString(loadValues.defaultElementScatter));
            SetDlgItemText(1103, CString(loadValues.minSlope));
            SetDlgItemText(1104, CString(loadValues.maxSlope));

            SetDlgItemText(DISPID_Flipper_Speed, CString(loadValues.speed));
            SetDlgItemText(19, CString(loadValues.strength));
            SetDlgItemText(21, CString(loadValues.elasticity));
            SetDlgItemText(112, CString(loadValues.scatter));
            SetDlgItemText(113, CString(loadValues.eosTorque));
            SetDlgItemText(189, CString(loadValues.eosTorqueAngle));
            SetDlgItemText(23, CString(loadValues.returnStrength));
            SetDlgItemText(22, CString(loadValues.elasticityFalloff));
            SetDlgItemText(109, CString(loadValues.friction));
            SetDlgItemText(110, CString(loadValues.coilRampup));

            SetDlgItemText(1110, CString(loadValues.name));
            SetFocus();
            break;
        }
        case 1112:
        {
            char szFileName[MAXSTRING];
            /*CComObject<PinTable>* const pt = g_pvp->GetActiveTable();
            if (pt)
            {
               strncpy_s(szFileName, pt->m_szFileName.c_str(), sizeof(szFileName)-1);
               const size_t idx = pt->m_szFileName.find_last_of('.');
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

            auto node = xmlDoc.NewElement("speed");
            node->SetText(GetItemText(DISPID_Flipper_Speed));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("strength");
            node->SetText(GetItemText(19));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("elasticity");
            node->SetText(GetItemText(21));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("scatter");
            node->SetText(GetItemText(112));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("eosTorque");
            node->SetText(GetItemText(113));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("eosTorqueAngle");
            node->SetText(GetItemText(189));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("returnStrength");
            node->SetText(GetItemText(23));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("elasticityFalloff");
            node->SetText(GetItemText(22));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("friction");
            node->SetText(GetItemText(109));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("coilRampUp");
            node->SetText(GetItemText(110));
            physFlip->InsertEndChild(node);

            node = xmlDoc.NewElement("gravityConstant");
            node->SetText(GetItemText(1100));
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement("contactFriction");
            node->SetText(GetItemText(1101));
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement("elasticity");
            node->SetText(GetItemText(1708));
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement("elasticityFalloff");
            node->SetText(GetItemText(1709));
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement("defaultElementScatter");
            node->SetText(GetItemText(1102));
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement("playfieldScatter");
            node->SetText(GetItemText(1710));
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement("playfieldminslope");
            node->SetText(GetItemText(1103));
            physTab->InsertEndChild(node);

            node = xmlDoc.NewElement("playfieldmaxslope");
            node->SetText(GetItemText(1104));
            physTab->InsertEndChild(node);

            auto settingName = xmlDoc.NewElement("name");
            settingName->SetText(GetItemText(1110));
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

void PhysicsOptionsDialog::OnDestroy()
{
    const HWND hwndList = GetDlgItem(IDC_PhysicsList).GetHwnd();
    const size_t size = ::SendMessage(hwndList, LB_GETCOUNT, 0, 0);
    for (size_t i = 0; i < size; i++)
    {
        if (physicsoptions[i])
        {
            delete[] physicsoptions[i];
            physicsoptions[i] = nullptr;
        }
        const int* sd = (int *)::SendMessage(hwndList, LB_GETITEMDATA, i, 0);
        delete sd;
    }
    ::SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
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
        auto xml = buffer.str();
        if (xmlDoc.Parse(xml.c_str()))
        {
            ShowError("Error parsing VPP XML file");
            return false;
        }
        auto root = xmlDoc.FirstChildElement("physics");
        auto table = root->FirstChildElement("table");
        auto flipper = root->FirstChildElement("flipper");

        strncpy_s(loadValues.gravityConstant, table->FirstChildElement("gravityConstant")->GetText(), sizeof(loadValues.gravityConstant) - 1);
        strncpy_s(loadValues.contactFriction, table->FirstChildElement("contactFriction")->GetText(), sizeof(loadValues.contactFriction) - 1);
        strncpy_s(loadValues.tableElasticity, table->FirstChildElement("elasticity")->GetText(), sizeof(loadValues.tableElasticity) - 1);
        strncpy_s(loadValues.tableElasticityFalloff, table->FirstChildElement("elasticityFalloff")->GetText(), sizeof(loadValues.tableElasticityFalloff) - 1);
        strncpy_s(loadValues.playfieldScatter, table->FirstChildElement("playfieldScatter")->GetText(), sizeof(loadValues.playfieldScatter) - 1);
        strncpy_s(loadValues.defaultElementScatter, table->FirstChildElement("defaultElementScatter")->GetText(), sizeof(loadValues.defaultElementScatter) - 1);
        auto tmp = table->FirstChildElement("playfieldminslope");
        if(tmp)
            strncpy_s(loadValues.minSlope, sizeof(loadValues.minSlope), tmp->GetText(), sizeof(loadValues.minSlope) - 1);
        else
           sprintf_s(loadValues.minSlope, sizeof(loadValues.minSlope), "%f", DEFAULT_TABLE_MIN_SLOPE);
        tmp = table->FirstChildElement("playfieldmaxslope");
        if(tmp)
           strncpy_s(loadValues.maxSlope, sizeof(loadValues.maxSlope), tmp->GetText(), sizeof(loadValues.maxSlope) - 1);
        else
           sprintf_s(loadValues.maxSlope, sizeof(loadValues.maxSlope), "%f", DEFAULT_TABLE_MAX_SLOPE);
        strncpy_s(loadValues.speed, flipper->FirstChildElement("speed")->GetText(), sizeof(loadValues.speed) - 1);
        strncpy_s(loadValues.strength, flipper->FirstChildElement("strength")->GetText(), sizeof(loadValues.strength) - 1);
        strncpy_s(loadValues.elasticity, flipper->FirstChildElement("elasticity")->GetText(), sizeof(loadValues.elasticity) - 1);
        strncpy_s(loadValues.scatter, flipper->FirstChildElement("scatter")->GetText(), sizeof(loadValues.scatter) - 1);
        strncpy_s(loadValues.eosTorque, flipper->FirstChildElement("eosTorque")->GetText(), sizeof(loadValues.eosTorque) - 1);
        strncpy_s(loadValues.eosTorqueAngle, flipper->FirstChildElement("eosTorqueAngle")->GetText(), sizeof(loadValues.eosTorqueAngle) - 1);
        strncpy_s(loadValues.returnStrength, flipper->FirstChildElement("returnStrength")->GetText(), sizeof(loadValues.returnStrength) - 1);
        strncpy_s(loadValues.elasticityFalloff, flipper->FirstChildElement("elasticityFalloff")->GetText(), sizeof(loadValues.elasticityFalloff) - 1);
        strncpy_s(loadValues.friction, flipper->FirstChildElement("friction")->GetText(), sizeof(loadValues.friction) - 1);
        strncpy_s(loadValues.coilRampup, flipper->FirstChildElement("coilRampUp")->GetText(), sizeof(loadValues.coilRampup) - 1);

        strncpy_s(loadValues.name, root->FirstChildElement("name")->GetText(), sizeof(loadValues.name) - 1);
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

CString PhysicsOptionsDialog::GetItemText(int id)
{
    return GetDlgItemText(id);
}

void PhysicsOptionsDialog::SetItemText(int id, float value)
{
    SetDlgItemText(id, f2sz(value).c_str());
}


void PhysicsOptionsDialog::SaveCurrentPhysicsSetting()
{
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsMass" + std::to_string(physicsselection), GetItemText(DISPID_Flipper_Speed).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsStrength" + std::to_string(physicsselection), GetItemText(19).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsElasticity" + std::to_string(physicsselection), GetItemText(21).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsScatter" + std::to_string(physicsselection), GetItemText(112).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsEOSTorque" + std::to_string(physicsselection), GetItemText(113).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsEOSTorqueAngle" + std::to_string(physicsselection), GetItemText(189).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsReturnStrength" + std::to_string(physicsselection), GetItemText(23).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsElasticityFalloff" + std::to_string(physicsselection), GetItemText(22).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsFriction" + std::to_string(physicsselection), GetItemText(109).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "FlipperPhysicsCoilRampUp" + std::to_string(physicsselection), GetItemText(110).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsGravityConstant" + std::to_string(physicsselection), GetItemText(1100).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsContactFriction" + std::to_string(physicsselection), GetItemText(1101).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsElasticity" + std::to_string(physicsselection), GetItemText(1708).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsElasticityFalloff" + std::to_string(physicsselection), GetItemText(1709).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsScatterAngle" + std::to_string(physicsselection), GetItemText(1710).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsContactScatterAngle" + std::to_string(physicsselection), GetItemText(1102).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsMinSlope" + std::to_string(physicsselection), GetItemText(1103).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "TablePhysicsMaxSlope" + std::to_string(physicsselection), GetItemText(1104).c_str());
    g_pvp->m_settings.SaveValue(Settings::Player, "PhysicsSetName" + std::to_string(physicsselection), GetItemText(1110).c_str());
}
