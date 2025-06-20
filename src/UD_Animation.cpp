#include <UD_Animation.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_Inventory.h>
#include <UD_Keywords.h>
#include <UD_SLPP.h>
#include <boost/json/src.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>

SINGLETONBODY(UD::AnimationManager);

int UD::AnimationManager::GetActorConstrainsInter(RE::Actor* a_actor) const
{
    if (a_actor == nullptr) return 0x00000000;

    auto loc_devices                = InventoryHandler::GetSingleton()->GetRenderDevices(a_actor, true);
    auto loc_invisibledevices       = InventoryHandler::GetSingleton()->GetInvisibleDevices(a_actor);
        
    int loc_res         = 0x00000000;
        
    loc_res |= ProccessDeviceArray(a_actor,loc_devices);
    loc_res |= ProccessDeviceArray(a_actor,loc_invisibledevices);

    LOG("GetActorConstrainsInter({}) - Result = {:08X}",a_actor->GetName(),loc_res)

    return loc_res;
}

inline bool UD::AnimationManager::CheckWeaponDisabled(RE::Actor* a_actor)
{
    if (a_actor == nullptr || a_actor->IsPlayerRef()) return false;
    Utils::UniqueLock lock(_SaveLock);
    return (_weapondisabled.find(a_actor->GetHandle().native_handle()) != _weapondisabled.end());
}

void UD::AnimationManager::DisableWeapons(RE::Actor* a_actor, bool a_state)
{
    LOG("DisableWeapons({},{}) called",a_actor ? a_actor->GetName() : "NONE", a_state)
    if (a_actor == nullptr || a_actor->IsPlayerRef()) return;
    
    const bool loc_disabled = CheckWeaponDisabled(a_actor);
    
    Utils::UniqueLock lock(_SaveLock);
    
    const Handle loc_handle = a_actor->GetHandle().native_handle();
    if (a_state)
    {
        const DisableCounter loc_cnt = _weapondisabled[loc_handle] + 1;
        _weapondisabled[loc_handle] = loc_cnt;

        LOG("DisableWeapons({},{}) - Counter = {}",a_actor ? a_actor->GetName() : "NONE", a_state,loc_cnt)
    }
    else if (loc_disabled && !a_state)
    {
        const DisableCounter loc_cnt = _weapondisabled[loc_handle] - 1;

        LOG("DisableWeapons({},{}) - Counter = {}",a_actor ? a_actor->GetName() : "NONE", a_state,loc_cnt)
        if (loc_cnt == 0)
        {
            LOG("DisableWeapons({},{}) - Removing weapon disable",a_actor ? a_actor->GetName() : "NONE", a_state)
            
            (void)_weapondisabled.erase(loc_handle);
        }
        else
        {
            _weapondisabled[loc_handle] = loc_cnt; 
        }
    }
}

std::vector<std::string> UD::AnimationManager::GetAllAnimationFiles() const
{
    std::vector<std::string> loc_res;

    for (auto&& [name,file] : _jsoncache)
    {
        loc_res.push_back(name);
    }

    LOG("AnimationManager::GetAllAnimationFiles() -> {}",boost::join(loc_res,", "))

    return loc_res;
}

std::vector<std::string> UD::AnimationManager::GetAllAnimationFilesErrors() const
{
    std::vector<std::string> loc_res;

    for (auto&& [name,file] : _jsoncache)
    {
        loc_res.push_back(file.error);
    }

    return loc_res;
}

std::vector<int> UD::AnimationManager::GetAllAnimationFilesStatus() const
{
    std::vector<int> loc_res;

    for (auto&& [name,file] : _jsoncache)
    {
        loc_res.push_back(file.status);
    }

    return loc_res;
}

std::vector<std::string> UD::AnimationManager::GetAnimationsFromJSON(std::string a_def, std::vector<RE::Actor*> a_actors, int a_constraintsORA1, int a_constraintsORA2)
{
    LOG("AnimationManager::GetAnimationsFromJSON({},{},{}) called",a_def,a_constraintsORA1,a_constraintsORA2)

    if (a_actors.size() == 0) return {"ERROR","Incorrect arguments passed"};

    int loc_part_index = a_def.find(":");
    if (loc_part_index == a_def.npos)
    {
        return {"ERROR","Incorrect arguments passed"};
    }

    std::string loc_filename = a_def.substr(0, loc_part_index);
    std::string loc_jsonpath = a_def.substr(loc_part_index + 2);
    std::vector<std::string> loc_actor_animVars(a_actors.size());

    std::regex loc_nameregex(R"regex(^(\w*)(?:\.[jJ][sS][oO][nN])*$)regex");

    //Remove possible sufix
    loc_filename = std::regex_replace(loc_filename,loc_nameregex,"$1");

    auto loc_it = _jsoncache.find(loc_filename);
    if (loc_it == _jsoncache.end())
    {
        ERROR("Could not find animation file {}",loc_filename)
        return {"ERROR",std::format("Could not find animation file {}",loc_filename)};
    }

    if (loc_it->second.status != sOK) return {"ERROR",std::format("JSON {} is disabled",loc_filename)}; 

    boost::json::object& loc_object = loc_it->second.json->as_object();

    if (loc_object.size() == 0)
    {
        return {"ERROR",std::format("{} is empty!",loc_filename)};
    }


    for (int i = 0; i < a_actors.size(); i++)
    {
        int loc_actor_constraints;
        if (a_constraintsORA1 >= 0 && i == 0)
            loc_actor_constraints = a_constraintsORA1;
        else if (a_constraintsORA2 >= 0 && i == 1)
            loc_actor_constraints = a_constraintsORA2;
        else
            loc_actor_constraints = GetActorConstrainsInter(a_actors[i]);
        
        const std::string loc_anim_var_path = loc_jsonpath + ".A" + std::to_string(i + 1);
        auto loc_A = RecursiveFind(loc_object,loc_anim_var_path);

        try
        {
            if (loc_A.is_array())
            {
                std::vector<std::string> loc_anim_var_path_array;
                for (int y = 0; y < loc_A.as_array().size(); y++)
                {
                    const std::string loc_const = std::format("{}[{}]",loc_anim_var_path,y);
                    if (_CheckConstraints(loc_object,loc_const,loc_actor_constraints)) loc_anim_var_path_array.push_back(loc_const);
                }

                if (loc_anim_var_path_array.size() > 0)
                {
                    loc_actor_animVars[i] = loc_anim_var_path_array[RandomGenerator::GetSingleton()->RandomInt(0, loc_anim_var_path_array.size() - 1)];
                }
                else
                {
                    return {"ERROR",std::format("Can't find valid animation variant in def {} for Actor with constraints 0x{:08X}",a_def,loc_actor_constraints)}; 
                }

            }
            else if (_CheckConstraints(loc_object,loc_anim_var_path,loc_actor_constraints))
            {
                loc_actor_animVars[i] = loc_anim_var_path;
            }
            else
            {
                return {"ERROR",std::format("Can't find valid animation variant in def {} for Actor with constraints 0x{:08X}",a_def,loc_actor_constraints)};
            }
        }
        catch(const std::exception& e)
        {
            ERROR("Error reading value from json! {} - {}",loc_filename,e.what())
            return {"ERROR",std::format("Error reading value from json! {} - {}",loc_filename,e.what())};
        }
    }

    //Bool is_sequence = JsonUtil.GetPathStringValue(file, actor_animVars[0] + ".anim[0]", "") != ""
    
    std::vector<std::string> loc_res = {"OK"};
    if (a_actors.size() == 2U)
    {
        auto loc_anim1 = RecursiveFind(loc_object, loc_actor_animVars[0] + ".anim");
        auto loc_anim2 = RecursiveFind(loc_object, loc_actor_animVars[1] + ".anim");
        if (loc_anim1.is_array() && loc_anim2.is_array())
        {
            auto loc_arr1_before_filter = loc_anim1.as_array();
            auto loc_arr2_before_filter = loc_anim2.as_array();
            auto loc_arr1 = loc_anim1.as_array();
            auto loc_arr2 = loc_anim2.as_array();
            size_t loc_size = loc_arr1.size() < loc_arr2.size() ? loc_arr1.size() : loc_arr2.size();
            
            loc_arr1.clear();
            loc_arr2.clear();
            for (size_t i = 0U; i < loc_size; i++)
            {
                if (ConvertAnimationSLPPNative(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(),std::string(loc_arr1_before_filter[i].as_string().c_str())) == "ERROR_NOT_FOUND" || ConvertAnimationSLPPNative(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(),std::string(loc_arr2_before_filter[i].as_string().c_str())) == "ERROR_NOT_FOUND") {
                    ERROR("animations {} {} missing",loc_arr1_before_filter[i].as_string().c_str(),loc_arr2_before_filter[i].as_string().c_str())
                    continue;
                }
                loc_arr1.push_back(loc_arr1_before_filter[i]);
                loc_arr2.push_back(loc_arr2_before_filter[i]);
                
            }
            loc_size=loc_arr1.size();
            loc_res.push_back(std::to_string(loc_size));

            std::vector<std::string> loc_anims(2*loc_size);
        
            for (size_t i = 0U; i < loc_size; i++)
            {
                loc_anims[i] = loc_arr1[i].as_string();
                loc_anims[i + loc_size] = loc_arr2[i].as_string();
            }

            loc_res.append_range(loc_anims);
            
        }
        else if (loc_anim1.is_string() && loc_anim2.is_string())
        {
            if (ConvertAnimationSLPPNative(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(),loc_anim1.as_string().c_str()) == "ERROR_NOT_FOUND" || ConvertAnimationSLPPNative(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(),loc_anim2.as_string().c_str()) == "ERROR_NOT_FOUND") {
                ERROR("animations {} {} missing",loc_anim1.as_string().c_str(),loc_anim2.as_string().c_str())
            } else {
                loc_res.push_back("-1");
                
                loc_res.push_back(loc_anim1.as_string().c_str());
                loc_res.push_back(loc_anim2.as_string().c_str());
            }
        }
    }
    else if (a_actors.size() == 1U)
    {
        auto loc_anim = RecursiveFind(loc_object, loc_actor_animVars[0] + ".anim");
        if (loc_anim.is_array())
        {
            auto loc_arr_before_filter = loc_anim.as_array();
            auto loc_arr = loc_anim.as_array();
            loc_arr.clear();
            size_t loc_size = loc_arr.size();
            for (size_t i = 0U; i < loc_size; i++)
            {
                if (ConvertAnimationSLPPNative(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(),std::string(loc_arr_before_filter[i].as_string().c_str())) == "ERROR_NOT_FOUND") {
                    ERROR("animations {} missing",loc_arr_before_filter[i].as_string().c_str())
                    continue;
                }
                loc_arr.push_back(loc_arr_before_filter[i]);
            }
            loc_size=loc_arr.size();
            loc_res.push_back(std::to_string(loc_size));
        
            std::vector<std::string> loc_anims(loc_size,"ERROR");
        
            for (size_t i = 0U; i < loc_size; i++)
            {
                loc_anims[i] = loc_arr[i].as_string();
            }
        
            loc_res.append_range(loc_anims);
        }
        else if (loc_anim.is_string())
        {
             if (ConvertAnimationSLPPNative(RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(),std::string(loc_anim.as_string().c_str())) == "ERROR_NOT_FOUND") {
                ERROR("animations {} missing",loc_anim.as_string().c_str())
                
            } else {
                loc_res.push_back("-1");
                loc_res.push_back(loc_anim.as_string().c_str());
            }
        }
    }

    LOG("AnimationManager::GetAnimationsFromJSON({},{},{}) Result:",a_def,a_constraintsORA1,a_constraintsORA2)
    for (auto&& it : loc_res)
    {
        LOG("\t{}",it)
    }

    return loc_res;
}

std::vector<std::string> UD::AnimationManager::GetAnimationsFromDB(std::string a_type, const std::vector<std::string>& a_kws, std::string a_field, const std::vector<int>& a_ActorConstraints, int a_lewdmin, int a_lewdmax, int a_aggromin, int a_aggromax)
{
    LOG("AnimationManager::GetAnimationsFromDB({},[{}],{},{},{}) called",a_type,boost::join(a_kws,","),a_field,a_lewdmin,a_lewdmax)

    std::vector<std::string> loc_result;

    for (auto&& kw : a_kws)
    {
        std::string loc_dict_path = a_type + kw;
        for (auto&& [name,animfile] : _jsoncache)
        {
            if (animfile.status != sOK) continue;

            auto loc_obj = animfile.json->as_object();
            auto loc_type = RecursiveFind(loc_obj,loc_dict_path);

            if (loc_type.is_array())
            {
                size_t loc_animid = 0;
                for (auto&& anim : loc_type.as_array())
                {
                    std::string loc_anim_path = loc_dict_path + "[" + std::to_string(loc_animid++) + "]";
                    bool loc_check = true;
                    for (int i = 0; i < a_ActorConstraints.size(); i++)
                    {
                        auto loc_A = RecursiveFind(anim,"A" + std::to_string(i+1));
                        if (loc_A.is_array())
                        {
                            for (auto&& Actor : loc_A.as_array())
                            {
                                loc_check = _CheckConstraints(Actor,"",a_ActorConstraints[i]);
                                if (loc_check) break;
                            }
                        }
                        else if (loc_A.is_object())
                        {
                            loc_check = _CheckConstraints(loc_A,"",a_ActorConstraints[i]);
                        }
                        else
                        {
                            ERROR("A element is neither array or object")
                            continue;
                        }
                        if (!loc_check) break;
                    }

                    if (loc_check /* && TODO: Check Lewd and Aggro */)
                    {
                        
                            
                            int constraints1=0;
                            int constraints2=0;
                            if (a_ActorConstraints.size() == 1) {
                                constraints1=a_ActorConstraints[0];
                            }
                            if (a_ActorConstraints.size() == 2) {
                                constraints2=a_ActorConstraints[1];
                            }
                            if ((a_ActorConstraints.size() == 1 || a_ActorConstraints.size() == 0) && GetAnimationsFromJSON(name+":"+loc_anim_path,{RE::PlayerCharacter::GetSingleton()->As<RE::Actor>()},constraints1,constraints2).size() >= 2) {
                                loc_result.push_back(name + ":" + loc_anim_path);
                            } else if (a_ActorConstraints.size() == 2 && GetAnimationsFromJSON(name+":"+loc_anim_path,{RE::PlayerCharacter::GetSingleton()->As<RE::Actor>(),RE::PlayerCharacter::GetSingleton()->As<RE::Actor>()},constraints1,constraints2).size() >= 2) {
                                loc_result.push_back(name + ":" + loc_anim_path);
                            }
                            
                    }
                    
                }
            }
            else
            {
                LOG("{} does not exist",loc_dict_path)
            }
        }
    }

    LOG("AnimationManager::GetAnimationsFromDB({},[{}],{},{},{}) - Result:",a_type,boost::join(a_kws,","),a_field,a_lewdmin,a_lewdmax)
    for (auto&& it : loc_result) LOG("\t{}",it)

    return loc_result;
}

void UD::AnimationManager::SyncAnimationSetting(const std::vector<std::string>& a_animationoff)
{
    LOG("SyncAnimationSetting({})",boost::join(a_animationoff,", "))

    for (auto&& [name,file] : _jsoncache)
    {
        if (file.status == sOK || file.status == sDisabled)
        {
            auto loc_find = std::find(a_animationoff.begin(),a_animationoff.end(),name);
            if (loc_find != a_animationoff.end())
            {
                file.status = sDisabled;
                LOG("SyncAnimationSetting({}) - {} disabled",boost::join(a_animationoff,", "),name)
            }
            else
            {
               file.status = sOK;
            }
        }
    }

    for (auto&& it : a_animationoff)
    {
        auto loc_find = _jsoncache.find(it);
        if (loc_find != _jsoncache.end())
        {
            loc_find->second.status = sDisabled;
            LOG("SyncAnimationSetting({}) - {} disabled",boost::join(a_animationoff,", "),loc_find->first)
        }
    }
}

inline int UD::AnimationManager::ProccessDeviceArray(RE::Actor* a_actor,const std::vector<RE::TESObjectARMO*> &a_array) const
{
    int loc_res = 0x00000000;
    for(auto& it : a_array)
    {
        //check heavy bondage
        static std::vector<RE::BGSKeyword*> loc_hb = std::vector<RE::BGSKeyword*>({KeywordManager::ddhb});
        if (it->HasKeywordInArray(loc_hb,true))
        {
            loc_res |= GetActorHBConstrains(a_actor,it);
        }

        //check hobble skirt
        static std::vector<RE::BGSKeyword*> loc_hobble = std::vector<RE::BGSKeyword*>({KeywordManager::ddhobbleskirt});
        if (it->HasKeywordInArray(loc_hobble,true))
        {
            static std::vector<RE::BGSKeyword*> loc_hobblerelaxed = std::vector<RE::BGSKeyword*>({KeywordManager::ddhobbleskirtrelax,KeywordManager::ddankleshacles});
            if (it->HasKeywordInArray(loc_hobblerelaxed,false)) loc_res |= 0x00000002;
            else loc_res |= 0x00000001;
            continue;
        }

        //check mittens
        static std::vector<RE::BGSKeyword*> loc_mittens = std::vector<RE::BGSKeyword*>({KeywordManager::ddmittens});
        if (it->HasKeywordInArray(loc_mittens,true))
        {
            loc_res |= 0x00000100;
            continue;
        }

        //check gag
        static std::vector<RE::BGSKeyword*> loc_gag = std::vector<RE::BGSKeyword*>({KeywordManager::ddgag});
        if (it->HasKeywordInArray(loc_gag,true))
        {
            loc_res |= 0x00000800;
            continue;
        }
    }

    return loc_res;
}

inline int UD::AnimationManager::GetActorHBConstrains(RE::Actor* a_actor,RE::TESObjectARMO* a_device) const
{
    for (auto& [kw,id] : KeywordManager::ddhbkwds)
    {
        std::vector<RE::BGSKeyword*> loc_hbkw = std::vector<RE::BGSKeyword*>({kw});
        if (a_device->HasKeywordInArray(loc_hbkw,true))
        {
            return id;
        }
    }
    return 0x00000000;
}

void UD::AnimationManager::Setup()
{
    if (!_init)
    {
        _init = true;
        REL::Relocation<std::uintptr_t> vtbl_player{RE::Character::VTABLE[0]};
        DrawWeaponMagicHands_old = vtbl_player.write_vfunc(REL::Module::GetRuntime() != REL::Module::Runtime::VR ? 0x0A6 : 0x0A8, DrawWeaponMagicHands);

        std::string loc_animpath = std::filesystem::current_path().string() + "\\Data\\skse\\plugins\\StorageUtilData\\UD\\Animations";
        std::regex loc_regex(R"regex(.*\\(.*\.[jJ][sS][oO][nN]))regex");


        for (const auto & entry : std::filesystem::directory_iterator(loc_animpath))
        {
            std::string loc_path = entry.path().string();

            if (entry.is_regular_file() && std::regex_match(loc_path,loc_regex)) 
            {
                const std::string loc_jsonname = std::regex_replace(loc_path,loc_regex,"$1");
                std::fstream loc_ifile(loc_path,std::ios::in);

                boost::json::value loc_json;
                if (loc_ifile.is_open())
                {
                    try
                    {
                        loc_json = boost::json::parse(loc_ifile);
                    }
                    catch(const std::exception& e)
                    {
                        ERROR("Error parsing json {} - {}",loc_jsonname,e.what())
                    }
                }
                else
                {
                    ERROR("Could not open file {}",loc_jsonname)
                }


                std::regex loc_regexname(R"regex((.*\\)(.*)(\.[jJ][sS][oO][nN]))regex");
                const std::string loc_name = std::regex_replace(loc_path,loc_regexname,"$2");

                _jsoncache[loc_name] = AnimationFile{std::shared_ptr<boost::json::value>(new boost::json::value(loc_json)),sOK,"OK"};

                // Check json status
                auto loc_jsoncond   = RecursiveFind(loc_json.as_object(),"conditions.json");
                auto loc_modcond    = RecursiveFind(loc_json.as_object(),"conditions.mod");

                const std::string loc_jsoncondstr   = (loc_jsoncond.is_string() ? loc_jsoncond.as_string().c_str() : "");
                const std::string loc_modcondstr    = (loc_modcond.is_string() ? loc_modcond.as_string().c_str() : "");

                bool loc_cond = true;
                if (loc_jsoncondstr != "")
                {
                    const std::string loc_condjsonpath = std::filesystem::current_path().string() + "\\Data\\skse\\plugins\\StorageUtilData\\" + loc_jsoncondstr;
                    if (!std::filesystem::exists(loc_condjsonpath))
                    {
                        loc_cond = false;
                        _jsoncache[loc_name].error = loc_jsoncondstr;
                    }
                    //DEBUG("Json {} found = {}",loc_condjsonpath,loc_cond)
                }
                if (loc_cond && loc_modcondstr != "")
                {
                    if (!(RE::TESDataHandler::GetSingleton()->GetLoadedModIndex(loc_modcondstr).has_value() || RE::TESDataHandler::GetSingleton()->GetLoadedLightModIndex(loc_modcondstr).has_value()))
                    {
                        loc_cond = false;
                        _jsoncache[loc_name].error = loc_modcondstr;
                    }
                   // DEBUG("Mod {} found = {}",loc_modcondstr,loc_cond)
                }

                if (!loc_cond)
                {
                    _jsoncache[loc_name].status = sMissingMaster;
                }
            }
        }

        DEBUG("=== Loaded animation files ===")
        for (auto&& [name,file] : _jsoncache)
        {
            DEBUG("\t{} - {} / {}",name, file.status, file.error)
        }
    }
}

void UD::AnimationManager::Reload()
{
    Utils::UniqueLock lock(_SaveLock);
    _weapondisabled.clear();
}

void UD::AnimationManager::DrawWeaponMagicHands(RE::Actor* a_actor, bool a_draw)
{
    static const int loc_boundcombatnpc = Config::GetSingleton()->GetVariable<int>("Combat.iNPCBoundCombat",0);

    bool loc_canattack = false;

    switch (loc_boundcombatnpc)
    {
    default:
    case 0:
        loc_canattack = false;
        break;
    case 1:
        // Only if Actor is follower
        loc_canattack = a_actor->GetActorRuntimeData().boolBits.any(RE::Actor::BOOL_BITS::kPlayerTeammate);
        break;
    case 2:
        loc_canattack = true;
        break;
    }

    // Check if Actor weapons are disabled
    if (a_draw)
    {
        if (AnimationManager::GetSingleton()->CheckWeaponDisabled(a_actor))
        {
            LOG("ControlManager::DrawWeaponMagicHands({}) - actors weapons are disabled and because of that cant draw weapon",a_actor ? a_actor->GetName() : "NONE")
            return;
        }
    }


    if (a_draw && (IsAnimating(a_actor) || ActorIsBoundCombatDisabled(a_actor) || (!loc_canattack && ActorIsBound(a_actor))))
    {
        
        LOG("ControlManager::DrawWeaponMagicHands({}) - Actor is animating/bound and because of that cant draw weapon",a_actor ? a_actor->GetName() : "NONE")
        return;
    } 
    else
    {
        DrawWeaponMagicHands_old(a_actor,a_draw);
    }

}

bool UD::AnimationManager::_CheckConstraints(boost::json::value a_obj, std::string a_ObjPath, int a_ActorConstraints) const
{
    LOG("AnimationManager::_CheckConstraints({},{})",a_ObjPath,a_ActorConstraints)
    try
    {
        auto req=RecursiveFind(a_obj,a_ObjPath + ".req");
        int loc_anim_reqConstr = 0;
        if (req.is_int64()) {
            loc_anim_reqConstr=req.as_int64();
            
        } else {
            WARN("anim req not int64 {}",req.kind());
        }
        if ((loc_anim_reqConstr & a_ActorConstraints) != loc_anim_reqConstr)
        {
            return false;
        }
        else
        {
            auto opt=RecursiveFind(a_obj,a_ObjPath + ".opt");
            int loc_anim_optConstr = 0;
            if (opt.is_int64()) {
                loc_anim_optConstr=opt.as_int64();
            } else {
                WARN("anim opt not int64 {}",req.kind());
            }
            
            if ((~(loc_anim_optConstr|loc_anim_reqConstr) & a_ActorConstraints)==0) {
                return true;
            }
            // TODO: sort by most optional constraints
            return false;
        }
    }
    catch (const std::exception& e)
    {
        WARN("AnimationManager::_CheckConstraints({},{}) - Cant get values from json - {}",a_ObjPath,a_ActorConstraints,e.what())
        return false;
    }

}

boost::json::value UD::AnimationManager::RecursiveFind(boost::json::value a_obj, std::string a_path) const
{
    LOG("RecursiveFind({})",a_path)
    boost::json::value loc_val = a_obj;

    std::vector<std::string> loc_objects;
    boost::split(loc_objects,a_path,boost::is_any_of("."));

    int      loc_arrindx = -1;
    std::regex  loc_arrregex("\\b(.*)\\[(\\d*)\\]");

    for (auto&& it : loc_objects)
    {
        if (it == "")
        {
            continue;
        }
        if (std::regex_match(it,loc_arrregex))
        {
            try {loc_arrindx = std::stoi(std::regex_replace(it,loc_arrregex,"$2"));}
            catch(...) {loc_arrindx = 0;} 
            it = std::regex_replace(it,loc_arrregex,"$1");
        }

        try
        {
            if (loc_val.is_object())
            {
                loc_val = loc_val.as_object()[it];
            }

            // Immidiatly dereference array
            if (loc_val.is_array() && loc_arrindx >= 0)
            {
                if (loc_arrindx < loc_val.as_array().size())
                {
                    loc_val = loc_val.as_array()[loc_arrindx];
                    loc_arrindx = -1;
                }
                else
                {
                    WARN("AnimationManager::RecursiveFind - Cant find {} - Array index out of range. Array size is {}",a_path,loc_val.as_array().size())
                    return boost::json::value();
                }

            }
        }
        catch(const std::exception& e)
        {
            WARN("AnimationManager::RecursiveFind - Cant find {} - {}",a_path,e.what())
            return boost::json::value();
        }
    }

    return loc_val;
}
