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
        u32 joystickID   = 0;
        std::string guid = "";
        std::string name = "Unknown Controller";
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

        std::optional<u32> getPowerPercentage() const;
        void setLEDColor(u8 const red, u8 const green, u8 const blue) const;
        // frequency 0-1
        void vibrate(f32 const lowFrequency, f32 const highFrequency,
                     u32 const durationMs) const;

        bool isConnected() const;
        void* getHandleSDL() const;
        u32 getJoystickID() const;
        std::string const& getGUID() const;
        std::string const& getName() const;
        ControllerType getType() const;

        bool operator==(Controller const& other) const;
        bool operator!=(Controller const& other) const;

        bool operator==(ControllerDescriptor const& other) const;
        bool operator!=(ControllerDescriptor const& other) const;

    private:
        void* m_handle        = nullptr;
        u32 m_joystickID      = 0;
        std::string m_guid    = "";
        std::string m_name    = "Unknown Controller";
        ControllerType m_type = ControllerType::Common;
        u16 m_firmwareVersion = 0;
    };

} // namespace worse