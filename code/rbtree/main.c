#include "rbtree.h"
#include "other.h"
#include <stdio.h>
struct node {
    int val;
    struct rb_node rb_node;
    int cc;
};

static inline struct node *node_of(struct rb_node *rb_node)
{
	return container_of(rb_node, struct node, rb_node);
}

int main (){
    struct node node1,node2,node3;
    node1.val = 1;
    node2.val = 2;
    node3.val = 3;
    node1.rb_node.rb_left = &node2.rb_node;
    node1.rb_node.rb_right = &node3.rb_node;
    node2.rb_node.rb_left = NULL;
    node2.rb_node.rb_right = NULL;
    node3.rb_node.rb_left = NULL;
    node3.rb_node.rb_right = NULL;
    struct node * prev_node = node_of(rb_prev(&node1.rb_node));
    printf("%d\n" ,prev_node->val);
    struct node * next_node = node_of(rb_next(&node1.rb_node));
    printf("%d\n" ,next_node->val);
    return 0;
}
