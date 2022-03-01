//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "types.h"
#include <variant>

namespace Fresa
{
    //---Heterogeneous map---
    struct PolyMap{
        PolyMap() = default;
        PolyMap(const PolyMap& rhs) { *this = rhs; }
        ~PolyMap() { clear(); }
        
        template<class T>
        static std::map<const PolyMap*, T> items;
        static std::map<ui32, const PolyMap*> ids;
        
        std::vector<std::function<void(PolyMap&)>> destructors;
        std::vector<std::function<void(const PolyMap&, PolyMap&)>> copiers;
        
        ui32 size = 0;

        template <class T>
        void emplace(const T& t, ui32 i) {
            if (items<T>.find(this) == std::end(items<T>)) {
                items<T>[this] = t;
                ids[i] = this;
                size++;
                
                destructors.emplace_back( [](PolyMap& map){items<T>.erase(&map);} );
                copiers.emplace_back([](const PolyMap& lhs, PolyMap& rhs) { items<T>[&rhs] = items<T>[&lhs]; });
            } else {
                throw std::runtime_error("You used an index of the PolyMap already in use");
            }
        }
        
        void clear() {
            for (auto && destructor : destructors)
                destructor(*this);
            size = 0;
        }
        
        PolyMap& operator=(const PolyMap& rhs) {
            clear();
            destructors = rhs.destructors;
            copiers = rhs.copiers;
            size = rhs.size;

            for (auto && copy : copiers) {
                copy(rhs, *this);
            }
            return *this;
        }
        
        template<class T>
        void visit(T&& visitor) {
            visit_(visitor, typename std::decay_t<T>::types{});
        }
        
        template<class T, template<typename...> class TypeList, typename... Types>
        void visit_(T&& visitor, TypeList<Types...>) {
            (..., visit_helper<std::decay_t<T>, Types>(visitor));
        }
        
        template<class T, class U>
        void visit_helper(T& visitor) {
            visitor(items<U>[this]);
        }
    };
    
    template<class T>
    std::map<const PolyMap*, T> PolyMap::items;
    inline std::map<ui32, const PolyMap*> PolyMap::ids;
    
    
    //---Visitor---
    template <typename... T>
    struct VisitorTypes {};
    
    template <typename... T>
    struct Visitor {
        using types = VisitorTypes<T...>;
    };
    
    /*struct ExampleVisitor : Visitor<type1, type2, .../> {
        template<class T>
        void operator()(T& t) { t = ...; }
    };*/
}
