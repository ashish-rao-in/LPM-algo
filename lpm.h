/******************************************************************************
Longest Prefix Match header file

Author: Ashish Rao 
Date: 08/Jan/2022

*******************************************************************************/

//DELETE FROM HERE
/*
 * GLOBAL DATA
 */
 
 #define EINVAL     -1
 #define EOK         0
 #define IPv4_SIZE  16
 #define MAX_DEPTH  32
 #define TRUE        1
 #define FALSE       0

/* 
 * LPM tree nodes store route prefix
 */
typedef struct lpm_node {
    int val;                    //either 0 or 1
    int nexthop;                //return value, valid only for leaf nodes else 0
    int src_port;               //return value, valid only for leaf nodes else 0
    struct lpm_node *left;      //left node pointer
    struct lpm_node *right;     //right node pointer
} lpm_node;

/*
 * Structure that stores the registered ip
 */

typedef struct lkp_result {
    int last_ip;                //last registered ip
    int nh;                     //nexthop from the lpm result
    int sp;                     //source port from the lpm result
} lkp_result;


/*
 * API DECLARATIONS
 */

/*
 * This function inserts the prefix into the LPM tree.
 * If the prefix value is '0' walk or create (if not present) a node to the left.
 * If the prefix value is '1' walk or create (if not present) a node to the right.
 * When the leaf node is reached enter the nexthop information.
 *
 * Input: prefix    (pointer to an array, that will be updated into the tree)
 * Input: size      (size of the prefix array)
 * Input: nexthop   (nexthop data to be updated in the leaf node)
 * Input: port      (source port to be updated in the leaf node)
 *
 * Output: 0 - Success
 *        -1 - Error
 */
extern int 
insert_prefix(int *prefix, int size, int nexthop, int port);

/*
 * This function deletes the prefix from the LPM tree.
 *
 * It recurssively does a Depth first walk to the leaf node 
 * and deletes the leaf nodes that are without any child.
 *
 * If the leaf node has a child, then do not delete the node, 
 * only remove the nexthop info.
 *
 * Input: prefix    (pointer to an array, that will be updated into the tree)
 * Input: size      (size of the prefix array)
 *                  if the size == 0, it signifies leaf node
 * Input: root      (Pointer to the root node)
 *
 * Output: Pointer to the current node or NULL
 */
extern lpm_node* 
delete_prefix(lpm_node *root, int *prefix, int size);


/*
 * This function searches for a given prefix in LPM and 
 * returns the nexthop info of the longest matched prefix.
 * If there is no match it returns the default nexthop i.e. 127.0.0.1
 *
 * Walk the LPM and keep updating the nexthop whenever a leaf node is traversed.
 *
 * Input: prefix    (pointer to an array, that will be searched in the tree)
 * Input: result    (pointer to the buffer that holds the last reregistered IP and nexthop info)
 * 
 * Output: 0 - Success
 *        -1 - Error
 */
extern int 
find_route(int *prefix, lkp_result* result);


/*
 * This function is called whenever add/delete operation is performed in the LPM tree.
 * It updates the cache (result) with the latest nexthop for a particular registered IP
 */
extern void
update_reg_ip(lkp_result *result);


/*
 * This function takes user input and calls:
 * insert_prefix() - to add prefix to LPM tree.
 * delete_prefix() - to delete prefix from LPM tree.
 * update_reg_ip() - to update the registered ip cache.
 */
extern void
add_delete_route(int add, lkp_result *result);


/*
 * This functions takes input from the user to register a new IP
 * it updates the cache (result) with the latest nexthop for the respective registered IP
 *
 * If the cache is up to date, return the nexthop from cache.
 * If the cache is outdated, find and return the latest matched nexthop and update the cache.
 * 
 * This function automatically deregisters the previously registered IP, when the new IP is registered.
 */
extern lkp_result*
register_ip(lkp_result *result); 


extern void
init_globals(void);

extern void 
fill_data(lpm_node **node, int data, int nh, int port);

extern void
dec2bin(unsigned int nw_ip, int *prefix, int size);
