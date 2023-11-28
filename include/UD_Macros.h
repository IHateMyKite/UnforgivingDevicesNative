//debug build
#define UDDEBUG 1U

//multiplier used for converting existing papyrus over time effects to native over time effect
//as native effects are much faster, we need to reduce its amplitude, otherwise it will be much stronger then papyrus one
#define UDCONVERTMULT 1.0f

//if skyui meters should force the new value (play the animation)
#define UDSKYUIFORCE 1.0f

//handle of papyrus functions
#define PAPYRUSFUNCHANDLE RE::StaticFunctionTag* a_psft

#define UDTRUNCVALUE(val,min,max)       \
{                                       \
    if (val > max) val = max;           \
    else if (val < min) val = min;      \
}

#define CLOG(...)    {RE::ConsoleLog::GetSingleton()->Print(std::format(__VA_ARGS__).c_str());}

#define LOG(...)    {if (UD::Config::GetSingleton()->GetVariable<int>("General.iLogging",1) >= 2) SKSE::log::info(__VA_ARGS__);}
#define WARN(...)   {if (UD::Config::GetSingleton()->GetVariable<int>("General.iLogging",1) >= 1) SKSE::log::warn(__VA_ARGS__);}
#define ERROR(...)  {SKSE::log::error(__VA_ARGS__);}

#define SINGLETONHEADER(cname)                          \
        public:                                         \
            cname(cname &) = delete;                    \
            void operator=(const cname &) = delete;     \
            static cname* GetSingleton();               \
        protected:                                      \
            cname(){}                                   \
            ~cname(){}                                  \
            static cname* _this;

#define SINGLETONBODY(cname)                            \
        cname * cname::_this = new cname;               \
        cname * cname::GetSingleton(){return _this;}

#define PLAYER RE::PlayerCharacter::GetSingleton()

#define CONFIGFILEARG(arg) const boost::property_tree::ptree& arg