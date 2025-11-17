#ifndef EVENT_H
#define EVENT_H

#include <stdbool.h>

#define EVENT_QUEUE_SIZE    15

typedef enum
{
    EVENT_NONE = 0,
    EVENT_ACCEL_ID_READY,
    EVENT_ACCEL_CONFIG_READY,
    EVENT_ACCEL_VECTOR_READY,
    EVENT_BUTTON_PRESSED,
    EVENT_TIMER_TICK,
    EVENT_ESP_NEXT,
    EVENT_ESP_LED_ON,
    EVENT_ESP_LED_OFF,
} EVENT_TYPE;

typedef struct
{
    EVENT_TYPE type;
    void* context;
} Event_t;

/*Brief: Event queue initialization
 * [in] - none
 * [out] - none
 * */
void EventQueueInit(void);

/*Brief: Put event into the queue
 * [in] - pointer to event
 * [out] - true - event successfully sent to queue; false - otherwise (queue is full)
 * */
bool EventQueue_Enqueue(const Event_t* const event);

/*Brief: Retrieve event from the queue
 * [in] - pointer to event
 * [out] - true - event successfully retrieved from queue; false - otherwise (queue is empty)
 * */
bool EventQueue_Dequeue(Event_t* const event);

#endif /* EVENT_H */
