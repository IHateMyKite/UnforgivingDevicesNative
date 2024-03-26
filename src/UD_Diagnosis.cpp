#include <UD_Diagnosis.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <Windows.h>
#include <UD_Macros.h>

SINGLETONBODY(UD::Diagnosis)

void UD::Diagnosis::Setup()
{
    if (!_installed)
    {
        _udid = reinterpret_cast<RE::BGSKeyword*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x1553DD,"UnforgivingDevices.esp"));
        _udrd = reinterpret_cast<RE::BGSKeyword*>(RE::TESDataHandler::GetSingleton()->LookupForm(0x11A352,"UnforgivingDevices.esp"));

        HINSTANCE dllHandle = LoadLibrary(TEXT("DeviousDevices.dll"));
        if (dllHandle != NULL)
        {
            FARPROC pGetDatabase = GetProcAddress(HMODULE (dllHandle),"GetDatabase");
            DDNGGetDatabase = GetDatabase(pGetDatabase);
            DEBUG("Diagnosis::Setup() - GetDatabase imported - addrs = 0x{:016X}",(uintptr_t)DDNGGetDatabase)
            _imported = DDNGGetDatabase != NULL;
            //FreeLibrary(dllHandle);
        }
        else
        {
            ERROR("Diagnosis::Setup() - ERROR: Cant find DeviousDevices.dll!!")
        }
        _installed = true;
    }
}

int UD::Diagnosis::CheckPatchedDevices()
{
    DEBUG("CheckPatchedDevices called")
    if (!_imported || (DDNGGetDatabase == nullptr)) 
    {
        ERROR("Cant check database because it was impossible to import the functions!!!!!!!!!!!")
        return 0;
    }
    auto loc_db = DDNGGetDatabase();
    DEBUG("Database size = {}",loc_db.size())

    int loc_res = 0;

    for (auto&& [id,unit] : loc_db)
    {
        //check if device have both id and rd set
        if (!unit.deviceInventory || !unit.deviceRendered)
        {
            ERROR("Device ID={} / RD=0x{:08X} have either not set ID or RD",unit.deviceInventory ? unit.deviceInventory->GetName() : "NONE",
                unit.deviceRendered ? unit.deviceRendered->GetFormID() : 0x0)
            continue;
        }
        //first lets check if currrent iteretio have both UD keywords. If yes, then the device is correctly patched and we can ignore it
        if (unit.deviceInventory->HasKeyword(_udid) && unit.deviceRendered->HasKeyword(_udrd))
        {
            //LOG("Device {} is patched correctly, skipping...",unit.deviceInventory->GetName())
            continue;
        }

        // Now lets check history. If at least once in the history had the device the keyword, then it should be also on last record. 
        // If not, then something is overriding the patch
        bool loc_ispatched = false;
        std::string loc_patchingmod = "";
        for (auto&& record : unit.history)
        {
            if (std::find(record.keywords.begin(),record.keywords.end(),_udid) != record.keywords.end())
            {
                //device had ID from mod
                loc_ispatched = true;
                loc_patchingmod = record.deviceMod->name;
            }
        }

        if (loc_ispatched)
        {
            if (!unit.deviceInventory->HasKeyword(_udid))
            {
                ERROR("Device ID=0x{:08X} ({}) / RD=0x{:08X} is patched incorrectly. !!ID keyword is missing!!",
                unit.deviceInventory->GetFormID(),
                unit.deviceInventory->GetName(),
                unit.deviceRendered->GetFormID())
                ERROR("\t Source mod for ID is {}",unit.history[0].deviceMod->name)
                ERROR("\t Last editing mod for ID is {}",unit.deviceMod->name)
                ERROR("\t Patching mod for ID is {}",loc_patchingmod)
                
            }
            if (!unit.deviceRendered->HasKeyword(_udrd))
            {
                ERROR("Device ID=0x{:08X} ({}) / RD=0x{:08X} is patched incorrectly. !!RD keyword is missing!!",
                unit.deviceInventory->GetFormID(),
                unit.deviceInventory->GetName(),
                unit.deviceRendered->GetFormID())
                ERROR("\t Source mod for ID is {}",unit.history[0].deviceMod->name)
                ERROR("\t Last editing mod for ID is {}",unit.deviceMod->name)
                ERROR("\t Patching mod for ID is {}",loc_patchingmod)
            }
            loc_res++;
        }
    }
    DEBUG("Total number of incorrectly patched devices = {}",loc_res)
    return loc_res;
}