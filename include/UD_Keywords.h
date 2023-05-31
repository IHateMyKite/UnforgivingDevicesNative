#pragma once

namespace UD 
{
    static class KeywordManager
    {
        public:
            static void Reload()
            {
                    RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
                    udinvdevice = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x1553DD,"UnforgivingDevices.esp"));
                    udrendevice = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x11A352,"UnforgivingDevices.esp"));
            }
            static RE::BGSKeyword* udinvdevice;
            static RE::BGSKeyword* udrendevice;
    };
}