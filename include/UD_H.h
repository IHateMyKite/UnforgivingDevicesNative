//we dont give a f*ck about lint or other static code analysis 
#pragma warning( disable : 4100 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4267 )
#pragma warning( disable : 4245 )

//include some usefull macros
#include <UD_Macros.h>

//#ifndef UD_H
//    #define UD_H
//    //This file contains includes for all project files
//    #include "windows.h"
//    #undef GetObject
//    #include <UD_Spinlock.h>
//    #include <UD_Config.h>
//    #include <UD_Lib.h>
//    #include <UD_Keywords.h>
//    #include <UD_ActorSlotManager.h>
//    #include <UD_Updater.h>
//    #include <UD_MinigameEffect.h>
//    #include <UD_RegisterPapyrus.h>
//    #include <UD_GameEvents.h>
//    #include <UD_UI.h>
//    #include <UD_Utility.h>
//    #include <UD_Inventory.h>
//    #include <UD_Animation.h>
//    #include <UD_Skill.h>
//    #include <UD_Modifiers.h>
//    #include <UD_Serialization.h>
//    #include <UD_ControlManager.h>
//    #include <UD_ModEvents.h>
//    #include <UD_PlayerStatus.h>
//    #include <UD_Input.h>
//    #include <UD_PapyrusDelegate.h>
//    #include <UD_Materials.h>
//    #include <UD_Diagnosis.h>
//    #include <UD_Lockpick.h>
//    #include <OrgasmSystem/OrgasmEvents.h>
//    #include <OrgasmSystem/OrgasmData.h>
//    #include <OrgasmSystem/OrgasmManager.h>
//    #include <OrgasmSystem/OrgasmConfig.h>
//#endif // !UD_H