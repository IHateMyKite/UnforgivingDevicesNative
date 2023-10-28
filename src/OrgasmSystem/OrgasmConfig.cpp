#include <OrgasmSystem/OrgasmConfig.h>

SINGLETONBODY(ORS::OrgasmConfig)

#include <boost/algorithm/clamp.hpp>
using boost::algorithm::clamp;

void ORS::OrgasmConfig::LoadConfig(const boost::property_tree::ptree& a_ptree)
{
    EXPRUPDATETIME = a_ptree.get<float>("Orgasm.fExpressionUpdateTime");
    EXPUPDATEMINTH = a_ptree.get<float>("Orgasm.fExpressionThresholdMin");
    EXPUPDATEMAXTH = a_ptree.get<float>("Orgasm.fExpressionThresholdMax");
    WIDGETSHOWTH   = a_ptree.get<float>("Orgasm.fWidgetShowThreshold");
    BASEDISTANCE   = clamp(a_ptree.get<float>("Orgasm.fBaseDistance"),100.0f,100000.0f);
    ORGASMTIMEOUT  = a_ptree.get<float>("Orgasm.fOrgasmTimeout");
    ORGASMDURATIONMIN = clamp(a_ptree.get<float>("Orgasm.fOrgasmDurationMin"),5.0f,300.0f);
    ORGASMDURATIONMAX = clamp(a_ptree.get<float>("Orgasm.fOrgasmDurationMax"),ORGASMDURATIONMIN,ORGASMDURATIONMIN + 300.0f);
    ORGASMDURATIONADD = a_ptree.get<float>("Orgasm.fOrgasmDurationAdd");
}
