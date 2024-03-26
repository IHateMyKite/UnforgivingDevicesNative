#pragma once

namespace UD 
{
    class KeywordManager
    {
        public:
            static void Reload()
            {
                    RE::TESDataHandler* loc_datahandler = RE::TESDataHandler::GetSingleton();
                    udinvdevice = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x1553DD,"UnforgivingDevices.esp"));
                    udrendevice = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x11A352,"UnforgivingDevices.esp"));

                    ddhobbleskirt       = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x05F4BA,"Devious Devices - Integration.esm"));
                    ddhobbleskirtrelax  = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x05FA1E,"Devious Devices - Integration.esm"));
                    ddankleshacles      = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x05F4BB,"Devious Devices - Integration.esm"));

                    ddhb = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x05226C,"Devious Devices - Integration.esm"));

                    //clear keywords
                    ddhbkwds.clear();

                    //elbowtie
                    ddhbkwds.push_back({static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x08A76C,"Devious Devices - Integration.esm")),0x00000080});
                    //yoke
                    ddhbkwds.push_back({static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x02C531,"Devious Devices - Assets.esm")),0x00000004});
                    //cuffs front
                    ddhbkwds.push_back({static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x063AD9,"Devious Devices - Integration.esm")),0x00000008});
                    //armbinder
                    ddhbkwds.push_back({static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x00CA3A,"Devious Devices - Assets.esm")),0x00000010});
                    //elbowbinder
                    ddhbkwds.push_back({static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x062539,"Devious Devices - Integration.esm")),0x00000020});
                    //pet suit
                    ddhbkwds.push_back({static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x086C1D,"Devious Devices - Integration.esm")),0x00000040});
                    //straitjacket
                    ddhbkwds.push_back({static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x060A46,"Devious Devices - Integration.esm")),0x00000200});
                    //yoke BB
                    ddhbkwds.push_back({static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x062538,"Devious Devices - Integration.esm")),0x00000400});
            
                    ddmittens = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x05FF81,"Devious Devices - Integration.esm"));

                    ddgag = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x007EB8,"Devious Devices - Assets.esm"));

                    //invisible devices
                    udinvhb = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x156EEE,"UnforgivingDevices.esp"));
                    udinvhs = static_cast<RE::BGSKeyword*>(loc_datahandler->LookupForm(0x156EEF,"UnforgivingDevices.esp"));
            }
        public:
            static RE::BGSKeyword* udinvdevice;
            static RE::BGSKeyword* udrendevice;

            static RE::BGSKeyword* ddhobbleskirt;
            static RE::BGSKeyword* ddhobbleskirtrelax;
            static RE::BGSKeyword* ddankleshacles;

            static RE::BGSKeyword* ddhb;
            static std::vector<std::pair<RE::BGSKeyword*,int>> ddhbkwds;

            static RE::BGSKeyword* ddmittens;
            static RE::BGSKeyword* ddgag;

            static RE::BGSKeyword* udinvhb;
            static RE::BGSKeyword* udinvhs;
    };
}