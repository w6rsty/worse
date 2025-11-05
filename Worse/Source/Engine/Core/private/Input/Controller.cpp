#include "math/math_includes.hpp"
#include "Input/Controller.hpp"
#include "logger/logger.hpp"

#include "SDL3/SDL_gamepad.h"

namespace Worse
{

    Controller::Controller(ControllerDescriptor const& descriptor)
    {
        m_joystickID = descriptor.joystickID;
        m_guid       = descriptor.guid;
        m_name       = descriptor.name;
        m_type       = descriptor.type;

        m_handle = SDL_OpenGamepad(descriptor.joystickID);
        if (!m_handle)
        {
            WORSE_LOG_ERROR("Controller",
                            "Failed to open gamepad: {}",
                            SDL_GetError());
            m_handle = nullptr;
        }
        m_firmwareVersion =
            SDL_GetGamepadFirmwareVersion(static_cast<SDL_Gamepad*>(m_handle));

        WORSE_LOG_DEBUG("Controller",
                        "Connected: {} (JoystickID: {}, GUID: {}, Version: {})",
                        m_name,
                        m_joystickID,
                        m_guid,
                        m_firmwareVersion);
    }

    Controller::~Controller()
    {
        if (m_handle)
        {
            SDL_CloseGamepad(static_cast<SDL_Gamepad*>(m_handle));
            m_handle = nullptr;

            WORSE_LOG_DEBUG(
                "Controller",
                "Disconnected: {} (JoystickID: {}, GUID: {}, Version: {})",
                m_name,
                m_joystickID,
                m_guid,
                m_firmwareVersion);
        }
    }

    std::optional<UInt> Controller::getPowerPercentage() const
    {
        if (!isConnected())
        {
            return std::nullopt;
        }

        int precentage;
        SDL_PowerState powerState =
            SDL_GetGamepadPowerInfo(static_cast<SDL_Gamepad*>(m_handle),
                                    &precentage);
        switch (powerState)
        {
        case SDL_POWERSTATE_ERROR:
        case SDL_POWERSTATE_UNKNOWN:
        case SDL_POWERSTATE_NO_BATTERY:
            return std::nullopt;
        case SDL_POWERSTATE_ON_BATTERY:
        case SDL_POWERSTATE_CHARGING:
        case SDL_POWERSTATE_CHARGED:
            return precentage;
        }
    }

    void Controller::setLEDColor(UByte const red, UByte const green,
                                 UByte const blue) const
    {
        if (!isConnected())
        {
            return;
        }

        if (!SDL_SetGamepadLED(static_cast<SDL_Gamepad*>(m_handle),
                               red,
                               green,
                               blue))
        {
            WORSE_LOG_ERROR("Controller",
                            "Failed to set LED color: {}",
                            SDL_GetError());
        }
    }

    void Controller::vibrate(Float const lowFrequency, Float const highFrequency, UInt const durationMs) const
    {
        if (!isConnected())
        {
            return;
        }

        Float low  = Math::Saturate(lowFrequency);
        Float high = Math::Saturate(highFrequency);

        if (!SDL_RumbleGamepad(static_cast<SDL_Gamepad*>(m_handle),
                               static_cast<UShort>(low * 65535.0f),
                               static_cast<UShort>(high * 65535.0f),
                               durationMs))
        {
            WORSE_LOG_ERROR("Controller",
                            "Failed to vibrate controller: {}",
                            SDL_GetError());
        }
    }

    Bool Controller::isConnected() const
    {
        if (!m_handle)
        {
            return false;
        }

        return SDL_GamepadConnected(static_cast<SDL_Gamepad*>(m_handle));
    }

    void* Controller::getHandleSDL() const
    {
        return m_handle;
    }

    UInt Controller::getJoystickID() const
    {
        return m_joystickID;
    }

    std::string const& Controller::getGUID() const
    {
        return m_guid;
    }

    std::string const& Controller::getName() const
    {
        return m_name;
    }

    ControllerType Controller::getType() const
    {
        return m_type;
    }

    Bool Controller::operator==(Controller const& other) const
    {
        return m_guid == other.m_guid;
    }

    Bool Controller::operator!=(Controller const& other) const
    {
        return !(*this == other);
    }

    Bool Controller::operator==(ControllerDescriptor const& other) const
    {
        return m_guid == other.guid;
    }

    Bool Controller::operator!=(ControllerDescriptor const& other) const
    {
        return m_guid != other.guid;
    }

} // namespace Worse