#pragma once

namespace ORS
{
    class OrgasmConfig
    {
    SINGLETONHEADER(OrgasmConfig)
    public:
        void LoadConfig(const boost::property_tree::ptree& a_ptree);
    public:
        //=== config variables ===
        //how often will be expression updated (if set)
        float EXPRUPDATETIME = 2.0f;

        //thresholds for updating expression
        float EXPUPDATEMINTH = 0.1f;
        float EXPUPDATEMAXTH = 0.2f;

        //orgasm progress required for linked widget to show
        float WIDGETSHOWTH   = 0.025f;

        //distance to travel in 1s for 100% orgasm rate
        float BASEDISTANCE   = 2500.0f;
    };


}