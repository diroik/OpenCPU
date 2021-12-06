#ifndef OS_CALLBACK_HOOK__H
#define OS_CALLBACK_HOOK__H


#define TASK_LIST_LEN   8

typedef struct _task_node_list
{
    //short ec_task_index;
    //short ec_task_tcb_numb;
    char ec_task_name[8];
    unsigned int ec_task_in_time;
    //unsigned int ec_task_out_time;
}task_node_list;

#endif


