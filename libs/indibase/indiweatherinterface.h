/*
    Weather Interface
    Copyright (C) 2018 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#pragma once

#include "indibase.h"
#include "indipropertynumber.h"
#include "indipropertyswitch.h"
#include "indipropertylight.h"
#include "inditimer.h"

#include <stdint.h>
#include <string>
#include <vector>

// Alias
using WI = INDI::WeatherInterface;

namespace INDI
{

/**
 * \class WeatherInterface
   \brief Provides interface to implement weather reporting functionality.

   The weather functionality can be an independent device (e.g. weather station), or weather-related reports within another device.

   When developing a driver for a fully independent weather device, use INDI::Weather directly. To add focus functionality to
   an existing driver, subclass INDI::WeatherInterface. In your driver, then call the necessary weather interface functions.

   <table>
   <tr><th>Function</th><th>Where to call it from your driver</th></tr>
   <tr><td>WI::initProperties</td><td>initProperties()</td></tr>
   <tr><td>WI::updateProperties</td><td>updateProperties()</td></tr>
   <tr><td>WI::processNumber</td><td>ISNewNumber(...) Check if the property name contains WEATHER_* and then call WI::processNumber(..) for such properties</td></tr>
   </table>

   Implement and overwrite the rest of the virtual functions as needed. INDI Pegasus Ultimate Power Box driver is a good example to check for an actual implementation
   of a weather interface within an auxiliary driver.
\author Jasem Mutlaq
*/
class WeatherInterface
{
    public:

    protected:
        explicit WeatherInterface(DefaultDevice *defaultDevice);
        virtual ~WeatherInterface();

        /**
         * \brief Initialize focuser properties. It is recommended to call this function within
         * initProperties() of your primary device
         * \param statusGroup group for status properties
         * \param paramsGroup group for parameter properties
         */
        void initProperties(const char *statusGroup, const char *paramsGroup);

        /**
         * @brief updateProperties Define or Delete Rotator properties based on the connection status of the base device
         * @return True if successful, false otherwise.
         */
        bool updateProperties();

        /** \brief Process weather number properties */
        bool processNumber(const char *dev, const char *name, double values[], char *names[], int n);

        /** \brief Process weather switch properties */
        bool processSwitch(const char *dev, const char *name, ISState *states, char *names[], int n);

        /**
         * @brief checkWeatherUpdate Calls updateWeather and update critical parameters accordingly.
         */
        void checkWeatherUpdate();

        /**
         * @brief updateWeather Update weather conditions from device or service. The function should
         * not change the state of any property in the device as this is handled by Weather. It
         * should only update the raw values.
         * @return Return overall state. The state should be IPS_OK if data is valid. IPS_BUSY if
         * weather update is in progress. IPS_ALERT is there is an error. The clients will only accept
         * values with IPS_OK state.
         */
        virtual IPState updateWeather();

        /**
         * @brief saveConfigItems Save parameters ranges in the config file.
         * @param fp pointer to open config file
         * @return true of success, false otherwise.
         */
        virtual bool saveConfigItems(FILE *fp);

        /**
         * @brief addParameter Add a physical weather measurable parameter to the weather driver.
         * The weather value has three zones:
         * <ol>
         * <li>OK: Set minimum and maximum values for acceptable values.</li>
         * <li>Warning: Set minimum and maximum values for values outside of Ok range and in the
         * dangerous warning zone.</li>
         * <li>Alert: Any value outsize of Ok and Warning zone is marked as Alert.</li>
         * </ol>
         * @param name Name of parameter
         * @param label Label of parameter (in GUI)
         * @param numMinOk minimum Ok range value.
         * @param numMaxOk maximum Ok range value.
         * @param percWarning percentage for Warning.
         * @param flipWarning boolean indicating if range warning should be flipped to in-bounds, rather than out-of-bounds
         */
        void addParameter(std::string name, std::string label, double numMinOk, double numMaxOk, double percWarning, bool flipWarning = false);

        /**
         * @brief setCriticalParameter Set parameter that is considered critical to the operation of the
         * observatory. The parameter state can affect the overall weather driver state which signals
         * the client to take appropriate action depending on the severity of the state.
         * @param name Name of critical parameter.
         * @return True if critical parameter was set, false if parameter is not found.
         */
        bool setCriticalParameter(std::string name);

        /**
         * @brief setParameterValue Update weather parameter value
         * @param name name of weather parameter
         * @param value new value of weather parameter;
         */
        void setParameterValue(std::string name, double value);

        /**
         * @brief checkParameterState Checks the given parameter against the defined bounds
         * @param param Name of parameter to check.
         * @returns IPS_IDLE:  The given parameter name is not valid.
         * @returns IPS_OK:    The given parameter is within the safe zone.
         * @returns IPS_BUSY:  The given parameter is in the warning zone.
         * @returns IPS_ALERT: The given parameter is in the danger zone.
         */
        IPState checkParameterState(const std::string &param) const;

        /**
         * @brief updateWeatherState Send update weather state to client
         * @returns true if any parameters changed from last update. False if no states changed.
         */
        bool syncCriticalParameters();

        // Parameters
        INDI::PropertyNumber ParametersNP {0};

        // Parameter Ranges
        std::vector<INDI::PropertyNumber> ParametersRangeNP;
        enum
        {
            MIN_OK,
            MAX_OK,
            PERCENT_WARNING,
	    FLIP_RANGE_TEST,
        };

        // Weather status
        INDI::PropertyLight critialParametersLP {0};

        // Update Period
        INDI::PropertyNumber UpdatePeriodNP {1};
        // Refresh data
        INDI::PropertySwitch RefreshSP {1};

        // Override
        INDI::PropertySwitch OverrideSP {1};


    private:
        void createParameterRange(std::string name, std::string label, double numMinOk, double numMaxOk, double percWarning, bool flipWarning);
        DefaultDevice *m_defaultDevice { nullptr };
        std::string m_ParametersGroup;
        INDI::Timer m_UpdateTimer;
};
}
