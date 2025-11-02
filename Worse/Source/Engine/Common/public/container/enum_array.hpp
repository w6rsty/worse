#pragma once
#include "base_type.hpp"
#include "common_macro.hpp"

#include <type_traits>
#include <array>

namespace Worse
{

    template <typename EnumType, typename T>
        requires std::is_enum_v<EnumType> && requires { EnumType::Max; }
    class EnumArray
    {
    public:
        static constexpr Size N = s_cast<Size>(EnumType::Max);

        using value_type      = T;
        using size_type       = Size;
        using reference       = T&;
        using const_reference = T const&;
        using iterator        = typename std::array<T, N>::iterator;
        using const_iterator  = typename std::array<T, N>::const_iterator;

        // clang-format off
        constexpr reference       operator[](Size index)           { return m_data[index]; }
        constexpr reference       operator[](EnumType index)       { return m_data[s_cast<Size>(index)]; }
        constexpr const_reference operator[](Size index) const     { return m_data[index]; }
        constexpr const_reference operator[](EnumType index) const { return m_data[s_cast<Size>(index)]; }
        constexpr reference       at(EnumType index)               { return m_data.at(s_cast<Size>(index)); }
        constexpr const_reference at(EnumType index) const         { return m_data.at(s_cast<Size>(index)); }
        constexpr iterator        begin() noexcept                 { return m_data.begin(); }
        constexpr const_iterator  begin() const noexcept           { return m_data.begin(); }
        constexpr const_iterator  cbegin() const noexcept          { return m_data.cbegin(); }
        constexpr iterator        end() noexcept                   { return m_data.end(); }
        constexpr const_iterator  end() const noexcept             { return m_data.end(); }
        constexpr const_iterator  cend() const noexcept            { return m_data.cend(); }
        constexpr size_type       size() const noexcept            { return N; }
        constexpr Bool            empty() const noexcept           { return N == 0; }
        constexpr reference       front()                          { return m_data.front(); }
        constexpr const_reference front() const                    { return m_data.front(); }
        constexpr reference       back()                           { return m_data.back(); }
        constexpr const_reference back() const                     { return m_data.back(); }
        constexpr T*              data() noexcept                  { return m_data.data(); }
        constexpr T const*        data() const noexcept            { return m_data.data(); }
        constexpr void            fill(T const& value)             { m_data.fill(value); }
        // clang-format on

        class IteratorPair
        {
        public:
            using difference_type   = PtrDiff;
            using value_type        = std::pair<EnumType, std::reference_wrapper<T>>;
            using poInter           = void;
            using reference         = std::pair<EnumType, std::reference_wrapper<T>>;
            using iterator_category = std::forward_iterator_tag;
            using iteartor          = typename std::array<T, N>::iterator;

            constexpr IteratorPair(iteartor it, Size index)
                : m_it{it}, m_index{index}
            {
            }

            constexpr reference operator*() const
            {
                return {s_cast<EnumType>(m_index), std::ref(*m_it)};
            }

            constexpr IteratorPair& operator++()
            {
                ++m_it;
                ++m_index;
                return *this;
            }

            constexpr IteratorPair operator++(Int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr Bool operator==(IteratorPair const& other) const
            {
                return m_it == other.m_it;
            }

            constexpr Bool operator!=(IteratorPair const& other) const
            {
                return !(*this == other);
            }

        private:
            iteartor m_it = {};
            Size m_index;
        };

        class ConstIteratorPair
        {
        public:
            using difference_type   = PtrDiff;
            using value_type        = std::pair<EnumType, std::reference_wrapper<const T>>;
            using poInter           = void;
            using reference         = std::pair<EnumType, std::reference_wrapper<const T>>;
            using iterator_category = std::forward_iterator_tag;
            using const_iterator    = typename std::array<T, N>::const_iterator;

            constexpr ConstIteratorPair(const_iterator it, Size index)
                : m_it{it}, m_index{index}
            {
            }

            constexpr reference operator*() const
            {
                return {s_cast<EnumType>(m_index), std::cref(*m_it)};
            }

            constexpr ConstIteratorPair& operator++()
            {
                ++m_it;
                ++m_index;
                return *this;
            }

            constexpr ConstIteratorPair operator++(Int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr Bool operator==(const ConstIteratorPair& other) const
            {
                return m_it == other.m_it;
            }

            constexpr Bool operator!=(const ConstIteratorPair& other) const
            {
                return !(*this == other);
            }

        private:
            const_iterator m_it = {};
            Size m_index;
        };

        // clang-format off
        constexpr IteratorPair       begin_pairs() noexcept        { return IteratorPair{m_data.begin(), 0}; }
        constexpr ConstIteratorPair begin_pairs() const noexcept  { return ConstIteratorPair{m_data.begin(), 0}; }
        constexpr ConstIteratorPair cbegin_pairs() const noexcept { return ConstIteratorPair{m_data.cbegin(), 0}; }
        constexpr IteratorPair       end_pairs() noexcept          { return IteratorPair{m_data.end(), N}; }
        constexpr ConstIteratorPair end_pairs() const noexcept    { return ConstIteratorPair{m_data.end(), N}; }
        constexpr ConstIteratorPair cend_pairs() const noexcept   { return ConstIteratorPair{m_data.cend(), N}; }
        // clang-format on

    private:
        std::array<T, N> m_data;
    };

} // namespace Worse