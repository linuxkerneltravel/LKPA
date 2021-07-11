#include "rbtree.h"
#include "other.h"
#include <stdio.h>
#include <stdlib.h>
struct sched_entity {
    int fair_key;
    struct rb_node run_node;
};

struct cfs_rq {
    struct rb_root tasks_timeline;
    struct rb_node* rb_leftmost;
    struct rb_node* rb_load_balance_curr;
};

static inline struct rb_node *first_fair(struct cfs_rq *cfs_rq){
	return cfs_rq->rb_leftmost;
}

static inline struct sched_entity *node_of(struct rb_node *run_node){
	return container_of(run_node, struct sched_entity, run_node);
}

static inline void 
show_tree(struct cfs_rq *cfs_rq){
    struct rb_node* pos;
    for (pos = first_fair(cfs_rq); pos; pos = rb_next(pos)){
        struct sched_entity * next_node = node_of(pos);
        printf("%d " ,next_node->fair_key);
    }
    printf("\n");
}

static inline void
__dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se){
	if (cfs_rq->rb_leftmost == &se->run_node)
		cfs_rq->rb_leftmost = rb_next(&se->run_node);
	rb_erase(&se->run_node, &cfs_rq->tasks_timeline);
	// update_load_sub(&cfs_rq->load, se->load.weight);
	// cfs_rq->nr_running--;
	// se->on_rq = 0;

	// schedstat_add(cfs_rq, wait_runtime, -se->wait_runtime);
}

static inline void 
__enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se){
    struct rb_node **link = &cfs_rq->tasks_timeline.rb_node;
	struct rb_node *parent = NULL;
	struct sched_entity *entry;
	int key = se->fair_key;
	int leftmost = 1;
    /*
	 * Find the right place in the rbtree:
	 */
	while (*link) {
		parent = *link;
		entry = rb_entry(parent, struct sched_entity, run_node);
		/*
		 * We dont care about collisions. Nodes with
		 * the same key stay together.
		 */
		if (key - entry->fair_key < 0) {
			link = &parent->rb_left;
		} else {
			link = &parent->rb_right;
			leftmost = 0;
		}
	}

	/*
	 * Maintain a cache of leftmost tree entries (it is frequently
	 * used):
	 */
	if (leftmost)
		cfs_rq->rb_leftmost = &se->run_node;

	rb_link_node(&se->run_node, parent, link);
	rb_insert_color(&se->run_node, &cfs_rq->tasks_timeline);
	// update_load_add(&cfs_rq->load, se->load.weight);
	// cfs_rq->nr_running++;
	// se->on_rq = 1;

	// schedstat_add(cfs_rq, wait_runtime, se->wait_runtime);
}

static struct cfs_rq* init(){
    struct cfs_rq* cfs_rq = (struct cfs_rq*)malloc(sizeof(struct cfs_rq));
    cfs_rq->tasks_timeline = RB_ROOT;
    struct sched_entity* node1 = (struct sched_entity*)malloc(sizeof(struct sched_entity));
    struct sched_entity* node2 = (struct sched_entity*)malloc(sizeof(struct sched_entity));
    struct sched_entity* node3 = (struct sched_entity*)malloc(sizeof(struct sched_entity));
    node1->fair_key = 1;
    node2->fair_key = 2;
    node3->fair_key = 3;
    __enqueue_entity(cfs_rq, node3);
    __enqueue_entity(cfs_rq, node1);
    __enqueue_entity(cfs_rq, node2);
    return cfs_rq;
}

int main (){
    struct cfs_rq* cfs_rq = init();
    show_tree(cfs_rq);
    struct sched_entity node4; node4.fair_key = 4;
    __enqueue_entity(cfs_rq, &node4);
    show_tree(cfs_rq);
    struct rb_node* rb_node3 = rb_prev(&node4.run_node);
    struct sched_entity* node3 = node_of(rb_node3);
    __dequeue_entity(cfs_rq, node3);
    show_tree(cfs_rq);
    return 0;
}
