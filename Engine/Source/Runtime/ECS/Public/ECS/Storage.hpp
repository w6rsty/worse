#pragma once
#include "Config.hpp"
#include "Entity.hpp"
#include "IndexSet.hpp"

#include <memory>
#include <vector>
#include <type_traits>

namespace worse::ecs
{

    namespace internal
    {
        template <typename Container, std::size_t PageSize>
        struct StorageIterator
        {
            // clang-format off
            using ContainerType = std::remove_cv_t<Container>;
            using AllocTraits   = std::allocator_traits<typename ContainerType::allocator_type>;

            using element_type = typename std::pointer_traits<typename Container::value_type>::element_type;
            using pointer_type = std::conditional_t<
                std::is_const_v<Container>,
                typename AllocTraits::template rebind_traits<element_type>::const_pointer,
                typename AllocTraits::template rebind_traits<element_type>::pointer
            >;

            using iterator_traits = std::iterator_traits<pointer_type>;

            using value_type        = typename iterator_traits::value_type;
            using pointer           = typename iterator_traits::pointer;
            using reference         = typename iterator_traits::reference;
            using difference_type   = typename iterator_traits::difference_type;
            using iterator_category = std::random_access_iterator_tag;
            // clang-format on

            StorageIterator() : payload(nullptr), offset(0)
            {
            }
            StorageIterator(Container& payload, std::size_t const offset)
                : payload(&payload), offset(offset)
            {
            }

            StorageIterator& operator++()
            {
                return --offset, *this;
            }
            StorageIterator operator++(int)
            {
                StorageIterator const copy = *this;
                return operator++(), copy;
            }
            StorageIterator& operator--()
            {
                return ++offset, *this;
            }
            StorageIterator operator--(int)
            {
                StorageIterator const copy = *this;
                return operator--(), copy;
            }
            StorageIterator& operator+=(difference_type const value)
            {
                return offset -= value, *this;
            }
            StorageIterator operator+(difference_type const value) const
            {
                StorageIterator copy = *this;
                return (copy += value);
            }
            StorageIterator& operator-=(difference_type const value)
            {
                return offset += value, *this;
            }
            StorageIterator operator-(difference_type const value) const
            {
                StorageIterator copy = *this;
                return (copy -= value);
            }

            reference operator[](difference_type const value)
            {
                difference_type const position =
                    static_cast<difference_type>(index() - value);
                return (*payload)[static_cast<std::size_t>(position) / PageSize]
                                 [fast_mod(static_cast<std::size_t>(position),
                                           PageSize)];
            }
            pointer operator->()
            {
                return std::addressof(operator[](0));
            }
            reference operator*()
            {
                return operator[](0);
            }
            std::size_t index() const
            {
                return offset - 1UL;
            }

            Container* payload;
            difference_type offset;
        };

        template <typename Container, std::size_t PageSize>
        inline typename StorageIterator<Container, PageSize>::difference_type
        operator-(StorageIterator<Container, PageSize> const& lhs,
                  StorageIterator<Container, PageSize> const& rhs)
        {
            return rhs.offset - lhs.offset;
        }

        template <typename Container, std::size_t PageSize>
        inline bool operator==(StorageIterator<Container, PageSize> const& lhs,
                               StorageIterator<Container, PageSize> const& rhs)
        {
            return lhs.offset == rhs.offset;
        }

        template <typename Container, std::size_t PageSize>
        inline bool operator!=(StorageIterator<Container, PageSize> const& lhs,
                               StorageIterator<Container, PageSize> const& rhs)
        {
            return !(lhs == rhs);
        }

        template <typename Container, std::size_t PageSize>
        inline bool operator<(StorageIterator<Container, PageSize> const& lhs,
                              StorageIterator<Container, PageSize> const& rhs)
        {
            return lhs.offset > rhs.offset;
        }

        template <typename Container, std::size_t PageSize>
        inline bool operator>(StorageIterator<Container, PageSize> const& lhs,
                              StorageIterator<Container, PageSize> const& rhs)
        {
            return lhs.offset < rhs.offset;
        }

        template <typename Container, std::size_t PageSize>
        inline bool operator<=(StorageIterator<Container, PageSize> const& lhs,
                               StorageIterator<Container, PageSize> const& rhs)
        {
            return !(lhs > rhs);
        }

        template <typename Container, std::size_t PageSize>
        inline bool operator>=(StorageIterator<Container, PageSize> const& lhs,
                               StorageIterator<Container, PageSize> const& rhs)
        {
            return !(lhs < rhs);
        }
    } // namespace internal

    template <typename T> class Storage : public IndexSet
    {
    public:
        // clang-format off
        static constexpr std::size_t PAGE_SIZE = PACKED_PAGE_SIZE;
        using ValueType                        = T;
        using ContainerType                    = std::vector<T*>;
        using BaseType                         = IndexSet;
        using Allocator                        = typename std::allocator_traits<typename ContainerType::allocator_type>::template rebind_alloc<T>;
        using AllocTraits                      = std::allocator_traits<Allocator>;
        // clang-format on

    private:
        ValueType& payloadRef(std::size_t const position)
        {
            return m_payload[position / PAGE_SIZE]
                            [fast_mod(position, PAGE_SIZE)];
        }

        ValueType& assureMemory(std::size_t const position)
        {
            std::size_t const page = position / PAGE_SIZE;

            // adujst page
            if (page >= m_payload.size())
            {
                std::size_t const currSize = m_payload.size();
                m_payload.resize(page + 1UL, nullptr);

                Allocator allocator = m_payload.get_allocator();
                // allocate page memory
                for (std::size_t i = currSize; i < m_payload.size(); ++i)
                {
                    // sizeof(T) * PAGE_SIZE
                    m_payload[i] = AllocTraits::allocate(allocator, PAGE_SIZE);
                }
            }

            // Return reference to the element at the specified position
            return m_payload[page][fast_mod(position, PAGE_SIZE)];
        }

        void shrinkToSize(std::size_t const size)
        {
            std::size_t const from = (size + PAGE_SIZE - 1) / PAGE_SIZE;
            Allocator allocator    = m_payload.get_allocator();

            for (std::size_t i = size; i < BaseType::size(); ++i)
            {
                AllocTraits::destroy(allocator, std::addressof(payloadRef(i)));
            }

            for (std::size_t i = from; i < m_payload.size(); ++i)
            {
                AllocTraits::deallocate(allocator, m_payload[i], PAGE_SIZE);
            }

            m_payload.resize(from, nullptr);
            m_payload.shrink_to_fit();
        }

    public:
        using Iterator = internal::StorageIterator<ContainerType, PAGE_SIZE>;
        using ConstIterator =
            internal::StorageIterator<ContainerType const, PAGE_SIZE>;

        Storage() : BaseType()
        {
        }

        Storage(Storage const&)            = delete;
        Storage& operator=(Storage const&) = delete;

        Storage(Storage&&) noexcept            = default;
        Storage& operator=(Storage&&) noexcept = default;

        ~Storage()
        {
            shrinkToSize(0UL);
        }

        Iterator find(Entity const entity)
        {
            std::size_t const index = BaseType::packedIndex(entity);
            return Iterator(m_payload, index + 1);
        }

        template <typename... Args>
            requires(std::is_constructible_v<T, Args...>)
        ValueType& emplace(Entity const entity, Args&&... args)
        {
            auto indexIt = BaseType::insert(entity);
            std::size_t const position =
                static_cast<std::size_t>(indexIt.index());

            // reference of uninitialized memory
            ValueType& ref = assureMemory(position);
            std::uninitialized_construct_using_allocator(
                std::addressof(ref),
                m_payload.get_allocator(),
                std::forward<Args>(args)...);

            return m_payload[position / PAGE_SIZE]
                            [fast_mod(position, PAGE_SIZE)];
        }

        // Get component reference by entity
        ValueType& get(Entity const entity)
        {
            std::size_t const index = BaseType::packedIndex(entity);
            return payloadRef(index);
        }

        ValueType const& get(Entity const entity) const
        {
            std::size_t const index = BaseType::packedIndex(entity);
            return const_cast<Storage*>(this)->payloadRef(index);
        }

        // =====================================================================
        // Iterators
        // =====================================================================

        // clang-format off
        Iterator begin()        { return Iterator(m_payload, BaseType::size()); }
        Iterator end()          { return Iterator(m_payload, 0); }
        Iterator cbegin() const { return Iterator(m_payload, BaseType::size()); }
        Iterator cend() const   { return Iterator(m_payload, 0); }
        // clang-format on

    private:
        ContainerType m_payload;
    };

    // specialization empty tage
    template <typename T>
        requires(std::is_empty_v<T>)
    class Storage<T> : public IndexSet
    {
    public:
        using BaseType  = IndexSet;
        using ValueType = T; // Add ValueType for consistency

        Storage() : BaseType()
        {
        }

        Storage(Storage const&)            = delete;
        Storage& operator=(Storage const&) = delete;

        Storage(Storage&&) noexcept            = default;
        Storage& operator=(Storage&&) noexcept = default;

        ~Storage() = default;

        void emplace(Entity const entity)
        {
            BaseType::insert(entity);
        }
    };

    // specialization for managing entities
    template <> class Storage<Entity> : public IndexSet
    {
    public:
        using BaseType = IndexSet;

        Storage() : BaseType(), m_nextId(0)
        {
        }

        Storage(Storage const&)            = delete;
        Storage& operator=(Storage const&) = delete;

        Storage(Storage&&) noexcept            = default;
        Storage& operator=(Storage&&) noexcept = default;

        ~Storage() = default;

        Entity generate()
        {
            Entity entity(m_nextId++, 0);
            BaseType::insert(entity);
            return entity;
        }

    private:
        Entity::EntityType m_nextId;
    };

    // Clean template-based type erasure for Storage
    class StorageBase
    {
    public:
        virtual ~StorageBase()                     = default;
        virtual void remove(Entity entity)         = 0;
        virtual std::size_t size() const           = 0;
        virtual bool contains(Entity entity) const = 0;
    };

    template <typename T> struct StorageWrapper : public StorageBase
    {
        Storage<T> storage;

        void remove(Entity entity) override
        {
            storage.remove(entity);
        }

        std::size_t size() const override
        {
            return storage.size();
        }

        bool contains(Entity entity) const override
        {
            return storage.contains(entity);
        }
    };

} // namespace worse::ecs