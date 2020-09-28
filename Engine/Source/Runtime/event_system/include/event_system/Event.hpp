#pragma once
// STD Headers

// Library headers

// Screwjank Headers

namespace sj {

    struct Event
    {
        /** Flag that allows receivers to consume the event */
        bool EventConsumed;

        /**
         * Constructor
         */
        Event();
    };

} // namespace sj
