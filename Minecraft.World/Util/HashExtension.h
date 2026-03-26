#pragma once
#if 0
//
namespace std {
namespace tr1 {

template <class T>
class hash<std::shared_ptr<T> > {
public:
    std::size_t operator()(const std::shared_ptr<T>& key) const {
        return (std::size_t)key.get();
    }
};
}  // namespace tr1
}  // namespace std
#endif