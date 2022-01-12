/******************************************************************************
Longest Prefix Match tree

Author: Ashish Rao 
Date: 08/Jan/2022

Functions to :
add/delete route entry into a routing table that uses LPM Tree DS
search for a given prefix

*******************************************************************************/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lpm.h"


/* root node pointer */
lpm_node *root;

/* flag to signify that 
 * the Registered IP is valid.
 */
int cache;


/*
 * API to insert a prefix into the LPM tree.
 * refer lpm.h for details.
 */
int
insert_prefix(int *prefix, int size, int nh_ip, int port) 
{
    lpm_node *curr_node = NULL; //pointer to track the current node in the tree
    int is_leaf = 0; //leaf node will have valid nexthop and port info
    
    /*
     * VERIFY INPUT DATA
     */
     
    if (root == NULL || size < 1 || prefix == NULL ||
        nh_ip < 0    || port < 0) {
        /* invalid input parameters */
        printf("Invalid parameter root 0x%p size %d, nh_ip %d, port %d\n",
                                                 root, size, nh_ip, port);
        return EINVAL;
    }
    
    for (int i=0; i<size; i++) {
        if (prefix[i] < 0 || prefix[i] > 1) {
        /* only value 0 or 1 is valid */
            printf("Invalid prefix val. prefix[%d] = %d\n", i, prefix[i]);
            return EINVAL;
        }
    }
    
    /*
     * WALK LPM TREE AND KEEP INSERTING THE NODES
     */
  
    /* point the current node at the root */
    curr_node = root;
    
    for (int i=0; i<size; i++) {
    /* loop to walk and insert the prefix into the lpm tree */
    
        if (prefix[i] == 0) {
        /* goto the left */
            if (curr_node->left == NULL) {
            /* insert a new node */
                lpm_node *new_node = calloc(1,sizeof(lpm_node));
                if (new_node == NULL) {
                    printf("calloc failed.\n");
                    exit(0);
                }
                new_node->left = NULL;
                new_node->right = NULL;
                curr_node->left = new_node;
                curr_node = new_node;
                if (is_leaf) {
                    fill_data(&new_node, prefix[i], nh_ip, port);
                } else {
                    fill_data(&new_node, prefix[i], -1, -1);
                }
            } else {
            /* node exists, continue the walk */
                curr_node = curr_node->left;
                if (is_leaf) {
                    fill_data(&curr_node, prefix[i], nh_ip, port);
                }
            } 
        } else { 
        /* goto the right */
            if (curr_node->right == NULL) {
            /* insert a new node */
                lpm_node *new_node = calloc(1,sizeof(lpm_node));
                if (new_node == NULL) {
                    printf("calloc failed.\n");
                    exit(0);
                }
                new_node->left = NULL;
                new_node->right = NULL;
                curr_node->right = new_node;
                curr_node = new_node;
                if (is_leaf) {
                    fill_data(&new_node, prefix[i], nh_ip, port);
                } else {
                    fill_data(&new_node, prefix[i], -1, -1);
                }
            } else {
            /* node exists, continue the walk */
                curr_node = curr_node->right;
                if (is_leaf) {
                    fill_data(&curr_node, prefix[i], nh_ip, port);
                }
            } 
        }
        
        if (i == size-2) {
            is_leaf = 1;
        }
        
    } //end of for loop - walking the prefix
    
    return EOK;
}

/*
 * API to delete a prefix from the LPM tree.
 * refer lpm.h for details.
 */
lpm_node *
delete_prefix(lpm_node *root, int *prefix, int size) 
{
    if (size < 0 || root == NULL) {
        return NULL;
    }

    /* DFS to the leaf node */
    if (prefix[0] == 0) {
        root->left = delete_prefix(root->left, prefix+1, size-1);
        if(size == 0) {
        /* reset the leaf node with child */
            root->nexthop = -1;
            root->src_port = -1;
        }
    } else {
        root->right = delete_prefix(root->right, prefix+1, size-1);
        if(size == 0) {
        /* reset the leaf node with child */
            root->nexthop = -1;
            root->src_port = -1;
        }
    }
    
    /* Delete the leaf node with no chidren */
    if (root->left == NULL && root->right == NULL &&
       ((root->nexthop == -1 && root->src_port == -1) || size == 0)) {
        root->val = -1;
        root->nexthop = -1;
        root->src_port = -1;
        root->left = NULL;
        root->right = NULL;
        root = NULL;
        free (root);
        return NULL;
    }
    
    return root;
}

/* 
 * API to search and register a IP
 * refer lpm.h for details.
 */
int
find_route(int *prefix, lkp_result* result) 
{
    
    int default_nh = 2130706433;    //local host 127.0.0.1
    int default_port = 999;         //CPU port
    lpm_node *curr_node = NULL;
    
    /*
     * VERIFY INPUT DATA
     */
     
    if (root == NULL || prefix == NULL || result == NULL) {
    /* invalid input parameters */
        printf("Invalid parameter root 0x%p prefix %p, result %p.\n",
                                            root, prefix, result);
        return EINVAL;
    }
    
    for (int i=0; i<MAX_DEPTH; i++) {
        if (prefix[i] < 0 || prefix[i] > 1) {
        /* only value 0 or 1 is valid */
            printf("Invalid prefix val. prefix[%d] = %d\n", i, prefix[i]);
            return EINVAL;
        }
    }
    
    /* point the current node at the root */
    curr_node = root;
    
    /*
     * Walk the tree and keep updating the nh and port
     */
    for (int i=0; i<MAX_DEPTH; i++) {
        if (prefix[i] == 0) {
        /* goto the left */
            if (curr_node->left == NULL) {
                break;
            } else {
                curr_node = curr_node->left;
                if (curr_node->nexthop != -1) {
                /* leaf node - update nh and port */
                    default_nh = curr_node->nexthop;
                    default_port = curr_node->src_port;
                }
            }
        } else {
        /* goto the right */
            if (curr_node->right == NULL) {
                break;
            } else {
                curr_node = curr_node->right;
                if (curr_node->nexthop != -1) {
                /* leaf node - update nh and port */
                    default_nh = curr_node->nexthop;
                    default_port = curr_node->src_port;
                }      
            }
        }
    }
    
    /* 
     * update the ip registration
     */
    result->nh = default_nh;
    result->sp = default_port;
    cache = TRUE;
    
    return EOK;
}

/* 
 * Callback API to update the Registered IP in the cache.
 * refer lpm.h for details.
 */
void
update_reg_ip(lkp_result *result) {

    /*
     * VERIFY INPUT DATA
     */
    if(result == NULL) {
        printf("%s:%d Invalid Input Parameters\n", __FUNCTION__, __LINE__);
        return;
    }
    
    if (result->last_ip) {
        int nw_ip = result->last_ip;
        int mask = MAX_DEPTH;
        int prefix[MAX_DEPTH];
        int rc = 0;
    
        (void)memset(prefix, -1, MAX_DEPTH*sizeof(int));
    
        (void) dec2bin(nw_ip, prefix, mask);
    
        rc = find_route(prefix, result);
        if (rc != EOK) {
            printf("Route lookup failed rc %d.Registered ip cache not updated\n",rc);
        }
    }
}

/*
 * API to add/delete user input into a LPM tree.
 * refer lpm.h for details.
 */
void
add_delete_route(int add, lkp_result *result) {
    
    char ip[IPv4_SIZE];             //IPv4 route
    int nw_ip = 0;                  //decimal equivalent of IPv4 route
    int mask = 0;                   //network mask
    char nh[IPv4_SIZE];             //nexthop IPv4 address
    int nw_nh = 0;                  //decimal equivalent of nexthop address
    int port = 0;                   //egress source port
    int prefix[MAX_DEPTH];          //binary equivalent of route 'bitwise AND' mask
    int size = 0;                   //final size of prefix array after bitwise AND
    int rc = 0;
    
    /*
     * Change in Routing Table.
     * Invalidate the register ip cache.
     */
    cache = FALSE;

    /*
     * initialise
     */
    (void)memset(prefix, -1, MAX_DEPTH*sizeof(int));
    (void)memset(ip, '\0',  IPv4_SIZE*sizeof(char));
    (void)memset(nh, '\0',  IPv4_SIZE*sizeof(char));
    
    /*
     * Get the static routing entry from the user.
     */
    printf("Enter the IPv4 route ip: ");
    scanf("%s",ip);
    printf("Enter the network mask (1 to 32): ");
    scanf("%d",&mask);
    if (add) {
        printf("Enter the IPv4 nextop ip: ");
        scanf("%s",nh);
        printf("Enter the o/p port: ");
        scanf("%d",&port);
    }
    
    /*
     * Validate the user Input
     */
    if ( (rc = inet_pton(AF_INET, ip, &nw_ip)) == 0) {
      printf("Invalid IPv4 address: %s\n", ip);
      return;
    } else if (rc == -1) {
      perror("inet_pton");
      return;
    }
    if(mask < 1 || mask > 32) {
        printf("Invalid mask: %d\n",mask);
        return;
    }
    if (add) {
        if ( (rc = inet_pton(AF_INET, nh, &nw_nh)) == 0) {
            printf("Invalid IPv4 nexthop address: %s\n", nh);
            return;
        } else if (rc == -1) {
            printf("inet_pton internal error");
            return;
        }
        if (port < 0) {
            printf("Invalid port: %d (negative value)\n",port);
            return;
        }
    }
    
    /* 
     * Calculate the prefix from the IP and mask, and convert it to binary
     */
    nw_ip = ntohl(nw_ip);
    nw_nh = ntohl(nw_nh);
    
    nw_ip = nw_ip >> (32-mask);
    (void) dec2bin(nw_ip, prefix, mask);
    
    if (add) {
    /* 
     * Insert the prefix into the LPM tree 
     */
        rc = insert_prefix(prefix, mask, nw_nh, port);
        if(rc == EOK) {
            printf("IPv4 Route %s/%d added successfully.\n", ip, mask);
        } else {
            printf("IPv4 Route add failed. rc = %d\n",rc);
        }
    } else {
    /*
     * DELETE PREFIX
     */
        lpm_node *curr;
        curr = delete_prefix(root, prefix, mask);
        printf("IPv4 Route %s/%d deleted.\n", ip, mask);
    }
    
    (void)update_reg_ip(result);
}


/*
 * API to get user input and update the register ip cache with the latest lookup request
 * refer lpm.h for details.
 */
lkp_result*
register_ip(lkp_result *result) {
    
    char ip[IPv4_SIZE]; 
    int prefix[MAX_DEPTH];
    int mask = MAX_DEPTH;
    int nw_ip = 0;
    int rc = 0;
    
    /*
     * initialise
     */
    (void)memset(prefix, -1, MAX_DEPTH*sizeof(int));
    (void)memset(ip, '\0',  IPv4_SIZE*sizeof(char));
    
    
    /*
     * get user Input
     */
    
    printf("IPv4 address to search: ");
    scanf("%s",ip);
    
    
    /*
     * Validate the user Input
     */
    if ( (rc = inet_pton(AF_INET, ip, &nw_ip)) == 0) {
      printf("Invalid IPv4 address: %s\n", ip);
      return NULL;
    } else if (rc == -1) {
      perror("inet_pton");
      return NULL;
    }
    
    
    /* 
     * Calculate the prefix from the IP and mask, and convert it to binary
     */
    nw_ip = ntohl(nw_ip);
    (void) dec2bin(nw_ip, prefix, mask);
    
    /*
     * if there is no change in the registered ip,
     * return previous cached result.
     */
    if (cache && result->last_ip == nw_ip) {
        return result;
    }
    
    /*
     * user wants to register new IP
     */
    rc = find_route(prefix, result);
    if (rc == EOK) {
        /*
         * update last registed ip
         */
        result->last_ip = nw_ip;
    } else {
        printf("Route lookup failed rc %d.Registered ip not updated\n",rc);
    }
    
    return result;
}


/*
 * Walk API to check the LPM tree.
 * Only for internal debugging.
 */
void 
walk(lpm_node *root) 
{
    if (root == NULL) {
        return;
    }
    
    printf("val %d nh %d port %d \n",
            root->val, root->nexthop,
            root->src_port);
    walk(root->left);
    walk(root->right);
}

int main()
{
    
    lkp_result *result; //pointer to the cache data that has Registered IP info.
    char user_data;     //to check the action user wants to perform.
    
    /* initialize the LPM tree root */
    init_globals();
    
    /* allocate the cache memory */
    result = (lkp_result *) calloc( 1, sizeof(lkp_result));
    if (result == NULL) {
        printf("calloc failed.\n");
        exit(0);
    }
    
    /* 
     * wait for user input and perform requested operations.
     */
    while (1) {
        fflush(stdin);
        printf("\nEnter [a]dd route, [d]elete route or [r]egister: ");
        scanf("%c",&user_data);
        fflush(stdin);

        switch (user_data) {
            case 'a':
            case 'A':
                add_delete_route(TRUE, result);
                break;
            
            case 'd':
            case 'D':
                add_delete_route(FALSE, result);
                break;
            
            case 'r':
            case 'R':
                (void) register_ip(result);
                int nw_nh = htonl(result->nh);
                char nh[IPv4_SIZE];
                inet_ntop(AF_INET, &nw_nh, nh, INET_ADDRSTRLEN);
                printf("Registered Nexthop %s\n",nh);
                printf("Source Port %d\n",result->sp);
                break;
            
            case 'W':
            /* hidden from user, only for internal debugging */
                (void) walk(root);
                break;
            
            default: 
                printf("Invalid Input.\n");
                break;
        }
    }

    return 0;
}
