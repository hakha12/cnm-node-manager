/* MIT License

Copyright (c) 2025 Hafizh Khairy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _CNM_CPP_HPP_
#define _CNM_CPP_HPP_

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <map>
#include <unordered_map>
#include <functional>
#include <typeinfo>
#include <typeindex>


namespace nm {
	class node;

	using node_t = std::shared_ptr<node>;
	using node_callback_t = std::function<void()>;

	using node_array_t = std::vector<node_t>;
	using node_list_t = std::list<node_t>;

	template<typename _Tk>
	using node_hash_t = std::unordered_map<_Tk, node_t>;

	using callback_hash = std::unordered_map<std::string, node_callback_t>;
	
	class node {
		public:
			template<class _Tn, typename... _Targs>
			static node_t create(_Targs&&... args){
				std::type_index l_type(typeid(_Tn));

				if (get_node_factory<_Targs...>().find(l_type) == get_node_factory<_Targs...>().end()){
					get_node_factory<_Targs...>()[l_type] = [](auto&&... params) -> node_t {
						node_t l_node = std::make_shared<_Tn>();
						l_node->init(std::forward<decltype(params)>(params)...);
						return l_node;
					};
				}
	
				return std::invoke(get_node_factory<_Targs...>()[l_type], std::forward<_Targs>(args)...);
			}	

			virtual ~node(){
				destroy();
			}

			virtual void init(){}
			virtual void destroy(){}

			virtual void awake(){}
			virtual void sleep(){}

			virtual void process(){}
			virtual void render(){}

			bool add_child(node_t p_child, const uint16_t p_index){
				if (m_child_list.find(p_index) != m_child_list.end()) return false;

				return m_child_list.emplace(p_index, p_child).second;
			}

			void remove_child(const uint16_t p_index){
				if (m_child_list.find(p_index) == m_child_list.end()) return;

				m_child_list.erase(p_index);
			}

			node_t get_parent(){
				if (!m_parent) return nullptr;

				return m_parent;
			}

			node_t get_child(const uint16_t p_index){
				if (m_child_list.find(p_index) == m_child_list.end()) return nullptr;

				return m_child_list.find(p_index)->second;
			}

			bool add_signal(const std::string& p_signal_name){
				if (m_signal_list.find(p_signal_name) != m_signal_list.end()) return false;

				callback_hash l_callback_list = {};

				return m_signal_list.emplace(p_signal_name, l_callback_list).second;
			}

			template <class _Tn>
			bool add_callback(const std::string& p_signal_name, const std::string& p_callback_name, void(_Tn::*p_callback_method)(), _Tn* p_instance){
				auto l_signal = m_signal_list.find(p_signal_name);

				if (l_signal == m_signal_list.end()) return false;

				callback_hash& l_callback_list = l_signal->second;

				node_callback_t l_callback = [p_instance, p_callback_method](){(p_instance->*p_callback_method)();};

				return l_callback_list.emplace(p_callback_name, l_callback).second;
			}

			void emit_signal(const std::string& p_signal_name){
				auto l_signal = m_signal_list.find(p_signal_name);

				if (l_signal == m_signal_list.end()) return;

				for (auto const& [l_key, l_callback] : l_signal->second){
					l_callback();
				}
			}

			void remove_signal(const std::string& p_signal_name){
				if (m_signal_list.find(p_signal_name) == m_signal_list.end()) return;

				m_signal_list.erase(p_signal_name);
			}

		private:			
			node_t m_parent;
			std::map<uint16_t, node_t> m_child_list;
			std::unordered_map<std::string, callback_hash> m_signal_list;

			template<typename... _Targs>
			static std::unordered_map<std::type_index, std::function<node_t(_Targs...)>>& get_node_factory(){
				static std::unordered_map<std::type_index, std::function<node_t(_Targs...)>> l_node_factory;

				return l_node_factory;
			}
	};
}

#endif
