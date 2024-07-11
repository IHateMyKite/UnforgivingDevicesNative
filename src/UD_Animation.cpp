#include <UD_Animation.h>
#include <UD_Utility.h>
#include <UD_Config.h>
#include <UD_Inventory.h>
#include <UD_Keywords.h>
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

    auto loc_it = _jsoncache.find(loc_filename);
    if (loc_it == _jsoncache.end())
    {
        ERROR("Could not find animation file {}",loc_filename)
        return {"ERROR",std::format("Could not find animation file {}",loc_filename)};
    }

    boost::json::object& loc_object = loc_it->second->as_object();

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
                    return {"ERROR",std::format("Can't find valid animation variant in def {} for actor with constraints 0x{:08X}",a_def,loc_actor_constraints)}; 
                }

            }
            else if (_CheckConstraints(loc_object,loc_anim_var_path,loc_actor_constraints))
            {
                loc_actor_animVars[i] = loc_anim_var_path;
            }
            else
            {
                return {"ERROR",std::format("Can't find valid animation variant in def {} for actor with constraints 0x{:08X}",a_def,loc_actor_constraints)};
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
            auto loc_arr1 = loc_anim1.as_array();
            auto loc_arr2 = loc_anim2.as_array();
        
            const size_t loc_size = loc_arr1.size() < loc_arr2.size() ? loc_arr1.size() : loc_arr2.size();
            loc_res.push_back(std::to_string(loc_size));
        
            std::vector<std::string> loc_anims(loc_size);
        
            for (size_t i = 0U; i < loc_size; i++)
            {
                loc_anims[i] = loc_arr1[i].as_string();
                loc_anims[i + loc_size] = loc_arr1[i].as_string();
            }
        
            loc_res.append_range(loc_anims);
        }
        else if (loc_anim1.is_string() && loc_anim2.is_string())
        {
            loc_res.push_back("-1");
            loc_res.push_back(loc_anim1.as_string().c_str());
            loc_res.push_back(loc_anim2.as_string().c_str());
        }
    }
    else if (a_actors.size() == 1U)
    {
        auto loc_anim = RecursiveFind(loc_object, loc_actor_animVars[0] + ".anim");
        if (loc_anim.is_array())
        {
            auto loc_arr = loc_anim.as_array();
        
            const size_t loc_size = loc_arr.size();
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
            loc_res.push_back("-1");
            loc_res.push_back(loc_anim.as_string().c_str());
        }
    }

    LOG("AnimationManager::GetAnimationsFromJSON({},{},{}) Result:",a_def,a_constraintsORA1,a_constraintsORA2)
    for (auto&& it : loc_res)
    {
        LOG("\t{}",it)
    }

    return loc_res;
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

                _jsoncache[loc_jsonname] = std::shared_ptr<boost::json::value>(new boost::json::value(loc_json));
            }
        }

        DEBUG("=== Loaded animation files ===")
        for (auto&& it : _jsoncache)
        {
            DEBUG("\t{}",it.first)
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
        // Only if actor is follower
        loc_canattack = a_actor->GetActorRuntimeData().boolBits.any(RE::Actor::BOOL_BITS::kPlayerTeammate);
        break;
    case 2:
        loc_canattack = true;
        break;
    }

    // Check if actor weapons are disabled
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
        
        LOG("ControlManager::DrawWeaponMagicHands({}) - actor is animating/bound and because of that cant draw weapon",a_actor ? a_actor->GetName() : "NONE")
        return;
    } 
    else
    {
        DrawWeaponMagicHands_old(a_actor,a_draw);
    }

}

bool UD::AnimationManager::_CheckConstraints(boost::json::object& a_obj, std::string a_ObjPath, int a_ActorConstraints) const
{
    LOG("AnimationManager::_CheckConstraints({},{})",a_ObjPath,a_ActorConstraints)
    try
    {
        const int loc_anim_reqConstr = RecursiveFind(a_obj,a_ObjPath + ".req").as_int64();
        if ((loc_anim_reqConstr & a_ActorConstraints) != loc_anim_reqConstr)
        {
            return false;
        }
        else
        {
            const int loc_anim_optConstr = RecursiveFind(a_obj,a_ObjPath + ".opt").as_int64();
            return ((loc_anim_optConstr | loc_anim_reqConstr) & a_ActorConstraints) == a_ActorConstraints;
        }
    }
    catch (const std::exception& e)
    {
        WARN("AnimationManager::_CheckConstraints - Cant get values from json - {}",e.what())
        return false;
    }

}

boost::json::value UD::AnimationManager::RecursiveFind(boost::json::object& a_obj, std::string a_path) const
{
    LOG("RecursiveFind({})",a_path)
    boost::json::value loc_val = a_obj;

    std::vector<std::string> loc_objects;
    boost::split(loc_objects,a_path,boost::is_any_of("."));

    size_t      loc_arrindx = 0;
    std::regex  loc_arrregex("\\b(.*)\\[(\\d*)\\]");

    for (auto&& it : loc_objects)
    {
        
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
            if (loc_val.is_array())
            {
                if (loc_arrindx < loc_val.as_array().size())
                {
                    loc_val = loc_val.as_array()[loc_arrindx];
                    loc_arrindx = 0;
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
