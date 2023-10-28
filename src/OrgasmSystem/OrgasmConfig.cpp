#include <OrgasmSystem/OrgasmConfig.h>

SINGLETONBODY(ORS::OrgasmConfig)

void ORS::OrgasmConfig::LoadConfig(const boost::property_tree::ptree& a_ptree)
{
    EXPRUPDATETIME = a_ptree.get<float>("Orgasm.fExpressionUpdateTime");
    EXPUPDATEMINTH = a_ptree.get<float>("Orgasm.fExpressionThresholdMin");
    EXPUPDATEMAXTH = a_ptree.get<float>("Orgasm.fExpressionThresholdMax");
    WIDGETSHOWTH   = a_ptree.get<float>("Orgasm.fWidgetShowThreshold");
    BASEDISTANCE   = a_ptree.get<float>("Orgasm.fBaseDistance");
}
