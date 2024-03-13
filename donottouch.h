#include<stdalign.h>
#define BUF_SIZE 128

typedef struct {
    unsigned long counter[BUF_SIZE];
} __attribute__((aligned(BUF_SIZE*8))) buf;


void update_buffer(buf *buffer, unsigned thread_id);

int check_buffer_alignment(buf *buffer); 
