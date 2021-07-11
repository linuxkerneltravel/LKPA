#include "list.h"
#include <stdio.h>
struct node {
    int val;
    struct list_head run_list;
    int cc;
};

static inline struct node *node_of(struct list_head *rb_node)
{
	return container_of(rb_node, struct node, run_list);
}

int main (){
    struct node head;
    head.val = 0;
    INIT_LIST_HEAD(&head.run_list);

    struct node node1; node1.val = 1;
    list_add(&node1.run_list, &head.run_list);

    struct node node2; node2.val = 2;
    list_add(&node2.run_list, &node1.run_list);

    struct list_head* pos;
    list_for_each(pos, &head.run_list){
        struct node* p = node_of(pos);
        printf("%d ", p->val);
    }
    printf("\n");
    return 0;
}