#include "StdAfx.h"
#include "resource.h"
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <fstream>
#include <sstream>
#include "PhysicsOptionsDialog.h"

using namespace rapidxml;

const unsigned int num_physicsoptions = 8;
static char * physicsoptions[num_physicsoptions] ={ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static unsigned int physicsselection = 0;


PhysicsOptionsDialog::PhysicsOptionsDialog() : CDialog(IDD_PHYSICS_OPTIONS)
{

}

BOOL PhysicsOptionsDialog::OnInitDialog()
{
    char tmp[256];

    HWND hwndList = GetDlgItem(IDC_PhysicsList).GetHwnd();

    const size_t size = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
    for(size_t i = 0; i < size; i++)
    {
        if(physicsoptions[i])
            delete[] physicsoptions[i];
        int* sd = (int *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
        delete sd;
    }
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

    for(unsigned int i = 0; i < num_physicsoptions; i++)
    {
        physicsoptions[i] = new char[256];
        sprintf_s(tmp, 256, "PhysicsSetName%u", i);
        if(GetRegString("Player", tmp, physicsoptions[i], 256) != S_OK)
            sprintf_s(physicsoptions[i], 256, "Set %u", i + 1);
        sprintf_s(tmp, 256, "%u: %s", i + 1, physicsoptions[i]);
        const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (size_t)tmp);
        int * const sd = new int;
        *sd = i;
        SendMessage(hwndList, LB_SETITEMDATA, index, (LPARAM)sd);
    }
    SendMessage(hwndList, LB_SETCURSEL, physicsselection, 0);

    HRESULT hr;

    float FlipperPhysicsMass = 1.f;
    sprintf_s(tmp, 256, "FlipperPhysicsMass%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsMass);
    if(hr != S_OK)
        FlipperPhysicsMass = 1.f;

    SetItemText(DISPID_Flipper_Speed, FlipperPhysicsMass);

    float FlipperPhysicsStrength = 2200.f;
    sprintf_s(tmp, 256, "FlipperPhysicsStrength%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsStrength);
    if(hr != S_OK)
        FlipperPhysicsStrength = 2200.f;

    SetItemText(19, FlipperPhysicsStrength);

    float FlipperPhysicsElasticity = 0.8f;
    sprintf_s(tmp, 256, "FlipperPhysicsElasticity%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsElasticity);
    if(hr != S_OK)
        FlipperPhysicsElasticity = 0.8f;

    SetItemText(21, FlipperPhysicsElasticity);

    float FlipperPhysicsScatter = 0.f;
    sprintf_s(tmp, 256, "FlipperPhysicsScatter%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsScatter);
    if(hr != S_OK)
        FlipperPhysicsScatter = 0.f;

    SetItemText(112, FlipperPhysicsScatter);

    float FlipperPhysicsTorqueDamping = 0.75f;
    sprintf_s(tmp, 256, "FlipperPhysicsEOSTorque%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsTorqueDamping);
    if(hr != S_OK)
        FlipperPhysicsTorqueDamping = 0.75f;

    SetItemText(113, FlipperPhysicsTorqueDamping);

    float FlipperPhysicsTorqueDampingAngle = 6.f;
    sprintf_s(tmp, 256, "FlipperPhysicsEOSTorqueAngle%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsTorqueDampingAngle);
    if(hr != S_OK)
        FlipperPhysicsTorqueDampingAngle = 6.f;

    SetItemText(189, FlipperPhysicsTorqueDampingAngle);

    float FlipperPhysicsReturnStrength = 0.058f;
    sprintf_s(tmp, 256, "FlipperPhysicsReturnStrength%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsReturnStrength);
    if(hr != S_OK)
        FlipperPhysicsReturnStrength = 0.058f;

    SetItemText(23, FlipperPhysicsReturnStrength);

    float FlipperPhysicsElasticityFalloff = 0.43f;
    sprintf_s(tmp, 256, "FlipperPhysicsElasticityFalloff%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsElasticityFalloff);
    if(hr != S_OK)
        FlipperPhysicsElasticityFalloff = 0.43f;

    SetItemText(22, FlipperPhysicsElasticityFalloff);

    float FlipperPhysicsFriction = 0.6f;
    sprintf_s(tmp, 256, "FlipperPhysicsFriction%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsFriction);
    if(hr != S_OK)
        FlipperPhysicsFriction = 0.6f;

    SetItemText(109, FlipperPhysicsFriction);

    float FlipperPhysicsCoilRampUp = 3.f;
    sprintf_s(tmp, 256, "FlipperPhysicsCoilRampUp%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsCoilRampUp);
    if(hr != S_OK)
        FlipperPhysicsCoilRampUp = 3.f;

    SetItemText(110, FlipperPhysicsCoilRampUp);

    float TablePhysicsGravityConstant = DEFAULT_TABLE_GRAVITY;
    sprintf_s(tmp, 256, "TablePhysicsGravityConstant%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsGravityConstant);
    if(hr != S_OK)
        TablePhysicsGravityConstant = DEFAULT_TABLE_GRAVITY;

    SetItemText(1100, TablePhysicsGravityConstant);

    float TablePhysicsContactFriction = DEFAULT_TABLE_CONTACTFRICTION;
    sprintf_s(tmp, 256, "TablePhysicsContactFriction%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsContactFriction);
    if(hr != S_OK)
        TablePhysicsContactFriction = DEFAULT_TABLE_CONTACTFRICTION;

    SetItemText(1101, TablePhysicsContactFriction);

    float TablePhysicsElasticity = DEFAULT_TABLE_ELASTICITY;
    sprintf_s(tmp, 256, "TablePhysicsElasticity%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsElasticity);
    if(hr != S_OK)
        TablePhysicsElasticity = DEFAULT_TABLE_ELASTICITY;

    SetItemText(1708, TablePhysicsElasticity);

    float TablePhysicsElasticityFalloff = DEFAULT_TABLE_ELASTICITY_FALLOFF;
    sprintf_s(tmp, 256, "TablePhysicsElasticityFalloff%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsElasticityFalloff);
    if(hr != S_OK)
        TablePhysicsElasticityFalloff = DEFAULT_TABLE_ELASTICITY_FALLOFF;

    SetItemText(1709, TablePhysicsElasticityFalloff);

    float TablePhysicsScatterAngle = DEFAULT_TABLE_PFSCATTERANGLE;
    sprintf_s(tmp, 256, "TablePhysicsScatterAngle%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsScatterAngle);
    if(hr != S_OK)
        TablePhysicsScatterAngle = DEFAULT_TABLE_PFSCATTERANGLE;

    SetItemText(1710, TablePhysicsScatterAngle);

    float TablePhysicsContactScatterAngle = DEFAULT_TABLE_SCATTERANGLE;
    sprintf_s(tmp, 256, "TablePhysicsContactScatterAngle%u", physicsselection);
    hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsContactScatterAngle);
    if(hr != S_OK)
        TablePhysicsContactScatterAngle = DEFAULT_TABLE_SCATTERANGLE;

    SetItemText(1102, TablePhysicsContactScatterAngle);

    CString txt(physicsoptions[physicsselection]);
    SetDlgItemText(1110, txt);
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
    char speed[16];
    char stength[16];
    char elasticity[16];
    char scatter[16];
    char eosTorque[16];
    char eosTorqueAngle[16];
    char returnStrength[16];
    char elasticityFalloff[16];
    char friction[16];
    char coilRampup[16];
    char name[32];
};

static PhysValues loadValues;
static CString txtStr;


BOOL PhysicsOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch(HIWORD(wParam))
    {
        case LBN_SELCHANGE:
        {
            HWND hwndList = GetDlgItem(IDC_PhysicsList).GetHwnd();

            const size_t tmp = SendMessage(hwndList, LB_GETCURSEL, 0, 0);

            if(tmp != physicsselection)
            {
                int result = ::MessageBox(NULL, "Save", "Save current physics set?", MB_YESNOCANCEL | MB_ICONQUESTION);
                if(result == IDYES)
                    SaveCurrentPhysicsSetting();

                if(result != IDCANCEL)
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
            if(LoadSetting() == false)
                break;

            SetDlgItemText(1100, CString(loadValues.gravityConstant));
            SetDlgItemText(1101, CString(loadValues.contactFriction));
            SetDlgItemText(1708, CString(loadValues.tableElasticity));
            SetDlgItemText(1709, CString(loadValues.tableElasticityFalloff));
            SetDlgItemText(1710, CString(loadValues.playfieldScatter));
            SetDlgItemText(1102, CString(loadValues.defaultElementScatter));

            SetDlgItemText(DISPID_Flipper_Speed, CString(loadValues.speed));
            SetDlgItemText(19, CString(loadValues.returnStrength));
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
            char szFileName[1024];
            char szInitialDir[1024];
            szFileName[0] = '\0';

            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hInstance = g_hinst;
            ofn.hwndOwner = g_pvp->m_hwnd;
            // TEXT
            ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = _MAX_PATH;
            ofn.lpstrDefExt = "vpp";
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

            const HRESULT hr = GetRegString("RecentDir", "LoadDir", szInitialDir, 1024);
            char szFoo[MAX_PATH];
            if(hr == S_OK)
            {
                ofn.lpstrInitialDir = szInitialDir;
            }
            else
            {
                lstrcpy(szFoo, "c:\\");
                ofn.lpstrInitialDir = szFoo;
            }

            const int ret = GetSaveFileName(&ofn);
            if(ret == 0)
                break;

            xml_document<> xmlDoc;
            xml_node<>*dcl = xmlDoc.allocate_node(node_declaration);
            dcl->append_attribute(xmlDoc.allocate_attribute("version", "1.0"));
            dcl->append_attribute(xmlDoc.allocate_attribute("encoding", "utf-8"));
            xmlDoc.append_node(dcl);

            //root node
            xml_node<>*root = xmlDoc.allocate_node(node_element, "physics");
            xml_node<>*flipper = xmlDoc.allocate_node(node_element, "flipper");
            xml_node<>*table = xmlDoc.allocate_node(node_element, "table");

            xml_node<>*flipSpeed = xmlDoc.allocate_node(node_element, "speed", (new string(GetItemText(DISPID_Flipper_Speed).c_str()))->c_str());
            flipper->append_node(flipSpeed);

            xml_node<>*flipPhysStrength = xmlDoc.allocate_node(node_element, "strength", (new string(GetItemText(19).c_str()))->c_str());
            flipper->append_node(flipPhysStrength);

            xml_node<>*flipElasticity = xmlDoc.allocate_node(node_element, "elasticity", (new string(GetItemText(21).c_str()))->c_str());
            flipper->append_node(flipElasticity);

            xml_node<>*flipScatter = xmlDoc.allocate_node(node_element, "scatter", (new string(GetItemText(112).c_str()))->c_str());
            flipper->append_node(flipScatter);

            xml_node<>*flipTorqueDamping = xmlDoc.allocate_node(node_element, "eosTorque", (new string(GetItemText(113).c_str()))->c_str());
            flipper->append_node(flipTorqueDamping);

            xml_node<>*flipTorqueDampingAngle = xmlDoc.allocate_node(node_element, "eosTorqueAngle", (new string(GetItemText(189).c_str()))->c_str());
            flipper->append_node(flipTorqueDampingAngle);

            xml_node<>*flipReturnStrength = xmlDoc.allocate_node(node_element, "returnStrength", (new string(GetItemText(23).c_str()))->c_str());
            flipper->append_node(flipReturnStrength);

            xml_node<>*flipElasticityFalloff = xmlDoc.allocate_node(node_element, "elasticityFalloff", (new string(GetItemText(22).c_str()))->c_str());
            flipper->append_node(flipElasticityFalloff);

            xml_node<>*flipfriction = xmlDoc.allocate_node(node_element, "friction", (new string(GetItemText(109).c_str()))->c_str());
            flipper->append_node(flipfriction);

            xml_node<>*flipCoilRampUp = xmlDoc.allocate_node(node_element, "coilRampUp", (new string(GetItemText(110).c_str()))->c_str());
            flipper->append_node(flipCoilRampUp);

            xml_node<>*tabGravityConst = xmlDoc.allocate_node(node_element, "gravityConstant", (new string(GetItemText(1100).c_str()))->c_str());
            table->append_node(tabGravityConst);

            xml_node<>*tabContactFriction = xmlDoc.allocate_node(node_element, "contactFriction", (new string(GetItemText(1101).c_str()))->c_str());
            table->append_node(tabContactFriction);

            xml_node<>*tabElasticity = xmlDoc.allocate_node(node_element, "elasticity", (new string(GetItemText(1708).c_str()))->c_str());
            table->append_node(tabElasticity);

            xml_node<>*tabElasticityFalloff = xmlDoc.allocate_node(node_element, "elasticityFalloff", (new string(GetItemText(1709).c_str()))->c_str());
            table->append_node(tabElasticityFalloff);

            xml_node<>*tabScatterAngle = xmlDoc.allocate_node(node_element, "playfieldScatter", (new string(GetItemText(1710).c_str()))->c_str());
            table->append_node(tabScatterAngle);

            xml_node<>*tabContactScatterAngle = xmlDoc.allocate_node(node_element, "defaultElementScatter", (new string(GetItemText(1102).c_str()))->c_str());
            table->append_node(tabContactScatterAngle);

            xml_node<>*settingName = xmlDoc.allocate_node(node_element, "name", (new string(GetItemText(1110).c_str()))->c_str());
            root->append_node(settingName);
            root->append_node(table);
            root->append_node(flipper);
            xmlDoc.append_node(root);

            std::ofstream myfile(ofn.lpstrFile);
            myfile << xmlDoc;
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
    HWND hwndList = GetDlgItem(IDC_PhysicsList).GetHwnd();
    const size_t size = ::SendMessage(hwndList, LB_GETCOUNT, 0, 0);
    for(size_t i = 0; i < size; i++)
    {
        if(physicsoptions[i])
            delete[] physicsoptions[i];
        int* sd = (int *)::SendMessage(hwndList, LB_GETITEMDATA, i, 0);
        delete sd;
    }
    ::SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
}

bool PhysicsOptionsDialog::LoadSetting()
{
    char szFileName[1024];
    char szInitialDir[1024];
    szFileName[0] = '\0';

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hInstance = g_hinst;
    ofn.hwndOwner = g_pvp->m_hwnd;
    // TEXT
    ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = _MAX_PATH;
    ofn.lpstrDefExt = "vpp";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

    const HRESULT hr = GetRegString("RecentDir", "LoadDir", szInitialDir, 1024);
    char szFoo[MAX_PATH];
    if(hr == S_OK)
    {
        ofn.lpstrInitialDir = szInitialDir;
    }
    else
    {
        lstrcpy(szFoo, "c:\\");
        ofn.lpstrInitialDir = szFoo;
    }

    const int ret = GetOpenFileName(&ofn);
    if(ret == 0)
        return false;

    xml_document<> xmlDoc;
    try
    {
        std::stringstream buffer;
        std::ifstream myFile(ofn.lpstrFile);
        buffer << myFile.rdbuf();
        myFile.close();

        std::string content(buffer.str());
        xmlDoc.parse<0>(&content[0]);
        xml_node<> *root = xmlDoc.first_node("physics");
        xml_node<> *table = root->first_node("table");
        xml_node<> *flipper = root->first_node("flipper");

        strncpy_s(loadValues.gravityConstant, table->first_node("gravityConstant")->value(), 16);
        strncpy_s(loadValues.contactFriction, table->first_node("contactFriction")->value(), 16);
        strncpy_s(loadValues.tableElasticity, table->first_node("elasticity")->value(), 16);
        strncpy_s(loadValues.tableElasticityFalloff, table->first_node("elasticityFalloff")->value(), 16);
        strncpy_s(loadValues.playfieldScatter, table->first_node("playfieldScatter")->value(), 16);
        strncpy_s(loadValues.defaultElementScatter, table->first_node("defaultElementScatter")->value(), 16);

        strncpy_s(loadValues.speed, flipper->first_node("speed")->value(), 16);
        strncpy_s(loadValues.elasticity, flipper->first_node("strength")->value(), 16);
        strncpy_s(loadValues.scatter, flipper->first_node("scatter")->value(), 16);
        strncpy_s(loadValues.eosTorque, flipper->first_node("eosTorque")->value(), 16);
        strncpy_s(loadValues.eosTorqueAngle, flipper->first_node("eosTorqueAngle")->value(), 16);
        strncpy_s(loadValues.returnStrength, flipper->first_node("returnStrength")->value(), 16);
        strncpy_s(loadValues.elasticityFalloff, flipper->first_node("elasticityFalloff")->value(), 16);
        strncpy_s(loadValues.friction, flipper->first_node("friction")->value(), 16);
        strncpy_s(loadValues.coilRampup, flipper->first_node("coilRampUp")->value(), 16);
        strncpy_s(loadValues.name, root->first_node("name")->value(), 30);
    }
    catch(...)
    {
        ShowError("Error parsing physics settings file");
        xmlDoc.clear();
        return false;
    }
    xmlDoc.clear();
    return true;
}

CString PhysicsOptionsDialog::GetItemText(int id)
{
    CString textStr;
    return GetDlgItemText(id);
}

void PhysicsOptionsDialog::SetItemText(int id, float value)
{
    char textBuf[256] ={ 0 };
    f2sz(value, textBuf);
    CString textStr(textBuf);
    SetDlgItemText(id, textStr);
}


void PhysicsOptionsDialog::SaveCurrentPhysicsSetting()
{
    char tmp2[256];
    CString str;

    str = GetItemText(DISPID_Flipper_Speed);
    sprintf_s(tmp2, 256, "FlipperPhysicsMass%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(19);
    sprintf_s(tmp2, 256, "FlipperPhysicsStrength%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(21);
    sprintf_s(tmp2, 256, "FlipperPhysicsElasticity%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str=GetItemText(112);
    sprintf_s(tmp2, 256, "FlipperPhysicsScatter%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(113);
    sprintf_s(tmp2, 256, "FlipperPhysicsEOSTorque%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(189);
    sprintf_s(tmp2, 256, "FlipperPhysicsEOSTorqueAngle%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(23);
    sprintf_s(tmp2, 256, "FlipperPhysicsReturnStrength%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(22);
    sprintf_s(tmp2, 256, "FlipperPhysicsElasticityFalloff%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(109);
    sprintf_s(tmp2, 256, "FlipperPhysicsFriction%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(110);
    sprintf_s(tmp2, 256, "FlipperPhysicsCoilRampUp%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(1100);
    sprintf_s(tmp2, 256, "TablePhysicsGravityConstant%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(1101);
    sprintf_s(tmp2, 256, "TablePhysicsContactFriction%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(1708);
    sprintf_s(tmp2, 256, "TablePhysicsElasticity%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(1709);
    sprintf_s(tmp2, 256, "TablePhysicsElasticityFalloff%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(1710);
    sprintf_s(tmp2, 256, "TablePhysicsScatterAngle%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(1102);
    sprintf_s(tmp2, 256, "TablePhysicsContactScatterAngle%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));

    str = GetItemText(1110);
    sprintf_s(tmp2, 256, "PhysicsSetName%u", physicsselection);
    SetRegValue("Player", tmp2, REG_SZ, str.c_str(), lstrlen(str.c_str()));
}
