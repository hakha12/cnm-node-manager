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
	// Base node class
	class node;

	// Type aliasing for node pointer
	using node_t = std::shared_ptr<node>;
	using node_callback_t = std::function<void()>;

	using callback_hash = std::unordered_map<std::string, node_callback_t>;
	
	class node {
		public:
			/*
			 *	Create a node object based on the inputted class.
			 *
			 *  Return a smart pointer to the created node.
			 */ 
			template<class _Tn, typename... _Targs>
			static node_t create(_Targs&&... args){
				std::type_index l_type(typeid(_Tn));

				if (_get_node_factory<_Targs...>().find(l_type) == _get_node_factory<_Targs...>().end()){
					_get_node_factory<_Targs...>()[l_type] = [](auto&&... params) -> node_t {
						node_t l_node = std::make_shared<_Tn>();
						l_node->init(std::forward<decltype(params)>(params)...);
						
						return l_node;
					};
				}
	
				return std::invoke(_get_node_factory<_Targs...>()[l_type], std::forward<_Targs>(args)...);
			}	

			/*
			 *	The node class virtual destructor
			 */
			virtual ~node(){
				destroy();
			}

			/*
			 *	Manually call the node's awake() method.
			 *
			 * 	Automatically called if a node is set as the current node by node_machine, or
			 * 	when this node's parent call this method.
			 */
			void call_awake(){
				_internal_awake();
			}

			/*
			 *	Manually call the node's sleep() method.
			 *
			 * 	Automatically called if a node is set as the current node by node_machine, or
			 * 	when this node's parent call this method.
			 */			
			void call_sleep(){
				_internal_sleep();
			}

			/*
			 *	Manually call the node's process() method.
			 *
			 * 	Automatically called if a node_machine called the process() method with this node as the current one, or
			 * 	when this node's parent call this method.
			 */	
			void call_process(){
				_internal_process();
			}

			/*
			 *	Manually call the node's render() method.
			 *
			 * 	Automatically called if a node_machine called the render() method with this node as the current one, or
			 * 	when this node's parent call this method.
			 */	
			void call_render(){
				_internal_render();
			}

			/*
			 *	Add a child node to this node at the specified index and set this node as the parent of the child node. 
			 *	The index specify the order of the child node's method call.
			 *  
			 * 	Return a boolean value to indicate whether the addition is succesful or not.
			 */
			bool add_child(node_t p_child, const uint16_t p_index){
				if (m_child_list.find(p_index) != m_child_list.end()) return false;

				p_child->m_parent = this;

				return m_child_list.emplace(p_index, p_child).second;
			}

			/*
			 *	Remove a child node at the specified index.
			 *
			 * 	Note that this won't automatically delete the child node if it still in use somewhere else.
			 */	
			void remove_child(const uint16_t p_index){
				if (m_child_list.find(p_index) == m_child_list.end()) return;

				m_child_list.erase(p_index);
			}

			/*
			 *	Get the parent node, if any.
			 *
			 * 	Return a pointer to the parent node if the parent exist and nullptr otherwise.
			 */	
			node_t get_parent(){
				if (!m_parent) return nullptr;

				return node_t(m_parent);
			}

			/*
			 *	Get the child node at the specified index.
			 *
			 * 	Return a pointer to the child node if the child exist and nullptr otherwise.
			 */	
			node_t get_child(const uint16_t p_index){
				if (m_child_list.find(p_index) == m_child_list.end()) return nullptr;

				return m_child_list.find(p_index)->second;
			}

			/*
			 *	Add a signal to the node. Signal is a way for a node to transmit message to another node.
			 *
			 * 	Return a boolean value to indicate whether the addition is succesful or not.
			 */	
			bool add_signal(const std::string& p_signal_name){
				if (m_signal_list.find(p_signal_name) != m_signal_list.end()) return false;

				callback_hash l_callback_list = {};

				return m_signal_list.emplace(p_signal_name, l_callback_list).second;
			}

			/*
			 *	Add a callback method to be executed when a signal is emitted.
			 *
			 * 	Return a boolean value to indicate whether the addition is succesful or not.
			 */	
			template <class _Tn>
			bool add_callback(const std::string& p_signal_name, const std::string& p_callback_name, void(_Tn::*p_callback_method)(), _Tn* p_instance){
				auto l_signal = m_signal_list.find(p_signal_name);

				if (l_signal == m_signal_list.end()) return false;

				callback_hash& l_callback_list = l_signal->second;
				node_callback_t l_callback = [p_instance, p_callback_method](){(p_instance->*p_callback_method)();};

				return l_callback_list.emplace(p_callback_name, l_callback).second;
			}

			/*
			 *	Emit the specified signal and execute all of the associated callback method.
			 */	
			void emit_signal(const std::string& p_signal_name){
				auto l_signal = m_signal_list.find(p_signal_name);

				if (l_signal == m_signal_list.end()) return;

				for (auto const& [l_key, l_callback] : l_signal->second){
					l_callback();
				}
			}

			/*
			 *	Remove the specified signal from the node.
			 */	
			void remove_signal(const std::string& p_signal_name){
				if (m_signal_list.find(p_signal_name) == m_signal_list.end()) return;

				m_signal_list.erase(p_signal_name);
			}
		
		protected:
			/*
			 *	Define the init() method.
			 *
			 * 	init() acts like the constructor and will be automatically called at the node creation.
			 */	
			virtual void init(){}

			/*
			 *	Define the destroy() method.
			 *
			 * 	destroy() acts like the destructor and will be automatically called at the node deletion.
			 */	
			virtual void destroy(){}

			/*
			 *	Define the awake() method.
			 *
			 * 	The method will be called with call_awake().
			 */	
			virtual void awake(){}

			/*
			 *	Define the sleep() method.
			 *
			 * 	The method will be called with call_sleep().
			 */	
			virtual void sleep(){}

			/*
			 *	Define the process() method.
			 *
			 * 	The method will be called with call_process().
			 */	
			virtual void process(){}

			/*
			 *	Define the render() method.
			 *
			 * 	The method will be called with call_render().
			 */	
			virtual void render(){}

		private:			
			node* m_parent;
			std::map<uint16_t, node_t> m_child_list;
			std::unordered_map<std::string, callback_hash> m_signal_list;

			void _internal_awake(){
				awake();

				for (auto const& [l_index, l_child] : m_child_list){
					if (!l_child) continue;

					l_child->_internal_awake();
				}
			}

			void _internal_sleep(){
				sleep();

				for (auto const& [l_index, l_child] : m_child_list){
					if (!l_child) continue;

					l_child->_internal_sleep();
				}
			}

			void _internal_process(){
				process();

				for (auto const& [l_index, l_child] : m_child_list){
					if (!l_child) continue;

					l_child->_internal_process();
				}
			}

			void _internal_render(){
				render();

				for (auto const& [l_index, l_child] : m_child_list){
					if (!l_child) continue;

					l_child->_internal_render();
				}
			}

			template<typename... _Targs>
			static std::unordered_map<std::type_index, std::function<node_t(_Targs...)>>& _get_node_factory(){
				static std::unordered_map<std::type_index, std::function<node_t(_Targs...)>> l_node_factory;

				return l_node_factory;
			}
	};
	class node_machine : public node {
		public:
			bool add_state(node_t p_node, uint16_t p_index){
				if (m_state_list.find(p_index) != m_state_list.end()) return false;

				return m_state_list.emplace(p_index, p_node).second;
			}

			void remove_state(uint16_t p_index){
				if (m_state_list.find(p_index) == m_state_list.end()) return;

				m_state_list.erase(p_index);
			}

			bool set_current_state(uint16_t p_index){
				if (m_state_list.find(p_index) == m_state_list.end()) return false;

				m_current_state->call_sleep();

				node_t l_new_state = m_state_list.find(p_index)->second;
				l_new_state->call_awake();

				m_current_state = l_new_state;

				return true;
			}

		protected:
			virtual void init(){
				m_current_state = nullptr;
			}

			virtual void process(){
				if (!m_current_state) return;

				m_current_state->call_process();
			}

			virtual void render(){
				if (!m_current_state) return;

				m_current_state->call_render();
			}

		private:
			std::unordered_map<uint16_t, node_t> m_state_list;
			node_t m_current_state;
	};
}

#endif
