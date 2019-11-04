#pragma once
#include <ControlTask.hpp>
#include "QuakeControlTask.h"
#include "radio_mode_t.enum"

// // Print the error code
// const char* TRANSLATE_ERR(int errCode){ 
//    switch(errCode){
//         case Devices::OK:
//             return "OK";
//         case Devices::TIMEOUT:
//             return "TIMEOUT";
//         case Devices::BAD_CHECKSUM:
//             return "BAD_CHECKSUM";
//         case Devices::WRONG_LENGTH:
//             return "WRONG_LENGTH";
//         case Devices::UNEXPECTED_RESPONSE:
//             return "UNEXPECTED_RESPONSE";
//         case Devices::WRITE_FAIL:
//             return "WRITE_FAIL";
//         case Devices::PORT_UNAVAILABLE:
//             return "PORT_UNAVAILABLE";
//         case Devices::UNKNOWN:
//             return "UNKNOWN";
//         case Devices::WRONG_FN_ORDER:
//             return "WRONG_FN_ORDER";
//         default:
//             return "Err code not found";
//     }
// }
class QuakeManager : public ControlTask<void> {
   public:
    QuakeManager(StateFieldRegistry& registry);
    void execute() override;

   protected:
   void dispatch_startup();
   void dispatch_waiting();
   void dispatch_transceiving();
   void dispatch_manual();

   /**
     * @brief Control cycle count, provided by ClockManager.
     */
   std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;
    
    /**
     * @brief Current radio mode (see radio_mode_t.enum)
     **/
   Serializer<unsigned int> radio_mode_sr;
   WritableStateField<unsigned int> radio_mode_f;

   /**
    * @brief Pointer to the downlink message stack
    **/ 
   InternalStateField<char [70]> radio_msg_queue_fp;

   QuakeControlTask qct;
   // The last cycle for which we had comms
   unsigned int last_checkin_cycle;

};