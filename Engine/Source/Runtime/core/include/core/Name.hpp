#pragma once
// STD Headers
#include <compare>
#include <string>
#include <ostream>

// Library Headers

// Void Engine Headers

namespace sj {
    constexpr uint32_t FNV1aSeed = 0x811c9dc5;

    /**
     * String hashing algorithm for names.
     */
    inline constexpr uint32_t FNV1aHash(const char* input,
                                        const uint32_t value = FNV1aSeed) noexcept
    {
        const uint32_t prime = 0x1000193;

        if (input[0] == '\0') {
            return value;
        } else {
            return FNV1aHash(&input[1], (value ^ input[0]) * prime);
        }
    }

    /**
     * @class Name
     * @brief Name is a class used to enable fast string comparisons through use of hashing
     */
    struct Name
    {
        /**
         * Name Constructor
         */
        Name();

        /**
         * Conversion constructor, char*
         */
        Name(const char* id);

        /**
         * Conversion construtor, std::string
         * @param id The string from which the Name is generated.
         */
        Name(const std::string& id);

        /**
         * Name Destructor
         */
        ~Name() = default;

        /**
         * Assignment operator, other name
         * @param other the name to assign to this instance
         */
        Name& operator=(const Name& other);

        /** Equality comparison operator */
        inline bool operator==(const Name& other) const;

        /** Inequality comparison operator */
        inline bool operator!=(const Name& other) const;

        /**
         * Output stream operator overload
         */
        friend inline std::ostream& operator<<(std::ostream& out, const Name& other);

        /**
         * Three-way comparison operator for Name objects
         */
        inline std::strong_ordering operator<=>(const Name& other) const;

        /** The string used to generate the Name*/
        std::string StringID;

        /** The Name's Unique Hash for fast comparisons */
        uint32_t ID;
    };

    inline Name::Name() : StringID(""), ID(FNV1aHash("")) {}

    inline Name::Name(const char* id) : StringID(id), ID(FNV1aHash(id)) {}

    inline Name::Name(const std::string& id) : StringID(id), ID(FNV1aHash(id.c_str())) {}

    inline Name& Name::operator=(const Name& other)
    {
        if (other == *this) {
            return *this;
        }

        ID = other.ID;
        StringID = other.StringID;
        return *this;
    }

    inline bool Name::operator==(const Name& other) const
    {
        return ID == other.ID;
    }

    inline bool Name::operator!=(const Name& other) const
    {
        return !(*this == other);
    }

    inline std::ostream& operator<<(std::ostream& out, const Name& name)
    {
        out << name.StringID;
        return out;
    }

    inline std::strong_ordering Name::operator<=>(const Name& other) const
    {
        return ID <=> other.ID;
    }
} // namespace sj

// Make sj::Name usable as a key in hashed containers
namespace std {
    template <>
    struct hash<sj::Name>
    { // Class to define hash function for Keyboard Input
        // Hash functor
        std::size_t operator()(const sj::Name& t) const
        {
            return t.ID;
        }
    };
} // namespace std
