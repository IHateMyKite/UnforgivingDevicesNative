#pragma once




namespace UD {
    namespace BSScript
    {
	    template <bool IS_LONG, class F, class R, class Base, class... Args>
	    class NativeFunctionVar : public RE::BSScript::NF_util::NativeFunctionBase
	    {
	    public:
		    using result_type = R;
		    using base_type = Base;
		    using function_type = F;

		    NativeFunctionVar() = delete;
		    NativeFunctionVar(const NativeFunctionVar&) = delete;
		    NativeFunctionVar(NativeFunctionVar&&) = delete;

		    NativeFunctionVar(std::string_view a_fnName, std::string_view a_className, function_type a_callback,RE::BSScript::TypeInfo::RawType a_rettype) :
			    RE::BSScript::NF_util::NativeFunctionBase(a_fnName, a_className, RE::BSScript::is_static_base_v<base_type>, sizeof...(Args)),
			    _stub(a_callback)
		    {
			    std::size_t i = 0;
			    ((_descTable.entries[i++].second.SetType(RE::BSScript::GetRawType<Args>{}())), ...);
			    _retType = RE::BSScript::TypeInfo(a_rettype);
		    }

		    ~NativeFunctionVar() override = default;  // 00

		    bool HasStub() const override  // 15
		    {
			    return static_cast<bool>(_stub);
		    }

		    bool MarshallAndDispatch(Variable& a_baseValue, [[maybe_unused]] RE::BSScript::Internal::VirtualMachine& a_vm, [[maybe_unused]] RE::VMStackID a_stackID, Variable& a_resultValue, const RE::BSScript::StackFrame& a_frame) const override  // 16
		    {
			    base_type base{};
			    if constexpr (std::negation_v<RE::BSScript::is_static_base<base_type>>) {
				    base = a_baseValue.Unpack<base_type>();
				    if (!base) {
					    return false;
				    }
			    }

			    auto page = a_frame.GetPageForFrame();
			    auto args = RE::BSScript::Impl::MakeTuple<Args...>(a_frame, page);
			    if constexpr (std::is_void_v<result_type>) {
					RE::BSScript::Impl::CallBack(_stub, std::move(args), std::move(base));
					a_resultValue.SetNone();
			    } else {
					auto result = RE::BSScript::Impl::CallBack(_stub, std::move(args), std::move(base));
					a_resultValue = result;
			    }

			    return true;
		    }

	    protected:
		    // members
		    std::function<function_type> _stub;  // 50
	    };
    }

	template <class F, class = void>
	class NativeFunctionVar;

	template <class R, class Cls, class... Args>
	class NativeFunctionVar<R(Cls, Args...)> :
		public BSScript::NativeFunctionVar<false, R(Cls, Args...), R, Cls, Args...>
	{
	private:
		using super = BSScript::NativeFunctionVar<false, R(Cls, Args...), R, Cls, Args...>;
    
	public:
		using result_type = typename super::result_type;
		using base_type = typename super::base_type;
		using function_type = typename super::function_type;
    
		using super::super;
	};

    //template <class Int, class R, class Cls, class... Args>
    //class NativeFunctionVar<R(RE::BSScript::Internal::VirtualMachine*, Int, Cls, Args...)> :
    //	public BSScript::NativeFunctionVar<true, R(RE::BSScript::Internal::VirtualMachine*, Int, Cls, Args...), R, Cls, Args...>
    //{
    //private:
    //	using super = BSScript::NativeFunctionVar<true, R(RE::BSScript::Internal::VirtualMachine*, Int, Cls, Args...), R, Cls, Args...>;
    //   
    //public:
    //	using result_type = typename super::result_type;
    //	using base_type = typename super::base_type;
    //	using function_type = typename super::function_type;
    //   
    //	using super::super;
    //};

	//template <class Int, class R, class Cls, class... Args>
	//class NativeFunctionVar<R(RE::BSScript::IVirtualMachine*, Int, Cls, Args...), std::enable_if_t<RE::BSScript::is_valid_long_sig_v<Int, R, Cls, Args...>>> :
	//	public BSScript::NativeFunctionVar<true, R(RE::BSScript::IVirtualMachine*, Int, Cls, Args...), R, Cls, Args...>
	//{
	//private:
	//	using super = BSScript::NativeFunctionVar<true, R(RE::BSScript::IVirtualMachine*, Int, Cls, Args...), R, Cls, Args...>;
    //
	//public:
	//	using result_type = typename super::result_type;
	//	using base_type = typename super::base_type;
	//	using function_type = typename super::function_type;
    //
	//	using super::super;
	//};

	template <class F>
	NativeFunctionVar(std::string_view, std::string_view, F, RE::BSScript::TypeInfo::RawType) -> NativeFunctionVar<std::remove_pointer_t<F>>;


	template <class F>
	void RegisterFunctionVar(std::string_view a_fnName, std::string_view a_className, F a_callback, bool a_callableFromTasklets,RE::BSScript::TypeInfo::RawType a_rettype)
	{
        auto loc_vm = InternalVM::GetSingleton();
		loc_vm->BindNativeMethod(new UD::NativeFunctionVar(a_fnName, a_className, a_callback,a_rettype));
		if (a_callableFromTasklets) {
			loc_vm->SetCallableFromTasklets(a_className.data(), a_fnName.data(), a_callableFromTasklets);
		}
	}


    bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine *vm);
}