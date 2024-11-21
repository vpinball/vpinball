#include "core/stdafx.h"

#include "main.h"
#include "editablereg.h"

robin_hood::unordered_map<ItemTypeEnum, EditableInfo> EditableRegistry::m_map;
