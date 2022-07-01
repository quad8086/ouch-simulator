#pragma once

#include <string.h>

#include <ostream>
#include <istream>
#include <boost/operators.hpp>
#include <boost/optional.hpp>

#include <boost/iterator/iterator_facade.hpp>

#include <type_traits>

namespace boost {
namespace detail {

	template <typename T>
	BOOST_DEDUCED_TYPENAME T::domain enum_cast(
		BOOST_DEDUCED_TYPENAME T::index_type index)
	{
		return static_cast<BOOST_DEDUCED_TYPENAME T::domain>(index);
	}

	template <typename T>
	class enum_iterator
		: public boost::iterator_facade
			< enum_iterator<T>
			, const T
			, boost::random_access_traversal_tag>
	{
		typedef
			boost::iterator_facade
				< enum_iterator<T>
				, const T
				, boost::random_access_traversal_tag>
			facade;

		typedef enum_iterator<T> this_type;

	public:
		enum_iterator(size_t index)
			: m_value(enum_cast<T>(index))
			, m_index(index)
		{}

	private:
		friend class boost::iterator_core_access;

		const T& dereference() const
		{
			return m_value;
		}

		void increment()
		{
			++m_index;
			m_value = enum_cast<T>(m_index);
		}

		void decrement()
		{
			--m_index;
			m_value = enum_cast<T>(m_index);
		}

		bool equal(const this_type& rhs) const
		{
			return m_index == rhs.m_index;
		}

		void advance(BOOST_DEDUCED_TYPENAME facade::difference_type n)
		{
			m_index += n;
			m_value = enum_cast<T>(m_index);
		}

		BOOST_DEDUCED_TYPENAME facade::difference_type distance_to(
			const this_type& rhs) const
		{
			return rhs.m_index - m_index;
		}

	private:
		T m_value;
		size_t m_index;
	};

} // detail
} // boost

namespace boost {
namespace detail {

	template <typename Derived, typename ValueType = int>
	class enum_base
		: private boost::totally_ordered<Derived>
	{
	public:
		typedef enum_base<Derived, ValueType> this_type;
		typedef size_t index_type;
		typedef ValueType value_type;
		typedef enum_iterator<Derived> const_iterator;
		typedef boost::optional<Derived> optional;

	public:
		enum_base() {}
		enum_base(index_type index) : m_index(index) {}

		static const_iterator begin()
		{
			return const_iterator(0);
		}

		static const_iterator end()
		{
			return const_iterator(Derived::size);
		}

		static optional get_by_value(value_type value)
		{
			for(index_type i = 0; i < Derived::size; ++i)
			{
				typedef boost::optional<value_type> optional_value;
				optional_value cur = Derived::values(enum_cast<Derived>(i));
				if(value == *cur)
					return Derived(enum_cast<Derived>(i));
			}
			return optional();
		}

	        static optional get_by_string(const char* s_enum)
		{
			for(index_type i = 0; i < Derived::size; ++i)
			{
			  const char* s_value = Derived::names(enum_cast<Derived>(i));
			  if(!strcmp(s_enum, s_value))
			    return Derived(enum_cast<Derived>(i));
			}
			return optional();
		}

	        static optional get_by_istring(const char* s_enum)
		{
			for(index_type i = 0; i < Derived::size; ++i)
			{
			  const char* s_value = Derived::names(enum_cast<Derived>(i));
			  if(!strcasecmp(s_enum, s_value))
			    return Derived(enum_cast<Derived>(i));
			}
			return optional();
		}


	        static Derived get_by_string_with_default(const char* s_enum, const Derived& def)
		{
			for(index_type i = 0; i < Derived::size; ++i)
			{
			  const char* s_value = Derived::names(enum_cast<Derived>(i));
			  if(!strcmp(s_enum, s_value))
			    return Derived(enum_cast<Derived>(i));
			}
			return def;
		}

	        static Derived get_by_istring_with_default(const char* s_enum, const Derived& def)
		{
			for(index_type i = 0; i < Derived::size; ++i)
			{
			  const char* s_value = Derived::names(enum_cast<Derived>(i));
			  if(!strcasecmp(s_enum, s_value))
			    return Derived(enum_cast<Derived>(i));
			}
			return def;
		}


		static optional get_by_index(index_type index)
		{
			if(index >= Derived::size) return optional();
			return optional(enum_cast<Derived>(index));
		}

		const char* str() const
		{
			const char* ret = Derived::names(enum_cast<Derived>(m_index));
			BOOST_ASSERT(ret);
			return ret;
		}

		value_type value() const
		{
			typedef boost::optional<value_type> optional_value;
			optional_value ret = Derived::values(enum_cast<Derived>(this->m_index));
			BOOST_ASSERT(ret);
			return *ret;
		}

		index_type index() const
		{
			return m_index;
		}

		bool operator == (const this_type& rhs) const
		{
			return m_index == rhs.m_index;
		}

		bool operator < (const this_type& rhs) const
		{
			value_type lhs_value = value();
			value_type rhs_value = rhs.value();
			if(lhs_value == rhs_value)
				return m_index < rhs.m_index;
			return lhs_value < rhs_value;
		}

	protected:
		friend class enum_iterator<Derived>;
		index_type m_index;
	};

	template <typename D, typename V>
	std::ostream& operator << (std::ostream& os, const enum_base<D, V>& rhs)
	{
		return (os << rhs.str());
	}

	template <typename D, typename V>
	std::istream& operator >> (std::istream& is, enum_base<D, V>& rhs)
	{
		std::string str;
		is >> str;
		BOOST_DEDUCED_TYPENAME D::optional ret = D::get_by_name(str.c_str());
		if(ret)
			rhs = *ret;
		else
			is.setstate(std::ios::badbit);
		return is;
	}

} // detail
} // boost


#include <boost/preprocessor.hpp>

#define BOOST_ENUM_IS_COLUMN_2(i, k) \
	BOOST_PP_EQUAL(BOOST_PP_MOD(i, 2), k)

#define BOOST_ENUM_GET_COLUMN_2(r, data, i, elem) \
	BOOST_PP_IF(BOOST_ENUM_IS_COLUMN_2(i, data), (elem), BOOST_PP_EMPTY())

#define BOOST_ENUM_VISITOR1(_seq, _macro, _col) \
	BOOST_PP_SEQ_FOR_EACH(_macro, _, _seq)

#define BOOST_ENUM_VISITOR2(_seq, _macro, _col) \
	BOOST_PP_SEQ_FOR_EACH( \
		_macro, \
		_, \
		BOOST_PP_SEQ_FOR_EACH_I(BOOST_ENUM_GET_COLUMN_2, _col, _seq) \
	)

#define BOOST_ENUM_DOMAIN_ITEM(r, data, elem) \
	elem BOOST_PP_COMMA()

#define BOOST_ENUM_domain(_seq, _col, _colsize) \
	enum domain \
	{ \
		BOOST_PP_CAT(BOOST_ENUM_VISITOR, _colsize) \
			(_seq, BOOST_ENUM_DOMAIN_ITEM, _col) \
	}; \
	BOOST_STATIC_CONSTANT(index_type, size = BOOST_ENUM_size(_seq, _colsize));

#define BOOST_ENUM_size(_seq, _colsize) \
	BOOST_PP_DIV(BOOST_PP_SEQ_SIZE(_seq), _colsize)

#define BOOST_ENUM_PARSE_ITEM(r, data, elem) \
	if(strcmp(str, BOOST_PP_STRINGIZE(elem)) == 0) return optional(elem);

#define BOOST_ENUM_get_by_name(_name, _seq, _col, _colsize) \
	static optional get_by_name(const char* str) \
	{ \
		BOOST_PP_CAT(BOOST_ENUM_VISITOR, _colsize) \
			(_seq, BOOST_ENUM_PARSE_ITEM, _col) \
		return optional(); \
	}

#define BOOST_ENUM_CASE_STRING(r, data, elem) \
	case elem: return BOOST_PP_STRINGIZE(elem);

#define BOOST_ENUM_names(_seq, _col, _colsize) \
	static const char* names(domain index) \
	{ \
		BOOST_ASSERT(static_cast<index_type>(index) < size); \
		switch(index) \
		{ \
		BOOST_PP_CAT(BOOST_ENUM_VISITOR, _colsize) \
			(_seq, BOOST_ENUM_CASE_STRING, _col) \
		default: return NULL; \
		} \
	}

#define BOOST_ENUM_CASE_PART(elem) \
	case (elem):

#define BOOST_ENUM_VALUE_PART(elem) \
	return optional_value(elem);

#define BOOST_ENUM_VALUE_LINE(r, data, i, elem) \
	BOOST_PP_IF(BOOST_ENUM_IS_COLUMN_2(i, 0), \
		BOOST_ENUM_CASE_PART(elem), \
		BOOST_ENUM_VALUE_PART(elem) \
	)

#define BOOST_ENUM_values_identity() \
	typedef boost::optional<value_type> optional_value; \
	static optional_value values(domain index) \
	{ \
		if(static_cast<index_type>(index) < size) return optional_value(index); \
		return optional_value(); \
	}

#define BOOST_ENUM_values(_seq, _name_col, _value_col, _colsize) \
	typedef boost::optional<value_type> optional_value; \
	static optional_value values(domain index) \
	{ \
		switch(index) \
		{ \
		BOOST_PP_SEQ_FOR_EACH_I(BOOST_ENUM_VALUE_LINE, _, _seq) \
		default: return optional_value(); \
		} \
	}

#define BOOST_BITFIELD_names(_seq, _col, _colsize) \
	static const char* names(domain index) \
	{ \
		BOOST_ASSERT(static_cast<index_type>(index) < size); \
		switch(index) \
		{ \
		case all_mask: return "all_mask"; \
		BOOST_PP_CAT(BOOST_ENUM_VISITOR, _colsize) \
			(_seq, BOOST_ENUM_CASE_STRING, _col) \
		default: return NULL; \
		} \
	}

#define BOOST_BITFIELD_OR_ITEM(r, data, elem) \
	| (elem)

#define BOOST_BITFIELD_domain(_seq, _col, _colsize) \
	enum domain \
	{ \
		BOOST_PP_CAT(BOOST_ENUM_VISITOR, _colsize) \
			(_seq, BOOST_ENUM_DOMAIN_ITEM, _col) \
		all_mask, \
		not_mask \
	}; \
	BOOST_STATIC_CONSTANT(index_type, size = BOOST_ENUM_size(_seq, _colsize));

#define BOOST_BITFIELD_all_mask(_seq, _col, _colsize) \
	0 BOOST_PP_CAT(BOOST_ENUM_VISITOR, _colsize) \
		(_seq, BOOST_BITFIELD_OR_ITEM, _col)

#define BOOST_BITFIELD_get_by_name(_name, _seq, _col, _colsize) \
	static optional get_by_name(const char* str) \
	{ \
		if(strcmp(str, "all_mask") == 0) return optional(all_mask); \
		if(strcmp(str, "not_mask") == 0) return optional(not_mask); \
		BOOST_PP_CAT(BOOST_ENUM_VISITOR, _colsize) \
			(_seq, BOOST_ENUM_PARSE_ITEM, _col) \
		return optional(); \
	}

#define BOOST_BITFIELD_values(_seq, _name_col, _value_col, _colsize) \
	typedef boost::optional<value_type> optional_value; \
	static optional_value values(domain index) \
	{ \
		switch(index) \
		{ \
		BOOST_PP_SEQ_FOR_EACH_I(BOOST_ENUM_VALUE_LINE, _, _seq) \
		case all_mask: return optional_value(BOOST_BITFIELD_all_mask(_seq, _value_col, _colsize)); \
		case not_mask: return optional_value(~(BOOST_BITFIELD_all_mask(_seq, _value_col, _colsize))); \
		default: return optional_value(); \
		} \
	}

#define BOOST_ENUM(_name, _seq) \
	class _name : public boost::detail::enum_base<_name> \
	{ \
	public: \
		BOOST_ENUM_domain(_seq, 0, 1) \
		_name() {} \
		_name(domain index) : boost::detail::enum_base<_name>(index) {} \
		BOOST_ENUM_get_by_name(_name, _seq, 0, 1) \
	private: \
		friend class boost::detail::enum_base<_name>; \
		BOOST_ENUM_names(_seq, 0, 1) \
		BOOST_ENUM_values_identity() \
	};

#define BOOST_ENUM_VALUES(_name, _type, _seq) \
	class _name : public boost::detail::enum_base<_name, _type> \
	{ \
	public: \
		BOOST_ENUM_domain(_seq, 0, 2) \
		_name() {} \
		_name(domain index) : boost::detail::enum_base<_name, _type>(index) {} \
		BOOST_ENUM_get_by_name(_name, _seq, 0, 2) \
	private: \
		friend class boost::detail::enum_base<_name, _type>; \
		BOOST_ENUM_names(_seq, 0, 2) \
		BOOST_ENUM_values(_seq, 0, 1, 2) \
	};

#define BOOST_BITFIELD(_name, _seq) \
	class _name : public boost::detail::bitfield_base<_name> \
	{ \
	public: \
		BOOST_BITFIELD_domain(_seq, 0, 2) \
		_name() {} \
		_name(domain index) : boost::detail::bitfield_base<_name>(index) {} \
		BOOST_ENUM_get_by_name(_name, _seq, 0, 2) \
	private: \
		friend class boost::detail::bitfield_access; \
		_name(value_type raw, int) : boost::detail::bitfield_base<_name>(raw, 0) {} \
		BOOST_BITFIELD_names(_seq, 0, 2) \
		BOOST_BITFIELD_values(_seq, 0, 1, 2) \
	};


namespace enumops {

// operator == to allow use such as RestrictionType::None == rest (where rest is RestrictionType)
// Import in your namespace as needed (via using ::ns::operator ==).
// (Can't do it as part of the macros above since those
// are used in class scope in certain cases)
template <typename T>
auto
operator == (const typename T::domain& constant, const T& other)
  -> decltype(std::is_base_of<boost::detail::enum_base<T,typename T::value_type>, T>::value, true) {
    return other == constant;
}

}
