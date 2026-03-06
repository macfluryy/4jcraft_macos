#pragma once
#ifdef _XBOX
//
namespace std
{
	namespace tr1
	{

		template<class T>
		class hash< std::shared_ptr<T> >
		{
		public:
			size_t operator()(const std::shared_ptr<T>& key) const
			{
				return (size_t)key.get();
			}
		};
	}
}
#endif