#pragma once
#include "Config.hpp"
#include "Entity.hpp"

#include <vector>
#include <memory>
#include <cassert>
#include <iterator>

namespace worse::ecs
{

    namespace internal
    {
        struct IndexSetIterator
        {
            using value_type          = Entity;
            using PackedContainerType = std::vector<value_type>;

            using iterator_traits = std::iterator_traits<
                typename PackedContainerType::const_iterator>;
            using pointer           = typename iterator_traits::pointer;
            using reference         = typename iterator_traits::reference;
            using difference_type   = typename iterator_traits::difference_type;
            using iterator_category = std::random_access_iterator_tag;

            IndexSetIterator() : packed(nullptr), offset(0)
            {
            }
            IndexSetIterator(PackedContainerType const& packed,
                             std::size_t const offset)
                : packed(&packed), offset(offset)
            {
            }

            IndexSetIterator& operator++()
            {
                return --offset, *this;
            }
            IndexSetIterator operator++(int)
            {
                IndexSetIterator const copy = *this;
                return operator++(), copy;
            }
            IndexSetIterator& operator--()
            {
                return ++offset, *this;
            }
            IndexSetIterator operator--(int)
            {
                IndexSetIterator const copy = *this;
                return operator--(), copy;
            }
            IndexSetIterator& operator+=(difference_type const value)
            {
                return offset -= value, *this;
            }
            IndexSetIterator operator+(difference_type const value) const
            {
                IndexSetIterator copy = *this;
                return (copy += value);
            }
            IndexSetIterator& operator-=(difference_type const value)
            {
                return offset += value, *this;
            }
            IndexSetIterator operator-(difference_type const value) const
            {
                IndexSetIterator copy = *this;
                return (copy -= value);
            }

            reference operator[](difference_type const value) const
            {
                return (*packed)[static_cast<std::size_t>(index() - value)];
            }
            pointer operator->() const
            {
                return std::addressof(operator[](0));
            }
            reference operator*() const
            {
                return operator[](0);
            }
            // index in packed container
            difference_type index() const
            {
                return offset - 1;
            }
            pointer data() const
            {
                return packed ? packed->data() : nullptr;
            }

            PackedContainerType const* packed;
            difference_type offset;
        };

        inline typename IndexSetIterator::difference_type
        operator-(IndexSetIterator const& lhs, IndexSetIterator const& rhs)
        {
            return rhs.offset - lhs.offset;
        }

        inline bool operator==(IndexSetIterator const& lhs,
                               IndexSetIterator const& rhs)
        {
            return lhs.offset == rhs.offset;
        }
        inline bool operator!=(IndexSetIterator const& lhs,
                               IndexSetIterator const& rhs)
        {
            return !(lhs == rhs);
        }
        inline bool operator<(IndexSetIterator const& lhs,
                              IndexSetIterator const& rhs)
        {
            return lhs.offset > rhs.offset;
        }
        inline bool operator>(IndexSetIterator const& lhs,
                              IndexSetIterator const& rhs)
        {
            return lhs.offset < rhs.offset;
        }
        inline bool operator<=(IndexSetIterator const& lhs,
                               IndexSetIterator const& rhs)
        {
            return !(lhs > rhs);
        }
        inline bool operator>=(IndexSetIterator const& lhs,
                               IndexSetIterator const& rhs)
        {
            return !(lhs < rhs);
        }
    } // namespace internal

    // A specialized sparse set for managing entity
    class IndexSet
    {
        // clang-format off
        static constexpr std::size_t PAGE_SIZE = SPARSE_PAGE_SIZE;
        using ValueType                        = Entity;
        using SparseContainerType              = std::vector<ValueType*>;
        using PackedContainerType              = std::vector<ValueType>;
        using Allocator                        = typename PackedContainerType::allocator_type;
        // clang-format on

    protected:
        using Iterator       = internal::IndexSetIterator;
        using ConstIterator  = Iterator;
        using DifferenceType = typename Iterator::difference_type;

        std::size_t positionToPage(std::size_t const position) const
        {
            return position / PAGE_SIZE;
        }

        auto entityToIterator(Entity const entity) const
        {
            return --(end() - static_cast<DifferenceType>(packedIndex(entity)));
        }

        // get entity address in sparse container.
        // return nullptr when page does not exist
        ValueType* sparePtr(Entity const entity) const
        {
            std::size_t const position =
                static_cast<std::size_t>(entity.toEntity());
            std::size_t const page = positionToPage(position);
            return (page < m_sparse.size() && m_sparse[page])
                       ? &m_sparse[page][fast_mod(position, PAGE_SIZE)]
                       : nullptr;
        }

        // get entity reference in sparse container
        ValueType& spareRef(Entity const entity) const
        {
            assert(sparePtr(entity) && "Sparse page fault");
            std::size_t const position =
                static_cast<std::size_t>(entity.toEntity());
            return m_sparse[positionToPage(position)]
                           [fast_mod(position, PAGE_SIZE)];
        }

        ValueType& assureMemory(Entity const entity)
        {
            std::size_t const position =
                static_cast<std::size_t>(entity.toEntity());
            std::size_t const page = positionToPage(position);

            // adujst page
            if (page >= m_sparse.size())
            {
                m_sparse.resize(page + 1, nullptr);
            }

            // allocate new page
            if (!m_sparse[page])
            {
                m_sparse[page] = m_packed.get_allocator().allocate(PAGE_SIZE);
                std::uninitialized_fill_n(m_sparse[page],
                                          PAGE_SIZE,
                                          ValueType::null());
            }

            return m_sparse[page][fast_mod(position, PAGE_SIZE)];
        }

        void releasePage()
        {
            Allocator allocator = m_packed.get_allocator();
            for (auto& page : m_sparse)
            {
                if (page)
                {
                    std::destroy_n(page, PAGE_SIZE);
                    allocator.deallocate(page, PAGE_SIZE);
                    page = nullptr;
                }
            }
        }

        Iterator insert(Entity const entity)
        {
            if (contains(entity))
            {
                return entityToIterator(entity);
            }

            std::size_t const position = m_packed.size();
            // Ensure the sparse page exists and is initialized
            ValueType& ref = assureMemory(entity);

            m_packed.emplace_back(entity);
            ref = Entity(m_packed.size() - 1UL, entity.toVersion());

            return --(end() - static_cast<DifferenceType>(position));
        }

    public:
        IndexSet() = default;

        IndexSet(IndexSet const&)            = delete;
        IndexSet& operator=(IndexSet const&) = delete;

        IndexSet(IndexSet&&) noexcept            = default;
        IndexSet& operator=(IndexSet&&) noexcept = default;

        ~IndexSet()
        {
            releasePage();
        }

        bool contains(Entity const entity) const
        {
            ValueType* ptr = sparePtr(entity);
            return ptr && (ptr->toVersion() == entity.toVersion());
        }

        // find entity in sparse container
        Iterator find(Entity const entity)
        {
            return contains(entity) ? entityToIterator(entity) : end();
        }

        void remove(Entity const entity)
        {
            if (!contains(entity))
            {
                return; // not exists
            }

            ValueType& removed             = spareRef(entity);
            std::size_t const removedIndex = removed.toEntity();
            ValueType const lastEntity     = m_packed.back();

            m_packed[removedIndex] = lastEntity; // swap with last element
            if (lastEntity != entity)
            {
                // update sparse reference of last element
                spareRef(lastEntity) =
                    Entity(removedIndex, lastEntity.toVersion());
            }
            m_packed.pop_back();      // remove last element
            removed = Entity::null(); // clear sparse
        }

        void remove(Iterator first, Iterator last)
        {
            for (auto it = first; it != last; ++it)
            {
                remove(*it);
            }
        }

        std::size_t packedIndex(Entity const entity) const
        {
            assert(contains(entity));
            // take out packed index stored in sparse container
            return spareRef(entity).toEntity();
        }

        // clear all entries but keep allocated memory
        void clear()
        {
            for (Entity& entity : m_packed)
            {
                spareRef(entity) = Entity::null();
            }
            m_packed.clear();
        }

        std::size_t size() const
        {
            return m_packed.size();
        }

        // =====================================================================
        // Iterators
        // =====================================================================

        // clang-format off
        Iterator      begin() const  { return Iterator(m_packed, m_packed.size()); }
        Iterator      end() const    { return Iterator(m_packed, {}); }
        ConstIterator cbegin() const { return begin(); }
        ConstIterator cend() const   { return end(); }
        // clang-format on

    protected:
        SparseContainerType m_sparse;
        PackedContainerType m_packed;
    };

} // namespace worse::ecs