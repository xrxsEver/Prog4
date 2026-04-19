#ifndef MINIGIN_EVENT_H
#define MINIGIN_EVENT_H

#pragma once

namespace dae {
    using EventId = unsigned int;

    template <size_t N>
    consteval EventId MakeEventId(const char(&text)[N]) {
        unsigned int hash = 0;
        for (size_t i = 0; i < N - 1; ++i) {
            hash = text[i] + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    struct Event {
        EventId id;
    };
}

#endif //MINIGIN_EVENT_H