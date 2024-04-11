#pragma once

//class FastTravelConfirmCallback : public IMessageBoxCallback
//{
//public:
//	inline static constexpr auto RTTI = RTTI___FastTravelConfirmCallback;
//	inline static constexpr auto VTABLE = VTABLE___FastTravelConfirmCallback;
//
//	~FastTravelConfirmCallback() override;  // 00
//
//	// override (IMessageBoxCallback)
//	void Run(Message a_msg) override;  //01
//
//	// members
//	MapMenu*     mapMenu;     // 10
//	std::int32_t cursorPosX;  // 18
//	std::int32_t cursorPosY;  // 1C
//};
//static_assert(sizeof(FastTravelConfirmCallback) == 0x20);

namespace UD
{
    class FastTravelManager
    {
    SINGLETONHEADER(FastTravelManager);
    public:
        void Setup();
    private:
        static void FastTravelConfirmCallback_Run_Patched(RE::FastTravelConfirmCallback* a_this, RE::IMessageBoxCallback::Message a_msg);
        REL::Relocation<decltype(FastTravelConfirmCallback_Run_Patched)> FastTravelConfirmCallback_Run;

        bool _init = false;
    };
}