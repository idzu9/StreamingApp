#include <functional>

template<typename ReturnType, typename... Args>
class Delegate
{
public:
	using TargetType = std::function<ReturnType(Args...)>;

	template<typename T, typename Function>
	void BindDelegate(T* Instance, Function BindFunction)
	{
		DelegateCallBack = [Instance, BindFunction](Args... Params) -> ReturnType
			{
				return (Instance->*BindFunction)(Params...);
			};
	}

	void BindDelegateLambda(TargetType CallBack)
	{
		DelegateCallBack = std::move(CallBack);
	}

	void Unbind()
	{
		DelegateCallBack = nullptr;
	}

	bool IsBound() const
	{
		return DelegateCallBack != nullptr;
	}

	ReturnType Execute(Args... Params) const
	{
		assert(IsBound() && " error - delegate is not bound!");
		return DelegateCallBack(Params...);
	}

	ReturnType ExecuteIfBound(Args... Params) const
	{
		if (IsBound())
		{
			return DelegateCallBack(Params...);
		}
	}

private:
	TargetType DelegateCallBack = nullptr;
};
