// license:GPLv3+

#pragma once

#define PUPPI_NAMESPACE "PUP"
#define PUPPI_MSG_QUEUE_EVENT "QueueEvent"

typedef struct PUPQueueEventMsg
{
   char source;
   int id;
   int value;
} PUPQueueEventMsg;
