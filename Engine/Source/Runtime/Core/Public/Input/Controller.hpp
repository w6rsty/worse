#pragma once
#include "Types.hpp"

#include <string>
#include <optional>

namespace worse
{
    enum class ControllerType
    {
        Common,
        Xbox,
        PlayStation,
    };

    struct ControllerDescriptor
    {
        std::uint32_t joystickID = 0;
        std::string guid         = "";
        std::string name         = "Unknown Controller";
        // between 0 and 100, or NULL to ignore. This will be filled in with -1
        // if can not determine a value or there is no battery.
        int powerPercentage = -1;
        ControllerType type = ControllerType::Common;
    };

    class Controller : public NonCopyable
    {
    public:
        explicit Controller(ControllerDescriptor const& descriptor);
        ~Controller();

        std::optional<std::uint32_t> getPowerPercentage() const;
        void setLEDColor(std::uint8_t const red, std::uint8_t const green,
                         std::uint8_t const blue) const;
        // frequency 0-1
        void vibrate(float const lowFrequency, float const highFrequency,
                     std::uint32_t const durationMs) const;

        bool isConnected() const;
        void* getHandleSDL() const;
        std::uint32_t getJoystickID() const;
        std::string const& getGUID() const;
        std::string const& getName() const;
        ControllerType getType() const;

        bool operator==(Controller const& other) const;
        bool operator!=(Controller const& other) const;

        bool operator==(ControllerDescriptor const& other) const;
        bool operator!=(ControllerDescriptor const& other) const;

    private:
        void* m_handle                  = nullptr;
        std::uint32_t m_joystickID      = 0;
        std::string m_guid              = "";
        std::string m_name              = "Unknown Controller";
        ControllerType m_type           = ControllerType::Common;
        std::uint16_t m_firmwareVersion = 0;
    };

} // namespace worse