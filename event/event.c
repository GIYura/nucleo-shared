#include <stddef.h>

#include "custom-assert.h"
#include "event.h"
#include "buffer.h"

static Buffer_t m_eventQueue;
static Event_t m_events[EVENT_QUEUE_SIZE + 1];

void EventQueueInit(void)
{
    BufferCreate(&m_eventQueue, m_events, sizeof(m_events), sizeof(Event_t), true);
}

bool EventQueue_Enqueue(const Event_t* const event)
{
    ASSERT(event != NULL);

    return BufferPut(&m_eventQueue, event, sizeof(Event_t));
}

bool EventQueue_Dequeue(Event_t* const event)
{
    ASSERT(event != NULL);

    return BufferGet(&m_eventQueue, event, sizeof(Event_t));
}
