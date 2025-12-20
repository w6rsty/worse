#pragma once
#include "BaseTypes.hpp"

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
        U32 joystickID  = 0;
        std::string guid = "";
        std::string name = "Unknown Controller";
        // between 0 and 100, or NULL to ignore. This will be filled in with -1
        // if can not determine a value or there is no battery.
        I32 powerPercentage = -1;
        ControllerType type = ControllerType::Common;
    };

    class Controller
    {
    public:
        explicit Controller(ControllerDescriptor const& descriptor);
        ~Controller();

        std::optional<U32> getPowerPercentage() const;
        void setLEDColor(U8 const red, U8 const green, U8 const blue) const;
        // frequency 0-1
        void vibrate(F32 const lowFrequency, F32 const highFrequency, U32 const durationMs) const;

        bool isConnected() const;
        void* getHandleSDL() const;
        U32 getJoystickID() const;
        std::string const& getGUID() const;
        std::string const& getName() const;
        ControllerType getType() const;

        bool operator==(Controller const& other) const;
        bool operator!=(Controller const& other) const;

        bool operator==(ControllerDescriptor const& other) const;
        bool operator!=(ControllerDescriptor const& other) const;

    private:
        void* m_handle           = nullptr;
        U32 m_joystickID        = 0;
        std::string m_guid       = "";
        std::string m_name       = "Unknown Controller";
        ControllerType m_type    = ControllerType::Common;
        U16 m_firmwareVersion = 0;
    };

} // namespace worse