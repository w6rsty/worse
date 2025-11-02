#pragma once
#include "base_type.hpp"

#include <string>
#include <optional>

namespace Worse
{
    enum class ControllerType
    {
        Common,
        Xbox,
        PlayStation,
    };

    struct ControllerDescriptor
    {
        UInt joystickID  = 0;
        std::string guid = "";
        std::string name = "Unknown Controller";
        // between 0 and 100, or NULL to ignore. This will be filled in with -1
        // if can not determine a value or there is no battery.
        Int powerPercentage = -1;
        ControllerType type = ControllerType::Common;
    };

    class Controller
    {
    public:
        explicit Controller(ControllerDescriptor const& descriptor);
        ~Controller();

        std::optional<UInt> getPowerPercentage() const;
        void setLEDColor(UByte const red, UByte const green, UByte const blue) const;
        // frequency 0-1
        void vibrate(Float const lowFrequency, Float const highFrequency, UInt const durationMs) const;

        Bool isConnected() const;
        void* getHandleSDL() const;
        UInt getJoystickID() const;
        std::string const& getGUID() const;
        std::string const& getName() const;
        ControllerType getType() const;

        Bool operator==(Controller const& other) const;
        Bool operator!=(Controller const& other) const;

        Bool operator==(ControllerDescriptor const& other) const;
        Bool operator!=(ControllerDescriptor const& other) const;

    private:
        void* m_handle           = nullptr;
        UInt m_joystickID        = 0;
        std::string m_guid       = "";
        std::string m_name       = "Unknown Controller";
        ControllerType m_type    = ControllerType::Common;
        UShort m_firmwareVersion = 0;
    };

} // namespace Worse