#pragma once
#include <string>
#include <vector>
#include <map>
#include <sys/time.h>
#include <stdio.h>

typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef uint32 EventCount;
typedef uint64 EventSize;

class SpanStats
{
public:
    static int SPAN_LENGTH;

public:
    int timestamp;
    EventCount count;
    EventSize size;

public:
    SpanStats(int timestamp) : timestamp(timestamp), count(0), size(0) {}

public:
    void AddEvent(EventSize sz)
    {
        count += 1;
        size += sz;
    }

    bool IsSameSpan(int ts)
    {
        return ts - timestamp < SPAN_LENGTH;
    }
};

int SpanStats::SPAN_LENGTH = 10;

enum EventType
{
    RPC = 1,
    PROPERTY = 2
};

class Event
{
public:
    std::string module;
    std::string event;
    enum EventType event_type;

public:
    Event(std::string mod, std::string name, enum EventType event_tp) :
        module(mod), event(name), event_type(event_tp) {}

public:
    bool operator<(const Event& other) const 
    {
        if (module == other.module)
        {
            if (event == other.event)
                return event_type < other.event_type;
            else
                return event < other.event;
        }
        else
            return module < other.module;
    }
};

class EventStats : public Event
{
private:
    EventCount count_;
    EventSize size_;
    std::vector<SpanStats> spans_;

public:
    EventCount count() const { return count_; }
    EventSize size() const { return size_; }

public:
    EventStats(std::string mod, std::string name, enum EventType event_tp) : 
        Event(mod, name, event_tp), count_(0), size_(0), spans_() {}

public:
    void AddEvent(int timestamp, EventSize size)
    {
        if (!spans_.size() || !spans_.back().IsSameSpan(timestamp))
            spans_.emplace_back(timestamp);

        spans_.back().AddEvent(size);
        count_ += 1;
        size_ += size;
    }
    
    const std::vector<SpanStats>& GetSpans() const
    {
        return spans_;
    }
};

class EventsStats
{
private:
    std::map<Event, EventStats> events_stats_;
    bool is_recording_;

public:
    EventsStats() : events_stats_(), is_recording_(false) {}

public:
    void AddEvent(std::string mod, std::string event, enum EventType event_tp, EventSize size)
    {
        if (!is_recording_)
            return;

        Event event_key(mod, event, event_tp);
        auto it = events_stats_.find(event_key);
        if (it == events_stats_.end())
        {
            events_stats_.emplace(event_key, EventStats{mod, event, event_tp});
            it = events_stats_.find(event_key);
        }

        it->second.AddEvent(GetTimeStamp(), size);
    }

    void StartRecord()
    {
        events_stats_.clear();
        is_recording_ = true;
    }

    void StopRecord()
    {
        is_recording_ = false;
    }

    const std::map<Event, EventStats>& GetEventsStats()
    {
        return events_stats_;
    }

    void PrintHistory()
    {
        auto& stats = GetEventsStats();
        for (auto it = stats.begin(); it != stats.end(); ++it)
        {
            auto& event = it->second;
            printf("module: %s event: %s type: %s count: %d size: %llu\n", 
                event.module.c_str(), event.event.c_str(), event.event_type == EventType::RPC ? "rpc" : "property", 
                event.count(), event.size());
            auto& spans = event.GetSpans();
            for (auto span_it = spans.begin(); span_it != spans.end(); ++span_it)
            {
                printf("\t%d: count(%d) size(%llu)\n", span_it->timestamp, span_it->count, span_it->size);
            }
        }
    }

private:
    int GetTimeStamp()
    {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        return tp.tv_sec;
    }
};
