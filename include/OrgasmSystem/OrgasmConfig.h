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

        // duration after orgasm, in which will be orgasm variables on actor not calculated (so orgasm system will be basicaly paused)
        int   ORGASMTIMEOUT     = 5;

        //orgasm duration in seconds. Scales with horny level from min value to max value
        float ORGASMDURATIONMIN = 10.0f;
        float ORGASMDURATIONMAX = 30.0f;

        //how much duration is added on additional orgasms, when actor didn't stop orgasming yet
        float ORGASMDURATIONADD = 5.0f;

        bool  HORNYMESSAGES     = true;
    };


}