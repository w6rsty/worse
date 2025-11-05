#pragma once
#include "base_type.hpp"
#include <limits>
#include <cstdint>

namespace Worse::ecs
{

    struct Entity
    {
        using ValueType   = ULong;
        using EntityType  = ULong;
        using VersionType = UInt;

        ValueType value;

        static constexpr ValueType ENTITY_MASK     = 0xFFFFFFFF;
        static constexpr VersionType VERSION_SHIFT = 32;

        // default constructor, initializes as a null entity
        Entity() : value(std::numeric_limits<ValueType>::max())
        {
        }

        explicit Entity(ValueType val) : value(val)
        {
        }

        Entity(EntityType id, VersionType version)
            : value((static_cast<EntityType>(version) << VERSION_SHIFT) | id)
        {
        }

        // clang-format off
        EntityType toEntity() const { return value & ENTITY_MASK; }
        VersionType toVersion() const { return static_cast<VersionType>(value >> VERSION_SHIFT); }
        
        Bool operator==(Entity const& other) const { return value == other.value; }
        Bool operator!=(Entity const& other) const { return !(*this == other); }
        Bool operator<(Entity const& other) const { return value < other.value; }

        static Entity null() { return Entity(std::numeric_limits<ValueType>::max()); }
        // clang-format on
    };

} // namespace Worse::ecs