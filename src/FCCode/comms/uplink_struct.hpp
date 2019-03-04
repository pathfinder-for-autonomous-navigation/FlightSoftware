/** @file uplink_struct.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definition for how the uplink struct is structured.
 */

#ifndef UPLINK_STRUCT_HPP_
#define UPLINK_STRUCT_HPP_

#include <array>
#include "../state/state_definitions.hpp"

namespace Comms {
    /** @brief Container for data and commands supplied by a ground uplink. */
    struct Uplink {
        /** Master State Control */
        //! Sets master state of satellite.
        State::Master::MasterState master_state;        
        //! Sets PAN-specific master state of satellite.
        State::Master::PANState pan_state;    
        //! Whether or not this satellite should be leader or follower.
        bool follower_set;

        /** hat Control */
        //! Set whether or not to ignore a device error in the main hat.
        std::array<bool, 7> hat_error_ignoreds;
        //! Set whether or not to ignore a device error in the ADCS hat.
        std::array<bool, 17> hat_error_ignoreds_adcs;
        
        /** ADCS control */
        //! Should the ADCS be manually controlled by this uplink packet?
        bool control_adcs;
        //! If there are upcoming propulsion-based ADCS commands, should this packet override them?
        bool override_propulsion_adcs_commands;
        //! Set the state of the ADCS system (required to be POINTING if we are manually commanding attitude).
        State::ADCS::ADCSState adcs_state;
        //! Reference frame in which the commanded attitude should be considered.
        unsigned int cmd_attitude_frame;
        //! If we are manually controlling the ADCS, this is where we specify the attitude.
        std::array<float, 4> cmd_attitude;

        /** Gomspace control */
        //! If Gomspace is behaving improperly, setting this to true will ensure the software rewrites the default
        // settings back on to the device in order to attempt to restore proper functionality.
        bool rewrite_gomspace_settings;
        //! If Gomspace is behaving improperly, setting this to true will reboot the Gomspace.
        bool power_cycle_gomspace;

        /** Propulsion control **/
        //! If set to true, the flight controller software will replace any existing scheduled firings with the one specified
        // in this packet.
        bool override_prop_firing;
        //! Firing data for a manually-specified propulsion manuever.
        State::Propulsion::Firing firing_data;

        // Piksi and Quake control
        //! If Piksi is behaving improperly, setting this to true will ensure the software rewrites the default
        // settings back on to the device in order to attempt to restore proper functionality.
        bool rewrite_piksi_settings;
        //! If Piksi is behaving improperly, setting this to true will reboot the Piksi.
        bool power_cycle_piksi;
        //! If Quake is behaving improperly, setting this to true will ensure the software rewrites the default
        // settings back on to the device in order to attempt to restore proper functionality.
        bool rewrite_quake_settings;
        //! If Quake is behaving improperly, setting this to true will reboot the Quake.
        bool power_cycle_quake;

        //! True if checksum value of packet matches data of packet.
        bool is_crc32_valid;
        
        //! Set to true by Master Controller if it has processed an uplink
        bool is_uplink_processed = false;
        //! GPS time at which the uplink packet was received
        gps_time_t time_received;
    };
}

#endif