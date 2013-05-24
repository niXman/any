
#ifndef _variant_hpp_included_
#define _variant_hpp_included_

#ifdef __GNUG__
#	define USE_DEMANGLING (1)
#	include <cxxabi.h>
#endif

#include <iostream>
#include <typeinfo>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <type_traits>

/***************************************************************************/

struct variant {
private:
	struct i_type_holder {
		virtual ~i_type_holder() {}

		virtual bool is_integral() const = 0;
		virtual bool is_signed() const = 0;
		virtual bool is_unsigned() const = 0;
		virtual bool is_floating() const = 0;
		virtual bool is_pointer() const = 0;
		virtual bool is_function_pointer() const = 0;
		virtual bool is_member_data_pointer() const = 0;
		virtual bool is_member_function_pointer() const = 0;
		virtual bool is_pod() const = 0;
		virtual bool is_class() const = 0;
		virtual bool is_union() const = 0;
		virtual bool is_polymorphic() const = 0;

		virtual std::size_t size_of() const = 0;
		virtual std::size_t alignment_of() const = 0;

		virtual const std::type_info& type_id() const = 0;
		virtual const std::string type_name() const = 0;

		virtual std::ostream& fmt_func(std::ostream&) const = 0;
	};

	template<typename T>
	struct type_holder: i_type_holder {
		T data_;

		type_holder(const T& v)
			:data_(v)
		{}

		virtual bool is_integral() const
			{ return std::is_integral<T>::value; }
		virtual bool is_signed() const
			{ return std::is_signed<T>::value; }
		virtual bool is_unsigned() const
			{ return std::is_unsigned<T>::value; }
		virtual bool is_floating() const
			{ return std::is_floating_point<T>::value; }
		virtual bool is_pointer() const
			{ return std::is_pointer<T>::value; }
		virtual bool is_function_pointer() const {
			return std::is_function<typename std::remove_pointer<T>::type>::value
				&& std::is_pointer<T>::value;
		}
		virtual bool is_member_data_pointer() const
			{ return std::is_member_object_pointer<T>::value; }
		virtual bool is_member_function_pointer() const
			{ return std::is_member_function_pointer<T>::value; }
		virtual bool is_pod() const
			{ return std::is_pod<T>::value; }
		virtual bool is_class() const
			{ return std::is_class<T>::value; }
		virtual bool is_union() const
			{ return std::is_union<T>::value; }
		virtual bool is_polymorphic() const
			{ return std::is_polymorphic<T>::value; }

		virtual std::size_t size_of() const
			{ return sizeof(T); }
		virtual std::size_t alignment_of() const
			{ return std::alignment_of<T>::value; }

		virtual const std::type_info& type_id() const
			{ return typeid(T); }
		virtual const std::string type_name() const
			{ return demangle(type_id()); }

		virtual std::ostream& fmt_func(std::ostream& os) const
			{ return (os << data_); }

	private:
		std::string demangle(const std::type_info& ti) const {
#ifdef USE_DEMANGLING
			int stat;
			char* ptr = abi::__cxa_demangle(ti.name(), 0, 0, &stat);
			if ( !ptr ) return std::string();
			std::string str(ptr);
			::free(ptr);
#else
			std::string str(ti.name());
#endif
			return str;
		}
	};

public:
	variant(const variant &v)
		:data_(v.data_)
	{}
	template<typename T>
	variant(const T& v)
		:data_(new type_holder<T>(v))
	{}

	bool is_valid() const
		{ return data_.get() != nullptr; }
	bool is_integral() const
		{ return data()->is_integral(); }
	bool is_signed() const
		{ return data()->is_signed(); }
	bool is_unsigned() const
		{ return data()->is_unsigned(); }
	bool is_floating() const
		{ return data()->is_floating(); }
	bool is_pointer() const
		{ return data()->is_pointer(); }
	bool is_function_pointer() const
		{ return data()->is_function_pointer(); }
	bool is_member_data_pointer() const
		{ return data()->is_member_data_pointer(); }
	bool is_member_function_pointer() const
		{ return data()->is_member_function_pointer(); }
	bool is_pod() const
		{ return data()->is_pod(); }
	bool is_class() const
		{ return data()->is_class(); }
	bool is_union() const
		{ return data()->is_union(); }
	bool is_polymorphic() const
		{ return data()->is_polymorphic(); }

	std::size_t size_of() const
		{ return data()->size_of(); }
	std::size_t alignment_of() const
		{ return data()->alignment_of(); }

	const std::type_info& type_id() const
		{ return data()->type_id(); }
	const std::string type_name() const
		{ return data()->type_name(); }

	template<typename T>
	bool is_same() const
		{ return (0 == strcmp(typeid(T).name(), data()->type_id().name())); }

	template<typename T>
	T& get() {
		if ( !data() ) { throw std::runtime_error("variant does not valid"); }
		if ( data()->is_pointer() ) {
			if ( !std::is_pointer<T>::value ) { throw std::bad_cast(); }
			return static_cast<type_holder<T>*>(data())->data_;
		}
		if ( !is_same<T>() ) { throw std::bad_cast(); }
		return static_cast<type_holder<T>*>(data())->data_;
	}
	template<typename T>
	const T& get() const {
		if ( !data() ) { throw std::runtime_error("variant does not valid"); }
		if ( data()->is_pointer() ) {
			if ( !std::is_pointer<T>::value ) { throw std::bad_cast(); }
			return static_cast<type_holder<T>*>(data())->data_;
		}
		if ( !is_same<T>() ) { throw std::bad_cast(); }
		return static_cast<type_holder<T>*>(data())->data_;
	}

	std::ostream& dump(std::ostream& os) const {
		if ( ! is_valid() ) { return os << "not valid" << std::endl; }
		return os
			<< std::boolalpha
			<< "is_valid                  : " << is_valid() << std::endl
#if __cplusplus == 201103L
			<< "type_id                   : " << type_id().hash_code() << std::endl
#endif
			<< "type_name                 : " << type_name() << std::endl
			<< "size_of                   : " << size_of() << std::endl
			<< "alignment_of              : " << alignment_of() << std::endl
			<< "is_integral               : " << is_integral() << std::endl
			<< "is_signed                 : " << is_signed() << std::endl
			<< "is_unsigned               : " << is_unsigned() << std::endl
			<< "is_floating               : " << is_floating() << std::endl
			<< "is_pointer                : " << is_pointer() << std::endl
			<< "is_function_pointer       : " << is_function_pointer() << std::endl
			<< "is_member_data_pointer    : " << is_member_data_pointer() << std::endl
			<< "is_member_function_pointer: " << is_member_function_pointer() << std::endl
			<< "is_pod                    : " << is_pod() << std::endl
			<< "is_class                  : " << is_class() << std::endl
			<< "is_union                  : " << is_union() << std::endl
			<< "is_polymorphic            : " << is_polymorphic() << std::endl
		;
	}

private:
	friend std::ostream& operator<< (std::ostream& os, const variant& v)
		{ return v.data()->fmt_func(os); }

	i_type_holder* data() const
		{ return data_?data_.get():throw std::runtime_error(__PRETTY_FUNCTION__); }

	std::shared_ptr<i_type_holder> data_;
};

/***************************************************************************/

#endif // _variant_hpp_included_
