#include <unistd.h>
#include "events_stats.h"

int main()
{
    int span_length = 3;
    EventsStats history;
    history.StartRecord();
    for (int i = 0; i < 5; ++i)
    {
        history.AddEvent("BW", "A", EventType::RPC, 1);
        history.AddEvent("BW", "A", EventType::RPC, 1);
        history.AddEvent("BW", "A", EventType::RPC, 1);
        history.AddEvent("Avatar", "B", EventType::PROPERTY, 10);
        history.AddEvent("Avatar", "B", EventType::PROPERTY, 10);
        history.AddEvent("Avatar", "C", EventType::RPC, 100);
        sleep(3);
    }
    history.PrintHistory();
    return 0;
}
